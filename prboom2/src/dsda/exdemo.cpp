// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Extended Demo

#include <utility>

#include <string.h>

#include "d_main.hpp"
#include "doomstat.hpp"
#include "g_overflow.hpp"
#include "i_system.hpp"
#include "lprintf.hpp"
#include "m_argv.hpp"
#include "m_file.hpp"
#include "wadtbl.hpp"
#include "z_zone.hpp"
#include "e6y.hpp"

#include "dsda/args.hpp"
#include "dsda/demo.hpp"
#include "dsda/features.hpp"
#include "dsda/playback.hpp"
#include "dsda/utility.hpp"

#include "exdemo.hpp"

typedef struct
{
	const char* name;
	byte* demo;
	byte* footer;
	size_t demo_size;
	size_t footer_size;
	byte features[FEATURE_SLOTS];
	int is_signed;
} exdemo_t;

static exdemo_t exdemo;

#define DEMOEX_PORTNAME_LUMPNAME "PORTNAME"
#define DEMOEX_PARAMS_LUMPNAME "CMDLINE"
#define DEMOEX_FEATURE_LUMPNAME "FEATURES"

static void ForgetExDemo()
{
	if(exdemo.demo)
		Z_Free(exdemo.demo);

	memset(&exdemo, 0, sizeof(exdemo));
}

static const filelump_t* DemoEx_LumpForName(const char* name, const wadinfo_t* header)
{
	int i;
	const filelump_t* lump_info;
	const byte* buffer;

	buffer = (const byte*)header;
	lump_info = (const filelump_t*)(buffer + header->infotableofs);
	for(i = 0; i < header->numlumps; i++, lump_info++)
		if(!strncmp(lump_info->name, name, 8))
			return lump_info;

	return nullptr;
}

static char* DemoEx_LumpAsString(const char* name, const wadinfo_t* header)
{
	char* str;
	const char* lump_data;
	const char* buffer;
	const filelump_t* lump_info;

	lump_info = DemoEx_LumpForName(name, header);
	if(!lump_info || !lump_info->size)
		return nullptr;

	str = static_cast<char*>(Z_Calloc(lump_info->size + 1, 1));

	buffer = (const char*)header;
	lump_data = buffer + lump_info->filepos;
	strncpy(str, lump_data, lump_info->size);

	return str;
}

static void DemoEx_GetParams(const wadinfo_t* header)
{
	char* str;
	char** params;
	int i, p, paramscount;

	str = DemoEx_LumpAsString(DEMOEX_PARAMS_LUMPNAME, header);
	if(!str)
		return;

	M_ParseCmdLine(str, nullptr, nullptr, &paramscount, &i);

	params = static_cast<char**>(Z_Malloc(paramscount * sizeof(char*) + i * sizeof(char) + 1));
	if(params)
	{
		struct
		{
			const char* param;
			WadSource source;
		} files[] = {
			{"-iwad", WadSource::Iwad},
			{"-file", WadSource::Pwad},
			{"-deh", WadSource::Deh},
			{nullptr}
		};

		M_ParseCmdLine(str, params, ((char*)params) + sizeof(char*) * paramscount, &paramscount, &i);

		if(!dsda_Flag(ArgId::Iwad) && !dsda_Flag(ArgId::File))
		{
			for(i = 0; files[i].param; ++i)
			{
				p = M_CheckParmEx(files[i].param, params, paramscount);
				if(p >= 0)
				{
					while(++p != paramscount && *params[p] != '-')
					{
						char* filename;

						if(files[i].source == WadSource::Deh)
							filename = I_FindDeh(params[p]);
						else
							filename = I_FindWad(params[p]);

						if(!filename)
							continue;

						if(files[i].source == WadSource::Iwad)
							AddIWAD(filename);
						else if(files[i].source == WadSource::Pwad)
							dsda_AppendStringArg(ArgId::File, filename);
						else if(files[i].source == WadSource::Deh)
							dsda_AppendStringArg(ArgId::Deh, filename);

						Z_Free(filename);
					}
				}
			}
		}

		if(!dsda_Arg(ArgId::Complevel)->found)
		{
			p = M_CheckParmEx("-complevel", params, paramscount);
			if(p >= 0 && p < (int)paramscount - 1)
				dsda_UpdateStringArg(ArgId::Complevel, params[p + 1]);
		}

		//for recording or playback using "single-player coop" mode
		if(!dsda_Flag(ArgId::SoloNet))
		{
			p = M_CheckParmEx("-solo-net", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::SoloNet, true);
		}

		//for recording or playback using "coop in single-player" mode
		if(!dsda_Flag(ArgId::CoopSpawns))
		{
			p = M_CheckParmEx("-coop_spawns", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::CoopSpawns, true);
		}

		//for recording multiple episodes in one demo
		if(!dsda_Flag(ArgId::ChainEpisodes))
		{
			p = M_CheckParmEx("-chain_episodes", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::ChainEpisodes, true);
		}

		if(!dsda_Flag(ArgId::Emulate))
		{
			p = M_CheckParmEx("-emulate", params, paramscount);
			if(p >= 0 && p < (int)paramscount - 1)
				dsda_UpdateStringArg(ArgId::Emulate, params[p + 1]);
		}

		// for doom 1.2
		if(!dsda_Flag(ArgId::Respawn))
		{
			p = M_CheckParmEx("-respawn", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::Respawn, true);
		}

		// for doom 1.2
		if(!dsda_Flag(ArgId::Fast))
		{
			p = M_CheckParmEx("-fast", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::Fast, true);
		}

		// for doom 1.2
		if(!dsda_Flag(ArgId::Nomonsters))
		{
			p = M_CheckParmEx("-nomonsters", params, paramscount);
			if(p >= 0)
				dsda_UpdateFlag(ArgId::Nomonsters, true);
		}

		p = M_CheckParmEx("-spechit", params, paramscount);
		if(p >= 0 && p < (int)paramscount - 1)
		{
			spechit_baseaddr = atoi(params[p + 1]);
		}

		//overflows
		{
			int overflow_i;
			for(overflow_i = 0; overflow_i < std::to_underlying(OverrunList::Max); overflow_i++)
			{
				OverrunList overflow = (OverrunList)overflow_i;

				size_t mask_size = strlen(overflow_cfgname[std::to_underlying(overflow)]) + 16;
				char* mask = static_cast<char*>(Z_Malloc(mask_size));

				if(mask)
				{
					snprintf(mask, mask_size, "-set %s", overflow_cfgname[std::to_underlying(overflow)]);
					char* pstr = strstr(str, mask);

					if(pstr)
					{
						strcat(mask, " = %d");
						int value;
						if(sscanf(pstr, mask, &value) == 1)
						{
							overflows[std::to_underlying(overflow)].footer = true;
							overflows[std::to_underlying(overflow)].footer_emulate = value;
						}
					}
					Z_Free(mask);
				}
			}
		}

		Z_Free(params);
	}

	Z_Free(str);
}

