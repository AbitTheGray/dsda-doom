// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *   Thanks Roman "Vortex" Marchenko
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gl_opengl.h"

#include <SDL.h>

#include "gl_intern.h"

#include "i_main.h"
#include "lprintf.h"

dboolean gl_use_FBO = false;

GLuint glSceneImageFBOTexID = 0;
GLuint glDepthBufferFBOTexID = 0;
GLuint glSceneImageTextureFBOTexID = 0;
int SceneInTexture = false;
static dboolean gld_CreateScreenSizeFBO(void);

void gld_InitFBO(void)
{
	gld_FreeScreenSizeFBO();

	gl_use_FBO = gl_ext_framebuffer_object;

	if(gl_use_FBO)
	{
		if(!gld_CreateScreenSizeFBO())
		{
			gld_FreeScreenSizeFBO();
			gl_use_FBO = false;
			gl_ext_framebuffer_object = false;
		}
	}
}

static dboolean gld_CreateScreenSizeFBO(void)
{
	int status = 0;
	GLenum internalFormat;
	dboolean attach_stencil = gl_ext_packed_depth_stencil;

	if(!gl_ext_framebuffer_object)
		return false;

	GLEXT_glGenFramebuffersEXT(1, &glSceneImageFBOTexID);
	GLEXT_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, glSceneImageFBOTexID);

	GLEXT_glGenRenderbuffersEXT(1, &glDepthBufferFBOTexID);
	GLEXT_glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);

	internalFormat = (attach_stencil ? GL_DEPTH_STENCIL_EXT : GL_DEPTH_COMPONENT);
	GLEXT_glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalFormat, SCREENWIDTH, SCREENHEIGHT);

	// attach a renderbuffer to depth attachment point
	GLEXT_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);

	if(attach_stencil)
	{
		// attach a renderbuffer to stencil attachment point
		GLEXT_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, glDepthBufferFBOTexID);
	}

	glGenTextures(1, &glSceneImageTextureFBOTexID);
	glBindTexture(GL_TEXTURE_2D, glSceneImageTextureFBOTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLEXT_glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glSceneImageTextureFBOTexID, 0);
	status = GLEXT_glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		GLEXT_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}
	else
	{
		lprintf(LO_ERROR, "gld_CreateScreenSizeFBO: Cannot create framebuffer object (error code: %d)\n", status);
	}

	return (status == GL_FRAMEBUFFER_COMPLETE_EXT);
}

void gld_FreeScreenSizeFBO(void)
{
	if(!gl_ext_framebuffer_object)
		return;

	GLEXT_glDeleteFramebuffersEXT(1, &glSceneImageFBOTexID);
	glSceneImageFBOTexID = 0;

	GLEXT_glDeleteRenderbuffersEXT(1, &glDepthBufferFBOTexID);
	glDepthBufferFBOTexID = 0;

	glDeleteTextures(1, &glSceneImageTextureFBOTexID);
	glSceneImageTextureFBOTexID = 0;
}
