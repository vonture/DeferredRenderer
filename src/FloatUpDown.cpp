#include "PCH.h"
#include "FloatUpDown.h"

#include "Gwen/Gwen.h"
#include "Gwen/Utility.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/NumericUpDown.h"
#include "Gwen/Controls/Layout/Splitter.h"

FloatUpDown::FloatUpDown(Gwen::Controls::Base *parent)
	: Gwen::Controls::TextBoxNumeric(parent)
{
	SetSize( 100, 20 );

	Gwen::Controls::Layout::Splitter* pSplitter = new Gwen::Controls::Layout::Splitter( this );
		pSplitter->Dock( Gwen::Pos::Right );
		pSplitter->SetSize( 13, 13 );

	Gwen::Controls::NumericUpDownButton_Up* pButtonUp = new Gwen::Controls::NumericUpDownButton_Up( pSplitter );
		pButtonUp->onPress.Add( this, &FloatUpDown::OnButtonUp );
		pButtonUp->SetTabable( false );

		pSplitter->SetPanel( 0, pButtonUp );
		

	Gwen::Controls::NumericUpDownButton_Down* pButtonDown = new Gwen::Controls::NumericUpDownButton_Down( pSplitter );
		pButtonDown->onPress.Add( this, &FloatUpDown::OnButtonDown );
		pButtonDown->SetTabable( false );
		pButtonUp->SetPadding( Gwen::Padding( 0, 1, 1, 0 ) );

		pSplitter->SetPanel( 1, pButtonDown );

	m_fMax = 100.0f;
	m_fMin = 0.0f;
	m_fNumber = 0.0f;
	m_fDelta = 1.0f;
	SetText( "0.0" );
}

FloatUpDown::~FloatUpDown()
{
}

void FloatUpDown::OnButtonUp(Base* /*control*/ )
{
	SyncNumberFromText();
	SetValue( m_fNumber + m_fDelta );
}

void FloatUpDown::OnButtonDown( Base* /*control*/ )
{
	SyncNumberFromText();
	SetValue( m_fNumber - m_fDelta );
}

void FloatUpDown::SyncTextFromNumber()
{
	SetText(Gwen::Utility::ToString(m_fNumber));
}

void FloatUpDown::SyncNumberFromText()
{
	SetValue(GetFloatFromText());
}

void FloatUpDown::SetValue(float val)
{
	val = clamp(val, m_fMin, m_fMax);

	if (val == m_fNumber)
	{
		return;
	}
	m_fNumber = val;

	// Don't update the text if we're typing in it..
	//if ( !HasFocus() )
	{
		SyncTextFromNumber();
	}

	OnChange();
}

void FloatUpDown::OnChange()
{
	onChanged.Call( this );
}

void FloatUpDown::OnTextChanged()
{
	BaseClass::OnTextChanged();

	SyncNumberFromText();
}

void FloatUpDown::OnEnter()
{
	SyncNumberFromText();
	SyncTextFromNumber();
}