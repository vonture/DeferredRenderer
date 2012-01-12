#pragma once

#include "PCH.h"
#include "Camera.h"
#include "Model.h"
#include "ModelInstance.h"

class ModelInstanceSet
{
private:
	std::vector<std::pair<Model*, std::vector<ModelInstance*>>> _instances;
	std::vector<UINT> _globalIndices;
	UINT _instanceCount;

public:
	ModelInstanceSet(std::vector<ModelInstance*>* instances, Camera* camera);
	
	UINT GetModelCount() const;
	Model* GetModel(UINT idx);

	UINT GetInstanceCount() const;
	UINT GetInstanceCount(UINT modelIdx) const;
	ModelInstance* GetInstance(UINT modelIdx, UINT instanceIdx);

	UINT GetGlobalIndex(UINT modelIndex, UINT instanceIdx) const;

};