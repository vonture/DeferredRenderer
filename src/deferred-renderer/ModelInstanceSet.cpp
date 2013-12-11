#include "PCH.h"
#include "ModelInstanceSet.h"

void ModelInstanceSet::createSet(std::vector<ModelInstance*>* instances)
{
    for (UINT i = 0; i < instances->size(); i++)
    {
        ModelInstance* instance = instances->at(i);
        Model* model = instance->GetModel();

        _instanceCount++;

        bool found = false;
        for (UINT j = 0; j < _instances.size(); j++)
        {
            if (_instances[j].first == model)
            {
                _instances[j].second.push_back(instance);
                found = true;
                break;
            }
        }

        if (!found)
        {
            _instances.push_back(std::pair<Model*, std::vector<ModelInstance*>>(model, std::vector<ModelInstance*>()));
            _instances[_instances.size() - 1].second.push_back(instance);
        }
    }

    UINT count = 0;
    for (UINT i = 0; i < _instances.size(); i++)
    {
        _globalIndices.push_back(count);
        count += _instances[i].second.size();
    }
}

ModelInstanceSet::ModelInstanceSet(std::vector<ModelInstance*>* instances, const Frustum* frust)
    : _instanceCount(0)
{
    std::vector<ModelInstance*> insideModels;
    for (UINT i = 0; i < instances->size(); i++)
    {
        ModelInstance* instance = instances->at(i);
        OrientedBox obb = instance->GetOrientedBox();

        if (Collision::IntersectOrientedBoxFrustum(&obb, frust))
        {
            insideModels.push_back(instance);
        }
    }
    createSet(&insideModels);
}

ModelInstanceSet::ModelInstanceSet(std::vector<ModelInstance*>* instances, const Sphere* sphere)
    : _instanceCount(0)
{
    std::vector<ModelInstance*> insideModels;
    for (UINT i = 0; i < instances->size(); i++)
    {
        ModelInstance* instance = instances->at(i);
        OrientedBox obb = instance->GetOrientedBox();

        if (Collision::IntersectSphereOrientedBox(sphere, &obb))
        {
            insideModels.push_back(instance);
        }
    }
    createSet(&insideModels);
}

ModelInstanceSet::ModelInstanceSet(std::vector<ModelInstance*>* instances, const OrientedBox* obb)
    : _instanceCount(0)
{
    std::vector<ModelInstance*> insideModels;
    for (UINT i = 0; i < instances->size(); i++)
    {
        ModelInstance* instance = instances->at(i);
        OrientedBox modelObb = instance->GetOrientedBox();

        if (Collision::IntersectOrientedBoxOrientedBox(&modelObb, obb))
        {
            insideModels.push_back(instance);
        }
    }
    createSet(&insideModels);
}

ModelInstanceSet::ModelInstanceSet(std::vector<ModelInstance*>* instances, const AxisAlignedBox* aabb)
    : _instanceCount(0)
{
    std::vector<ModelInstance*> insideModels;
    for (UINT i = 0; i < instances->size(); i++)
    {
        ModelInstance* instance = instances->at(i);
        OrientedBox obb = instance->GetOrientedBox();

        if (Collision::IntersectAxisAlignedBoxOrientedBox(aabb, &obb))
        {
            insideModels.push_back(instance);
        }
    }
    createSet(&insideModels);
}

UINT ModelInstanceSet::GetModelCount() const
{
    return _instances.size();
}

Model* ModelInstanceSet::GetModel(UINT idx)
{
    return _instances[idx].first;
}

UINT ModelInstanceSet::GetInstanceCount() const
{
    return _instanceCount;
}

UINT ModelInstanceSet::GetInstanceCount(UINT modelIdx) const
{
    return _instances[modelIdx].second.size();
}

ModelInstance* ModelInstanceSet::GetInstance(UINT modelIdx, UINT instanceIdx)
{
    return _instances[modelIdx].second[instanceIdx];
}

UINT ModelInstanceSet::GetGlobalIndex(UINT modelIndex, UINT instanceIdx) const
{
    return _globalIndices[modelIndex] + instanceIdx;
}