#pragma once
#include <vtf/vtf.h>

enum ETFDmgCustom {
  TF_DMG_CUSTOM_NONE = 0,
  TF_DMG_CUSTOM_HEADSHOT,
  TF_DMG_CUSTOM_BACKSTAB,
  TF_DMG_CUSTOM_BURNING,
  TF_DMG_WRENCH_FIX,
  TF_DMG_CUSTOM_MINIGUN,
  TF_DMG_CUSTOM_SUICIDE,
  TF_DMG_CUSTOM_TAUNTATK_HADOUKEN,
  TF_DMG_CUSTOM_BURNING_FLARE,
  TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON,
  TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM,
  TF_DMG_CUSTOM_PENETRATE_MY_TEAM,
  TF_DMG_CUSTOM_PENETRATE_ALL_PLAYERS,
  TF_DMG_CUSTOM_TAUNTATK_FENCING,
  TF_DMG_CUSTOM_PENETRATE_NONBURNING_TEAMMATE,
  TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB,
  TF_DMG_CUSTOM_TELEFRAG,
  TF_DMG_CUSTOM_BURNING_ARROW,
  TF_DMG_CUSTOM_FLYINGBURN,
  TF_DMG_CUSTOM_PUMPKIN_BOMB,
  TF_DMG_CUSTOM_DECAPITATION,
  TF_DMG_CUSTOM_TAUNTATK_GRENADE,
  TF_DMG_CUSTOM_BASEBALL,
  TF_DMG_CUSTOM_CHARGE_IMPACT,
  TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING,
  TF_DMG_CUSTOM_AIR_STICKY_BURST,
  TF_DMG_CUSTOM_DEFENSIVE_STICKY,
  TF_DMG_CUSTOM_PICKAXE,
  TF_DMG_CUSTOM_ROCKET_DIRECTHIT,
  TF_DMG_CUSTOM_TAUNTATK_UBERSLICE,
  TF_DMG_CUSTOM_PLAYER_SENTRY,
  TF_DMG_CUSTOM_STANDARD_STICKY,
  TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT,
  TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH,
  TF_DMG_CUSTOM_BLEEDING,
  TF_DMG_CUSTOM_GOLD_WRENCH,
  TF_DMG_CUSTOM_CARRIED_BUILDING,
  TF_DMG_CUSTOM_COMBO_PUNCH,
  TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KILL,
  TF_DMG_CUSTOM_FISH_KILL,
  TF_DMG_CUSTOM_TRIGGER_HURT,
  TF_DMG_CUSTOM_DECAPITATION_BOSS,
  TF_DMG_CUSTOM_STICKBOMB_EXPLOSION,
  TF_DMG_CUSTOM_AEGIS_ROUND,
  TF_DMG_CUSTOM_FLARE_EXPLOSION,
  TF_DMG_CUSTOM_BOOTS_STOMP,
  TF_DMG_CUSTOM_PLASMA,
  TF_DMG_CUSTOM_PLASMA_CHARGED,
  TF_DMG_CUSTOM_PLASMA_GIB,
  TF_DMG_CUSTOM_PRACTICE_STICKY,
  TF_DMG_CUSTOM_EYEBALL_ROCKET,
  TF_DMG_CUSTOM_HEADSHOT_DECAPITATION,
  TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON,
  TF_DMG_CUSTOM_FLARE_PELLET,
  TF_DMG_CUSTOM_CLEAVER,
  TF_DMG_CUSTOM_CLEAVER_CRIT,
  TF_DMG_CUSTOM_SAPPER_RECORDER_DEATH,
  TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB,
  TF_DMG_CUSTOM_MERASMUS_GRENADE,
  TF_DMG_CUSTOM_MERASMUS_ZAP,
  TF_DMG_CUSTOM_MERASMUS_DECAPITATION,
  TF_DMG_CUSTOM_CANNONBALL_PUSH,
  TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF,
  TF_DMG_CUSTOM_THROWABLE,
  TF_DMG_CUSTOM_THROWABLE_KILL,
  TF_DMG_CUSTOM_SPELL_TELEPORT,
  TF_DMG_CUSTOM_SPELL_SKELETON,
  TF_DMG_CUSTOM_SPELL_MIRV,
  TF_DMG_CUSTOM_SPELL_METEOR,
  TF_DMG_CUSTOM_SPELL_LIGHTNING,
  TF_DMG_CUSTOM_SPELL_FIREBALL,
  TF_DMG_CUSTOM_SPELL_MONOCULUS,
  TF_DMG_CUSTOM_SPELL_BLASTJUMP,
  TF_DMG_CUSTOM_SPELL_BATS,
  TF_DMG_CUSTOM_SPELL_TINY,
  TF_DMG_CUSTOM_KART,
  TF_DMG_CUSTOM_GIANT_HAMMER,
  TF_DMG_CUSTOM_RUNE_REFLECT,
  //
  // INSERT NEW ITEMS HERE TO AVOID BREAKING DEMOS
  //

