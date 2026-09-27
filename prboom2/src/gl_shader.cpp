// SPDX-License-Identifier: GPL-2.0-or-later

#ifdef HAVE_CONFIG_H
#include <utility>

#include "config.h"
#endif

#include <assert.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <math.h>
#include <stdarg.h>
#include "doomstat.hpp"
#include "v_video.hpp"
#include "gl_opengl.hpp"
#include "gl_intern.hpp"
#include "r_main.hpp"
#include "w_wad.hpp"
#include "i_system.hpp"
#include "r_bsp.hpp"
#include "lprintf.hpp"
#include "m_file.hpp"
#include "e6y.hpp"
#include "r_things.hpp"
#include "doomdef.hpp"
#include "dsda/configuration.hpp"

#include "dsda/utility/string_view.hpp"

#define MAX_TEXTURES 3
#define MAX_UNIFORMS 10
#define MAX_STACK 10

#define UNIF_VAL_END (-1)

#define UNIF(num, name, type) [num] = {(name), (type)}
#define UNIF_END {NULL, ShaderUniformType::Count}

enum struct ShaderUniformType : int32_t
{
	Float1,
	Float2,
	Int1,
	Tex0,
	Tex1,
	Tex2,
	Tex0D,
	Tex1D,
	Tex2D,
	Count
};

typedef struct
{
	const char* name;
	ShaderUniformType type;
} shader_uniform_t;

typedef struct
{
	const char* name;
	shader_uniform_t unifs[];
} shader_info_t;

typedef struct
{
	const shader_info_t* info;
	GLhandleARB hShader;
	GLhandleARB hVertProg;
	GLhandleARB hFragProg;
	int indices[MAX_UNIFORMS];
	int texds[MAX_TEXTURES];
} shader_t;

typedef struct
{
	const GLchar* name;
	const GLchar* value;
} shader_define_t;

typedef struct
{
	GLchar const** strs;
	GLint* lens;
	unsigned int size;
	unsigned int cap;
} shader_source_t;

typedef union
{
	int i[2];
	float f[2];
} shader_uniform_value_t;

typedef struct
{
	shader_t* shader;
	shader_uniform_value_t unifs[MAX_UNIFORMS];
} shader_frame_t;

static void glsl_ShaderSrcInit(shader_source_t* src)
{
	memset(src, 0, sizeof(*src));
}

static void glsl_ShaderSrcDestroy(shader_source_t* src)
{
	if(src->strs)
		Z_Free(src->strs);
	if(src->lens)
		Z_Free(src->lens);
	glsl_ShaderSrcInit(src);
}

static void glsl_ShaderSrcAppend(shader_source_t* src, const GLchar* str, GLint len)
{
	unsigned int size = src->size;

	if(size && src->lens[size - 1] >= 0 && src->strs[size - 1] + src->lens[size - 1] == str)
	{
		// Fast path -- expand last string to include more of source buffer
		src->lens[src->size - 1] += len;
		return;
	}

	if(src->size == src->cap)
	{
		if(src->cap == 0)
			src->cap = 8;
		else
			src->cap *= 2;

		src->strs = static_cast<decltype(src->strs)>(Z_Realloc(src->strs, src->cap * sizeof(*src->strs)));
		src->lens = static_cast<decltype(src->lens)>(Z_Realloc(src->lens, src->cap * sizeof(*src->lens)));
	}

	src->strs[src->size] = str;
	src->lens[src->size] = len;
	++src->size;
}

static void glsl_ShaderLookup(const char* name, GLchar const** text, GLint* len)
{
	int lump = W_CheckNumForName2(name, LumpNamespace::Prboom);

	if(lump == LUMP_NOT_FOUND)
		Log::Fatal("Could not find shader source: {}\n", name);

	*text = static_cast<GLchar const*>(W_LumpByNum(lump));
	*len = W_LumpLength(lump);
}

#define CSLEN(x) (sizeof((x)) - 1)

