#pragma once

#include "DebugCbk.h"

#include "windows.h"

#if defined(_WINDLL)
#define DLL_EXP __declspec(dllexport)
#else
#define DLL_EXP __declspec(dllimport)
#endif

extern "C" {
    // CF_PIN_STATE clone
    typedef enum CFP_PIN_STATE {
        CFP_PIN_STATE_UNSPECIFIED = 0,
        CFP_PIN_STATE_PINNED = 1,
        CFP_PIN_STATE_UNPINNED = 2,
        CFP_PIN_STATE_EXCLUDED = 3,
        CFP_PIN_STATE_INHERIT = 4
    } CFP_PIN_STATE;

    DLL_EXP int __cdecl CFPInitCloudFileProvider(
        TraceCbk *traceCbk,
        const wchar_t *appName);

    DLL_EXP int __cdecl CFPStartCloudFileProvider(
        const wchar_t *driveId,
        const wchar_t *userId,
        const wchar_t *folderId,
        const wchar_t *folderName,
        const wchar_t *folderPath,
        wchar_t *namespaceCLSID,
        DWORD *namespaceCLSIDSize);

    DLL_EXP int __cdecl CFPStopCloudFileProvider(
        const wchar_t *driveId,
        const wchar_t *folderId);

    DLL_EXP int __cdecl CFPGetPlaceHolderStatus(
        const wchar_t *filePath, 
        bool *isPlaceholder, 
        bool *isDehydrated,
        bool *isDirectory);

    DLL_EXP int __cdecl CFPSetPlaceHolderStatus(
        const wchar_t *filePath,
        bool inSync);

    DLL_EXP int __cdecl CFPDehydratePlaceHolder(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl CFPHydratePlaceHolder(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl CFPCreatePlaceHolder(
        const wchar_t *fileId,
        const wchar_t *destPath,
        WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl CFPConvertToPlaceHolder(
        const wchar_t *fileId,
        const wchar_t *destPath,
        WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl CFPUpdateFetchStatus(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath,
        const wchar_t *fromFilePath,
        LONGLONG completed);

    DLL_EXP int __cdecl CFPCancelFetch(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl CFPGetPinState(
        const wchar_t *filePath,
        CFP_PIN_STATE *state);

    DLL_EXP int __cdecl CFPSetPinState(
        const wchar_t *filePath,
        CFP_PIN_STATE state);
}
