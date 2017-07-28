#pragma once

#include "Model/Entities/Descriptions.h"

#include "Gui/Definitions.h"

class VisualDescription
	: public QObject
{
	Q_OBJECT
public:
	VisualDescription(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~VisualDescription() = default;

	virtual DataDescription& getDataRef();
	virtual CellDescription& getCellDescRef(uint64_t cellId);
	virtual void setData(DataDescription const &data);
	virtual void setSelection(set<uint64_t> const &cellIds, set<uint64_t> const &particleIds);
	virtual bool isInSelection(uint64_t id) const;
	virtual bool isInExtendedSelection(uint64_t id) const;

	virtual void moveSelection(QVector2D const &delta);

private:
	virtual void updateInternals();

	DataDescription _data;

	set<uint64_t> _selectedCellIds;
	set<uint64_t> _selectedClusterIds;
	set<uint64_t> _selectedParticleIds;

	map<uint64_t, uint64_t> _clusterIdsByCellIds;
	map<uint64_t, int> _clusterIndicesByCellIds;
	map<uint64_t, int> _cellIndicesByCellIds;
};
