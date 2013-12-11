#pragma once

#include "PCH.h"
#include "xnaCollision.h"
#include "Model.h"
#include "ModelInstance.h"

class ModelInstanceSet
{
private:
    std::vector<std::pair<Model*, std::vector<ModelInstance*>>> _instances;
    std::vector<UINT> _globalIndices;
    UINT _instanceCount;

    void createSet(std::vector<ModelInstance*>* instances);

public:
    ModelInstanceSet(std::vector<ModelInstance*>* instances, const Frustum* frust);
    ModelInstanceSet(std::vector<ModelInstance*>* instances, const Sphere* sphere);
    ModelInstanceSet(std::vector<ModelInstance*>* instances, const OrientedBox* obb);
    ModelInstanceSet(std::vector<ModelInstance*>* instances, const AxisAlignedBox* aabb);

    UINT GetModelCount() const;
    Model* GetModel(UINT idx);

    UINT GetInstanceCount() const;
    UINT GetInstanceCount(UINT modelIdx) const;
    ModelInstance* GetInstance(UINT modelIdx, UINT instanceIdx);

    UINT GetGlobalIndex(UINT modelIndex, UINT instanceIdx) const;
};