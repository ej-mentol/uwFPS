#ifndef PLUGINS_H
#define PLUGINS_H

#include <metahook.h>
#include "HLSDK/common/interface.h"
#include "HLSDK/common/cvardef.h"
#include "HLSDK/common/ref_params.h"
#include "HLSDK/common/entity_state.h"
#include "HLSDK/common/cl_entity.h"
#include "HLSDK/pm_shared/pm_shared.h"

#include <string>

#define UWFPS_VERSION "1.4.0"

extern cvar_t* uw_auto;
extern cvar_t* uw_fps_underwater;
extern cvar_t* uw_fps_normal;
extern cvar_t* uw_delay;
extern cvar_t* uw_mode;
extern cvar_t* uw_trigger_level;
extern cvar_t* uw_saved_fps;   // crash-safety: original fps_max persisted to config.cfg

extern cl_enginefunc_t gEngfuncs;
extern cl_exportfuncs_t gExportfuncs;
extern metahook_api_t* g_pMetaHookAPI;

extern void (*g_pfnHUD_Init)(void);
extern void (*g_pfnHUD_Frame)(double time);
extern int  (*g_pfnHUD_VidInit)(void);
extern void (*g_pfnHUD_Reset)(void);
extern void (*g_pfnV_CalcRefdef)(struct ref_params_s* pparams);

void HUD_Init(void);
void HUD_Frame(double time);
int  HUD_VidInit(void);
void HUD_Reset(void);
void V_CalcRefdef(struct ref_params_s* pparams);
void uwFPS_Init(void);
void uw_toggle_f(void);
void LoadMapList(void);
void RestoreFPS(void);
bool IsMapAllowed(const char* mapname);

inline bool IsCvarExists(const cvar_t* cv) { return cv != nullptr; }

#endif
