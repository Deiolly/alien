﻿#pragma once

#include "EngineInterface/GpuSettings.h"
#include "EngineInterface/ShallowUpdateSelectionData.h"

#include "Base.cuh"
#include "Definitions.cuh"
#include "DataAccessKernels.cuh"
#include "Macros.cuh"
#include "GarbageCollectorKernelsLauncher.cuh"

class DataAccessKernelsLauncher
{
public:
    void getData(GpuSettings const& gpuSettings, SimulationData const& data, int2 const& rectUpperLeft, int2 const& rectLowerRight, DataAccessTO const& dataTO);
    void getSelectedData(GpuSettings const& gpuSettings, SimulationData const& data, bool includeClusters, DataAccessTO const& dataTO);
    void getInspectedData(GpuSettings const& gpuSettings, SimulationData const& data, InspectedEntityIds entityIds, DataAccessTO const& dataTO);

    void addData(GpuSettings const& gpuSettings, SimulationData data, DataAccessTO dataTO, bool selectData);
    void clearData(GpuSettings const& gpuSettings, SimulationData data);

private:
    GarbageCollectorKernelsLauncher _garbageCollector;
};
