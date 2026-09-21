// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA zipfile support using libzip

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

const char* dsda_UnzipFile(const char* zipped_file_name);
const char* dsda_ReadUnzippedFile(const char* zipped_file_name);

void dsda_CleanZipTempDirs();

#ifdef __cplusplus
}
#endif
