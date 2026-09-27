// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Thanks Roman "Vortex" Marchenko
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include <SDL.h>
#include "gl_opengl.hpp"

#include "doomtype.hpp"
#include "lprintf.hpp"

#include "dsda/configuration.hpp"

#define isExtensionSupported(ext) strstr(extensions, ext)

int gl_max_texture_size = 0;

SDL_PixelFormat RGBAFormat;

dboolean gl_ext_texture_filter_anisotropic = false;
dboolean gl_arb_texture_compression = false;
dboolean gl_ext_framebuffer_object = false;
dboolean gl_ext_packed_depth_stencil = false;
dboolean gl_ext_blend_color = false;
dboolean gl_use_stencil = false;
dboolean gl_ext_arb_vertex_buffer_object = false;
dboolean gl_arb_pixel_buffer_object = false;
dboolean gl_arb_shader_objects = false;

int active_texture_enabled[32];
int clieant_active_texture_enabled[32];

/* EXT_framebuffer_object */
PFNGLBINDFRAMEBUFFEREXTPROC GLEXT_glBindFramebufferEXT = nullptr;
PFNGLGENFRAMEBUFFERSEXTPROC GLEXT_glGenFramebuffersEXT = nullptr;
PFNGLGENRENDERBUFFERSEXTPROC GLEXT_glGenRenderbuffersEXT = nullptr;
PFNGLBINDRENDERBUFFEREXTPROC GLEXT_glBindRenderbufferEXT = nullptr;
PFNGLRENDERBUFFERSTORAGEEXTPROC GLEXT_glRenderbufferStorageEXT = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC GLEXT_glFramebufferRenderbufferEXT = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC GLEXT_glFramebufferTexture2DEXT = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC GLEXT_glCheckFramebufferStatusEXT = nullptr;
PFNGLDELETEFRAMEBUFFERSEXTPROC GLEXT_glDeleteFramebuffersEXT = nullptr;
PFNGLDELETERENDERBUFFERSEXTPROC GLEXT_glDeleteRenderbuffersEXT = nullptr;

/* ARB_multitexture command function pointers */
PFNGLACTIVETEXTUREARBPROC GLEXT_glActiveTextureARB = nullptr;
PFNGLCLIENTACTIVETEXTUREARBPROC GLEXT_glClientActiveTextureARB = nullptr;
PFNGLMULTITEXCOORD2FARBPROC GLEXT_glMultiTexCoord2fARB = nullptr;
PFNGLMULTITEXCOORD2FVARBPROC GLEXT_glMultiTexCoord2fvARB = nullptr;

/* ARB_texture_compression */
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC GLEXT_glCompressedTexImage2DARB = nullptr;

PFNGLBLENDCOLOREXTPROC GLEXT_glBlendColorEXT = nullptr;

/* VBO */
PFNGLGENBUFFERSARBPROC GLEXT_glGenBuffersARB = nullptr;
PFNGLDELETEBUFFERSARBPROC GLEXT_glDeleteBuffersARB = nullptr;
PFNGLBINDBUFFERARBPROC GLEXT_glBindBufferARB = nullptr;
PFNGLBUFFERDATAARBPROC GLEXT_glBufferDataARB = nullptr;

/* PBO */
PFNGLBUFFERSUBDATAARBPROC GLEXT_glBufferSubDataARB = nullptr;
PFNGLGETBUFFERPARAMETERIVARBPROC GLEXT_glGetBufferParameterivARB = nullptr;
PFNGLMAPBUFFERARBPROC GLEXT_glMapBufferARB = nullptr;
PFNGLUNMAPBUFFERARBPROC GLEXT_glUnmapBufferARB = nullptr;

