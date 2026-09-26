// SPDX-License-Identifier: GPL-2.0-or-later

// DESCRIPTION:
//	DSDA Aim

#include "p_map.hpp"
#include "tables.hpp"

#include "dsda/excmd.hpp"

#include "aim.hpp"

angle_t dsda_PlayerPitch(player_t* player)
{
	return dsda_FreeAim() ? player->mo->pitch : -(angle_t)(player->lookdir * ANG1 / M_PI);
}

fixed_t dsda_PlayerSlope(player_t* player)
{
	return dsda_FreeAim() ? finetangent[(ANG90 - player->mo->pitch) >> ANGLETOFINESHIFT] : raven ? ((player->lookdir) << FRACBITS) / 173 : 0;
}

int dsda_PitchToLookDir(angle_t pitch)
{
	return -(int)((((long long)(int)pitch * FIXED_PI) >> FRACBITS) / ANG1);
}

angle_t dsda_LookDirToPitch(int lookdir)
{
	return (angle_t)-FixedDiv(lookdir * ANG1, FIXED_PI);
}

int dsda_PlayerLookDir(player_t* player)
{
	return dsda_FreeAim() ? dsda_PitchToLookDir(player->mo->pitch) : player->lookdir;
}

void dsda_PlayerAim(mobj_t* source, angle_t angle, aim_t* aim, MobjFlag target_mask)
{
	aim->angle = angle;

	if(dsda_FreeAim())
	{
		aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
		aim->z_offset = 0;
	}
	else
	{
		do
		{
			aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

			if(!linetarget)
			{
				aim->angle += 1 << 26;
				aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
			}

			if(!linetarget)
			{
				aim->angle -= 2 << 26;
				aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
			}

			if(!linetarget)
			{
				aim->angle = angle;
				aim->slope = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
			}
		}
		while(target_mask != MobjFlag{} && (target_mask = MobjFlag{}, !linetarget)); // killough 8/2/98

		aim->z_offset = raven ? ((source->player->lookdir) << FRACBITS) / 173 : 0;
	}
}

void dsda_PlayerAimBad(mobj_t* source, angle_t angle, aim_t* aim, MobjFlag target_mask)
{
	aim->angle = angle;
	aim->z_offset = 0;

	if(dsda_FreeAim())
	{
		aim->slope = finetangent[(ANG90 - source->pitch) >> ANGLETOFINESHIFT];
		aim->z_offset = 0;
	}
	else
	{
		do
		{
			aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);

			if(!linetarget)
			{
				aim->angle += 1 << 26;
				aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
			}

			if(!linetarget)
			{
				aim->angle -= 2 << 26;
				aim->slope = P_AimLineAttack(source, aim->angle, 16 * 64 * FRACUNIT, target_mask);
			}

			if(heretic && !linetarget)
				aim->slope = ((source->player->lookdir) << FRACBITS) / 173;
		}
		while(target_mask != MobjFlag{} && (target_mask = MobjFlag{}, !linetarget)); // killough 8/2/98
	}
}
