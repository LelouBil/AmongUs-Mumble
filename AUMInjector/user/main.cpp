// Generated C++ file by Il2CppInspector - http://www.djkaty.com - https://github.com/djkaty
// Custom injected code entry point

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include "il2cpp-init.h"
#include "il2cpp-appdata.h"
#include "helpers.h"
#include <detours.h>
#include "mumble-link.h"


using namespace app;

extern const LPCWSTR LOG_FILE = L"il2cpp-log.txt";
extern HANDLE hExit; // Thread exit event
extern std::string mumble_exe;

// Game state
bool alive = true;
bool voting = false;
InnerNetClient_IFLBIJFJPMK__Enum last_game_state = InnerNetClient_IFLBIJFJPMK__Enum_NotJoined;


// Fixed loop for a player object
void PlayerControl_FixedUpdate_Hook(PlayerControl* __this, MethodInfo* method)
{
    PlayerControl_FixedUpdate(__this, method);
    
    if (lm != NULL && __this->fields.LightPrefab != nullptr && !voting)
    {
        app::Vector2 pos = PlayerControl_GetTruePosition(__this, method);
        writeMumble();
        lm->fAvatarPosition[0] = pos.x;
        lm->fCameraPosition[0] = pos.x;
        lm->fAvatarPosition[2] = pos.y;
        lm->fCameraPosition[2] = pos.y;
        lm->uiTick++;
    }
}

// Gets called when a player dies
void PlayerControl_Die_Hook(PlayerControl* __this, NPLMBOLMMLB__Enum NMGPLGPEHPP, MethodInfo* method)
{
    PlayerControl_Die(__this, NMGPLGPEHPP, method);

    if (__this->fields.LightPrefab != nullptr)
    {
        printf("You died\n");
        muteMumble(true);
        alive = false;
    }
}

// Gets called when a meeting ends
void MeetingHud_Close_Hook(MeetingHud* __this, MethodInfo* method)
{
    MeetingHud_Close(__this, method);
    printf("Meeting ended\n");
    voting = false;
}

// Gets called when a meeting starts
void MeetingHud_Start_Hook(MeetingHud* __this, MethodInfo* method)
{
    MeetingHud_Start(__this, method);
    printf("Meeting started\n");
    voting = true;
}

// Fixed loop for the game client
void InnerNetClient_FixedUpdate_Hook(InnerNetClient* __this, MethodInfo* method)
{
    InnerNetClient_FixedUpdate(__this, method);

    // Check if game state changed to lobby
    if (__this->fields.GameState != last_game_state &&
            (__this->fields.GameState == InnerNetClient_IFLBIJFJPMK__Enum_Joined ||
            __this->fields.GameState == InnerNetClient_IFLBIJFJPMK__Enum_Ended))
    {
        printf("Game joined or ended");
        muteMumble(false);
        alive = true;
    }
    last_game_state = __this->fields.GameState;

    // When voting, all players can hear each other -> same position
    if (voting)
    {
        writeMumble();
        lm->fAvatarPosition[0] = 0.0f;
        lm->fCameraPosition[0] = 0.0f;
        lm->fAvatarPosition[2] = 0.0f;
        lm->fCameraPosition[2] = 0.0f;
        lm->uiTick++;
    }
}

// Entrypoint of the injected thread
void Run()
{
    NewConsole();
    printf("AmongUs-Mumble mod by StarGate01 (chrz.de)\nDLL hosting successfull\n\n");

    // Setup mumble
    int lErrMumble = initMumble();
    if (lErrMumble == NO_ERROR)
    {
        printf("Mumble link init successfull\n");
        printf("Mumble exe: %s\n", mumble_exe.c_str());
    }
    else printf("Cannot init Mumble link: %d\n", lErrMumble);
    
    // Setup type and memory info
    printf("Waiting 10s for Unity to load\n");
    Sleep(10000);
    init_il2cpp();
    printf("Type and function memory mapping successfull\n");
    
    // Setup hooks
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)PlayerControl_FixedUpdate, PlayerControl_FixedUpdate_Hook);
    DetourAttach(&(PVOID&)PlayerControl_Die, PlayerControl_Die_Hook);
    DetourAttach(&(PVOID&)MeetingHud_Close, MeetingHud_Close_Hook);
    DetourAttach(&(PVOID&)MeetingHud_Start, MeetingHud_Start_Hook);
    DetourAttach(&(PVOID&)InnerNetClient_FixedUpdate, InnerNetClient_FixedUpdate_Hook);
    LONG lError = DetourTransactionCommit();
    if (lError == NO_ERROR) printf("Successfully detoured game functions\n\n");
    else printf("Detouring game functions failed: %d\n", lError);

    // Wait for thread exit and then clean up
    WaitForSingleObject(hExit, INFINITE);
    closeMumble();
    printf("Unloading done\n");
}