static void DemoEx_AddParams(wadtbl_t* wadtbl)
{
	dsda_arg_t* arg;
	size_t i;
	char buf[200];

	const char* filename_p;

	dsda_string_t files;
	dsda_string_t iwad;
	dsda_string_t pwads;
	dsda_string_t dehs;

	dsda_InitString(&files, nullptr);
	dsda_InitString(&iwad, nullptr);
	dsda_InitString(&pwads, nullptr);
	dsda_InitString(&dehs, nullptr);

	for(i = 0; i < numwadfiles; i++)
	{
		const char* fileext_p;
		dsda_string_t* item = nullptr;

		filename_p = PathFindFileName(wadfiles[i].name);
		fileext_p = filename_p + strlen(filename_p) - 1;

		while(fileext_p != filename_p && *(fileext_p - 1) != '.')
			fileext_p--;

		if(fileext_p == filename_p)
			continue;

		if(wadfiles[i].src == WadSource::Iwad && !iwad.string && !strcasecmp(fileext_p, "wad"))
			item = &iwad;

		if(wadfiles[i].src == WadSource::Pwad && !strcasecmp(fileext_p, "wad"))
			item = &pwads;

		if(item)
		{
			dsda_StringCat(item, "\"");
			dsda_StringCat(item, filename_p);
			dsda_StringCat(item, "\" ");
		}
	}

	arg = dsda_Arg(ArgId::Deh);
	if(arg->found)
	{
		for(i = 0; i < arg->count; ++i)
		{
			char* file;

			file = I_FindDeh(arg->value.v_string_array[i]);
			if(file)
			{
				filename_p = PathFindFileName(file);
				dsda_StringCat(&dehs, "\"");
				dsda_StringCat(&dehs, filename_p);
				dsda_StringCat(&dehs, "\" ");
				Z_Free(file);
			}
		}
	}

	if(iwad.string)
	{
		dsda_StringCat(&files, "-iwad ");
		dsda_StringCat(&files, iwad.string);
	}

	if(pwads.string)
	{
		dsda_StringCat(&files, "-file ");
		dsda_StringCat(&files, pwads.string);
	}

	if(dehs.string)
	{
		dsda_StringCat(&files, "-deh ");
		dsda_StringCat(&files, dehs.string);
	}

	// add complevel for formats which do not have it in header
	if(demo_compatibility)
	{
		snprintf(buf, sizeof(buf), "-complevel %d ", compatibility_level);
		dsda_StringCat(&files, buf);
	}

	// for recording or playback using "single-player coop" mode
	if(dsda_Flag(ArgId::SoloNet))
	{
		snprintf(buf, sizeof(buf), "-solo-net ");
		dsda_StringCat(&files, buf);
	}

	// for recording or playback using "coop in single-player" mode
	if(dsda_Flag(ArgId::CoopSpawns))
	{
		snprintf(buf, sizeof(buf), "-coop_spawns ");
		dsda_StringCat(&files, buf);
	}

	// for recording multiple episodes in one demo
	if(dsda_Flag(ArgId::ChainEpisodes))
	{
		snprintf(buf, sizeof(buf), "-chain_episodes ");
		dsda_StringCat(&files, buf);
	}

	arg = dsda_Arg(ArgId::Emulate);
	if(arg->found)
	{
		snprintf(buf, sizeof(buf), "-emulate %s", arg->value.v_string);
		dsda_StringCat(&files, buf);
	}

	// doom 1.2 does not store these params in header
	if(compatibility_level == CompLevel::Doom12)
	{
		if(dsda_Flag(ArgId::Respawn))
		{
			snprintf(buf, sizeof(buf), "-respawn ");
			dsda_StringCat(&files, buf);
		}

		if(dsda_Flag(ArgId::Fast))
		{
			snprintf(buf, sizeof(buf), "-fast ");
			dsda_StringCat(&files, buf);
		}

		if(dsda_Flag(ArgId::Nomonsters))
		{
			snprintf(buf, sizeof(buf), "-nomonsters ");
			dsda_StringCat(&files, buf);
		}
	}

	if(spechit_baseaddr != 0 && spechit_baseaddr != DEFAULT_SPECHIT_MAGIC)
	{
		snprintf(buf, sizeof(buf), "-spechit %d ", spechit_baseaddr);
		dsda_StringCat(&files, buf);
	}

	//overflows
	{
		for(int overflow_i = 0; overflow_i < std::to_underlying(OverrunList::Max); overflow_i++)
		{
			OverrunList overflow = (OverrunList)overflow_i;
			if(overflows[std::to_underlying(overflow)].happened)
			{
				snprintf(buf, sizeof(buf), "-set %s=%d ", overflow_cfgname[std::to_underlying(overflow)], overflows[std::to_underlying(overflow)].emulate);
				dsda_StringCat(&files, buf);
			}
		}
	}

	if(files.string)
		AddPWADTableLump(wadtbl, DEMOEX_PARAMS_LUMPNAME,
			(const byte*)files.string, strlen(files.string));

	dsda_FreeString(&files);
	dsda_FreeString(&iwad);
	dsda_FreeString(&pwads);
	dsda_FreeString(&dehs);
}

