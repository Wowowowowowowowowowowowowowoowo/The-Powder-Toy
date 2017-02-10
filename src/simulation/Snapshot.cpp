
#include "Snapshot.h"
#include "gravity.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"
#include "elements/STKM.h"
#include "elements/PRTI.h"
#include "game/Sign.h"

std::vector<Snapshot*> snapshots = std::vector<Snapshot*>();

Snapshot::~Snapshot()
{
	for (int i = 0; i < PT_NUM; i++)
		if (elementData[i])
			delete elementData[i];
	for (std::vector<Sign*>::iterator iter = Signs.begin(), end = Signs.end(); iter != end; ++iter)
		delete *iter;
}

void Snapshot::TakeSnapshot(Simulation * sim)
{
	while (snapshots.size())
	{
		delete snapshots.back();
		snapshots.pop_back();
	}
	snapshots.push_back(CreateSnapshot(sim));
}

void Snapshot::RestoreSnapshot(Simulation * sim)
{
	if (!snapshots.size())
		return;
	Restore(sim, *snapshots[0]);
}

void Snapshot::ClearSnapshots()
{
	while (snapshots.size())
	{
		delete snapshots.back();
		snapshots.pop_back();
	}
}

Snapshot * Snapshot::CreateSnapshot(Simulation * sim)
{
	Snapshot * snap = new Snapshot();
	snap->AirPressure.insert(snap->AirPressure.begin(), &pv[0][0], &pv[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AirVelocityX.insert(snap->AirVelocityX.begin(), &vx[0][0], &vx[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AirVelocityY.insert(snap->AirVelocityY.begin(), &vy[0][0], &vy[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->AmbientHeat.insert(snap->AmbientHeat.begin(), &hv[0][0], &hv[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->Particles.insert(snap->Particles.begin(), parts, parts+sim->parts_lastActiveIndex+1);
	snap->GravVelocityX.insert(snap->GravVelocityX.begin(), gravx, gravx+((XRES/CELL)*(YRES/CELL)));
	snap->GravVelocityY.insert(snap->GravVelocityY.begin(), gravy, gravy+((XRES/CELL)*(YRES/CELL)));
	snap->GravValue.insert(snap->GravValue.begin(), gravp, gravp+((XRES/CELL)*(YRES/CELL)));
	snap->GravMap.insert(snap->GravMap.begin(), gravmap, gravmap+((XRES/CELL)*(YRES/CELL)));
	snap->BlockMap.insert(snap->BlockMap.begin(), &bmap[0][0], &bmap[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->ElecMap.insert(snap->ElecMap.begin(), &emap[0][0], &emap[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->FanVelocityX.insert(snap->FanVelocityX.begin(), &fvx[0][0], &fvx[0][0]+((XRES/CELL)*(YRES/CELL)));
	snap->FanVelocityY.insert(snap->FanVelocityY.begin(), &fvy[0][0], &fvy[0][0]+((XRES/CELL)*(YRES/CELL)));
	for (std::vector<Sign*>::iterator iter = signs.begin(), end = signs.end(); iter != end; ++iter)
		snap->Signs.push_back(new Sign(*(Sign*)*iter));

	sim->RecountElements();
	for (int i = 0; i < PT_NUM; i++)
	{
		if (sim->elementData[i] && sim->elementCount[i])
			snap->elementData[i] = sim->elementData[i]->Clone();
		else
			snap->elementData[i] = NULL;
	}
	return snap;
}

void Snapshot::Restore(Simulation * sim, const Snapshot &snap)
{
	std::copy(snap.AirPressure.begin(), snap.AirPressure.end(), &pv[0][0]);
	std::copy(snap.AirVelocityX.begin(), snap.AirVelocityX.end(), &vx[0][0]);
	std::copy(snap.AirVelocityY.begin(), snap.AirVelocityY.end(), &vy[0][0]);
	std::copy(snap.AmbientHeat.begin(), snap.AmbientHeat.end(), &hv[0][0]);
	for (int i = 0; i < NPART; i++)
		parts[i].type = 0;
	std::copy(snap.Particles.begin(), snap.Particles.end(), parts);
	sim->parts_lastActiveIndex = NPART-1;
	sim->RecalcFreeParticles();
	if (ngrav_enable)
	{
		std::copy(snap.GravVelocityX.begin(), snap.GravVelocityX.end(), gravx);
		std::copy(snap.GravVelocityY.begin(), snap.GravVelocityY.end(), gravy);
		std::copy(snap.GravValue.begin(), snap.GravValue.end(), gravp);
		std::copy(snap.GravMap.begin(), snap.GravMap.end(), gravmap);
	}
	std::copy(snap.BlockMap.begin(), snap.BlockMap.end(), &bmap[0][0]);
	std::copy(snap.ElecMap.begin(), snap.ElecMap.end(), &emap[0][0]);
	std::copy(snap.FanVelocityX.begin(), snap.FanVelocityX.end(), &fvx[0][0]);
	std::copy(snap.FanVelocityY.begin(), snap.FanVelocityY.end(), &fvy[0][0]);
	ClearSigns();
	for (std::vector<Sign*>::const_iterator iter = snap.Signs.begin(), end = snap.Signs.end(); iter != end; ++iter)
		signs.push_back(new Sign(*(Sign*)*iter));

	for (int i = 0; i < PT_NUM; i++)
	{
		if (snap.elementData[i])
		{
			if (sim->elementData[i])
			{
				delete sim->elementData[i];
				sim->elementData[i] = NULL;
			}
			sim->elementData[i] = snap.elementData[i]->Clone();
		}
	}

	sim->parts_lastActiveIndex = NPART-1;
	sim->forceStackingCheck = true;
	sim->RecountElements();
}
