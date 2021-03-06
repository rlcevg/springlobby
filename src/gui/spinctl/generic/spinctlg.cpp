/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/spinctlg.cpp
// Purpose:     implements SlSpinCtrl as a composite control
// Author:      Vadim Zeitlin
// Modified by:
// Created:     29.01.01
// RCS-ID:      $Id: spinctlg.cpp 62129 2009-09-25 15:25:25Z JS $
// Copyright:   (c) 2001 Vadim Zeitlin <zeitlin@dptmaths.ens-cachan.fr>
// License:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "../spinctrl.h"

#if wxUSE_SPINCTRL



#include "wx/spinbutt.h"
#include <wx/string.h>
#include "../../../utils/conversion.h"

#if wxUSE_SPINBTN


//DEFINE_EVENT_TYPE( SLEVT_COMMAND_SPINCTRL_UPDATED )
// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// the margin between the text control and the spin
static const wxCoord MARGIN = 2;

#define SPINCTRLBUT_MAX 32000 // large to avoid wrap around trouble

// ----------------------------------------------------------------------------
// SlSpinCtrlTextGeneric: text control used by spin control
// ----------------------------------------------------------------------------

SlSpinCtrlTextGeneric::SlSpinCtrlTextGeneric(SlSpinCtrlGenericBase *spin, const wxString& value, long style)
    : wxTextCtrl(spin->GetParent(), wxID_ANY, value, wxDefaultPosition, wxDefaultSize,
                 ( style & wxALIGN_MASK ) | wxTE_NOHIDESEL | wxTE_PROCESS_ENTER)
{
    m_spin = spin;

    // remove the default minsize, the spinctrl will have one instead
    SetSizeHints(wxDefaultCoord, wxDefaultCoord);
}

SlSpinCtrlTextGeneric::~SlSpinCtrlTextGeneric()
{
    // MSW sends extra kill focus event on destroy
    if (m_spin)
        m_spin->m_textCtrl = NULL;

    m_spin = NULL;
}

void SlSpinCtrlTextGeneric::OnTextEnter(wxCommandEvent& event)
{
    if (m_spin)
        m_spin->OnTextEnter(event);
}

void SlSpinCtrlTextGeneric::OnChar( wxKeyEvent &event )
{
    if (m_spin)
        m_spin->OnTextChar(event);
}

void SlSpinCtrlTextGeneric::OnKillFocus(wxFocusEvent& event)
{
    if (m_spin)
    {
        m_spin->SyncSpinToText();
        m_spin->DoSendEvent();
    }

    event.Skip();
}


BEGIN_EVENT_TABLE(SlSpinCtrlTextGeneric, wxTextCtrl)
    EVT_TEXT_ENTER(wxID_ANY, SlSpinCtrlTextGeneric::OnTextEnter)

    EVT_CHAR(SlSpinCtrlTextGeneric::OnChar)

    EVT_KILL_FOCUS(SlSpinCtrlTextGeneric::OnKillFocus)
END_EVENT_TABLE()

// ----------------------------------------------------------------------------
// SlSpinCtrlButtonGeneric: spin button used by spin control
// ----------------------------------------------------------------------------

class SlSpinCtrlButtonGeneric : public wxSpinButton
{
public:
    SlSpinCtrlButtonGeneric(SlSpinCtrlGenericBase *spin, int style)
        : wxSpinButton(spin->GetParent(), wxID_ANY, wxDefaultPosition,
                       wxDefaultSize, style | wxSP_VERTICAL)
    {
        m_spin = spin;

        SetRange(-SPINCTRLBUT_MAX, SPINCTRLBUT_MAX);

        // remove the default minsize, the spinctrl will have one instead
        SetSizeHints(wxDefaultCoord, wxDefaultCoord);
    }

    void OnSpinButton(wxSpinEvent& event)
    {
        if (m_spin)
            m_spin->OnSpinButton(event);
    }

    SlSpinCtrlGenericBase *m_spin;

private:
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SlSpinCtrlButtonGeneric, wxSpinButton)
    EVT_SPIN_UP(  wxID_ANY, SlSpinCtrlButtonGeneric::OnSpinButton)
    EVT_SPIN_DOWN(wxID_ANY, SlSpinCtrlButtonGeneric::OnSpinButton)
END_EVENT_TABLE()

// ============================================================================
// SlSpinCtrlGenericBase
// ============================================================================

// ----------------------------------------------------------------------------
// SlSpinCtrlGenericBase creation
// ----------------------------------------------------------------------------

void SlSpinCtrlGenericBase::Init()
{
    m_value         = 0;
    m_min           = 0;
    m_max           = 100;
    m_increment     = 1;
    m_snap_to_ticks = false;
	m_format        = _T("%g");

    m_spin_value    = 0;

    m_textCtrl = NULL;
    m_spinButton  = NULL;
}

