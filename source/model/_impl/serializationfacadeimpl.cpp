#include "serializationfacadeimpl.h"

#include "model/entities/cell.h"
#include "model/entities/cellcluster.h"
#include "model/entities/energyparticle.h"
#include "model/entities/token.h"
#include "model/entities/entityfactory.h"
#include "model/features/cellfunction.h"
#include "model/features/cellfunctioncomputer.h"
#include "model/features/energyguidance.h"
#include "model/features/cellfeaturefactory.h"
#include "model/metadata/symboltable.h"
#include "model/config.h"
#include "model/cellmap.h"
#include "model/energyparticlemap.h"
#include "model/topology.h"
#include "model/_impl/simulationcontextimpl.h"
#include "model/simulationparameters.h"
#include "global/servicelocator.h"
#include "global/numbergenerator.h"

namespace
{
	SerializationFacadeImpl serializationFacadeImpl;
}


SerializationFacadeImpl::SerializationFacadeImpl()
{
    ServiceLocator::getInstance().registerService<SerializationFacade>(this);
}

void SerializationFacadeImpl::serializeSimulationContext(SimulationContext * context, QDataStream & stream) const
{
	context->getTopology()->serializePrimitives(stream);

	auto const& clusters = context->getClustersRef();
	quint32 numCluster = clusters.size();
	stream << numCluster;
	foreach(CellCluster* cluster, clusters) {
		serializeCellCluster(cluster, stream);
	}

	auto const& energyParticles = context->getEnergyParticlesRef();
	quint32 numEnergyParticles = energyParticles.size();
	stream << numEnergyParticles;
	foreach(EnergyParticle* e, energyParticles) {
		serializeEnergyParticle(e, stream);
	}

	context->getCellMap()->serializePrimitives(stream);
	context->getEnergyParticleMap()->serializePrimitives(stream);
	context->getSymbolTable()->serializePrimitives(stream);
	context->getSimulationParameters()->serializePrimitives(stream);
}

void SerializationFacadeImpl::deserializeSimulationContext(SimulationContext* prevContext, QDataStream & stream) const
{
	//mapping old ids to new ids
	QMap< quint64, quint64 > oldNewCellIdMap;
	QMap< quint64, quint64 > oldNewClusterIdMap;

	//mapping old ids to new entities
	QMap< quint64, Cell* > oldIdCellMap;
	QMap< quint64, EnergyParticle* > oldIdEnergyMap;

	//deserialize map size
	prevContext->getTopology()->deserializePrimitives(stream);
	prevContext->initWithoutTopology();

	//deserialize clusters
	quint32 numCluster;
	stream >> numCluster;
	for (quint32 i = 0; i < numCluster; ++i) {
		CellCluster* cluster = deserializeCellCluster(stream, oldNewClusterIdMap, oldNewCellIdMap, oldIdCellMap, prevContext);
		prevContext->getClustersRef() << cluster;
	}

	//deserialize energy particles
	quint32 numEnergyParticles;
	stream >> numEnergyParticles;
	for (quint32 i = 0; i < numEnergyParticles; ++i) {
        EnergyParticle* e = deserializeEnergyParticle(stream, oldIdEnergyMap, prevContext);
		prevContext->getEnergyParticlesRef() << e;
	}

	//deserialize maps
	prevContext->getCellMap()->deserializePrimitives(stream, oldIdCellMap);
	prevContext->getEnergyParticleMap()->deserializePrimitives(stream, oldIdEnergyMap);
	prevContext->getSymbolTable()->deserializePrimitives(stream);
	prevContext->getSimulationParameters()->deserializePrimitives(stream);
}

void SerializationFacadeImpl::serializeSimulationParameters(SimulationParameters* parameters, QDataStream& stream) const
{
	parameters->serializePrimitives(stream);
}

SimulationParameters* SerializationFacadeImpl::deserializeSimulationParameters(QDataStream& stream) const
{
	SimulationParameters* parameters = new SimulationParameters();
	parameters->deserializePrimitives(stream);
	return parameters;
}

