#ifndef ETRD_H
#define ETRD_H

#include <algorithm>
#include <vector>
#include "common/Point.h"
#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"


class ETRD_deltaWithLength
{
public:
	ETRD_deltaWithLength(Point a, int b):
		d(a),
		length(b)
	{

	}

	Point d;
	int length;
};


class ETRD_ElementDataContainer : public ElementDataContainer
{
public:
	std::vector<ETRD_deltaWithLength> deltaPos;
	static const int maxLength = 12;
	bool isValid;
	int countLife0;

	ETRD_ElementDataContainer()
	{
		invalidate();
		initDeltaPos();
	}

	virtual ElementDataContainer * Clone() { return new ETRD_ElementDataContainer(*this); }

	void invalidate()
	{
		isValid = false;
		countLife0 = 0;
	}

	virtual void Simulation_Cleared(Simulation *sim)
	{
		invalidate();
	}

	virtual void Simulation_BeforeUpdate(Simulation *sim)
	{
		invalidate();
	}

	virtual void Simulation_AfterUpdate(Simulation *sim)
	{
		invalidate();
	}

private:
	static bool compareFunc(const ETRD_deltaWithLength &a, const ETRD_deltaWithLength &b)
	{
		return a.length < b.length;
	}

	void initDeltaPos()
	{
		deltaPos.clear();
		for (int ry = -maxLength; ry <= maxLength; ry++)
			for (int rx = -maxLength; rx <= maxLength; rx++)
			{
				Point d(rx, ry);
				if (std::abs(d.X) + std::abs(d.Y) <= maxLength)
					deltaPos.push_back(ETRD_deltaWithLength(d, std::abs(d.X) + std::abs(d.Y)));
			}
		std::stable_sort(deltaPos.begin(), deltaPos.end(), &ETRD_ElementDataContainer::compareFunc);
	}
};

int nearestSparkablePart(Simulation *sim, int targetId);

#endif