bool SlSpinCtrlGenericBase::Create(wxWindow *parent,
                                   wxWindowID id,
                                   const wxString& value,
                                   const wxPoint& pos, const wxSize& size,
                                   long style,
                                   double min, double max, double initial,
                                   double increment,
                                   const wxString& name)
{
    // don't use borders for this control itself, it wouldn't look good with
    // the text control borders (but we might want to use style border bits to
    // select the text control style)
    if ( !wxControl::Create(parent, id, wxDefaultPosition, wxDefaultSize,
                            (style & ~wxBORDER_MASK) | wxBORDER_NONE,
                            wxDefaultValidator, name) )
    {
        return false;
    }

    m_value = initial;
    m_min   = min;
    m_max   = max;
    m_increment = increment;

    m_textCtrl   = new SlSpinCtrlTextGeneric(this, value, style);
    m_spinButton = new SlSpinCtrlButtonGeneric(this, style);

    m_spin_value = m_spinButton->GetValue();

    // the string value overrides the numeric one (for backwards compatibility
    // reasons and also because it is simpler to satisfy the string value which
    // comes much sooner in the list of arguments and leave the initial
    // parameter unspecified)
	m_textCtrl->SetValue(wxFormat(m_format ) % m_value);
    if ( !value.empty() )
    {
        double d;
        if ( value.ToDouble(&d) )
        {
            m_value = d;
			m_textCtrl->SetValue(wxFormat(m_format ) % m_value);
        }
    }

    SetInitialSize(size);
    Move(pos);

    // have to disable this window to avoid interfering it with message
    // processing to the text and the button... but pretend it is enabled to
    // make IsEnabled() return true
    wxControl::Enable(false); // don't use non virtual Disable() here!
    m_isEnabled = true;

    // we don't even need to show this window itself - and not doing it avoids
    // that it overwrites the text control
    wxControl::Show(false);
    m_isShown = true;
    return true;
}

SlSpinCtrlGenericBase::~SlSpinCtrlGenericBase()
{
    // delete the controls now, don't leave them alive even though they would
    // still be eventually deleted by our parent - but it will be too late, the
    // user code expects them to be gone now

    if (m_textCtrl)
    {
        // null this since MSW sends KILL_FOCUS on deletion, see ~SlSpinCtrlTextGeneric
        wxDynamicCast(m_textCtrl, SlSpinCtrlTextGeneric)->m_spin = NULL;

        SlSpinCtrlTextGeneric *text = (SlSpinCtrlTextGeneric*)m_textCtrl;
        m_textCtrl = NULL;
        delete text;
    }

    delete m_spinButton;
    m_spinButton = NULL;
}

// ----------------------------------------------------------------------------
// geometry
// ----------------------------------------------------------------------------

wxSize SlSpinCtrlGenericBase::DoGetBestSize() const
{
    wxSize sizeBtn  = m_spinButton->GetBestSize(),
           sizeText = m_textCtrl->GetBestSize();

    return wxSize(sizeBtn.x + sizeText.x + MARGIN, sizeText.y);
}

void SlSpinCtrlGenericBase::DoMoveWindow(int x, int y, int width, int height)
{
    wxControl::DoMoveWindow(x, y, width, height);

    // position the subcontrols inside the client area
    wxSize sizeBtn = m_spinButton->GetSize();

    wxCoord wText = width - sizeBtn.x;
    m_textCtrl->SetSize(x, y, wText, height);
    m_spinButton->SetSize(x + wText + MARGIN, y, wxDefaultCoord, height);
}

// ----------------------------------------------------------------------------
// operations forwarded to the subcontrols
// ----------------------------------------------------------------------------

bool SlSpinCtrlGenericBase::Enable(bool enable)
{
    if ( !wxControl::Enable(enable) )
        return false;

    m_spinButton->Enable(enable);
    m_textCtrl->Enable(enable);

    return true;
}

bool SlSpinCtrlGenericBase::Show(bool show)
{
    if ( !wxControl::Show(show) )
        return false;

    // under GTK Show() is called the first time before we are fully
    // constructed
    if ( m_spinButton )
    {
        m_spinButton->Show(show);
        m_textCtrl->Show(show);
    }

    return true;
}

bool SlSpinCtrlGenericBase::Reparent(wxWindowBase *newParent)
{
    if ( m_spinButton )
    {
        m_spinButton->Reparent(newParent);
        m_textCtrl->Reparent(newParent);
    }

    return true;
}

// ----------------------------------------------------------------------------
// Handle sub controls events
// ----------------------------------------------------------------------------

