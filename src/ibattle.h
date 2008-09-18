#ifndef SPRINGLOBBY_HEADERGUARD_IBATTLE_H
#define SPRINGLOBBY_HEADERGUARD_IBATTLE_H


#include <wx/string.h>
#include <list>
#include <map>

#include "iunitsync.h"
#include "user.h"
#include "mmoptionswrapper.h"


typedef int HostInfo;

typedef int StartType;
enum {
  ST_Fixed = 0,
  ST_Random = 1,
  ST_Choose = 2,
  ST_Pick = 3
};


typedef int GameType;
enum  {
  GT_ComContinue = 0,
  GT_ComEnds = 1,
  GT_Lineage = 2
};


typedef int BattleType;
enum {
  BT_Unknown = 0,
  BT_Multi = 1,
  BT_Single = 2
};


struct BattleStartRect
{
  BattleStartRect() { toadd = false; todelete = false; exist = false; toresize = false; }
  bool toadd;
  bool todelete;
  bool toresize;
  bool exist;

  unsigned int ally;
  unsigned int top;
  unsigned int left;
  unsigned int right;
  unsigned int bottom;
};


struct BattleBot {
  UserBattleStatus bs;
  int posx;
  int posy;
  int handicap;
  wxString name;
  wxString owner;
  wxString aidll;

};


class IBattle
{
  public:

    IBattle();
    virtual ~IBattle();

    virtual void SetHostMap( const wxString& mapname, const wxString& hash );
    virtual void SetLocalMap( const UnitSyncMap& map );
    virtual const UnitSyncMap& LoadMap();
    virtual wxString GetHostMapName() const;
    virtual wxString GetHostMapHash() const;

    virtual void SetHostMod( const wxString& modname, const wxString& hash );
    virtual void SetLocalMod( const UnitSyncMod& mod );
    virtual const UnitSyncMod& LoadMod();
    virtual wxString GetHostModName() const;
    virtual wxString GetHostModHash() const;

    virtual bool MapExists();
    virtual bool ModExists();

    virtual wxColour GetFreeColour( User *for_whom ) const = 0;

    virtual BattleStartRect GetStartRect( unsigned int allyno ) { BattleStartRect foo; return foo; };
    virtual void AddStartRect( unsigned int allyno, unsigned int left, unsigned int top, unsigned int right, unsigned int bottom ) {};
    virtual void RemoveStartRect( unsigned int allyno ) {};
    virtual void ResizeStartRect( unsigned int allyno ) {};
    virtual void StartRectRemoved( unsigned int allyno ) {};
    virtual void StartRectResized( unsigned int allyno ) {};
    virtual void StartRectAdded( unsigned int allyno ) {};
    virtual void ClearStartRects(){};

    virtual int GetMyAlly() = 0;
    virtual void SetMyAlly( int ally ) = 0;

    virtual bool IsFounderMe() const =0;

    virtual void SendHostInfo( HostInfo update ) = 0;
    virtual void SendHostInfo( const wxString& Tag ) = 0;

    virtual BattleBot* GetBotByStartPosition( unsigned int startpos ) { return 0; };
    virtual BattleBot* GetBot( unsigned int index ) const  = 0;
    virtual BattleBot* GetBot( const wxString& name ) const { return 0; };
    virtual unsigned int GetNumBots() const = 0;
    virtual unsigned int AddBot( int ally, int posx, int posy, int handicap, const wxString& aidll );
    virtual void RemoveBot( unsigned int index ) {};

    virtual void GetFreePosition( int& x, int& y ) {}
    virtual int GetFreeAlly() { return 0; }

    virtual void DisableUnit( const wxString& unitname );
    virtual void EnableUnit( const wxString& unitname );
    virtual void EnableAllUnits();
    virtual wxArrayString DisabledUnits();

    virtual void OnUnitSyncReloaded();

    virtual std::map<unsigned int,BattleStartRect>::size_type GetNumRects() =0;

    virtual mmOptionsWrapper& CustomBattleOptions() =0;

    virtual void LoadOptionsPreset( const wxString& name );
    virtual void SaveOptionsPreset( const wxString& name );

  protected:

    bool m_map_loaded;
    bool m_mod_loaded;
    bool m_map_exists;
    bool m_mod_exists;
    UnitSyncMap m_local_map;
    UnitSyncMod m_local_mod;
    wxString m_host_map_name;
    wxString m_host_mod_name;
    wxString m_host_map_hash;
    wxString m_host_mod_hash;

    wxArrayString m_units;
};

#endif // SPRINGLOBBY_HEADERGUARD_IBATTLE_H
