// SPDX-License-Identifier: GPL-2.0-or-later

/* DESCRIPTION:
 *  External simple file handling.
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include "i_glob.hpp"
#include "lprintf.hpp"
#include "z_zone.hpp"

#include "m_file.hpp"

#ifdef _MSC_VER
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define F_OK 0
#define W_OK 2
#define R_OK 4
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define MKDIR_NO_ERROR 0

#ifdef _WIN32
static wchar_t* ConvertMultiByteToWide(const char* str, UINT code_page)
{
	wchar_t* wstr = nullptr;
	int wlen = 0;

	wlen = MultiByteToWideChar(code_page, 0, str, -1, nullptr, 0);

	if(!wlen)
	{
		errno = EINVAL;
		lprintf(OutputLevels::Info, "Warning: Failed to convert path to wide encoding\n");
		return nullptr;
	}

	wstr = static_cast<wchar_t*>(Z_Malloc(sizeof(wchar_t) * wlen));

	if(!wstr)
	{
		lprintf(OutputLevels::Info, "ConvertMultiByteToWide: Failed to allocate new string\n");
		return nullptr;
	}

	if(MultiByteToWideChar(code_page, 0, str, -1, wstr, wlen) == 0)
	{
		errno = EINVAL;
		lprintf(OutputLevels::Info, "Warning: Failed to convert path to wide encoding\n");
		Z_Free(wstr);
		return nullptr;
	}

	return wstr;
}

static char* ConvertWideToMultiByte(const wchar_t* wstr, UINT code_page)
{
	char* str = nullptr;
	int len = 0;

	len = WideCharToMultiByte(code_page, 0, wstr, -1, nullptr, 0, nullptr, nullptr);

	if(!len)
	{
		errno = EINVAL;
		lprintf(OutputLevels::Info, "Warning: Failed to convert path to multi byte encoding\n");
		return nullptr;
	}

	str = static_cast<char*>(Z_Malloc(sizeof(char) * len));

	if(!str)
	{
		lprintf(OutputLevels::Info, "ConvertWideToMultiByte: Failed to allocate new string\n");
		return nullptr;
	}

	if(WideCharToMultiByte(code_page, 0, wstr, -1, str, len, nullptr, nullptr) == 0)
	{
		errno = EINVAL;
		lprintf(OutputLevels::Info, "Warning: Failed to convert path to multi byte encoding\n");
		Z_Free(str);
		return nullptr;
	}

	return str;
}

wchar_t* ConvertUtf8ToWide(const char* str)
{
	return ConvertMultiByteToWide(str, CP_UTF8);
}

char* ConvertWideToUtf8(const wchar_t* wstr)
{
	return ConvertWideToMultiByte(wstr, CP_UTF8);
}

static wchar_t* ConvertSysNativeMBToWide(const char* str)
{
	return ConvertMultiByteToWide(str, CP_ACP);
}

static char* ConvertWideToSysNativeMB(const wchar_t* wstr)
{
	return ConvertWideToMultiByte(wstr, CP_ACP);
}
#endif

char* ConvertSysNativeMBToUtf8(const char* str)
{
#ifdef _WIN32
	char* ret = nullptr;
	wchar_t* wstr = nullptr;

	wstr = ConvertSysNativeMBToWide(str);

	if(!wstr)
		return nullptr;

	ret = ConvertWideToUtf8(wstr);

	Z_Free(wstr);

	return ret;
#else
	return Z_Strdup(str);
#endif
}

char* ConvertUtf8ToSysNativeMB(const char* str)
{
#ifdef _WIN32
	char* ret = nullptr;
	wchar_t* wstr = nullptr;

	wstr = ConvertUtf8ToWide(str);

	if(!wstr)
		return nullptr;

	ret = ConvertWideToSysNativeMB(wstr);

	Z_Free(wstr);

	return ret;
#else
	return Z_Strdup(str);
#endif
}

static int M_open(const char* filename, int oflag)
{
#ifdef _WIN32
	wchar_t* wname;
	int ret;

	wname = ConvertUtf8ToWide(filename);

	if(!wname)
		return 0;

	ret = _wopen(wname, oflag);

	Z_Free(wname);

	return ret;
#else
	return open(filename, oflag);
#endif
}

static int M_access(const char* path, int mode)
{
#ifdef _WIN32
	wchar_t* wpath;
	int ret;

	wpath = ConvertUtf8ToWide(path);

	if(!wpath)
		return 0;

	ret = _waccess(wpath, mode);

	Z_Free(wpath);

	return ret;
#else
	return access(path, mode);
#endif
}

static int M_mkdir(const char* path)
{
#ifdef _WIN32
	wchar_t* wdir = nullptr;
	int ret;

	wdir = ConvertUtf8ToWide(path);

	if(!wdir)
		return -1;

	ret = _wmkdir(wdir);

	Z_Free(wdir);

	return ret;
#else
	return mkdir(path, 0755);
#endif
}

static int M_stat(const char* path, struct stat* buf)
{
#ifdef _WIN32
	wchar_t* wpath;
	struct _stat wbuf;
	int ret;

	wpath = ConvertUtf8ToWide(path);

	if(!wpath)
		return -1;

	ret = _wstat(wpath, &wbuf);

	// The _wstat() function expects a struct _stat* parameter that is
	// incompatible with struct stat*. We copy only the required compatible
	// field.
	buf->st_mode = wbuf.st_mode;
	buf->st_mtime = wbuf.st_mtime;

	Z_Free(wpath);

	return ret;
#else
	return stat(path, buf);
#endif
}

int M_remove(const char* path)
{
#ifdef _WIN32
	wchar_t* wpath;
	int ret;

	wpath = ConvertUtf8ToWide(path);

	if(!wpath)
		return -1;

	ret = _wremove(wpath);

	Z_Free(wpath);

	return ret;
#else
	return remove(path);
#endif
}

int M_MakeDir(const char* path, int require)
{
	int error;

	if(M_IsDir(path))
		return MKDIR_NO_ERROR;

	error = M_mkdir(path);

	if(require && error)
		I_Error("Unable to create directory %s (%d)", path, errno);

	return error;
}

dboolean M_IsDir(const char* name)
{
	struct stat sbuf;

	return !M_stat(name, &sbuf) && S_ISDIR(sbuf.st_mode);
}

dboolean M_ReadWriteAccess(const char* name)
{
	return !M_access(name, R_OK | W_OK);
}

dboolean M_ReadAccess(const char* name)
{
	return !M_access(name, R_OK);
}

dboolean M_WriteAccess(const char* name)
{
	return !M_access(name, W_OK);
}

FILE* M_OpenFile(const char* name, const char* mode)
{
#ifdef _WIN32
	FILE* file;
	wchar_t *wname, *wmode;

	wname = ConvertUtf8ToWide(name);

	if(!wname)
		return nullptr;

	wmode = ConvertUtf8ToWide(mode);

	if(!wmode)
	{
		Z_Free(wname);
		return nullptr;
	}

	file = _wfopen(wname, wmode);

	Z_Free(wname);
	Z_Free(wmode);

	return file;
#else
	return fopen(name, mode);
#endif
}

int M_OpenRB(const char* name)
{
	return M_open(name, O_RDONLY | O_BINARY);
}

dboolean M_FileExists(const char* name)
{
	FILE* fp;

	fp = M_OpenFile(name, "rb");

	if(fp)
	{
		fclose(fp);

		return true;
	}

	return false;
}

/*
 * M_WriteFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

dboolean M_WriteFile(char const* name, const void* source, size_t length)
{
	FILE* fp;

	errno = 0;

	if(!(fp = M_OpenFile(name, "wb"))) // Try opening file
		return 0;                      // Could not open file for writing

	length = fwrite(source, 1, length, fp) == (size_t)length; // Write data
	fclose(fp);

	if(!length) // Remove partially written file
		M_remove(name);

	return length;
}

/*
 * M_ReadFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

int M_ReadFile(char const* name, byte** buffer)
{
	FILE* fp;

	if((fp = M_OpenFile(name, "rb")))
	{
		size_t length;

		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		*buffer = static_cast<byte*>(Z_Malloc(length));
		if(fread(*buffer, 1, length, fp) == length)
		{
			fclose(fp);
			return length;
		}
		fclose(fp);
	}

	/* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
	* because we could have a legit 0-length file. So make it -1. */
	return -1;
}