/* GL_ARB_shader_objects */
PFNGLDELETEOBJECTARBPROC GLEXT_glDeleteObjectARB = nullptr;
PFNGLGETHANDLEARBPROC GLEXT_glGetHandleARB = nullptr;
PFNGLDETACHOBJECTARBPROC GLEXT_glDetachObjectARB = nullptr;
PFNGLCREATESHADEROBJECTARBPROC GLEXT_glCreateShaderObjectARB = nullptr;
PFNGLSHADERSOURCEARBPROC GLEXT_glShaderSourceARB = nullptr;
PFNGLCOMPILESHADERARBPROC GLEXT_glCompileShaderARB = nullptr;
PFNGLCREATEPROGRAMOBJECTARBPROC GLEXT_glCreateProgramObjectARB = nullptr;
PFNGLATTACHOBJECTARBPROC GLEXT_glAttachObjectARB = nullptr;
PFNGLLINKPROGRAMARBPROC GLEXT_glLinkProgramARB = nullptr;
PFNGLUSEPROGRAMOBJECTARBPROC GLEXT_glUseProgramObjectARB = nullptr;
PFNGLVALIDATEPROGRAMARBPROC GLEXT_glValidateProgramARB = nullptr;

PFNGLUNIFORM1FARBPROC GLEXT_glUniform1fARB = nullptr;
PFNGLUNIFORM2FARBPROC GLEXT_glUniform2fARB = nullptr;
PFNGLUNIFORM1IARBPROC GLEXT_glUniform1iARB = nullptr;

PFNGLGETOBJECTPARAMETERFVARBPROC GLEXT_glGetObjectParameterfvARB = nullptr;
PFNGLGETOBJECTPARAMETERIVARBPROC GLEXT_glGetObjectParameterivARB = nullptr;
PFNGLGETINFOLOGARBPROC GLEXT_glGetInfoLogARB = nullptr;
PFNGLGETATTACHEDOBJECTSARBPROC GLEXT_glGetAttachedObjectsARB = nullptr;
PFNGLGETUNIFORMLOCATIONARBPROC GLEXT_glGetUniformLocationARB = nullptr;
PFNGLGETACTIVEUNIFORMARBPROC GLEXT_glGetActiveUniformARB = nullptr;
PFNGLGETUNIFORMFVARBPROC GLEXT_glGetUniformfvARB = nullptr;

int gl_major_version;
int gl_minor_version;

void gld_InitOpenGLVersion()
{
	sscanf((const char*)glGetString(GL_VERSION), "%d.%d", &gl_major_version, &gl_minor_version);
}

