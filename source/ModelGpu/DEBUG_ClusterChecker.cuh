#pragma once

#include "Cluster.cuh"
#include "Token.cuh"
#include "ConstantMemory.cuh"

#define STOP(a, b) printf("parameter: %d, %d\n", a, b); while(true) {};

class DEBUG_ClusterChecker
{
private:
    template<typename T>
    __inline__ __device__ static bool checkPointer(T* pointer, Array<T> array)
    {
        return array.getEntireArray() <= pointer && pointer < (array.getEntireArray() + array.getNumEntries());
    }

public:
    __inline__ __device__ static void check_blockCall(SimulationData* data, Cluster* cluster, int a, int b = -1)
    {
        if (0 == threadIdx.x) {
            if (!checkPointer(cluster, data->entities.clusters)) {
                printf("wrong cluster pointer\n");
                STOP(a, b);
            }
        }
        __syncthreads();
        auto const cellBlock = calcPartition(cluster->numCellPointers, threadIdx.x, blockDim.x);
        for (int cellIndex = cellBlock.startIndex; cellIndex <= cellBlock.endIndex; ++cellIndex) {
            auto const& cell = cluster->cellPointers[cellIndex];

            if (!checkPointer(cluster->cellPointers + cellIndex, data->entities.cellPointers)) {
                printf("wrong cell pointer pointer\n");
                STOP(a, b);
            }

            if (!checkPointer(cell, data->entities.cells)) {
                printf("wrong cell pointer\n");
                STOP(a, b);
            }

            for (int otherCellIndex = cellIndex + 1; otherCellIndex < cluster->numCellPointers; ++otherCellIndex) {
                auto const& otherCell = cluster->cellPointers[otherCellIndex];
                if (cell == otherCell) {
                    printf("cells are not unique within cluster\n");
                    STOP(a, b)
                }
            }

            if (cell->energy < 0) {
                printf("negative cell energy: %f\n", cell->energy);
                STOP(a, b)
            }
            if (cell->energy > 100000000) {
                printf("cell energy too high: %f\n", cell->energy);
                STOP(a, b)
            }
            if (cell->numConnections > cell->maxConnections) {
                printf("numConnections > maxConnections\n");
                STOP(a, b)
            }

            if (cell->maxConnections > cudaSimulationParameters.cellMaxBonds) {
                printf("maxConnections > cellMaxBonds\n");
                STOP(a, b)
            }

            if (cell->cluster != cluster) {
                printf("cell is from different cluster\n");
                STOP(a, b)
            }

            if (cell->numStaticBytes > MAX_CELL_STATIC_BYTES) {
                printf("numStaticBytes too large\n");
            }

            if (cell->numMutableBytes > MAX_CELL_MUTABLE_BYTES) {
                printf("numMutableBytes too large\n");
            }

            for (int i = 0; i < cell->numConnections; ++i) {
                auto const& connectingCell = cell->connections[i];
                if (connectingCell->cluster != cluster) {
                    printf("connecting cell is from different cluster\n");
                    STOP(a, b)
                }
                bool found = false;
                for (int j = 0; j < connectingCell->numConnections; ++j) {
                    auto const& connectingConnectingCell = connectingCell->connections[j];
                    if (connectingConnectingCell == cell) {
                        found = true;
                    }
                }
                if (!found) {
                    printf("cells are only connected in one way\n");
                    STOP(a, b)
                }
            }
        }

        auto const tokenBlock = calcPartition(cluster->numTokenPointers, threadIdx.x, blockDim.x);
        for (int tokenIndex = tokenBlock.startIndex; tokenIndex <= tokenBlock.endIndex; ++tokenIndex) {
            auto const& token = cluster->tokenPointers[tokenIndex];
            if (!checkPointer(cluster->tokenPointers + tokenIndex, data->entities.tokenPointers)) {
                printf("wrong token pointer pointer\n");
                STOP(a, b);
            }
            if (!checkPointer(token, data->entities.tokens)) {
                printf("wrong token pointer\n");
                STOP(a, b);
            }
            if (token->energy < 0) {
                printf("negative token energy: %f\n", token->energy);
                STOP(a, b)
            }

            if (token->energy > 100000000) {
                printf("token energy too high: %f\n", token->energy);
                STOP(a, b)
            }
        }
    }
};