void SerializationFacadeImpl::serializeSymbolTable(SymbolTable* symbolTable, QDataStream& stream) const
{
	symbolTable->serializePrimitives(stream);
}

SymbolTable* SerializationFacadeImpl::deserializeSymbolTable(QDataStream& stream) const
{
	SymbolTable* symbolTable = new SymbolTable();
	symbolTable->deserializePrimitives(stream);
	return symbolTable;
}

void SerializationFacadeImpl::serializeCellCluster(CellCluster* cluster, QDataStream& stream) const
{
	cluster->serializePrimitives(stream);
	QList<Cell*>& cells = cluster->getCellsRef();
    stream << static_cast<quint32>(cells.size());
    foreach (Cell* cell, cells) {
		serializeFeaturedCell(cell, stream);
	}
	CellClusterMetadata meta = cluster->getMetadata();
	stream << meta.name;
}

CellCluster* SerializationFacadeImpl::deserializeCellCluster(QDataStream& stream
    , QMap< quint64, quint64 >& oldNewClusterIdMap, QMap< quint64, quint64 >& oldNewCellIdMap
    , QMap< quint64, Cell* >& oldIdCellMap, SimulationContext* context) const
{
    EntityFactory* entityFactory = ServiceLocator::getInstance().getService<EntityFactory>();
    CellCluster* cluster = entityFactory->buildCellCluster(context);
    cluster->deserializePrimitives(stream);

    //read data and reconstructing structures
    QMap< quint64, QList< quint64 > > connectingCells;
    QMap< quint64, Cell* > idCellMap;
    quint32 numCells(0);
    stream >> numCells;

    //assigning new cell ids
    QList< Cell* >& cells = cluster->getCellsRef();
    for (quint32 i = 0; i < numCells; ++i) {
        Cell* cell = deserializeFeaturedCell(stream, connectingCells, context);
        cell->setCluster(cluster);
        cells << cell;
        idCellMap[cell->getId()] = cell;

        quint64 newId = NumberGenerator::getInstance().createNewTag();
        oldNewCellIdMap[cell->getId()] = newId;
        oldIdCellMap[cell->getId()] = cell;
        cell->setId(newId);
    }
    quint64 oldClusterId = cluster->getId();

    //assigning new cluster id
    quint64 id = NumberGenerator::getInstance().createNewTag();
    cluster->setId(id);
    oldNewClusterIdMap[oldClusterId] = id;

    //set cell connections
    QMapIterator< quint64, QList< quint64 > > it(connectingCells);
    while (it.hasNext()) {
        it.next();
        Cell* cell(idCellMap[it.key()]);
        QList< quint64 > cellIdList(it.value());
        int i(0);
        foreach(quint64 cellId, cellIdList) {
            cell->setConnection(i, idCellMap[cellId]);
            ++i;
        }
    }

    cluster->updateTransformationMatrix();
    cluster->updateRelCoordinates();
    cluster->updateAngularMass();

	CellClusterMetadata meta;
	stream >> meta.name;
	cluster->setMetadata(meta);

    return cluster;
}

CellCluster* SerializationFacadeImpl::deserializeCellCluster(QDataStream& stream
    , SimulationContext* context) const
{
    QMap< quint64, quint64 > oldNewClusterIdMap;
    QMap< quint64, quint64 > oldNewCellIdMap;
    QMap< quint64, Cell* > oldIdCellMap;

    return deserializeCellCluster(stream, oldNewClusterIdMap, oldNewCellIdMap, oldIdCellMap, context);
}

void SerializationFacadeImpl::serializeFeaturedCell(Cell* cell, QDataStream& stream) const
{
	cell->serializePrimitives(stream);
	CellFeature* features = cell->getFeatures();
	CellFunction* cellFunction = features->findObject<CellFunction>();
	if (cellFunction) {
		stream << static_cast<quint8>(cellFunction->getType());
		cellFunction->serializePrimitives(stream);
	}

	int numToken = cell->getNumToken();
	for (int i = 0; i < numToken; ++i) {
		serializeToken(cell->getToken(i), stream);
	}

	int numConnections = cell->getNumConnections();
	for (int i = 0; i < numConnections; ++i) {
		stream << cell->getConnection(i)->getId();
	}

	CellMetadata meta = cell->getMetadata();
	stream << meta.color << meta.computerSourcecode << meta.description << meta.name;
}

