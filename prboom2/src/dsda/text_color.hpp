// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Text Color

#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

enum struct TextColorIndex : int32_t
{
	ExhudTimeLabel,
	ExhudLevelTime,
	ExhudTotalTime,
	ExhudDemoLength,
	ExhudArmorZero,
	ExhudArmorOne,
	ExhudArmorTwo,
	ExhudCommandEntry,
	ExhudCommandQueue,
	ExhudCoordsBase,
	ExhudCoordsMf50,
	ExhudCoordsSr40,
	ExhudCoordsSr50,
	ExhudCoordsFast,
	ExhudFpsBad,
	ExhudFpsFine,
	ExhudHealthBad,
	ExhudHealthWarning,
	ExhudHealthOk,
	ExhudHealthSuper,
	ExhudLineClose,
	ExhudLineFar,
	ExhudLineSpecial,
	ExhudLineNormal,
	ExhudMobjAlive,
	ExhudMobjDead,
	ExhudPlayerDamage,
	ExhudPlayerNeutral,
	ExhudAmmoLabel,
	ExhudAmmoMana1,
	ExhudAmmoMana2,
	ExhudAmmoValue,
	ExhudAmmoBad,
	ExhudAmmoWarning,
	ExhudAmmoOk,
	ExhudAmmoFull,
	ExhudRenderLabel,
	ExhudRenderGood,
	ExhudRenderBad,
	ExhudSectorActive,
	ExhudSectorSpecial,
	ExhudSectorNormal,
	ExhudSpeedLabel,
	ExhudSpeedSlow,
	ExhudSpeedNormal,
	ExhudSpeedFast,
	ExhudTotalsLabel,
	ExhudTotalsValue,
	ExhudTotalsMax,
	ExhudWeaponLabel,
	ExhudWeaponOwned,
	ExhudWeaponBerserk,
	ExhudAttempts,
	ExhudEventSplit,
	ExhudLineActivation,
	ExhudLocalTime,
	ExhudFreeText,
	HudMessage,
	HudSecretMessage,
	MapCoords,
	MapTimeLevel,
	MapTimeTotal,
	MapTitle,
	MapTotalsLabel,
	MapTotalsValue,
	MapTotalsMax,
	InterSplitNormal,
	InterSplitGood,
	InterSplitBest,
	MenuTitle,
	MenuTab,
	MenuTabHighlight,
	MenuLabel,
	MenuLabelHighlight,
	MenuLabelEdit,
	MenuValue,
	MenuValueHighlight,
	MenuValueEdit,
	MenuInfoHighlight,
	MenuInfoEdit,
	MenuWarning,
	MenuScrollbar,
	StbarHealthBad,
	StbarHealthWarning,
	StbarHealthOk,
	StbarHealthSuper,
	StbarArmorZero,
	StbarArmorOne,
	StbarArmorTwo,
	StbarAmmoBad,
	StbarAmmoWarning,
	StbarAmmoOk,
	StbarAmmoFull,
};

void dsda_LoadTextColor();
const char* dsda_TextColor(TextColorIndex i);
ColorRange dsda_TextCR(TextColorIndex i);
int dsda_ColorNameToIndex(const char* name);

#ifdef __cplusplus
}
#endif