static void glsl_ShaderSrcAppendDefine(shader_source_t* src,
	const shader_define_t* def)
{
	static const char cdefine[] = "#define ";
	static const char cspace[] = " ";
	static const char cnl[] = "\n";

	glsl_ShaderSrcAppend(src, cdefine, CSLEN(cdefine));
	glsl_ShaderSrcAppend(src, def->name, strlen(def->name));
	glsl_ShaderSrcAppend(src, cspace, CSLEN(cspace));
	glsl_ShaderSrcAppend(src, def->value, strlen(def->value));
	glsl_ShaderSrcAppend(src, cnl, CSLEN(cnl));
}

static void glsl_ShaderSrcProcess(shader_source_t* src, const GLchar* text,
	GLint len, const shader_define_t* defs,
	const shader_define_t* userdefs)
{
	static const char vdir[] = "#version";
	static const char edir[] = "#extension";
	static const char idir[] = "#include";
	static const char iext[] = "GL_GOOGLE_include_directive";
	dsda_string_view_t v;
	dsda_string_view_t line;

	dsda_InitStringView(&v, text, len);

	while(dsda_GetStringViewLine(&v, &line))
	{
		// Output any version and extension directives before defines
		if(dsda_StringViewStartsWith(&line, vdir))
		{
			glsl_ShaderSrcAppend(src, line.string, line.size);
			continue;
		}

		if(dsda_StringViewStartsWith(&line, edir))
		{
			dsda_string_view_t cur;

			dsda_StringViewAtOffset(&line, CSLEN(edir), &cur);
			dsda_StringViewAfterChars(&cur, " \t", &cur);

			if(dsda_StringViewStartsWith(&cur, iext))
				// Omit include extension from output since we're handling it
				continue;
			glsl_ShaderSrcAppend(src, line.string, line.size);
			continue;
		}

		// Output any outstanding defines
		for(; defs && defs->name; ++defs)
			glsl_ShaderSrcAppendDefine(src, defs);

		for(; userdefs && userdefs->name; ++userdefs)
			glsl_ShaderSrcAppendDefine(src, userdefs);

		// Handle include directives
		if(dsda_StringViewStartsWith(&line, idir))
		{
			dsda_string_view_t cur = line;
			char lumpname[9] = {0};
			const GLchar* itext;
			GLint ilen;

			// Parse include name
			if(!dsda_SplitStringViewAfterChar(&line, '"', nullptr, &cur) ||
				!dsda_SplitStringViewBeforeChar(&cur, '"', &cur, nullptr))
				Log::Fatal("Invalid include syntax: {}\n", std::string_view(line.string, line.size));

			// Trim off extension if present
			dsda_SplitStringViewBeforeChar(&cur, '.', &cur, nullptr);

			// Truncate and NUL-terminate lump name
			memcpy(lumpname, cur.string, MIN(8, cur.size));

			// Recursively process include source text
			glsl_ShaderLookup(lumpname, &itext, &ilen);
			glsl_ShaderSrcProcess(src, itext, ilen, nullptr, nullptr);
			continue;
		}

		// Pass line through verbatim
		glsl_ShaderSrcAppend(src, line.string, line.size);
	}
}

static void glsl_ShaderSrcLoad(shader_source_t* src, const char* name,
	const shader_define_t* defs,
	const shader_define_t* userdefs)
{
	const GLchar* text;
	int len;

	glsl_ShaderLookup(name, &text, &len);
	glsl_ShaderSrcInit(src);
	glsl_ShaderSrcProcess(src, text, len, defs, userdefs);
}

