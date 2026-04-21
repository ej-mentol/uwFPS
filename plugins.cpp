#include "plugins.h"
#include "Interface/IPlugins.h"

cl_enginefunc_t  gEngfuncs;
cl_exportfuncs_t gExportfuncs;
metahook_api_t*  g_pMetaHookAPI = NULL;
mh_interface_t*  g_pInterface   = NULL;
mh_enginesave_t* g_pMetaSave    = NULL;

cvar_t* uw_auto          = NULL;
cvar_t* uw_fps_underwater = NULL;
cvar_t* uw_fps_normal    = NULL;
cvar_t* uw_delay         = NULL;
cvar_t* uw_mode          = NULL;
cvar_t* uw_trigger_level = NULL;

void (*g_pfnHUD_Init)(void)                        = NULL;
void (*g_pfnHUD_Frame)(double time)                = NULL;
int  (*g_pfnHUD_VidInit)(void)                     = NULL;
void (*g_pfnV_CalcRefdef)(struct ref_params_s* pp) = NULL;

void uwFPS_Init(void)
{
    static bool bInit = false;
    if (bInit) return;

    if (gEngfuncs.pfnRegisterVariable) {
        uw_auto          = gEngfuncs.pfnRegisterVariable("uw_auto",          "1",   FCVAR_ARCHIVE);
        uw_fps_underwater = gEngfuncs.pfnRegisterVariable("uw_fps_underwater","20",  FCVAR_ARCHIVE);
        uw_fps_normal    = gEngfuncs.pfnRegisterVariable("uw_fps_normal",    "0",   FCVAR_ARCHIVE);
        uw_delay         = gEngfuncs.pfnRegisterVariable("uw_delay",         "0.3", FCVAR_ARCHIVE);
        uw_mode          = gEngfuncs.pfnRegisterVariable("uw_mode",          "0",   FCVAR_ARCHIVE);
        uw_trigger_level = gEngfuncs.pfnRegisterVariable("uw_trigger_level", "3",   FCVAR_ARCHIVE);
    }

    if (gEngfuncs.pfnAddCommand)
        gEngfuncs.pfnAddCommand("uw_toggle", uw_toggle_f);

    bInit = true;
}

void IPluginsV4::Init(metahook_api_t* pAPI, mh_interface_t* pIface, mh_enginesave_t* pSave)
{
    g_pMetaHookAPI = pAPI;
    g_pInterface   = pIface;
    g_pMetaSave    = pSave;
}

void IPluginsV4::Shutdown(void) {}

void IPluginsV4::LoadEngine(cl_enginefunc_t* pEngfuncs)
{
    memcpy(&gEngfuncs, pEngfuncs, sizeof(gEngfuncs));
}

void IPluginsV4::LoadClient(cl_exportfuncs_t* pExportFunc)
{
    if (!pExportFunc) return;
    memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

    g_pfnHUD_Init     = pExportFunc->HUD_Init;     pExportFunc->HUD_Init     = HUD_Init;
    g_pfnHUD_Frame    = pExportFunc->HUD_Frame;    pExportFunc->HUD_Frame    = HUD_Frame;
    g_pfnHUD_VidInit  = pExportFunc->HUD_VidInit;  pExportFunc->HUD_VidInit  = HUD_VidInit;
    g_pfnV_CalcRefdef = pExportFunc->V_CalcRefdef; pExportFunc->V_CalcRefdef = V_CalcRefdef;
}

void IPluginsV4::ExitGame(int iResult) { RestoreFPS(); }

const char* IPluginsV4::GetVersion(void) { return "1.3.0"; }

EXPOSE_SINGLE_INTERFACE(IPluginsV4, IPluginsV4, METAHOOK_PLUGIN_API_VERSION_V4);