// Same as above, but add null terminator
int M_ReadFileToString(char const* name, char** buffer)
{
	FILE* fp;

	if((fp = M_OpenFile(name, "rb")))
	{
		size_t length;

		fseek(fp, 0, SEEK_END);
		length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		*buffer = static_cast<char*>(Z_Malloc(length + 1));
		if(fread(*buffer, 1, length, fp) == length)
		{
			fclose(fp);
			(*buffer)[length] = '\0';
			return length;
		}
		Z_Free(*buffer);
		*buffer = nullptr;
		fclose(fp);
	}

	/* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
	* because we could have a legit 0-length file. So make it -1. */
	return -1;
}

char* M_getcwd(char* buffer, int len)
{
#ifdef _WIN32
	wchar_t* wret;
	char* ret;

	wret = _wgetcwd(NULL, 0);

	if(!wret)
		return nullptr;

	ret = ConvertWideToUtf8(wret);

	free(wret);

	if(!ret)
		return nullptr;

	if(!buffer)
		return ret;

	if(strlen(ret) >= len)
	{
		Z_Free(ret);
		return nullptr;
	}

	strcpy(buffer, ret);
	Z_Free(ret);

	return buffer;
#else
	return getcwd(buffer, len);
#endif
}

#ifdef _WIN32
typedef struct
{
	char* var;
	const char* name;
} env_var_t;

static env_var_t* env_vars;
static int num_vars;
#endif

char* M_getenv(const char* name)
{
#ifdef _WIN32
	int i;
	wchar_t *wenv, *wname;
	char* env;

	for(i = 0; i < num_vars; ++i)
	{
		if(!strcasecmp(name, env_vars[i].name))
			return env_vars[i].var;
	}

	wname = ConvertUtf8ToWide(name);

	if(!wname)
		return nullptr;

	wenv = _wgetenv(wname);

	Z_Free(wname);

	if(wenv)
		env = ConvertWideToUtf8(wenv);
	else
		env = nullptr;

	env_vars = static_cast<env_var_t*>(Z_Realloc(env_vars, (num_vars + 1) * sizeof(*env_vars)));
	env_vars[num_vars].var = env;
	env_vars[num_vars].name = Z_Strdup(name);
	num_vars++;

	return env;
#else
	return getenv(name);
#endif
}

dboolean M_RemoveFilesAtPath(const char* path)
{
	glob_t* glob;
	const char* filename;
	dboolean success = true;

	glob = I_StartGlob(path, "*.*", GLOB_FLAG_NOCASE | GLOB_FLAG_SORTED);

	for(;;)
	{
		filename = I_NextGlob(glob);
		if(filename == nullptr)
			break;

		if(M_remove(filename) != 0)
		{
			lprintf(OutputLevels::Error, "M_RemoveFilesAtPath: unable to delete file %s\n", filename);
			success = false;
			break;
		}
	}
	I_EndGlob(glob);
	return success;
}
