#pragma once

#include "DebugCbk.h"
#include "NotifyCbk.h"

#include "windows.h"

#if defined(_WINDLL)
#define DLL_EXP __declspec(dllexport)
#else
#define DLL_EXP __declspec(dllimport)
#endif

extern "C" {
    DLL_EXP int __cdecl CFPInitCloudFileProvider(
        TraceCbk *traceCbk,
        NotifyCbk *notifyCbk,
        const wchar_t *appName);

    DLL_EXP int __cdecl CFPStartCloudFileProvider(
        const wchar_t *driveID,
        const wchar_t *driveAlias,
        const wchar_t *userID,
        const wchar_t *clientFolder);

    DLL_EXP int __cdecl CFPStopCloudFileProvider(const wchar_t *driveID);

    DLL_EXP bool __cdecl CFPIsHydrating(const wchar_t *driveID);

    DLL_EXP int __cdecl CFPCreatePlaceHolder(
        const wchar_t *fileId,
        const wchar_t *sourcePath,
        const wchar_t *destPath,
        WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl CFPUpdateFetchStatus(
        const wchar_t *driveID,
        const wchar_t *filePath,
        const wchar_t *fromFilePath,
        FetchStatus status,
        LONGLONG completed);
}
