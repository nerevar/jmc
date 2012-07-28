#include <windows.h>

HINSTANCE __hDllInstance_base;
BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved)
{   

    switch (reason)
    {
        case DLL_PROCESS_ATTACH :
            __hDllInstance_base = hInst;
            break;
        default:
            break;
    }
    return TRUE;
}

