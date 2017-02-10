#ifndef SNAPSHOT
#define SNAPSHOT

#include <vector>

#include "ElementNumbers.h"
#include "Particle.h"
#include "game/Sign.h"

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
	static void ClearSnapshots();

private:
	// actual creation / restoration of snapshots
	static Snapshot * CreateSnapshot(Simulation * sim);
	static void Restore(Simulation * sim, const Snapshot &snap);
};

extern std::vector<Snapshot*> snapshots;

#endif // SNAPSHOT