int dsda_IsExDemoSigned()
{
	return exdemo.is_signed;
}

void dsda_MergeExDemoFeatures()
{
	if(!exdemo.is_signed)
		dsda_TrackFeature(FeatureFlag::Unknown);
	else
	{
		dsda_MergeFeatures(exdemo.features);

		if(exdemo.is_signed == -1)
			dsda_TrackFeature(FeatureFlag::Invalid);
	}
}

static void DemoEx_GetFeatures(const wadinfo_t* header)
{
	char* str;
	char signature[33];
	char ftext[2 * FEATURE_SLOTS + 1];
	int ftext_start = 0;
	int ftext_end = 0;

	exdemo.is_signed = 0;
	for(int f = 0; f < FEATURE_SLOTS; f++)
	{
		exdemo.features[f] = 0;
	}

	str = DemoEx_LumpAsString(DEMOEX_FEATURE_LUMPNAME, header);
	if(!str)
		return;

	if(sscanf(str, "%*[^\n]\n0x%n%[^-]%n-%32s", &ftext_start, ftext, &ftext_end, signature) == 2)
	{
		dsda_cksum_t cksum;
		int ftext_slots = (ftext_end - ftext_start) / 2;
		byte* features;
		int i;

		features = static_cast<byte*>(Z_Calloc(ftext_slots, sizeof(byte)));

		for(i = 0; i < ftext_slots; i++)
		{
			char current_text[3];
			strncpy(current_text, ftext + i * 2, 2);
			current_text[2] = '\0';
			features[ftext_slots - i - 1] = strtol(current_text, nullptr, 16);

			// Add it to the padded features as well
			if(i < FEATURE_SLOTS)
				exdemo.features[FEATURE_SLOTS - i - 1] = features[ftext_slots - i - 1];
		}

		dsda_GetDemoCheckSum(&cksum, features, ftext_slots, exdemo.demo, exdemo.demo_size);

		if(!strcmp(signature, cksum.string))
			exdemo.is_signed = 1;
		else
			exdemo.is_signed = -1;

		Z_Free(features);
	}
	else
		exdemo.is_signed = -1;

	Z_Free(str);
}

