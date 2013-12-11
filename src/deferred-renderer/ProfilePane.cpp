#include "PCH.h"
#include "ProfilePane.h"

ProfilePane::ProfilePane(Gwen::Controls::Base* parent, Logger* logger)
    : ConfigurationPane(parent, L"Profile", logger)
{
    const int treeHeight = 400;

    _tree = new Gwen::Controls::TreeControl(this);
    _tree->SetHeight(treeHeight);
    _tree->Dock(Gwen::Pos::Top);

    _captureButton = new Gwen::Controls::Button(this);
    _captureButton->SetText("Capture frame");
    _captureButton->onPress.Add(this, &ProfilePane::onCaptureButtonPressed);
    _captureButton->Dock(Gwen::Pos::Top);

    _clearButton = new Gwen::Controls::Button(this);
    _clearButton->SetText("Clear");
    _clearButton->onPress.Add(this, &ProfilePane::onClearButtonPressed);
    _clearButton->Dock(Gwen::Pos::Top);
}

void ProfilePane::onCaptureButtonPressed(Gwen::Controls::Base* button)
{
    Logger* logger = GetConfiguredObject();
    buildTree(_tree, logger->GetRootEvent());
}

void ProfilePane::onClearButtonPressed(Gwen::Controls::Base* button)
{
    _tree->Clear();
}

void ProfilePane::buildTree(Gwen::Controls::TreeNode* node, Logger::EventIterator it)
{
    if (it.IsValid())
    {
        // Create this node
        Gwen::UnicodeString text = Gwen::Utility::Format(L"%s (%f ms)", it.GetName(), it.GetDuration() * 1000.0f);
        Gwen::Controls::TreeNode* child = node->AddNode(text);

        // Create the children
        if (it.HasChildren())
        {
            buildTree(child, it.GetFirstChild());
        }

        // Create the siblings
        if (it.HasSiblings())
        {
            buildTree(node, it.GetNextSibling());
        }
    }
}

void ProfilePane::OnFrameMove(double totalTime, float dt)
{
}