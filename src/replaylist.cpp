/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include <wx/file.h>
#include <wx/filefn.h>
#include <wx/log.h>
#include <wx/datetime.h>

#include "replaylist.h"
#include "storedgame.h"
#include "utils/conversion.h"
#include <lslutils/globalsmanager.h>

IPlaybackList& replaylist()
{
    static LSL::Util::LineInfo<ReplayList> m( AT );
    static LSL::Util::GlobalObjectHolder<ReplayList, LSL::Util::LineInfo<ReplayList> > m_replay_list( m );
    return m_replay_list;
}

ReplayList::ReplayList()
{
}

void ReplayList::LoadPlaybacks(const std::vector<std::string> &filenames )
{
	m_replays.clear();
	for ( size_t i = 0; i < filenames.size(); ++i) {
		const std::string wfilename = filenames[i];
		StoredGame& playback = AddPlayback(i);
		if (!GetReplayInfos(wfilename, playback)) {
			// looks like funny add/remove logic, but the Replay contains OfflineBattle which is IBattle which is ultimately boost::noncopyable.
			wxLogError(_T("Couldn't open replay %s"), TowxString(wfilename).c_str() );
			RemovePlayback(i);
		}
	}
}


int ReplayList::replayVersion(wxFile& replay ) const
{
	if(replay.Seek(16)==wxInvalidOffset) {
		return 0;
	}
	int version = 0;
	replay.Read( &version, 4);
	return version;
}

bool ReplayList::GetReplayInfos(const std::string& ReplayPath, StoredGame& ret ) const
{
	const std::string FileName = LSL::Util::AfterLast( ReplayPath, SEP ); // strips file path
	ret.type = StoredGame::REPLAY;
	ret.Filename = ReplayPath;
	ret.battle.SetPlayBackFilePath(ReplayPath);
	ret.SpringVersion = LSL::Util::BeforeLast(LSL::Util::AfterLast(FileName, "_"),".");
	ret.MapName = LSL::Util::BeforeLast(FileName, "_");

	if (!wxFileExists(TowxString(ReplayPath))) {
		return false;
	}
	wxFile replay(TowxString(ReplayPath), wxFile::read );
	if (!replay.IsOpened()) {
		return false;
	}

	const int replay_version = replayVersion( replay );
	ret.battle.SetScript(GetScriptFromReplay( replay, replay_version ));

	if ( ret.battle.GetScript().empty() ) {
		return false;
	}

	GetHeaderInfo(replay, ret, replay_version );
	ret.battle.GetBattleFromScript( false );
	ret.ModName = ret.battle.GetHostModName();
	ret.battle.SetBattleType( BT_Replay );
	ret.battle.SetEngineName("spring");
	ret.battle.SetEngineVersion(ret.SpringVersion);
	ret.battle.SetPlayBackFilePath(ReplayPath);

	//getting this from filename seems more reliable than from demoheader
	wxDateTime rdate;

	if (rdate.ParseFormat(TowxString(FileName), _T("%Y%m%d_%H%M%S")) == 0) {
		wxLogError(_T("Parsing %s failed"), TowxString(FileName).c_str());
		return false;
	}
	ret.date=rdate.GetTicks(); // now it is sorted properly
	ret.date_string=STD_STRING(rdate.FormatISODate()+_T(" ")+rdate.FormatISOTime());

	return true;
}

std::string ReplayList::GetScriptFromReplay(wxFile& replay, const int version) const
{

	std::string script;
	if ( !replay.IsOpened() ) return script;
	if(replay.Seek(20)==wxInvalidOffset) {
		return script;
	}
	int headerSize=0 ;
	replay.Read( &headerSize, 4);
	const int seek = 64 + (version < 5 ? 0 : 240);
	if(replay.Seek(seek)==wxInvalidOffset) {
		return script;
	}
	wxFileOffset scriptSize=0;
	replay.Read( &scriptSize, 4);
	scriptSize = LSL::Util::Clamp( wxFileOffset(scriptSize), wxFileOffset(0), replay.Length() );
	if(replay.Seek(headerSize) == wxInvalidOffset)return script;
	script.resize(scriptSize,0);
	replay.Read( &script[0], scriptSize );
	return script;
}

void ReplayList::GetHeaderInfo(wxFile& replay, StoredGame& rep, const int version) const
{
	const int seek = 72 + (version < 5 ? 0 : 240);
	if(replay.Seek(seek)==wxInvalidOffset) {
		return;
	}
	int gametime = 0 ;
	replay.Read( &gametime, 4);
	rep.duration = gametime;
	rep.size = replay.Length();
	//! \todo don't use long long? (pedantic)
	wxLongLong_t unixtime = 0;
	if(replay.Seek(56)==wxInvalidOffset) {
		return;
	}
	replay.Read( &unixtime, 8 );
	wxDateTime dt;
	dt.Set( (time_t) unixtime );
	// todo: add 2 strings one for date other for time?
	rep.date_string = STD_STRING(dt.FormatISODate()+_T(" ")+dt.FormatISOTime());
	//  rep.date = (time_t) unixtime ;
}
