#include "PCH.h"
#include "ModelInstanceSet.h"

ModelInstanceSet::ModelInstanceSet(std::vector<ModelInstance*>* instances, Camera* camera)
	: _instanceCount(0)
{
	Frustum cameraFrust = camera->CreateFrustum();

	for (UINT i = 0; i < instances->size(); i++)
	{
		ModelInstance* instance = instances->at(i);
		Model* model = instance->GetModel();

		OrientedBox modelBounds = instance->GetOrientedBox();
		if (Collision::IntersectOrientedBoxFrustum(&modelBounds, &cameraFrust))
		{
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
	}

	UINT count = 0;
	for (UINT i = 0; i < _instances.size(); i++)
	{
		_globalIndices.push_back(count);
		count += _instances[i].second.size();
	}
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