Cell* SerializationFacadeImpl::deserializeFeaturedCell(QDataStream& stream
	, QMap< quint64, QList< quint64 > >& connectingCells, SimulationContext* context) const
{
	EntityFactory* entityFactory = ServiceLocator::getInstance().getService<EntityFactory>();
	CellFeatureFactory* featureFactory = ServiceLocator::getInstance().getService<CellFeatureFactory>();
	Cell* cell = entityFactory->buildCell(context);
	featureFactory->addEnergyGuidance(cell, context);

	cell->deserializePrimitives(stream);
	quint8 rawType;
	stream >> rawType;
	Enums::CellFunction::Type type = static_cast<Enums::CellFunction::Type>(rawType);
	CellFeature* feature = featureFactory->addCellFunction(cell, type, context);
	feature->deserializePrimitives(stream);

	SimulationParameters* parameters = context->getSimulationParameters();
	for (int i = 0; i < cell->getNumToken(); ++i) {
		Token* token = deserializeToken(stream, context);
		if (i < parameters->cellMaxToken)
			cell->setToken(i, token);
		else
			delete token;
	}

	quint64 id;
	for (int i = 0; i < cell->getNumConnections(); ++i) {
		stream >> id;
		connectingCells[cell->getId()] << id;
	}

	CellMetadata meta;
	stream >> meta.color >> meta.computerSourcecode >> meta.description >> meta.name;
	cell->setMetadata(meta);

	return cell;
}

Cell* SerializationFacadeImpl::deserializeFeaturedCell(QDataStream& stream, SimulationContext* context) const
{
	QMap< quint64, QList< quint64 > > temp;
	Cell* cell = deserializeFeaturedCell(stream, temp, context);
	cell->setId(NumberGenerator::getInstance().createNewTag());
	return cell;
}

void SerializationFacadeImpl::serializeToken(Token* token, QDataStream& stream) const
{
    token->serializePrimitives(stream);
}

Token* SerializationFacadeImpl::deserializeToken(QDataStream& stream, SimulationContext* context) const
{
    EntityFactory* entityFactory = ServiceLocator::getInstance().getService<EntityFactory>();
    Token* token = entityFactory->buildToken(context);
    token->deserializePrimitives(stream);
    return token;
}

void SerializationFacadeImpl::serializeEnergyParticle(EnergyParticle* particle, QDataStream& stream) const
{
	particle->serializePrimitives(stream);
	auto metadata = particle->getMetadata();
	stream << metadata.color;
}

EnergyParticle* SerializationFacadeImpl::deserializeEnergyParticle(QDataStream& stream
    , QMap< quint64, EnergyParticle* >& oldIdEnergyMap, SimulationContext* context) const
{
    EntityFactory* factory = ServiceLocator::getInstance().getService<EntityFactory>();
    EnergyParticle* particle = factory->buildEnergyParticle(context);
    particle->deserializePrimitives(stream);
	EnergyParticleMetadata metadata;
	stream >> metadata.color;
	particle->setMetadata(metadata);
	oldIdEnergyMap[particle->getId()] = particle;
	particle->setId(NumberGenerator::getInstance().createNewTag());
	return particle;
}

EnergyParticle* SerializationFacadeImpl::deserializeEnergyParticle(QDataStream& stream
	, SimulationContext* context) const
{
	QMap< quint64, EnergyParticle* > temp;
	EnergyParticle* particle = deserializeEnergyParticle(stream, temp, context);
	EnergyParticleMetadata metadata;
	stream >> metadata.color;
	particle->setMetadata(metadata);
	particle->setId(NumberGenerator::getInstance().createNewTag());
	return particle;
}