void gld_InitOpenGL()
{
	GLenum texture;
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	dboolean gl_arb_multitexture = false;
	dboolean gl_arb_texture_non_power_of_two = false;

	gld_InitOpenGLVersion();

	gl_ext_texture_filter_anisotropic = isExtensionSupported("GL_EXT_texture_filter_anisotropic") != nullptr;
	if(gl_ext_texture_filter_anisotropic)
		Log::Debug("using GL_EXT_texture_filter_anisotropic\n");

	// Any textures sizes are allowed
	gl_arb_texture_non_power_of_two = isExtensionSupported("GL_ARB_texture_non_power_of_two") != nullptr;
	if(!gl_arb_texture_non_power_of_two)
		Log::Fatal("gld_InitOpenGL: OpenGL driver does not support GL_ARB_texture_non_power_of_two");

	//
	// ARB_multitexture command function pointers
	//

	gl_arb_multitexture = isExtensionSupported("GL_ARB_multitexture") != nullptr;
	if(gl_arb_multitexture)
	{
		GLEXT_glActiveTextureARB = reinterpret_cast<decltype(GLEXT_glActiveTextureARB)>(SDL_GL_GetProcAddress("glActiveTextureARB"));
		GLEXT_glClientActiveTextureARB = reinterpret_cast<decltype(GLEXT_glClientActiveTextureARB)>(SDL_GL_GetProcAddress("glClientActiveTextureARB"));
		GLEXT_glMultiTexCoord2fARB = reinterpret_cast<decltype(GLEXT_glMultiTexCoord2fARB)>(SDL_GL_GetProcAddress("glMultiTexCoord2fARB"));
		GLEXT_glMultiTexCoord2fvARB = reinterpret_cast<decltype(GLEXT_glMultiTexCoord2fvARB)>(SDL_GL_GetProcAddress("glMultiTexCoord2fvARB"));

		if(!GLEXT_glActiveTextureARB || !GLEXT_glClientActiveTextureARB ||
			!GLEXT_glMultiTexCoord2fARB || !GLEXT_glMultiTexCoord2fvARB)
			gl_arb_multitexture = false;
	}
	if(!gl_arb_multitexture)
		Log::Fatal("gld_InitOpenGL: OpenGL driver does not support GL_ARB_multitexture");

	//
	// ARB_texture_compression
	//

	gl_arb_texture_compression = isExtensionSupported("GL_ARB_texture_compression") != nullptr;
	if(gl_arb_texture_compression)
	{
		GLEXT_glCompressedTexImage2DARB = reinterpret_cast<decltype(GLEXT_glCompressedTexImage2DARB)>(SDL_GL_GetProcAddress("glCompressedTexImage2DARB"));

		if(!GLEXT_glCompressedTexImage2DARB)
			gl_arb_texture_compression = false;
	}
	if(gl_arb_texture_compression)
		Log::Debug("using GL_ARB_texture_compression\n");

	//
	// EXT_framebuffer_object
	//
	gl_ext_framebuffer_object = isExtensionSupported("GL_EXT_framebuffer_object") != nullptr;

	if(gl_ext_framebuffer_object)
	{
		GLEXT_glGenFramebuffersEXT = reinterpret_cast<decltype(GLEXT_glGenFramebuffersEXT)>(SDL_GL_GetProcAddress("glGenFramebuffersEXT"));
		GLEXT_glBindFramebufferEXT = reinterpret_cast<decltype(GLEXT_glBindFramebufferEXT)>(SDL_GL_GetProcAddress("glBindFramebufferEXT"));
		GLEXT_glGenRenderbuffersEXT = reinterpret_cast<decltype(GLEXT_glGenRenderbuffersEXT)>(SDL_GL_GetProcAddress("glGenRenderbuffersEXT"));
		GLEXT_glBindRenderbufferEXT = reinterpret_cast<decltype(GLEXT_glBindRenderbufferEXT)>(SDL_GL_GetProcAddress("glBindRenderbufferEXT"));
		GLEXT_glRenderbufferStorageEXT = reinterpret_cast<decltype(GLEXT_glRenderbufferStorageEXT)>(SDL_GL_GetProcAddress("glRenderbufferStorageEXT"));
		GLEXT_glFramebufferRenderbufferEXT = reinterpret_cast<decltype(GLEXT_glFramebufferRenderbufferEXT)>(SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT"));
		GLEXT_glFramebufferTexture2DEXT = reinterpret_cast<decltype(GLEXT_glFramebufferTexture2DEXT)>(SDL_GL_GetProcAddress("glFramebufferTexture2DEXT"));
		GLEXT_glCheckFramebufferStatusEXT = reinterpret_cast<decltype(GLEXT_glCheckFramebufferStatusEXT)>(SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT"));
		GLEXT_glDeleteFramebuffersEXT = reinterpret_cast<decltype(GLEXT_glDeleteFramebuffersEXT)>(SDL_GL_GetProcAddress("glDeleteFramebuffersEXT"));
		GLEXT_glDeleteRenderbuffersEXT = reinterpret_cast<decltype(GLEXT_glDeleteRenderbuffersEXT)>(SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT"));

		if(!GLEXT_glGenFramebuffersEXT || !GLEXT_glBindFramebufferEXT ||
			!GLEXT_glGenRenderbuffersEXT || !GLEXT_glBindRenderbufferEXT ||
			!GLEXT_glRenderbufferStorageEXT || !GLEXT_glFramebufferRenderbufferEXT ||
			!GLEXT_glFramebufferTexture2DEXT || !GLEXT_glCheckFramebufferStatusEXT ||
			!GLEXT_glDeleteFramebuffersEXT || !GLEXT_glDeleteRenderbuffersEXT)
			gl_ext_framebuffer_object = false;
	}
	if(gl_ext_framebuffer_object)
		Log::Debug("using GL_EXT_framebuffer_object\n");

	gl_ext_packed_depth_stencil = isExtensionSupported("GL_EXT_packed_depth_stencil") != nullptr;
	if(gl_ext_packed_depth_stencil)
		Log::Debug("using GL_EXT_packed_depth_stencil\n");

	//
	// Blending
	//

	gl_ext_blend_color = isExtensionSupported("GL_EXT_blend_color") != nullptr;
	if(gl_ext_blend_color)
	{
		GLEXT_glBlendColorEXT = reinterpret_cast<decltype(GLEXT_glBlendColorEXT)>(SDL_GL_GetProcAddress("glBlendColorEXT"));

		if(!GLEXT_glBlendColorEXT)
			gl_ext_blend_color = false;
	}
	if(gl_ext_blend_color)
		Log::Debug("using GL_EXT_blend_color\n");

	// VBO
	if(dsda_IntConfig(ConfigId::GlUsevbo))
	{
		gl_ext_arb_vertex_buffer_object = isExtensionSupported("GL_ARB_vertex_buffer_object") != nullptr;
		if(gl_ext_arb_vertex_buffer_object)
		{
			GLEXT_glGenBuffersARB = reinterpret_cast<decltype(GLEXT_glGenBuffersARB)>(SDL_GL_GetProcAddress("glGenBuffersARB"));
			GLEXT_glDeleteBuffersARB = reinterpret_cast<decltype(GLEXT_glDeleteBuffersARB)>(SDL_GL_GetProcAddress("glDeleteBuffersARB"));
			GLEXT_glBindBufferARB = reinterpret_cast<decltype(GLEXT_glBindBufferARB)>(SDL_GL_GetProcAddress("glBindBufferARB"));
			GLEXT_glBufferDataARB = reinterpret_cast<decltype(GLEXT_glBufferDataARB)>(SDL_GL_GetProcAddress("glBufferDataARB"));

			if(!GLEXT_glGenBuffersARB || !GLEXT_glDeleteBuffersARB ||
				!GLEXT_glBindBufferARB || !GLEXT_glBufferDataARB)
				gl_ext_arb_vertex_buffer_object = false;
		}
		if(gl_ext_arb_vertex_buffer_object)
			Log::Debug("using GL_ARB_vertex_buffer_object\n");
	}

	gl_arb_pixel_buffer_object = isExtensionSupported("GL_ARB_pixel_buffer_object") != nullptr;
	if(gl_arb_pixel_buffer_object)
	{
		GLEXT_glGenBuffersARB = reinterpret_cast<decltype(GLEXT_glGenBuffersARB)>(SDL_GL_GetProcAddress("glGenBuffersARB"));
		GLEXT_glBindBufferARB = reinterpret_cast<decltype(GLEXT_glBindBufferARB)>(SDL_GL_GetProcAddress("glBindBufferARB"));
		GLEXT_glBufferDataARB = reinterpret_cast<decltype(GLEXT_glBufferDataARB)>(SDL_GL_GetProcAddress("glBufferDataARB"));
		GLEXT_glBufferSubDataARB = reinterpret_cast<decltype(GLEXT_glBufferSubDataARB)>(SDL_GL_GetProcAddress("glBufferSubDataARB"));
		GLEXT_glDeleteBuffersARB = reinterpret_cast<decltype(GLEXT_glDeleteBuffersARB)>(SDL_GL_GetProcAddress("glDeleteBuffersARB"));
		GLEXT_glGetBufferParameterivARB = reinterpret_cast<decltype(GLEXT_glGetBufferParameterivARB)>(SDL_GL_GetProcAddress("glGetBufferParameterivARB"));
		GLEXT_glMapBufferARB = reinterpret_cast<decltype(GLEXT_glMapBufferARB)>(SDL_GL_GetProcAddress("glMapBufferARB"));
		GLEXT_glUnmapBufferARB = reinterpret_cast<decltype(GLEXT_glUnmapBufferARB)>(SDL_GL_GetProcAddress("glUnmapBufferARB"));

		if(!GLEXT_glGenBuffersARB || !GLEXT_glBindBufferARB ||
			!GLEXT_glBufferDataARB || !GLEXT_glBufferSubDataARB ||
			!GLEXT_glDeleteBuffersARB || !GLEXT_glGetBufferParameterivARB ||
			!GLEXT_glMapBufferARB || !GLEXT_glUnmapBufferARB)
			gl_arb_pixel_buffer_object = false;
	}
	if(gl_arb_pixel_buffer_object)
		Log::Debug("using GL_ARB_pixel_buffer_object\n");

	//
	// Stencil support
	//

	gl_use_stencil = true;

	//
	// GL_ARB_shader_objects
	//
	gl_arb_shader_objects = isExtensionSupported("GL_ARB_shader_objects") &&
		isExtensionSupported("GL_ARB_vertex_shader") &&
		isExtensionSupported("GL_ARB_fragment_shader") &&
		isExtensionSupported("GL_ARB_shading_language_100");
	if(gl_arb_shader_objects)
	{
		GLEXT_glDeleteObjectARB = reinterpret_cast<decltype(GLEXT_glDeleteObjectARB)>(SDL_GL_GetProcAddress("glDeleteObjectARB"));
		GLEXT_glGetHandleARB = reinterpret_cast<decltype(GLEXT_glGetHandleARB)>(SDL_GL_GetProcAddress("glGetHandleARB"));
		GLEXT_glDetachObjectARB = reinterpret_cast<decltype(GLEXT_glDetachObjectARB)>(SDL_GL_GetProcAddress("glDetachObjectARB"));
		GLEXT_glCreateShaderObjectARB = reinterpret_cast<decltype(GLEXT_glCreateShaderObjectARB)>(SDL_GL_GetProcAddress("glCreateShaderObjectARB"));
		GLEXT_glShaderSourceARB = reinterpret_cast<decltype(GLEXT_glShaderSourceARB)>(SDL_GL_GetProcAddress("glShaderSourceARB"));
		GLEXT_glCompileShaderARB = reinterpret_cast<decltype(GLEXT_glCompileShaderARB)>(SDL_GL_GetProcAddress("glCompileShaderARB"));
		GLEXT_glCreateProgramObjectARB = reinterpret_cast<decltype(GLEXT_glCreateProgramObjectARB)>(SDL_GL_GetProcAddress("glCreateProgramObjectARB"));
		GLEXT_glAttachObjectARB = reinterpret_cast<decltype(GLEXT_glAttachObjectARB)>(SDL_GL_GetProcAddress("glAttachObjectARB"));
		GLEXT_glLinkProgramARB = reinterpret_cast<decltype(GLEXT_glLinkProgramARB)>(SDL_GL_GetProcAddress("glLinkProgramARB"));
		GLEXT_glUseProgramObjectARB = reinterpret_cast<decltype(GLEXT_glUseProgramObjectARB)>(SDL_GL_GetProcAddress("glUseProgramObjectARB"));
		GLEXT_glValidateProgramARB = reinterpret_cast<decltype(GLEXT_glValidateProgramARB)>(SDL_GL_GetProcAddress("glValidateProgramARB"));

		GLEXT_glUniform1fARB = reinterpret_cast<decltype(GLEXT_glUniform1fARB)>(SDL_GL_GetProcAddress("glUniform1fARB"));
		GLEXT_glUniform2fARB = reinterpret_cast<decltype(GLEXT_glUniform2fARB)>(SDL_GL_GetProcAddress("glUniform2fARB"));
		GLEXT_glUniform1iARB = reinterpret_cast<decltype(GLEXT_glUniform1iARB)>(SDL_GL_GetProcAddress("glUniform1iARB"));

		GLEXT_glGetObjectParameterfvARB = reinterpret_cast<decltype(GLEXT_glGetObjectParameterfvARB)>(SDL_GL_GetProcAddress("glGetObjectParameterfvARB"));
		GLEXT_glGetObjectParameterivARB = reinterpret_cast<decltype(GLEXT_glGetObjectParameterivARB)>(SDL_GL_GetProcAddress("glGetObjectParameterivARB"));
		GLEXT_glGetInfoLogARB = reinterpret_cast<decltype(GLEXT_glGetInfoLogARB)>(SDL_GL_GetProcAddress("glGetInfoLogARB"));
		GLEXT_glGetAttachedObjectsARB = reinterpret_cast<decltype(GLEXT_glGetAttachedObjectsARB)>(SDL_GL_GetProcAddress("glGetAttachedObjectsARB"));
		GLEXT_glGetUniformLocationARB = reinterpret_cast<decltype(GLEXT_glGetUniformLocationARB)>(SDL_GL_GetProcAddress("glGetUniformLocationARB"));
		GLEXT_glGetActiveUniformARB = reinterpret_cast<decltype(GLEXT_glGetActiveUniformARB)>(SDL_GL_GetProcAddress("glGetActiveUniformARB"));
		GLEXT_glGetUniformfvARB = reinterpret_cast<decltype(GLEXT_glGetUniformfvARB)>(SDL_GL_GetProcAddress("glGetUniformfvARB"));

		if(!GLEXT_glDeleteObjectARB || !GLEXT_glGetHandleARB ||
			!GLEXT_glDetachObjectARB || !GLEXT_glCreateShaderObjectARB ||
			!GLEXT_glShaderSourceARB || !GLEXT_glCompileShaderARB ||
			!GLEXT_glCreateProgramObjectARB || !GLEXT_glAttachObjectARB ||
			!GLEXT_glLinkProgramARB || !GLEXT_glUseProgramObjectARB ||
			!GLEXT_glValidateProgramARB ||
			!GLEXT_glUniform1fARB || !GLEXT_glUniform2fARB ||
			!GLEXT_glUniform1iARB ||
			!GLEXT_glGetObjectParameterfvARB || !GLEXT_glGetObjectParameterivARB ||
			!GLEXT_glGetInfoLogARB || !GLEXT_glGetAttachedObjectsARB ||
			!GLEXT_glGetUniformLocationARB || !GLEXT_glGetActiveUniformARB ||
			!GLEXT_glGetUniformfvARB)
			gl_arb_shader_objects = false;
	}

	if(!gl_arb_shader_objects)
	{
		Log::Fatal("gld_InitOpenGL: Insufficient support for shader objects");
	}

	Log::Debug("using GL_ARB_shader_objects\n");
	Log::Debug("using GL_ARB_vertex_shader\n");
	Log::Debug("using GL_ARB_fragment_shader\n");
	Log::Debug("using GL_ARB_shading_language_100\n");

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gl_max_texture_size);
	Log::Debug("GL_MAX_TEXTURE_SIZE={}\n", gl_max_texture_size);

	//init states manager
	gld_EnableMultisample(true);
	gld_EnableMultisample(false);

	for(texture = GL_TEXTURE0_ARB; texture <= GL_TEXTURE31_ARB; texture++)
	{
		gld_EnableTexture2D(texture, true);
		gld_EnableTexture2D(texture, false);

		gld_EnableClientCoordArray(texture, true);
		gld_EnableClientCoordArray(texture, false);
	}

	//init global variables
	RGBAFormat.palette = nullptr;
	RGBAFormat.BitsPerPixel = 32;
	RGBAFormat.BytesPerPixel = 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	RGBAFormat.Rmask = 0xFF000000; RGBAFormat.Rshift = 0; RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x00FF0000; RGBAFormat.Gshift = 8; RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x0000FF00; RGBAFormat.Bshift = 16; RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0x000000FF; RGBAFormat.Ashift = 24; RGBAFormat.Aloss = 0;
#else
	RGBAFormat.Rmask = 0x000000FF;
	RGBAFormat.Rshift = 24;
	RGBAFormat.Rloss = 0;
	RGBAFormat.Gmask = 0x0000FF00;
	RGBAFormat.Gshift = 16;
	RGBAFormat.Gloss = 0;
	RGBAFormat.Bmask = 0x00FF0000;
	RGBAFormat.Bshift = 8;
	RGBAFormat.Bloss = 0;
	RGBAFormat.Amask = 0xFF000000;
	RGBAFormat.Ashift = 0;
	RGBAFormat.Aloss = 0;
#endif
}

void gld_EnableTexture2D(GLenum texture, int enable)
{
	int arb;

	arb = texture - GL_TEXTURE0_ARB;

#ifdef RANGECHECK
	if(arb < 0 || arb > 31)
		Log::Fatal("gld_EnableTexture2D: wronge ARB texture unit {}", arb);
#endif

	if(enable)
	{
		if(!active_texture_enabled[arb])
		{
			if(arb != 0)
			{
				GLEXT_glActiveTextureARB(texture);
				glEnable(GL_TEXTURE_2D);
				GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
			}
			else
			{
				glEnable(GL_TEXTURE_2D);
			}
			active_texture_enabled[arb] = enable;
		}
	}
	else
	{
		if(active_texture_enabled[arb])
		{
			if(arb != 0)
			{
				GLEXT_glActiveTextureARB(texture);
				glDisable(GL_TEXTURE_2D);
				GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
			active_texture_enabled[arb] = enable;
		}
	}
}

void gld_EnableClientCoordArray(GLenum texture, int enable)
{
	int arb;

	arb = texture - GL_TEXTURE0_ARB;

#ifdef RANGECHECK
	if(arb < 0 || arb > 31)
		Log::Fatal("gld_EnableTexture2D: wronge ARB texture unit {}", arb);
#endif

	if(enable)
	{
		if(!clieant_active_texture_enabled[arb])
		{
			GLEXT_glClientActiveTextureARB(texture);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			GLEXT_glClientActiveTextureARB(GL_TEXTURE0_ARB);

			clieant_active_texture_enabled[arb] = enable;
		}
	}
	else
	{
		if(clieant_active_texture_enabled[arb])
		{
			GLEXT_glClientActiveTextureARB(texture);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			GLEXT_glClientActiveTextureARB(GL_TEXTURE0_ARB);

			clieant_active_texture_enabled[arb] = enable;
		}
	}
}

void gld_EnableMultisample(int enable)
{
	static int multisample_is_enabled = 0;
	if(enable)
	{
		if(!multisample_is_enabled)
		{
			glEnable(GL_MULTISAMPLE_ARB);

			multisample_is_enabled = enable;
		}
	}
	else
	{
		if(multisample_is_enabled)
		{
			glDisable(GL_MULTISAMPLE_ARB);

			multisample_is_enabled = enable;
		}
	}
}

void SetTextureMode(TexMode type)
{
	if(type == TexMode::Mask)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
	else if(type == TexMode::Opaque)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
	else if(type == TexMode::Invert)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	}
	else if(type == TexMode::InvertOpaque)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	}
	else // if (type == TM_MODULATE)
	{
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
}
