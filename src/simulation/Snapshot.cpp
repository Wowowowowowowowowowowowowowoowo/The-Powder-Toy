
#include "Snapshot.h"
#include "common/tpt-compat.h"
#include "elements/STKM.h"
#include "elements/PRTI.h"
#include "game/Authors.h"
#include "game/Sign.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"

unsigned int Snapshot::undoHistoryLimit = 5;
unsigned int Snapshot::historyPosition = 0;
std::deque<std::unique_ptr<Snapshot>> Snapshot::history = std::deque<std::unique_ptr<Snapshot>>();
std::unique_ptr<Snapshot> Snapshot::beforeRestore = nullptr;

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
	std::unique_ptr<Snapshot> snap = CreateSnapshot(sim);
	if (!snap)
		return;
	while (historyPosition < history.size())
	{
		history.pop_back();
	}
	if (history.size() >= undoHistoryLimit)
	{
		history.pop_front();
		if (historyPosition > history.size())
			historyPosition--;
	}
	history.push_back(std::move(snap));
	historyPosition = std::min((size_t)historyPosition+1, history.size());
}

void Snapshot::RestoreSnapshot(Simulation * sim)
{
	if (!history.size())
		return;
	// When undoing, save the current state as a final redo
	// This way ctrl+y will always bring you back to the point right before your last ctrl+z
	if (historyPosition == history.size())
	{
		std::unique_ptr<Snapshot> newSnap = CreateSnapshot(sim);
		beforeRestore.swap(newSnap);
	}
	auto &snap = *history[std::max((int)historyPosition-1, 0)];
	Restore(sim, snap);
	historyPosition = std::max((int)historyPosition-1, 0);
}

void Snapshot::RestoreRedoSnapshot(Simulation *sim)
{
	if (historyPosition >= history.size())
		return;

	unsigned int newHistoryPosition = std::min((size_t)historyPosition+1, history.size());

	auto &current = newHistoryPosition == history.size() ? *beforeRestore : *history[newHistoryPosition];
	Restore(sim, current);
	historyPosition = newHistoryPosition;
	// Block continuous redoing to the same snapshot
	if (&current == beforeRestore.get())
	{
		beforeRestore.reset();
	}
}

std::unique_ptr<Snapshot> Snapshot::CreateSnapshot(Simulation * sim)
{
	auto snap = std::make_unique<Snapshot>();
	snap->AirPressure  .insert(snap->AirPressure.begin(),   &sim->air->pv[0][0],    &sim->air->pv   [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->AirVelocityX .insert(snap->AirVelocityX.begin(),  &sim->air->vx[0][0],    &sim->air->vx   [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->AirVelocityY .insert(snap->AirVelocityY.begin(),  &sim->air->vy[0][0],    &sim->air->vy   [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->AmbientHeat  .insert(snap->AmbientHeat.begin(),   &sim->air->hv[0][0],    &sim->air->hv   [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->GravVelocityX.insert(snap->GravVelocityX.begin(), &sim->grav->gravx[0],   &sim->grav->gravx  [0] + ((XRES / CELL) * (YRES / CELL)));
	snap->GravVelocityY.insert(snap->GravVelocityY.begin(), &sim->grav->gravy[0],   &sim->grav->gravy  [0] + ((XRES / CELL) * (YRES / CELL)));
	snap->GravValue    .insert(snap->GravValue.begin(),     &sim->grav->gravp[0],   &sim->grav->gravp  [0] + ((XRES / CELL) * (YRES / CELL)));
	snap->GravMap      .insert(snap->GravMap.begin(),       &sim->grav->gravmap[0], &sim->grav->gravmap[0] + ((XRES / CELL) * (YRES / CELL)));
	snap->BlockMap     .insert(snap->BlockMap.begin(),      &bmap[0][0],            &bmap           [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->ElecMap      .insert(snap->ElecMap.begin(),       &emap[0][0],            &emap           [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->FanVelocityX .insert(snap->FanVelocityX.begin(),  &sim->air->fvx[0][0],   &sim->air->fvx  [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->FanVelocityY .insert(snap->FanVelocityY.begin(),  &sim->air->fvy[0][0],   &sim->air->fvy  [0][0] + ((XRES / CELL) * (YRES / CELL)));
	snap->Particles    .insert(snap->Particles.begin(),     &parts[0],              &parts[sim->parts_lastActiveIndex + 1]);
	for (std::vector<Sign*>::iterator iter = signs.begin(), end = signs.end(); iter != end; ++iter)
		snap->Signs.push_back(new Sign(**iter));
	snap->Authors = authors;

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
	for (int i = 0; i < NPART; i++)
		parts[i].type = 0;

	std::copy(snap.AirPressure .begin(), snap.AirPressure .end(), &sim->air->pv [0][0]);
	std::copy(snap.AirVelocityX.begin(), snap.AirVelocityX.end(), &sim->air->vx [0][0]);
	std::copy(snap.AirVelocityY.begin(), snap.AirVelocityY.end(), &sim->air->vy [0][0]);
	std::copy(snap.AmbientHeat .begin(), snap.AmbientHeat .end(), &sim->air->hv [0][0]);
	std::copy(snap.BlockMap    .begin(), snap.BlockMap    .end(), &bmap         [0][0]);
	std::copy(snap.ElecMap     .begin(), snap.ElecMap     .end(), &emap         [0][0]);
	std::copy(snap.FanVelocityX.begin(), snap.FanVelocityX.end(), &sim->air->fvx[0][0]);
	std::copy(snap.FanVelocityY.begin(), snap.FanVelocityY.end(), &sim->air->fvy[0][0]);
	std::copy(snap.Particles   .begin(), snap.Particles   .end(), &parts        [0]);

	if (sim->grav->IsEnabled())
	{
		std::copy(snap.GravVelocityX.begin(), snap.GravVelocityX.end(), &sim->grav->gravx  [0]);
		std::copy(snap.GravVelocityY.begin(), snap.GravVelocityY.end(), &sim->grav->gravy  [0]);
		std::copy(snap.GravValue    .begin(), snap.GravValue    .end(), &sim->grav->gravp  [0]);
		std::copy(snap.GravMap      .begin(), snap.GravMap      .end(), &sim->grav->gravmap[0]);
	}

	ClearSigns();
	for (std::vector<Sign*>::const_iterator iter = snap.Signs.begin(), end = snap.Signs.end(); iter != end; ++iter)
		signs.push_back(new Sign(**iter));
	authors = snap.Authors;

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

	sim->air->RecalculateBlockAirMaps(sim);
	sim->parts_lastActiveIndex = NPART - 1;
	sim->RecalcFreeParticles(false);
	sim->forceStackingCheck = true;
	sim->grav->gravWallChanged = true;
	sim->RecountElements();
}
