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

int HEAC_update(UPDATE_FUNC_ARGS)
{
	const int rad = 4;
	int rry, rrx, r, count = 0;
	float tempAgg = 0;
	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			rry = ry * rad;
			rrx = rx * rad;
			if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES)
			{
				r = pmap[y+rry][x+rrx];
				if (r && sim->elements[r&0xFF].HeatConduct > 0)
				{
					count++;
					tempAgg += parts[r>>8].temp;
				}
				r = photons[y+rry][x+rrx];
				if (r && sim->elements[r&0xFF].HeatConduct > 0)
				{
					count++;
					tempAgg += parts[r>>8].temp;
				}
			}
		}
	}

	if (count > 0)
	{
		parts[i].temp = tempAgg/count;

		for (int rx = -1; rx <= 1; rx++)
		{
			for (int ry = -1; ry <= 1; ry++)
			{
				rry = ry * rad;
				rrx = rx * rad;
				if (x+rrx >= 0 && x+rrx < XRES && y+rry >= 0 && y+rry < YRES)
				{
					r = pmap[y+rry][x+rrx];
					if (r && sim->elements[r&0xFF].HeatConduct > 0)
					{
						parts[r>>8].temp = parts[i].temp;
					}
					r = photons[y+rry][x+rrx];
					if (r && sim->elements[r&0xFF].HeatConduct > 0)
					{
						parts[r>>8].temp = parts[i].temp;
					}
				}
			}
		}
	}

	return 0;
}

void HEAC_init_element(ELEMENT_INIT_FUNC_ARGS)
{
	elem->Identifier = "DEFAULT_PT_HEAC";
	elem->Name = "HEAC";
	elem->Colour = PIXPACK(0xCB6351);
	elem->MenuVisible = 1;
	elem->MenuSection = SC_SPECIAL;
	elem->Enabled = 1;

	elem->Advection = 0.0f;
	elem->AirDrag = 0.00f * CFDS;
	elem->AirLoss = 0.90f;
	elem->Loss = 0.00f;
	elem->Collision = 0.0f;
	elem->Gravity = 0.0f;
	elem->Diffusion = 0.00f;
	elem->HotAir = 0.000f	* CFDS;
	elem->Falldown = 0;

	elem->Flammable = 0;
	elem->Explosive = 0;
	elem->Meltable = 1;
	elem->Hardness = 50;

	elem->Weight = 100;

	elem->DefaultProperties.temp = R_TEMP+273.15f;
	elem->HeatConduct = 251;
	elem->Description = "Rapid heat conductor.";

	elem->Properties = TYPE_SOLID;

	elem->LowPressureTransitionThreshold = IPL;
	elem->LowPressureTransitionElement = NT;
	elem->HighPressureTransitionThreshold = IPH;
	elem->HighPressureTransitionElement = NT;
	elem->LowTemperatureTransitionThreshold = ITL;
	elem->LowTemperatureTransitionElement = NT;
	elem->HighTemperatureTransitionThreshold = ITH;
	elem->HighTemperatureTransitionElement = NT;

	elem->Update = &HEAC_update;
}