void SlSpinCtrlGenericBase::OnSpinButton(wxSpinEvent& event)
{
    event.Skip();

    // Sync the textctrl since the user expects that the button will modify
    // what they see in the textctrl.
    if ( m_textCtrl && m_textCtrl->IsModified() )
        SyncSpinToText();

    int spin_value = event.GetPosition();
    double step = (event.GetEventType() == wxEVT_SCROLL_LINEUP) ? 1 : -1;

    // Use the spinbutton's acceleration, if any, but not if wrapping around
    if (((spin_value >= 0) && (m_spin_value >= 0)) || ((spin_value <= 0) && (m_spin_value <= 0)))
        step *= abs(spin_value - m_spin_value);

    double value = AdjustToFitInRange(m_value + step*m_increment);

    // Ignore the edges when it wraps since the up/down event may be opposite
    // They are in GTK and Mac
    if (abs(spin_value - m_spin_value) > SPINCTRLBUT_MAX)
    {
        m_spin_value = spin_value;
        return;
    }

    m_spin_value = spin_value;

    if ( DoSetValue(value) )
        DoSendEvent();
}

void SlSpinCtrlGenericBase::OnTextEnter(wxCommandEvent& event)
{
    SyncSpinToText();
    DoSendEvent();
    event.Skip();
}

void SlSpinCtrlGenericBase::OnTextChar(wxKeyEvent& event)
{
    if ( !HasFlag(wxSP_ARROW_KEYS) )
    {
        event.Skip();
        return;
    }

    double value = m_value;
    switch ( event.GetKeyCode() )
    {
        case WXK_UP :
            value += m_increment;
            break;

        case WXK_DOWN :
            value -= m_increment;
            break;

        case WXK_PAGEUP :
            value += m_increment * 10.0;
            break;

        case WXK_PAGEDOWN :
            value -= m_increment * 10.0;
            break;

        default:
            event.Skip();
            return;
    }

    value = AdjustToFitInRange(value);

    if ( m_textCtrl && m_textCtrl->IsModified() )
        SyncSpinToText();

    if ( DoSetValue(value) )
        DoSendEvent();
}

// ----------------------------------------------------------------------------
// Textctrl functions
// ----------------------------------------------------------------------------

void SlSpinCtrlGenericBase::SyncSpinToText()
{
    if (!m_textCtrl)
        return;

    double textValue;
    if ( m_textCtrl->GetValue().ToDouble(&textValue) )
    {
        if (textValue > m_max)
            textValue = m_max;
        else if (textValue < m_min)
            textValue = m_min;

        if (m_value != textValue)
        {
            DoSetValue(textValue);
        }
    }
    else
    {
        // textctrl is out of sync, discard and reset
        DoSetValue(m_value);
    }
}

// ----------------------------------------------------------------------------
// changing value and range
// ----------------------------------------------------------------------------

void SlSpinCtrlGenericBase::SetValue(const wxString& text)
{
    wxCHECK_RET( m_textCtrl, wxT("invalid call to SlSpinCtrl::SetValue") );

    double val;
    if ( text.ToDouble(&val) && InRange(val) )
    {
        DoSetValue(val);
    }
    else // not a number at all or out of range
    {
        m_textCtrl->SetValue(text);
        m_textCtrl->SetSelection(0, -1);
        m_textCtrl->SetInsertionPointEnd();
    }
}

bool SlSpinCtrlGenericBase::DoSetValue(double val)
{
    wxCHECK_MSG( m_textCtrl, false, wxT("invalid call to SlSpinCtrl::SetValue") );

    if (!InRange(val))
        return false;

    if ( m_snap_to_ticks && (m_increment != 0) )
    {
        double snap_value = val / m_increment;

        if (wxFinite(snap_value)) // FIXME what to do about a failure?
        {
            if ((snap_value - floor(snap_value)) < (ceil(snap_value) - snap_value))
                val = floor(snap_value) * m_increment;
            else
                val = ceil(snap_value) * m_increment;
        }
    }

	wxString str = wxFormat(m_format) % val;

    if ((val != m_value) || (str != m_textCtrl->GetValue()))
    {
        m_value = val;
        str.ToDouble( &m_value );    // wysiwyg for textctrl
        m_textCtrl->SetValue( str );
        m_textCtrl->SetInsertionPointEnd();
        m_textCtrl->DiscardEdits();
        return true;
    }

    return false;
}

double SlSpinCtrlGenericBase::AdjustToFitInRange(double value) const
{
    if (value < m_min)
        value = HasFlag(wxSP_WRAP) ? m_max : m_min;
    if (value > m_max)
        value = HasFlag(wxSP_WRAP) ? m_min : m_max;

    return value;
}

void SlSpinCtrlGenericBase::DoSetRange(double min, double max)
{
    m_min = min;
    m_max = max;
}

void SlSpinCtrlGenericBase::DoSetIncrement(double inc)
{
    m_increment = inc;
}

void SlSpinCtrlGenericBase::SetSnapToTicks(bool snap_to_ticks)
{
    m_snap_to_ticks = snap_to_ticks;
    DoSetValue(m_value);
}

void SlSpinCtrlGenericBase::SetSelection(long from, long to)
{
    wxCHECK_RET( m_textCtrl, wxT("invalid call to SlSpinCtrl::SetSelection") );

    m_textCtrl->SetSelection(from, to);
}


#endif // wxUSE_SPINBTN


#endif // wxUSE_SPINCTRL
