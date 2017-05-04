#ifndef SIMULATIONCONTROLLER_H
#define SIMULATIONCONTROLLER_H

#include "Definitions.h"

class SimulationController
	: public QObject
{
    Q_OBJECT
public:
	SimulationController(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~SimulationController() = default;

    virtual void init (SimulationContext* context) = 0;
    Q_SLOT virtual void setRun (bool run) = 0;

private:
    SimulationContext* _context = nullptr;
};

#endif // SIMULATIONCONTROLLER_H
