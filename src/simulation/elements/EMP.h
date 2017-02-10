#ifndef EMP_H
#define EMP_H

#include "simulation/ElementDataContainer.h"
#include "simulation/Simulation.h"

class EMP_ElementDataContainer : public ElementDataContainer
{
public:
	unsigned int emp_decor;
	unsigned int emp_trigger_count;
	EMP_ElementDataContainer()
	{
		emp_decor = 0;
		emp_trigger_count = 0;
	}

	virtual ElementDataContainer * Clone() { return new EMP_ElementDataContainer(*this); }

	virtual void Simulation_Cleared(Simulation *sim)
	{
		emp_decor = 0;
		emp_trigger_count = 0;
	}

	virtual void Simulation_AfterUpdate(Simulation *sim);

	void Activate()
	{
		emp_trigger_count++;
		emp_decor += 3;
		if (emp_decor > 40)
			emp_decor = 40;
	}

	void Deactivate()
	{
		if (emp_decor)
			emp_decor -= emp_decor/25+2;

		if (emp_decor > 40)
			emp_decor = 40;
	}
};

#endif
