// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA zipfile support using libzip

#ifndef __DSDA_ZIPFILE__
#define __DSDA_ZIPFILE__

const char* dsda_UnzipFile(const char *zipped_file_name);
const char* dsda_ReadUnzippedFile(const char *zipped_file_name);

void dsda_CleanZipTempDirs(void);

#endif /* __DSDA_ZIPFILE__ */