  TF_DMG_CUSTOM_END // END
};

struct MovieInfo_t;
class IVideoMode {
public:
  virtual ~IVideoMode() {}
  virtual bool Init() = 0;
  virtual void Shutdown(void) = 0;

  // Shows the start-up graphics based on the mod
  // (Filesystem path for the mod must be set up first)
  virtual void DrawStartupGraphic() = 0;

  // Creates the game window, plays the startup movie, starts up the material
  // system
  virtual bool CreateGameWindow(int nWidth, int nHeight, bool bWindowed) = 0;

  // Sets the game window in editor mode
  virtual void SetGameWindow(void *hWnd) = 0;

  // Sets the video mode, and re-sizes the window
  virtual bool SetMode(int nWidth, int nHeight, bool bWindowed) = 0;

  // Returns the fullscreen modes for the adapter the game was started on
  virtual int GetModeCount(void) = 0;
  virtual struct vmode_s *GetMode(int num) = 0;

  // Purpose: This is called in response to a WM_MOVE message
  // or whatever the equivalent that would be under linux
  virtual void UpdateWindowPosition(void) = 0;

  // Alt-tab handling
  virtual void RestoreVideo(void) = 0;
  virtual void ReleaseVideo(void) = 0;

  virtual void DrawNullBackground(void *hdc, int w, int h) = 0;
  virtual void InvalidateWindow() = 0;

  // Returns the video mode width + height. In the case of windowed mode,
  // it returns the width and height of the drawable region of the window.
  // (it doesn't include the window borders)
  virtual int GetModeWidth() const = 0;
  virtual int GetModeHeight() const = 0;
  virtual bool IsWindowedMode() const = 0;

  // Returns the video mode width + height for a single stereo display.
  // if the game isn't running in side by side stereo, this is the same
  // as GetModeWidth and GetModeHeight
  virtual int GetModeStereoWidth() const = 0;
  virtual int GetModeStereoHeight() const = 0;

  // Returns the size of full screen UI. This might be wider than a single
  // eye on displays like the Oculus Rift where a single eye is very narrow.
  // if the game isn't running in side by side stereo, this is the same
  // as GetModeWidth and GetModeHeight
  virtual int GetModeUIWidth() const = 0;
  virtual int GetModeUIHeight() const = 0;

  // Returns the subrect to draw the client view into.
  // Coordinates are measured relative to the drawable region of the window
  virtual const vrect_t &GetClientViewRect() const = 0;
  virtual void SetClientViewRect(const vrect_t &viewRect) = 0;

  // Lazily recomputes client view rect
  virtual void MarkClientViewRectDirty() = 0;

  virtual void TakeSnapshotTGA(const char *pFileName) = 0;
  virtual void
  TakeSnapshotTGARect(const char *pFilename, int x, int y, int w, int h,
                      int resampleWidth, int resampleHeight, bool bPFM = false,
                      CubeMapFaceIndex_t faceIndex = CUBEMAP_FACE_RIGHT) = 0;
  virtual void WriteMovieFrame(const MovieInfo_t &info) = 0;

  // Takes snapshots
  virtual void TakeSnapshotJPEG(const char *pFileName, int quality) = 0;
  virtual bool TakeSnapshotJPEGToBuffer(CUtlBuffer &buf, int quality) = 0;

  virtual void ReadScreenPixels(int x, int y, int w, int h, void *pBuffer,
                                ImageFormat format) = 0;
};
