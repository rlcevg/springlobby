/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef CONTENTDOWNLOADDIALOG_H
#define CONTENTDOWNLOADDIALOG_H

#include "windowattributespickle.h"
#include <wx/dialog.h>
class wxBoxSizer;
class ContentSearchResultsListctrl;
class ContentDownloadDialog : public wxDialog, public WindowAttributesPickle
{
public:
ContentDownloadDialog(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long int style = wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxDEFAULT_DIALOG_STYLE , const wxString& name = wxDialogNameStr);
virtual ~ContentDownloadDialog();
virtual bool Show(bool show = true);
private:
  wxBoxSizer* m_main_sizer;
ContentSearchResultsListctrl* m_search_res_w;
};

#endif // CONTENTDOWNLOADDIALOG_H