#pragma once

#include "DebugCbk.h"

#if defined(_WINDLL)
#define DLL_EXP __declspec(dllexport)
#else
#define DLL_EXP __declspec(dllimport)
#endif

extern "C" {
    DLL_EXP int __cdecl StartCloudFileProvider(
        const wchar_t *serverFolder,
        const wchar_t *clientFolder,
        TraceCbk *traceCbk);

    DLL_EXP int __cdecl StopCloudFileProvider();
}
