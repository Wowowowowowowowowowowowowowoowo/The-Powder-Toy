#ifndef SNAPSHOT
#define SNAPSHOT

#include <deque>
#include <memory>
#include <vector>

#include "SimulationData.h"
#include "Particle.h"
#include "common/tpt-minmax.h"
#include "game/Sign.h"
#include "json/json.h"

class ElementDataContainer;
class Simulation;
class Snapshot
{
public:
	std::vector<float> AirPressure;
	std::vector<float> AirVelocityX;
	std::vector<float> AirVelocityY;
	std::vector<float> AmbientHeat;

	std::vector<particle> Particles;

	ElementDataContainer *elementData[PT_NUM];

	std::vector<float> GravVelocityX;
	std::vector<float> GravVelocityY;
	std::vector<float> GravValue;
	std::vector<float> GravMap;

	std::vector<unsigned char> BlockMap;
	std::vector<unsigned char> ElecMap;

	std::vector<float> FanVelocityX;
	std::vector<float> FanVelocityY;

	std::vector<Sign*> Signs;

	Json::Value Authors;

	Snapshot() :
		AirPressure(),
		AirVelocityX(),
		AirVelocityY(),
		AmbientHeat(),
		Particles(),
		GravVelocityX(),
		GravVelocityY(),
		GravValue(),
		GravMap(),
		BlockMap(),
		ElecMap(),
		FanVelocityX(),
		FanVelocityY(),
		Signs()
	{

	}

	~Snapshot();

	// manage snapshots list
	static void TakeSnapshot(Simulation * sim);
	static void RestoreSnapshot(Simulation * sim);
	static void RestoreRedoSnapshot(Simulation *sim);

	static void SetUndoHistoryLimit(unsigned int newLimit) { undoHistoryLimit = std::min(newLimit, (unsigned int)200); }
	static unsigned int GetUndoHistoryLimit() { return undoHistoryLimit; }

private:
	static unsigned int undoHistoryLimit;
	static unsigned int historyPosition;
	static std::deque<std::unique_ptr<Snapshot>> history;
	static std::unique_ptr<Snapshot> beforeRestore;

	// actual creation / restoration of snapshots
	static std::unique_ptr<Snapshot> CreateSnapshot(Simulation * sim);
	static void Restore(Simulation * sim, const Snapshot &snap);
};

#endif // SNAPSHOT
