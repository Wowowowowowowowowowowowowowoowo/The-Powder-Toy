/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "simulation/ElementsCommon.h"

int LSNS_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt, rd = parts[i].tmp2;
	if (rd > 25) parts[i].tmp2 = rd = 25;
	if (parts[i].life)
	{
		parts[i].life = 0;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (parts_avg(i,ID(r),PT_INSL) != PT_INSL)
					{
						if ((sim->elements[rt].Properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[ID(r)].life==0)
						{
							parts[ID(r)].life = 4;
							parts[ID(r)].ctype = rt;
							sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
						}
					}
				}
	}
	for (rx=-rd; rx<rd+1; rx++)
		for (ry=-rd; ry<rd+1; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = photons[y+ry][x+rx];
				if(!r)
					continue;
				if (parts[ID(r)].life > parts[i].temp-273.15)
					parts[i].life = 1;
			}
	return 0;
	
}

void LSNS_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_LSNS";
 	elem->Name = "LSNS";
 	elem->Colour = PIXPACK(0x336699);
 	elem->MenuVisible = 1;
 	elem->MenuSection = SC_SENSOR;
 	elem->Enabled = 1;

 	elem->Advection = 0.0f;
 	elem->AirDrag = 0.00f * CFDS;
 	elem->AirLoss = 0.96f;
 	elem->Loss = 0.00f;
 	elem->Collision = 0.0f;
 	elem->Gravity = 0.0f;
 	elem->Diffusion = 0.00f;
 	elem->HotAir = 0.000f * CFDS;
 	elem->Falldown = 0;

 	elem->Flammable = 0;
 	elem->Explosive = 0;
 	elem->Meltable = 0;
 	elem->Hardness = 1;

 	elem->Weight = 100;

 	elem->DefaultProperties.temp = 4.0f + 273.15f;
 	elem->HeatConduct = 0;
 	elem->Description = "Life sensor, creates a spark when there's a nearby particle with a life higher than its temperature.";

 	elem->Properties = TYPE_SOLID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->DefaultProperties.tmp2 = 2;

 	elem->Update = &LSNS_update;
}
