#pragma once

#include "PCH.h"
#include "ConfigurationPaneBase.h"
#include "ModelInstance.h"
#include "Gwen/Controls/TreeControl.h"
#include "Gwen/Controls/Properties.h"

class ModelConfigurationPane : public ConfigurationPaneBase
{
	std::vector<ModelInstance*> _models;

	Gwen::Controls::TreeControl* _tree;
	Gwen::Controls::Properties* _properties;

	void buildTree(ModelInstance* instance);
	void removeTree(ModelInstance* instance);

	void onModelSelect(Gwen::Controls::Base* control);
	void onMeshSelect(Gwen::Controls::Base* control);
	void onMeshPartSelect(Gwen::Controls::Base* control);
	void onMaterialSelect(Gwen::Controls::Base* control);
	void onTextureSelect(Gwen::Controls::Base* control);

public:
	ModelConfigurationPane(Gwen::Controls::Base* parent);

	void AddModelInstance(ModelInstance* model);

	ModelInstance* GetModelInstance(UINT idx);
	UINT GetModelInstanceCount() const;
};