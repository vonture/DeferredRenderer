#pragma once 

#include "Defines.h"
#include "ConfigurationPane.h"
#include "Logger.h"
#include "Gwen/Controls/TreeControl.h"

class ProfilePane : public ConfigurationPane<Logger>
{
private:
	Gwen::Controls::TreeControl* _tree;

	Gwen::Controls::Button* _captureButton;
	Gwen::Controls::Button* _clearButton;

	virtual void onCaptureButtonPressed(Gwen::Controls::Base* button);
	virtual void onClearButtonPressed(Gwen::Controls::Base* button);

	void buildTree(Gwen::Controls::TreeNode* node, Logger::EventIterator it);

public:
	ProfilePane(Gwen::Controls::Base* parent, Logger* logger);
	
	void OnFrameMove(double totalTime, float dt);
};