static shader_t* glsl_ShaderLoad(const shader_info_t* info,
	const shader_define_t* userdefs)
{
	shader_source_t src;
	int status;
	char buffer[2048];
	shader_t* shader = nullptr;
	const shader_uniform_t* unif;
	unsigned int i;
	int t;

	shader = static_cast<shader_t*>(Z_Malloc(sizeof(*shader)));
	shader->info = info;

	for(i = 0; i < MAX_TEXTURES; ++i)
		shader->texds[i] = -1;

	shader->hVertProg = GLEXT_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	glsl_ShaderSrcLoad(&src, "gls_v", nullptr, userdefs);
	GLEXT_glShaderSourceARB(shader->hVertProg, src.size, src.strs, src.lens);
	glsl_ShaderSrcDestroy(&src);

	GLEXT_glCompileShaderARB(shader->hVertProg);
	GLEXT_glGetInfoLogARB(shader->hVertProg, sizeof(buffer), nullptr, buffer);
	GLEXT_glGetObjectParameterivARB(shader->hVertProg,
		GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(status)
		Log::Debug("ShaderLoad: Shader \"{}\" (vertex) compiled OK: {}\n",
			info->name, std::string_view(buffer));
	else
		Log::Fatal("ShaderLoad: Error compiling shader \"{}\" (vertex): {}\n",
			info->name, std::string_view(buffer));

	shader->hFragProg = GLEXT_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	glsl_ShaderSrcLoad(&src, info->name, nullptr, userdefs);
	GLEXT_glShaderSourceARB(shader->hFragProg, src.size, src.strs, src.lens);
	glsl_ShaderSrcDestroy(&src);

	GLEXT_glCompileShaderARB(shader->hFragProg);
	GLEXT_glGetInfoLogARB(shader->hFragProg, sizeof(buffer), nullptr, buffer);
	GLEXT_glGetObjectParameterivARB(shader->hFragProg,
		GL_OBJECT_COMPILE_STATUS_ARB, &status);
	if(status)
		Log::Debug("ShaderLoad: Shader \"{}\" (fragment) compiled OK: {}\n",
			info->name, std::string_view(buffer));
	else
		Log::Fatal("ShaderLoad: Error compiling shader \"{}\" (fragment): {}\n",
			info->name, std::string_view(buffer));

	shader->hShader = GLEXT_glCreateProgramObjectARB();
	GLEXT_glAttachObjectARB(shader->hShader, shader->hVertProg);
	GLEXT_glAttachObjectARB(shader->hShader, shader->hFragProg);
	GLEXT_glLinkProgramARB(shader->hShader);
	GLEXT_glGetInfoLogARB(shader->hShader, sizeof(buffer), nullptr, buffer);
	GLEXT_glGetObjectParameterivARB(shader->hShader, GL_OBJECT_LINK_STATUS_ARB,
		&status);

	if(status)
		Log::Debug("ShaderLoad: Shader \"{}\" linked OK: {}\n", info->name,
			std::string_view(buffer));
	else
		Log::Fatal("ShaderLoad: Error linking shader \"{}\": {}\n", info->name,
			std::string_view(buffer));

	GLEXT_glUseProgramObjectARB(shader->hShader);

	for(unif = info->unifs, i = 0; unif->name; ++unif, ++i)
	{
		int idx;

		if(i >= MAX_UNIFORMS)
			Log::Fatal("ShaderLoad: Too many uniforms in shader \"{}\"\n", info->name);

		idx = GLEXT_glGetUniformLocationARB(shader->hShader, unif->name);
		if(idx == -1)
			Log::Fatal("ShaderLoad: No such uniform \"{}\" in shader \"{}\"\n",
				unif->name, info->name);
		shader->indices[i] = idx;

		switch(unif->type)
		{
			case ShaderUniformType::Tex0:
				GLEXT_glUniform1iARB(idx, 0);
				break;
			case ShaderUniformType::Tex1:
				GLEXT_glUniform1iARB(idx, 1);
				break;
			case ShaderUniformType::Tex2:
				GLEXT_glUniform1iARB(idx, 2);
				break;
			case ShaderUniformType::Tex0D:
			case ShaderUniformType::Tex1D:
			case ShaderUniformType::Tex2D:
				t = std::to_underlying(unif->type) - std::to_underlying(ShaderUniformType::Tex0D);
				if(shader->texds[t] != -1)
					Log::Fatal("ShaderLoad: Duplicate texture dimension uniform: {}\n", i);
				shader->texds[t] = idx;
				break;
			default:
				continue;
		}
	}

	GLEXT_glUseProgramObjectARB(0);

	return shader;
}

static unsigned int texds[MAX_TEXTURES][2];
static shader_frame_t stack[MAX_STACK];
static unsigned int sp = 0;

static shader_frame_t* glsl_ShaderFramePush()
{
	if(sp == MAX_STACK - 1)
		Log::Fatal("ShaderFramePush: Max shader stack depth exceeded\n");

	return &stack[sp++];
}

static void glsl_ShaderFrameActivate(const shader_frame_t* frame)
{
	unsigned int i;
	shader_t* shader = frame->shader;
	const shader_uniform_t* unif;

	GLEXT_glUseProgramObjectARB(shader ? shader->hShader : 0);

	if(!frame->shader)
		return;

	for(unif = frame->shader->info->unifs, i = 0; unif->name; ++unif, ++i)
	{
		const shader_uniform_value_t* val = &frame->unifs[i];
		int idx = frame->shader->indices[i];

		switch(unif->type)
		{
			case ShaderUniformType::Int1:
				GLEXT_glUniform1iARB(idx, val->i[0]);
				break;
			case ShaderUniformType::Float1:
				GLEXT_glUniform1fARB(idx, val->f[0]);
				break;
			case ShaderUniformType::Float2:
				GLEXT_glUniform2fARB(idx, val->f[0], val->f[1]);
				break;
			default:
				continue;
		}
	}

	for(i = 0; i < MAX_TEXTURES; ++i)
	{
		int idx = shader->texds[i];
		if(idx != -1)
			GLEXT_glUniform2fARB(idx, (float)texds[i][0], (float)texds[i][1]);
	}
}

static void glsl_ShaderPush(shader_t* shader, ...)
{
	shader_frame_t* frame = glsl_ShaderFramePush();
	va_list ap;
	int num;

	frame->shader = shader;

	if(shader != nullptr)
	{
		va_start(ap, shader);

		while((num = va_arg(ap, int)) >= 0)
		{
			const shader_uniform_t* unif = &frame->shader->info->unifs[num];
			shader_uniform_value_t* val;

			val = &frame->unifs[num];

			switch(unif->type)
			{
				case ShaderUniformType::Int1:
					val->i[0] = va_arg(ap, GLint);
					break;
				case ShaderUniformType::Float1:
					val->f[0] = va_arg(ap, double);
					break;
				case ShaderUniformType::Float2:
					val->f[0] = va_arg(ap, double);
					val->f[1] = va_arg(ap, double);
					break;
				default:
					Log::Fatal("ShaderPush: Can't dynamically set texture uniform type");
			}
		}

		va_end(ap);
	}

	glsl_ShaderFrameActivate(frame);
}

static void glsl_ShaderPop(shader_t* shader)
{
	if(sp == 0)
		Log::Fatal("ShaderPop: Pop of empty shader stack\n");

	if(stack[sp - 1].shader != shader)
		Log::Fatal("ShaderPop: Pop of incorrect shader (\"{}\" != \"{}\"\n",
			shader->info->name, stack[sp - 1].shader->info->name);

	if(--sp != 0)
		glsl_ShaderFrameActivate(&stack[sp - 1]);
	else
		GLEXT_glUseProgramObjectARB(0);
}

static void glsl_ShaderUniform(shader_t* shader, int num, ...)
{
	shader_frame_t* frame;
	va_list ap;
	const shader_uniform_t* unif = &shader->info->unifs[num];
	int idx = shader->indices[num];
	shader_uniform_value_t* val;

	if(sp == 0)
		Log::Fatal("ShaderUniform: Can't modify shader uniform with empty stack\n");

	frame = &stack[sp - 1];
	if(frame->shader != shader)
		Log::Fatal("ShaderUniform: Can't modify shader uniform for inactive shader\n");

	val = &frame->unifs[num];

	va_start(ap, num);

	switch(unif->type)
	{
		case ShaderUniformType::Int1:
			val->i[0] = va_arg(ap, GLint);
			GLEXT_glUniform1iARB(idx, val->i[0]);
			break;
		case ShaderUniformType::Float1:
			val->f[0] = va_arg(ap, double);
			GLEXT_glUniform1fARB(idx, val->f[0]);
			break;
		case ShaderUniformType::Float2:
			val->f[0] = va_arg(ap, double);
			val->f[1] = va_arg(ap, double);
			GLEXT_glUniform2fARB(idx, val->f[0], val->f[1]);
			break;
		default:
			Log::Fatal("ShaderUniform: Can't dynamically set texture uniform type");
	}

	va_end(ap);
}

void glsl_SetTextureDims(int unit, unsigned int width, unsigned int height)
{
	shader_t* shader;
	int idx;

	assert(unit < MAX_TEXTURES);

	texds[unit][0] = width;
	texds[unit][1] = height;

	if(sp > 0 && (shader = stack[sp - 1].shader))
	{
		idx = shader->texds[unit];
		if(idx != -1)
			GLEXT_glUniform2fARB(idx, (float)width, (float)height);
	}
}

enum struct MainShaderUniform : int32_t
{
	Tex,
	ColorMap,
	LightLevel,
	FadeMode
};

enum struct FuzzShaderUniform : int32_t
{
	Tex,
	Fuzz,
	TexD,
	FuzzD,
	Ratio,
	Seed
};

static shader_t* sh_main = nullptr;
static shader_t* sh_fuzz = nullptr;

static const shader_info_t main_info =
{
	.name = "gls_main",
	.unifs =
	{
		UNIF(std::to_underlying(MainShaderUniform::Tex), "tex", ShaderUniformType::Tex0),
		UNIF(std::to_underlying(MainShaderUniform::ColorMap), "colormap", ShaderUniformType::Tex2),
		UNIF(std::to_underlying(MainShaderUniform::LightLevel), "lightlevel", ShaderUniformType::Float1),
		UNIF(std::to_underlying(MainShaderUniform::FadeMode), "fade_mode", ShaderUniformType::Int1),
		UNIF_END
	}
};

static const shader_info_t fuzz_info =
{
	.name = "gls_fuzz",
	.unifs =
	{
		UNIF(std::to_underlying(FuzzShaderUniform::Tex), "tex", ShaderUniformType::Tex0),
		UNIF(std::to_underlying(FuzzShaderUniform::TexD), "tex_d", ShaderUniformType::Tex0D),
		UNIF(std::to_underlying(FuzzShaderUniform::Fuzz), "fuzz", ShaderUniformType::Tex1),
		UNIF(std::to_underlying(FuzzShaderUniform::FuzzD), "fuzz_d", ShaderUniformType::Tex1D),
		UNIF(std::to_underlying(FuzzShaderUniform::Ratio), "ratio", ShaderUniformType::Float1),
		UNIF(std::to_underlying(FuzzShaderUniform::Seed), "seed", ShaderUniformType::Float1),
		UNIF_END
	}
};

void glsl_Init()
{
	sh_main = glsl_ShaderLoad(&main_info, nullptr);
	sh_fuzz = glsl_ShaderLoad(&fuzz_info, nullptr);
}

void glsl_PushNullShader()
{
	glsl_ShaderPush(nullptr);
}

void glsl_PopNullShader()
{
	glsl_ShaderPop(nullptr);
}

void glsl_PushMainShader()
{
	int mode = dsda_IntConfig(ConfigId::GlFadeMode);

	glsl_ShaderPush(sh_main,
		std::to_underlying(MainShaderUniform::FadeMode), mode,
		UNIF_VAL_END);
}

void glsl_PopMainShader()
{
	glsl_ShaderPop(sh_main);
}

void glsl_SetLightLevel(float lightlevel)
{
	glsl_ShaderUniform(sh_main, std::to_underlying(MainShaderUniform::LightLevel), lightlevel);
}

void glsl_PushFuzzShader(int tic, int sprite, float ratio)
{
	// Large integers converted to float can lose precision, causing
	// problems in the shader.  Since the tic and sprite count are just
	// used for randomness, munge them down and convert to float with
	// double precision here
	const int factor = 1103515245;
	int seed = 0xD00D;

	seed = seed * factor + tic;
	seed = seed * factor + sprite;
	seed *= factor;

	glsl_ShaderPush(sh_fuzz,
		std::to_underlying(FuzzShaderUniform::Ratio), ratio,
		std::to_underlying(FuzzShaderUniform::Seed), (double)seed / INT_MAX,
		UNIF_VAL_END);
}

void glsl_PopFuzzShader()
{
	glsl_ShaderPop(sh_fuzz);
}
