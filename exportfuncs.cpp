#include "plugins.h"
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

static float s_enter_time  = 0.f;
static bool  s_underwater  = false;
static bool  s_map_allowed = true;
static float s_original_fps = 100.0f;
static bool  s_fps_saved   = false;
static std::vector<std::string> s_map_list;

void LoadMapList(void)
{
    s_map_list.clear();
    std::string path = std::string(g_pMetaHookAPI->GetGameDirectory()) + "/uwFPS/maps.ini";
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        s_map_list.push_back(line.substr(first, last - first + 1));
    }
}

void RestoreFPS(void)
{
    if (!s_fps_saved) return;
    float target = (IsCvarExists(uw_fps_normal) && uw_fps_normal->value > 0)
                   ? uw_fps_normal->value : s_original_fps;
    char cmd[64];
    sprintf_s(cmd, "fps_max %.0f", target);
    gEngfuncs.pfnClientCmd(cmd);
    s_fps_saved = false;
}

bool IsMapAllowed(const char* mapname)
{
    if (!mapname || !mapname[0]) return true;

    std::string map = mapname;
    if (map.find("maps/") == 0) map.erase(0, 5);
    if (map.size() > 4 && map.substr(map.size() - 4) == ".bsp") map.resize(map.size() - 4);

    bool found = false;
    for (const auto& m : s_map_list) {
        if (m == map) { found = true; break; }
    }

    int mode = IsCvarExists(uw_mode) ? (int)uw_mode->value : 0;
    return (mode != 0) ? found : !found;
}

void uw_toggle_f(void)
{
    if (!IsCvarExists(uw_auto)) return;
    float newVal = (uw_auto->value > 0) ? 0.0f : 1.0f;
    gEngfuncs.Cvar_SetValue("uw_auto", newVal);

    if (newVal > 0) {
        gEngfuncs.pfnCenterPrint("uwFPS: ENABLED");
    } else {
        gEngfuncs.pfnCenterPrint("uwFPS: DISABLED");
        RestoreFPS();
    }
    gEngfuncs.pfnClientCmd("spk buttons/lightswitch2\n");
}

void HUD_Init(void)
{
    uwFPS_Init();
    if (g_pfnHUD_Init) g_pfnHUD_Init();
}

int HUD_VidInit(void)
{
    LoadMapList();
    s_fps_saved  = false;
    s_underwater = false;
    const char* levelname = gEngfuncs.pfnGetLevelName();
    if (levelname) s_map_allowed = IsMapAllowed(levelname);
    return g_pfnHUD_VidInit ? g_pfnHUD_VidInit() : 1;
}

void HUD_Frame(double time)
{
    if (g_pfnHUD_Frame) g_pfnHUD_Frame(time);
}

void V_CalcRefdef(struct ref_params_s* pparams)
{
    if (g_pfnV_CalcRefdef) g_pfnV_CalcRefdef(pparams);

    if (!IsCvarExists(uw_auto) || uw_auto->value == 0 || !s_map_allowed) return;

    if (pparams->spectator != 0 || pparams->health <= 0 || pparams->intermission != 0)
        return;

    float now = (float)gEngfuncs.GetClientTime();
    int threshold = IsCvarExists(uw_trigger_level) ? (int)uw_trigger_level->value : 3;
    bool in_water = pparams->waterlevel >= threshold;

    if (in_water != s_underwater) {
        s_enter_time = now;
        s_underwater = in_water;
    }

    float delay = IsCvarExists(uw_delay) ? uw_delay->value : 0.3f;
    if (now - s_enter_time < delay) return;

    if (s_underwater) {
        if (!s_fps_saved) {
            s_original_fps = gEngfuncs.pfnGetCvarFloat("fps_max");
            s_fps_saved = true;
            float target = IsCvarExists(uw_fps_underwater) ? uw_fps_underwater->value : 20.0f;
            char cmd[64];
            sprintf_s(cmd, "fps_max %.0f", target);
            gEngfuncs.pfnClientCmd(cmd);
        }
    } else {
        RestoreFPS();
    }
}