static void DemoEx_AddFeatures(wadtbl_t* wadtbl)
{
	dsda_cksum_t cksum;
	char* description;
	char* buffer;
	size_t buffer_length;
	byte* features;
	char current_feature[3];

	dsda_GetDemoRecordingCheckSum(&cksum);
	description = dsda_DescribeFeatures();
	features = dsda_UsedFeatures();

	// (2 + 2 * FEATURE_SLOTS) for the bitmap size in hex + \n + \0 + \- + extra space :^)
	buffer_length = strlen(cksum.string) + strlen(description) + 8 + 2 * FEATURE_SLOTS;
	buffer = static_cast<char*>(Z_Calloc(buffer_length, 1));

	strcpy(buffer, description);
	strcat(buffer, "\n0x");

	for(int f = 0; f < FEATURE_SLOTS; f++)
	{
		snprintf(current_feature, 3, "%02" PRIx8, features[FEATURE_SLOTS - f - 1]);
		strcat(buffer, current_feature);
	}

	strcat(buffer, "-");
	strcat(buffer, cksum.string);

	AddPWADTableLump(wadtbl, DEMOEX_FEATURE_LUMPNAME, (const byte*)buffer, buffer_length);

	Z_Free(buffer);
	Z_Free(description);
}

static void DemoEx_AddPort(wadtbl_t* wadtbl)
{
	AddPWADTableLump(wadtbl, DEMOEX_PORTNAME_LUMPNAME,
		(const byte*)PROJECT_STRING, strlen(PROJECT_STRING));
}

static void PartitionDemo(const char* filename)
{
	size_t file_size;

	file_size = M_ReadFile(filename, &exdemo.demo);

	if(file_size > 0)
	{
		const byte* p;

		p = dsda_DemoMarkerPosition(exdemo.demo, file_size);

		if(p)
		{
			//skip DEMOMARKER
			p++;

			exdemo.demo_size = p - exdemo.demo;

			//seach for the "PWAD" signature after ENDDEMOMARKER
			while(p - exdemo.demo + sizeof(wadinfo_t) < file_size)
			{
				if(!memcmp(p, PWAD_SIGNATURE, strlen(PWAD_SIGNATURE)))
				{
					exdemo.footer = exdemo.demo + (p - exdemo.demo);
					exdemo.footer_size = file_size - (p - exdemo.demo);

					break;
				}
				p++;
			}
		}
		else
		{
			exdemo.demo_size = file_size;
		}
	}
	else
		ForgetExDemo();
}

static void DemoEx_NewLine(wadtbl_t* wadtbl)
{
	const char* const separator = "\n";

	AddPWADTableLump(wadtbl, nullptr, (const byte*)separator, strlen(separator));
}

static void DemoEx_WritePWADTable(wadtbl_t* wadtbl)
{
	dsda_WriteToDemo(&wadtbl->header, sizeof(wadtbl->header));
	dsda_WriteToDemo(wadtbl->data, wadtbl->datasize);
	dsda_WriteToDemo(wadtbl->lumps, wadtbl->header.numlumps * sizeof(wadtbl->lumps[0]));
}

void dsda_WriteExDemoFooter()
{
	wadtbl_t demoex;

	InitPWADTable(&demoex);

	DemoEx_NewLine(&demoex);
	DemoEx_NewLine(&demoex);

	DemoEx_AddFeatures(&demoex);
	DemoEx_NewLine(&demoex);

	DemoEx_AddPort(&demoex);
	DemoEx_NewLine(&demoex);

	DemoEx_AddParams(&demoex);
	DemoEx_NewLine(&demoex);

	DemoEx_WritePWADTable(&demoex);

	FreePWADTable(&demoex);
}

void dsda_LoadExDemo(const char* filename)
{
	PartitionDemo(filename);

	if(exdemo.footer)
	{
		wadinfo_t* header;

		header = ReadPWADTable(exdemo.footer, exdemo.footer_size);

		if(!header)
			lprintf(OutputLevels::Error, "LoadExDemo: demo footer is corrupted\n");
		else
		{
			DemoEx_GetFeatures(header);

			// get needed wads and dehs
			// restore all critical params like -spechit x
			DemoEx_GetParams(header);
		}
	}
}

int dsda_CopyExDemo(const byte** buffer, int* length)
{
	if(exdemo.demo)
	{
		*buffer = exdemo.demo;
		*length = exdemo.demo_size;

		return true;
	}

	return false;
}
