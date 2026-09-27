// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Compatibility

#include <utility>

#include "md5.hpp"
#include "doomdata.hpp"
#include "doomstat.hpp"
#include "doomtype.hpp"
#include "w_wad.hpp"
#include "lprintf.hpp"

#include "dsda/utility.hpp"

#include "compatibility.hpp"

typedef struct
{
	const char* cksum_string;
	const CompOption options[];
} dsda_compatibility_t;

static const dsda_compatibility_t eternal_doom_25 = {
	"6da6fcba8089161bdec6a1d3f6c8d60f",
	{CompOption::Stairs, CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t doomsday_of_uac_e1m8 = {
	"32fc3115a3162b623f0d0f4e7dee6861",
	{CompOption::Value666, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t hell_revealed_map19 = {
	"811a0c97777a198bc9b2bb558cb46e6a",
	{CompOption::Pain, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t roger_ritenour_phobos_map03 = {
	"8fa29398776146189396aa1ac6bb9e13",
	{CompOption::Floors, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t hell_revealed_map26 = {
	"145c4dfcf843f2b92c73036ba0e1d98a",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t hell_to_pay_map14 = {
	"5379c080299eb961792b50ad96821543",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t hell_to_pay_map22 = {
	"7837b5334a277f107515d649bcefb682",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t icarus_map24 = {
	"2eeb1e12fa9f9545de9d99990a4a78e5",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t plutonia2_map32 = {
	"65a53a09a09525ae42ea210bf879cd37",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t requiem_map23 = {
	"2499cf9a9351be9bc4e9c66fc9f291a7",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t the_waterfront_map01 = {
	"3ca5493feff2e27bfd4181e6c4a3c2bf",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t gather2_map05_and_darkside_map01 = {
	"cbdfefac579a62de8f1b48ca4a09d381",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t reverie_map18 = {
	"c7a2fafb0afb2632c50ad625cdb50e51",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t project_x_map14 = {
	"9e5724bc6135aa6f86ee54fd4d91f1e2",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t archie_map01 = {
	"01899825ffeae016d39c02a7da4b218f",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t seej_map01 = {
	"1d9f3afdc2517c2e450491ed13896712",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t sixpack2_map02 = {
	"0ae745a3ab86d15fb2fb74489962c421",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t squadron_417_map21 = {
	"2ea635c6b6aec76b6bc77448dab22f9a",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t mayhem_2013_map05 = {
	"1e998262ee319b7d088e01de782e6b41",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t imps_are_ghost_gods_map01 = {
	"a81e2734f735a82720d8e0f1442ba0c9",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t confinement_map31 = {
	"aad7502cb39bc050445e17b15f72356f",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t conf256b_map07 = {
	"5592ea1ca3b8ee0dbb2cb352aaa00911",
	{CompOption::Pain, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t conf256b_map12 = {
	"cecedae33b970f2bf7f8b8631da0c8dd",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t sunlust_map30 = {
	"41efe03223e41935849f64114c5cb471",
	{CompOption::Telefrag, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t tnt_map30 = {
	"42b68b84ff8e55f264c31e6f4cfea82d",
	{CompOption::Stairs, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t intercep2_map03 = {
	"86587e4f8c8086991c8fc5c1ccfd30b9",
	{CompOption::End, CompOption::LedgeBlock, CompOption::End}
};

static const dsda_compatibility_t skulltiverse_map02 = {
	"b3fa4a18b31bd96e724f9aab101776a1",
	{CompOption::End, CompOption::LedgeBlock, CompOption::End}
};

static const dsda_compatibility_t tntr_map30 = {
	"1d3c6d456bfcf360ce14aeecc155a96c",
	{CompOption::Telefrag, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t roomblow_e1m1 = {
	"68ffa69f2eaa5ced3dc4da5a300d022a",
	{CompOption::Stairs, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t esp_map21 = {
	"97088f2849904bc1cd5ae1d92d163b13",
	{CompOption::Stairs, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t av_map07 = {
	"941e4cb56ee4184e0b1ed43486ab0bbf",
	{CompOption::Model, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t sin2_9_map02 = {
	"9aa5aa3020434f824624eba88916ee23",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t d2reload_map09 = {
	"c8de798a4d658ffc94151884c6c2bf37",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t amoreupho_map02 = {
	"66a8310a0a7d2af99e3a0089b2d6c897",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t dbp20_dnd_map07 = {
	"e26c1b6f4dfd90bb6533e6381bf61be5",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t arch_map01 = {
	"1d37cbd32a1ecf4763437631e7b3c29a",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t ur_map06 = {
	"cfb054683af1ed187d0565942d3dbb8f",
	{CompOption::Vile, CompOption::End, CompOption::End}
};

static const dsda_compatibility_t* entry_0[] = {
	&archie_map01,
	&sixpack2_map02,
	nullptr
};

static const dsda_compatibility_t* entry_1[] = {
	&hell_revealed_map26,
	&seej_map01,
	&mayhem_2013_map05,
	&tntr_map30,
	&arch_map01,
	nullptr
};

static const dsda_compatibility_t* entry_2[] = {
	&icarus_map24,
	&requiem_map23,
	&squadron_417_map21,
	nullptr
};

static const dsda_compatibility_t* entry_3[] = {
	&doomsday_of_uac_e1m8,
	&the_waterfront_map01,
	nullptr
};

static const dsda_compatibility_t* entry_4[] = {
	&sunlust_map30,
	&tnt_map30,
	nullptr
};

static const dsda_compatibility_t* entry_5[] = {
	&hell_to_pay_map14,
	&conf256b_map07,
	nullptr
};

static const dsda_compatibility_t* entry_6[] = {
	&eternal_doom_25,
	&plutonia2_map32,
	&roomblow_e1m1,
	&amoreupho_map02,
	nullptr
};

static const dsda_compatibility_t* entry_7[] = {
	&hell_to_pay_map22,
	nullptr
};

static const dsda_compatibility_t* entry_8[] = {
	&hell_revealed_map19,
	&roger_ritenour_phobos_map03,
	&intercep2_map03,
	nullptr
};

static const dsda_compatibility_t* entry_9[] = {
	&project_x_map14,
	&esp_map21,
	&av_map07,
	&sin2_9_map02,
	nullptr
};

static const dsda_compatibility_t* entry_a[] = {
	&imps_are_ghost_gods_map01,
	&confinement_map31,
	nullptr
};

static const dsda_compatibility_t* entry_b[] = {
	&skulltiverse_map02,
	nullptr
};

static const dsda_compatibility_t* entry_c[] = {
	&gather2_map05_and_darkside_map01,
	&reverie_map18,
	&conf256b_map12,
	&d2reload_map09,
	&ur_map06,
	nullptr
};

static const dsda_compatibility_t* entry_d[] = {
	nullptr
};

static const dsda_compatibility_t* entry_e[] = {
	&dbp20_dnd_map07,
	nullptr
};

static const dsda_compatibility_t* entry_f[] = {
	nullptr
};

// This is clumsy but I couldn't figure out how to inline everything
static const dsda_compatibility_t** level_compatibilities[16] = {
	entry_0,
	entry_1,
	entry_2,
	entry_3,
	entry_4,
	entry_5,
	entry_6,
	entry_7,
	entry_8,
	entry_9,
	entry_a,
	entry_b,
	entry_c,
	entry_d,
	entry_e,
	entry_f,
};

static void dsda_MD5UpdateLump(int lump, struct MD5Context* md5)
{
	MD5Update(md5, static_cast<const unsigned char *>(W_LumpByNum(lump)), W_LumpLength(lump));
}

static void dsda_GetLevelCheckSum(int lump, dsda_cksum_t* cksum)
{
	struct MD5Context md5;

	MD5Init(&md5);

	dsda_MD5UpdateLump(lump + std::to_underlying(MapLump::Label), &md5);
	dsda_MD5UpdateLump(lump + std::to_underlying(MapLump::Things), &md5);
	dsda_MD5UpdateLump(lump + std::to_underlying(MapLump::Linedefs), &md5);
	dsda_MD5UpdateLump(lump + std::to_underlying(MapLump::Sidedefs), &md5);
	dsda_MD5UpdateLump(lump + std::to_underlying(MapLump::Sectors), &md5);

	// ML_BEHAVIOR when it becomes applicable to comp options

	MD5Final(cksum->bytes, &md5);

	dsda_TranslateCheckSum(cksum);
}

// For casual players that aren't careful about setting complevels,
//   this function will apply comp options to automatically fix some issues
//   that appear when playing wads in mbf21 (since this is the default).
void dsda_ApplyLevelCompatibility(int lump)
{
	unsigned int i;
	dsda_cksum_t cksum;
	const dsda_compatibility_t** level_compatibility;

	if(demorecording || demoplayback || !mbf21) return;

	dsda_GetLevelCheckSum(lump, &cksum);

	Log::Debug("Level checksum: {}\n", std::string_view(cksum.string));

	if(cksum.string[0] >= 'a')
		i = cksum.string[0] - 'a' + 10;
	else
		i = cksum.string[0] - '0';

	level_compatibility = level_compatibilities[i];

	while(*level_compatibility)
	{
		if(!strncmp((*level_compatibility)->cksum_string, cksum.string, 32))
		{
			const CompOption* option;

			for(option = (*level_compatibility)->options; *option != CompOption::End; option++)
			{
				comp[std::to_underlying(*option)] = 1;
				Log::Info("Automatically setting comp option {} on\n", std::to_underlying(*option));
			}

			for(option++; *option != CompOption::End; option++)
			{
				comp[std::to_underlying(*option)] = 0;
				Log::Info("Automatically setting comp option {} off\n", std::to_underlying(*option));
			}

			return;
		}

		level_compatibility++;
	}
}
