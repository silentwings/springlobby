#ifndef SPRINGLOBBY_HEADERGUARD_SPRINGLOBBYAPP_H
#define SPRINGLOBBY_HEADERGUARD_SPRINGLOBBYAPP_H

#include <wx/app.h>

class Ui;
class wxTimer;
class wxIcon;
class wxLocale;
class HttpDownloader;

//! @brief SpringLobby wxApp
class SpringLobbyApp : public wxApp
{
  public:
    SpringLobbyApp();
    ~SpringLobbyApp();

    virtual bool OnInit();
    virtual int OnExit();

    virtual void OnFatalException();

    // System Events
    void OnTimer( wxTimerEvent& event );

  protected:

    void SetupUserFolders();
    void InitDirs();

    wxTimer* m_timer;

    Ui* m_ui;

    wxLocale* m_locale;
    HttpDownloader* m_otadownloader ;

    DECLARE_EVENT_TABLE()
};

DECLARE_APP(SpringLobbyApp)

#endif // SPRINGLOBBY_HEADERGUARD_SPRINGLOBBYAPP_H
