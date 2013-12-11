#pragma once

#include "PCH.h"
#include "Gwen/Controls/Base.h"
#include "Gwen/Controls/TextBox.h"

class FloatUpDown : public Gwen::Controls::TextBoxNumeric
{
private:
    float m_fNumber;
    float m_fMax;
    float m_fMin;
    float m_fDelta;

    virtual void OnEnter();
    virtual void OnChange();
    virtual void OnTextChanged();

    virtual void OnButtonUp(Gwen::Controls::Base* control );
    virtual void OnButtonDown(Gwen::Controls::Base* control );

    virtual bool OnKeyUp(bool bDown) {    if (bDown) OnButtonUp( NULL ); return true; }
    virtual bool OnKeyDown(bool bDown) { if (bDown) OnButtonDown( NULL ); return true; }

    virtual void SyncTextFromNumber();
    virtual void SyncNumberFromText();

public:
    FloatUpDown(Gwen::Controls::Base *parent);
    ~FloatUpDown();

    virtual void SetMin(float min) { m_fMin = min; }
    virtual float GetMin() { return m_fMin; }

    virtual void SetMax(float max) { m_fMax = max; }
    virtual float GetMax() { return m_fMax; }

    virtual void SetValue(float val);
    virtual float GetValue() { return m_fNumber; }

    virtual void SetDelta(float delta) { m_fDelta = delta; }
    virtual float GetDelta() { return m_fDelta; }

    Gwen::Event::Caller onChanged;
};