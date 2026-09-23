#include "plugins.h"
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

static float s_enter_time    = 0.f;
static bool  s_underwater    = false;
static bool  s_map_allowed   = true;
static float s_original_fps  = 100.0f;
static bool  s_fps_saved     = false;
static bool  s_crash_checked = false;
static std::vector<std::string> s_map_list;

// -----------------------------------------------------------------------------
// Пишем в cvar напрямую, минуя командный буфер: pfnClientCmd может не
// исполниться во время смены карты / ExitGame, Cvar_SetValue работает всегда.
// -----------------------------------------------------------------------------
static void SetFpsMax(float value)
{
    if (gEngfuncs.Cvar_SetValue)
        gEngfuncs.Cvar_SetValue((char*)"fps_max", value);
}

static void SetSavedFpsCvar(float value)
{
    if (gEngfuncs.Cvar_SetValue)
        gEngfuncs.Cvar_SetValue((char*)"uw_saved_fps", value);
}

static float GetPositiveCvarValue(const cvar_t* cv, float fallback)
{
    return (IsCvarExists(cv) && cv->value > 0.0f) ? cv->value : fallback;
}

void LoadMapList(void)
{
    s_map_list.clear();
    std::string path = std::string(g_pMetaHookAPI->GetGameDirectory()) + "/uwFPS/maps.ini";
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        size_t last = line.find_last_not_of(" \t\r\n");
        std::string map = line.substr(first, last - first + 1);
        if (map.empty() || map[0] == ';' || map[0] == '#') continue;
        s_map_list.push_back(map);
    }
}

void RestoreFPS(void)
{
    if (!s_fps_saved) return;
    float target = GetPositiveCvarValue(uw_fps_normal, s_original_fps);
    SetFpsMax(target);
    SetSavedFpsCvar(0.0f);   // бэкап больше не нужен
    s_fps_saved = false;
}

// Любой "неактивный" выход: восстановить FPS (если зажат) и сбросить состояние.
static void Deactivate(void)
{
    if (s_fps_saved) RestoreFPS();
    s_underwater = false;
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
    if (gEngfuncs.Cvar_SetValue)
        gEngfuncs.Cvar_SetValue((char*)"uw_auto", newVal);

    if (newVal > 0) {
        if (gEngfuncs.pfnCenterPrint)
            gEngfuncs.pfnCenterPrint("uwFPS: ENABLED");
    } else if (gEngfuncs.pfnCenterPrint) {
        gEngfuncs.pfnCenterPrint("uwFPS: DISABLED");
        Deactivate();
    } else {
        Deactivate();
    }
    if (gEngfuncs.pfnClientCmd)
        gEngfuncs.pfnClientCmd("spk buttons/lightswitch2\n");
}

void HUD_Init(void)
{
    uwFPS_Init();
    if (g_pfnHUD_Init) g_pfnHUD_Init();
}

int HUD_VidInit(void)
{
    // ВАЖНО: восстановить ДО сброса состояния, иначе оригинальный fps_max
    // теряется при смене карты под водой.
    RestoreFPS();

    LoadMapList();
    s_underwater = false;
    const char* levelname = gEngfuncs.pfnGetLevelName();
    if (levelname) s_map_allowed = IsMapAllowed(levelname);
    return g_pfnHUD_VidInit ? g_pfnHUD_VidInit() : 1;
}

// Вызывается движком при рестарте карты / принудительном сбросе клиента.
void HUD_Reset(void)
{
    Deactivate();
    if (g_pfnHUD_Reset) g_pfnHUD_Reset();
}

void HUD_Frame(double time)
{
    // Одноразовая проверка после запуска игры: если прошлая сессия упала,
    // пока fps был зажат, uw_saved_fps > 0 придёт из config.cfg — вернуть.
    // Делается здесь, а не в uwFPS_Init: на момент HUD_Init архивные значения
    // из config.cfg ещё не применены.
    if (!s_crash_checked) {
        s_crash_checked = true;
        if (IsCvarExists(uw_saved_fps) && uw_saved_fps->value > 0) {
            SetFpsMax(uw_saved_fps->value);
            SetSavedFpsCvar(0.0f);
            gEngfuncs.Con_Printf((char*)"uwFPS: restored fps_max after abnormal shutdown\n");
        }
    }

    // Страховка на дисконнект: HUD_VidInit при выходе в меню не вызывается,
    // а HUD_Frame тикает и в меню.
    if (s_fps_saved) {
        const char* lvl = gEngfuncs.pfnGetLevelName();
        if (!lvl || !lvl[0])
            RestoreFPS();
    }

    if (g_pfnHUD_Frame) g_pfnHUD_Frame(time);
}

void V_CalcRefdef(struct ref_params_s* pparams)
{
    if (g_pfnV_CalcRefdef) g_pfnV_CalcRefdef(pparams);

    if (!pparams) {
        Deactivate();
        return;
    }

    if (!IsCvarExists(uw_auto) || !(uw_auto->value > 0.0f) || !s_map_allowed) {
        Deactivate();
        return;
    }

    // Мёртв / HLTV-спектатор / интермиссия — раньше тут был голый return
    // и fps застревал, если умереть под водой.
    if (pparams->spectator != 0 || pparams->health <= 0 || pparams->intermission != 0) {
        Deactivate();
        return;
    }

    // Камера не на локальном игроке: серверный SetView (скрипты вида .view /
    // ghostjail с камерой на ghost-энтити), trigger_camera и т.п.
    // iuser1 хранит OBS_* режим; смерть может переключить observer раньше,
    // чем ref_params::health/spectator успеют стабильно обновиться.
    // Заодно закрывает обратный случай: тело игрока в воде, а камера — нет.
    cl_entity_t* local = gEngfuncs.GetLocalPlayer();
    bool local_inactive = local && (local->curstate.iuser1 != OBS_NONE || local->curstate.spectator != 0);
    if (!local || pparams->viewentity != local->index || local_inactive) {
        Deactivate();
        return;
    }

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
            SetSavedFpsCvar(s_original_fps);   // бэкап на случай краша
            float target = GetPositiveCvarValue(uw_fps_underwater, 20.0f);
            SetFpsMax(target);
        }
    } else {
        RestoreFPS();
    }
}
