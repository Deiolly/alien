#pragma once

#include "Base.cuh"
#include "Definitions.cuh"
#include "Entities.cuh"
#include "CellFunctionData.cuh"

struct SimulationData
{
    int2 size;

    Map<Cell> cellMap;
    Map<Particle> particleMap;
    CellFunctionData cellFunctionData;

    Entities entities;
    Entities entitiesForCleanup;

    DynamicMemory dynamicMemory;

    CudaNumberGenerator numberGen;

    void init(int2 const& universeSize, CudaConstants const& cudaConstants)
    {
        size = universeSize;

        entities.init(cudaConstants);
        entitiesForCleanup.init(cudaConstants);
        cellFunctionData.init(universeSize);
        cellMap.init(size, cudaConstants.MAX_CELLPOINTERS);
        particleMap.init(size, cudaConstants.MAX_PARTICLEPOINTERS);
        dynamicMemory.init(cudaConstants.DYNAMIC_MEMORY_SIZE);
        numberGen.init(40312357);
    }

    void free()
    {
        entities.free();
        entitiesForCleanup.free();
        cellFunctionData.free();
        cellMap.free();
        particleMap.free();
        numberGen.free();
        dynamicMemory.free();
    }
};

