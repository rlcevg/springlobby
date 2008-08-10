#ifndef MAINTORRENTTAB_H
#define MAINTORRENTTAB_H

#ifndef NO_TORRENT_SYSTEM

#include <wx/panel.h>
#include <map>
#include "torrentwrapper.h"
typedef std::map<int,TorrentInfos> map_infos;
typedef map_infos::iterator map_infos_iter;

class wxStaticText;
class wxButton;
class TorrentListCtrl;
class Ui;
struct TorrentInfos;
class wxBoxSizer;
class FileListDialog;
class ColorButton;

class MainTorrentTab: public wxPanel
{
	public:

		MainTorrentTab(wxWindow* parent,Ui& ui);
		~MainTorrentTab();

		wxStaticText* m_incoming;
		wxStaticText* m_outgoing;
		wxStaticText* m_incoming_lbl;
		wxStaticText* m_outgoing_lbl;
		wxStaticText* m_status_color_text;

		wxButton* m_but_cancel;
		wxButton* m_but_publish;
		wxButton* m_but_download;
		ColorButton* m_status_color;
		TorrentListCtrl* m_torrent_list;

		Ui& m_ui;
        void OnUpdate();

	protected:

		enum
		{
            ID_LIST = wxID_HIGHEST,
            ID_BUTTON_CANCEL,
            ID_BUTTON_PUB,
            ID_OUTGOING,
            ID_INCOMING,
            ID_OUTGOING_LBL,
            ID_INCOMING_LBL,
            ID_DOWNLOAD_DIALOG

        };

        map_infos info_map;
        void AddTorrentInfo(  TorrentInfos& info );
        void UpdateInfo(  TorrentInfos& info );
        void SetInfo(int index,  TorrentInfos& info );
        void OnCancelButton( wxCommandEvent& event );
        void OnDownloadDialog( wxCommandEvent& event );

        wxBoxSizer* m_mainbox;
        FileListDialog* m_download_dialog;

	private:

		//(*Handlers(MainTorrentTab)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif

#endif
