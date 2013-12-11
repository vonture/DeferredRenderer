#include "PCH.h"
#include "ModelConfigurationPane.h"

ModelConfigurationPane::ModelConfigurationPane(Gwen::Controls::Base* parent)
    : ConfigurationPaneBase(parent, L"Models")
{
    const int treeHeight = 400;
    const int propertiesHeight = 300;

    _tree = new Gwen::Controls::TreeControl(this);
    _tree->SetHeight(treeHeight);
    _tree->Dock(Gwen::Pos::Top);

    _properties = new Gwen::Controls::Properties(this);
    _properties->SetHeight(propertiesHeight);
    _properties->Dock(Gwen::Pos::Top);
}

Gwen::Controls::TreeNode* ModelConfigurationPane::buildTree(ModelInstance* instance)
{
    const Model* model = instance->GetModel();
    Gwen::Controls::TreeNode* modelNode = _tree->AddNode(Gwen::UnicodeString(model->GetName()));
    modelNode->onSelect.Add(this, &ModelConfigurationPane::onModelSelect);
    modelNode->SetUserData((void*)model);

    Gwen::Controls::TreeNode* meshRootNode = modelNode->AddNode(L"Meshes");
    for (UINT j = 0; j < model->GetMeshCount(); j++)
    {
        const Mesh* mesh = model->GetMesh(j);
        Gwen::Controls::TreeNode* meshNode = meshRootNode->AddNode(Gwen::UnicodeString(mesh->GetName()));
        meshNode->onSelect.Add(this, &ModelConfigurationPane::onMeshSelect);
        meshNode->SetUserData((void*)mesh);

        Gwen::Controls::TreeNode* meshPartRootNode = meshNode->AddNode(L"Parts");
        for (UINT k = 0; k < mesh->GetMeshPartCount(); k++)
        {
            const MeshPart* part = mesh->GetMeshPart(k);
            Gwen::Controls::TreeNode* partNode = meshPartRootNode->AddNode(Gwen::Utility::Format(L"Part %u", k));
            partNode->onSelect.Add(this, &ModelConfigurationPane::onMeshPartSelect);
            partNode->SetUserData((void*)part);
        }
    }

    Gwen::Controls::TreeNode* materialRootNode = modelNode->AddNode(L"Materials");
    for (UINT j = 0; j < model->GetMaterialCount(); j++)
    {
        const Material* material = model->GetMaterial(j);
        Gwen::Controls::TreeNode* materialNode = materialRootNode->AddNode(Gwen::UnicodeString(material->GetName()));
        materialNode->onSelect.Add(this, &ModelConfigurationPane::onMaterialSelect);
        materialNode->SetUserData((void*)material);

        ID3D11ShaderResourceView* diffSRV = material->GetDiffuseSRV();
        if (diffSRV)
        {
            Gwen::Controls::TreeNode* diffuseNode = materialNode->AddNode(L"Diffuse texture");
            diffuseNode->onSelect.Add(this, &ModelConfigurationPane::onTextureSelect);
            diffuseNode->SetUserData((void*)diffSRV);
        }

        ID3D11ShaderResourceView* normSRV = material->GetNormalSRV();
        if (normSRV)
        {
            Gwen::Controls::TreeNode* normNode = materialNode->AddNode(L"Normal texture");
            normNode->onSelect.Add(this, &ModelConfigurationPane::onTextureSelect);
            normNode->SetUserData((void*)normSRV);
        }

        ID3D11ShaderResourceView* specSRV = material->GetSpecularSRV();
        if (specSRV)
        {
            Gwen::Controls::TreeNode* specNode = materialNode->AddNode(L"Specular texture");
            specNode->onSelect.Add(this, &ModelConfigurationPane::onTextureSelect);
            specNode->SetUserData((void*)specSRV);
        }
    }

    return modelNode;
}

void ModelConfigurationPane::removeTree(ModelInstance* instance)
{
}

void ModelConfigurationPane::AddModelInstance(ModelInstance* model)
{
    _modelVec.push_back(model);
    _modelMap[model] = buildTree(model);
}

void ModelConfigurationPane::SelectModelInstance(ModelInstance* model)
{
    std::map<ModelInstance*, Gwen::Controls::TreeNode*>::iterator i = _modelMap.find(model);
    if (i != _modelMap.end())
    {
        _tree->DeselectAll();
        i->second->SetSelected(true);
    }
}

ModelInstance* ModelConfigurationPane::GetModelInstance(UINT idx)
{
    return (idx < _modelVec.size()) ? _modelVec[idx] : NULL;
}

UINT ModelConfigurationPane::GetModelInstanceCount() const
{
    return _modelVec.size();
}

void ModelConfigurationPane::onModelSelect(Gwen::Controls::Base* control)
{
}

void ModelConfigurationPane::onMeshSelect(Gwen::Controls::Base* control)
{
}

void ModelConfigurationPane::onMeshPartSelect(Gwen::Controls::Base* control)
{
}

void ModelConfigurationPane::onMaterialSelect(Gwen::Controls::Base* control)
{
}

void ModelConfigurationPane::onTextureSelect(Gwen::Controls::Base* control)
{
}