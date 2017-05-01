#include "model/context/simulationunit.h"
#include "model/context/topology.h"

#include "simulationgridimpl.h"

SimulationGridImpl::SimulationGridImpl(QObject * parent)
	: SimulationGrid(parent)
{
}

SimulationGridImpl::~SimulationGridImpl()
{
	deleteUnits();
}

void SimulationGridImpl::init(IntVector2D gridSize, Topology* topology)
{
	deleteUnits();

	if (_topology != topology) {
		delete _topology;
		_topology = topology;
	}
	_gridSize = gridSize;
	for (int x = 0; x < gridSize.x; ++x) {
		_units.push_back(std::vector<SimulationUnit*>(gridSize.y, nullptr));
	}
}

void SimulationGridImpl::registerUnit(IntVector2D gridPos, SimulationUnit * unit)
{
	_units[gridPos.x][gridPos.y] = unit;
}

IntVector2D SimulationGridImpl::getSize() const
{
	return _gridSize;
}

IntRect SimulationGridImpl::calcMapRect(IntVector2D gridPos) const
{
	IntVector2D universeSize = _topology->getSize();
	IntVector2D p1 = { universeSize.x * gridPos.x / _gridSize.x, universeSize.y * gridPos.y / _gridSize.y };
	IntVector2D p2 = { universeSize.x * (gridPos.x + 1) / _gridSize.x - 1, universeSize.y * (gridPos.y + 1) / _gridSize.y - 1 };
	return{ p1, p2 };
}

void SimulationGridImpl::deleteUnits()
{
	for (auto const& unitVec : _units) {
		for (auto const& unit : unitVec) {
			delete unit;
		}
	}
}
