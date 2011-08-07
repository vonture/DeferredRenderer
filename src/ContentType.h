#pragma once

#include "Defines.h"
#include "ContentTypeBase.h"

template <class T>
class ContentType : public ContentTypeBase
{
private:
	T* _content;

protected:
	void SetContent(T* content) { _content = content; }

public:
	ContentType() 
		: _content(NULL) 
	{
	}

	T* GetContent() { return _content; }
};