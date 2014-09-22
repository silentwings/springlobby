/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "platform.h"

#include <wx/string.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <iostream>

#include <lslutils/misc.h>

#include "conversion.h"
#include "gui/customdialogs.h"
#include "utils/version.h"

bool SafeMkdir(const wxString& dir)
{
    if (!wxDirExists(dir))
        return wxMkdir(dir);
	return true;
}

void ErrorMsgBox(const wxString& err, bool silent)
{
	if (!silent) {
		wxMessageBox(err, _T("Error") );
	}
}

bool MoveDirWithFilebackupRename( wxString from, wxString to, bool backup, bool silent)
{
    // first make sure that the source dir exists
    if(!wxDir::Exists(from)) {
			ErrorMsgBox(from + _T(" does not exist.  Can not copy directory."), silent);
            return false;
    }

	if (from == to) {
		ErrorMsgBox(_T("Cannot copy: source == destination: ") + from, silent);
		return false;
	}

	if (from.empty() || to.empty()){
		ErrorMsgBox(_T("Cannot copy empty directory"), silent);
		return false;
	}

    SafeMkdir(to);

    wxString sep = wxFileName::GetPathSeparator();

    wxDir dir(from);
    wxString filename;
    if (!dir.GetFirst(&filename)) {
		return false;
	}

    // append a slash if there is not one (for easier parsing)
    // because who knows what people will pass to the function.
    if ( !to.EndsWith( sep ) ) {
            to += sep;
    }
    // for both dirs
    if ( !from.EndsWith( sep ) ) {
            from += sep;
    }

	do {
		if (wxDirExists(from + filename) )
		{
			MoveDirWithFilebackupRename(from + filename, to + filename, false, silent); //no backup in subdirs
		} else{
			//if files exists move it to backup, this way we can use this func on windows to replace 'active' files
			if ( backup && wxFileExists( to + filename ) ) {
				//delete prev backup
				if ( wxFileExists( to + filename + _T(".old") ) ) {
					wxRemoveFile( to + filename + _T(".old")  );
				}
				//make backup
				if ( !wxRenameFile( to + filename, to + filename + _T(".old") ) ) {
					ErrorMsgBox( _T("could not rename %s, copydir aborted") + to + filename, silent);
					return false;
				}
			}
			//do the actual copy
			if ( !wxCopyFile(from + filename, to + filename, true) ) {
				ErrorMsgBox( _T("could not copy %s to %s, copydir aborted") + from + filename + _T("\n") + to + filename, silent);
				return false;
			}
			wxRemoveFile(from + filename); // remove source file
		}
	} while (dir.GetNext(&filename) );
    return true;
}

bool RmDir(wxString path, bool silent)
{
	if (path.empty()) {
		ErrorMsgBox(_T("Invalid path. Can't remove empty path."), silent);
		return false;
	}
	// first make sure that the dir exists
	if(!wxDir::Exists(path)) {
		ErrorMsgBox(TowxString(path) + _T(" does not exist.  Could not remove directory."), silent);
		return false;
	}

	// append a slash if we don't have one
	if (!path.EndsWith(wxString(wxFILE_SEP_PATH))) {
		path += wxFILE_SEP_PATH;
	}

	// define our directory object.  When we begin traversing it, the
	// os will not let go until the object goes out of scope.
	wxDir* dir = new wxDir(path);

	// check for allocation failure
	if (dir == NULL) {
		ErrorMsgBox(_T("Could not allocate new memory on the heap!"), silent);
		return false;
	}

	wxString filename;
	bool cont = dir->GetFirst(&filename);

	if (cont){
		do {
			if (wxDirExists(path + filename)) {
				RmDir(path + filename, silent);
			} else {
				if(!wxRemoveFile(path + filename)) {
					ErrorMsgBox(_T("Could not remove file \"") + TowxString(path) + TowxString(filename) + "\"", silent);
				}
			}
		} while (dir->GetNext(&filename));
	}

	// Remove our directory object, so the OS will let go of it and
	// allow us to delete it
	delete dir;

	// now actually try to delete it
	if (!wxFileName::Rmdir(path)) {
		ErrorMsgBox("Could not remove directory " + TowxString(path), silent);
		return false;
	} else {
		return true;
	}

}


#ifdef __WXMSW__
#include <wx/msw/registry.h>
#include <windows.h>
#include <wx/msw/winundef.h>
#include <shellapi.h>

bool IsPreVistaWindows()
{
    return wxPlatformInfo().GetOSMajorVersion() < 6;
}

bool IsUACenabled()
{
    wxRegKey UACpath( _T("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System") ); // check if UAC is on, skip dialog if not
    if(!UACpath.Exists() ) {
		return false;
	}

	const wxString LUA = _T("EnableLUA");
	if (!UACpath.HasValue(LUA)) {
		return false;
	}

	long value;
	if( UACpath.QueryValue(LUA, &value ) ) { // reg key not present -> not vista
		if( value != 0 ) {
			return true;
		}
	}

    return false;
}
#endif


CwdGuard::CwdGuard( const wxString& new_cwd )
    : m_old_cwd( wxGetCwd() )
{
    wxSetWorkingDirectory( new_cwd );
}
CwdGuard::~CwdGuard()
{
    wxSetWorkingDirectory( m_old_cwd );
}

wxString IdentityString(const wxString& format, bool lowerCase )
{
	return wxFormat( format ) % TowxString(getSpringlobbyName(lowerCase));
}

static wxString escapeStr(const wxString& str)
{
	if (str.Find(_T(" ")) == wxNOT_FOUND) {
		return str;
	}
	if (str.StartsWith(_T("\"")) && str.EndsWith(_T("\""))){ //no double escape
		return str;
	}
	return _T("\"") + str + _T("\"");
}

int RunProcess(const wxString& cmd, const wxArrayString& params, const bool async, const bool root)
{
	wxString paramstring;
	for (wxString param: params) {
		if (!paramstring.empty()) {
			paramstring += _T(" ");
		}
		paramstring += escapeStr(param);
	}
	wxLogDebug(_T("going to run %s %s"), cmd.c_str(), paramstring.c_str());
#ifdef __WXMSW__
	SHELLEXECUTEINFO ShExecInfo;
	DWORD exitCode = 0;
	ShExecInfo.lpVerb = _T("open");
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpFile = cmd.t_str();
	ShExecInfo.lpParameters = paramstring.t_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;

	if (root && IsUACenabled()) {
		if (!IsPreVistaWindows() )
			ShExecInfo.lpVerb = _T("runas");
	}

	int res = ShellExecuteEx(&ShExecInfo);
	if (async) return (res > 32);
	WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
	GetExitCodeProcess(ShExecInfo.hProcess, &exitCode);
	return exitCode;
#else
	wxString realcmd = escapeStr(cmd);
	if (!paramstring.empty()) {
		realcmd += _T(" ") + paramstring;
	}
	return system( realcmd.mb_str( wxConvUTF8 ) );
#endif
}

int BrowseFolder(const wxString& path)
{
	wxArrayString param;
#ifdef __WXMSW__
	return RunProcess(path, param);
#elif defined(__APPLE__)
	param.push_back(path);
	return RunProcess(_T("open"), param);
#else
	param.push_back(path);
	return RunProcess(_T("xdg-open"), param);
#endif
}

int WaitForExit(int pid)
{
#ifdef __WXMSW__
	HANDLE h = OpenProcess(0, false, pid);
	if (h == NULL) {
        return 0;
	}
	WaitForSingleObject(h, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(h, &exitCode);
	CloseHandle(h);
	return exitCode;
#endif
	return 0;
}
