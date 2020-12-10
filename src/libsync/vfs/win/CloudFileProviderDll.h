/*
Infomaniak Drive Windows Explorer Extension DLL
Copyright (C) 2020 christophe.larchier@infomaniak.com
*/

#pragma once

#if defined(_WINDLL)
#define DLL_EXP __declspec(dllexport)
#else
#define DLL_EXP __declspec(dllimport)
#endif

extern "C" {
    // CF_PIN_STATE clone
    typedef enum {
        CFP_PIN_STATE_UNSPECIFIED = 0,
        CFP_PIN_STATE_PINNED = 1,
        CFP_PIN_STATE_UNPINNED = 2,
        CFP_PIN_STATE_EXCLUDED = 3,
        CFP_PIN_STATE_INHERIT = 4
    } CfpPinState;

    DLL_EXP int __cdecl cfpInitCloudFileProvider(
        TraceCbk *traceCbk,
        const wchar_t *appName);

    DLL_EXP int __cdecl cfpStartCloudFileProvider(
        const wchar_t *driveId,
        const wchar_t *userId,
        const wchar_t *folderId,
        const wchar_t *folderName,
        const wchar_t *folderPath,
        wchar_t *namespaceCLSID,
        DWORD *namespaceCLSIDSize);

    DLL_EXP int __cdecl cfpStopCloudFileProvider(
        const wchar_t *driveId,
        const wchar_t *folderId);

    DLL_EXP int __cdecl cfpGetPlaceHolderStatus(
        const wchar_t *filePath, 
        bool *isPlaceholder, 
        bool *isDehydrated,
        bool *isSynced,
        bool *isDirectory);

    DLL_EXP int __cdecl cfpSetPlaceHolderStatus(
        const wchar_t *path,
        bool directory,
        bool inSync);

    DLL_EXP int __cdecl cfpDehydratePlaceHolder(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *path);

    DLL_EXP int __cdecl cfpHydratePlaceHolder(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *path);

    DLL_EXP int __cdecl cfpCreatePlaceHolder(
        const wchar_t *relativePath,
        const wchar_t *destPath,
        WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl cfpConvertToPlaceHolder(
        const wchar_t *fileId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl cfpUpdateFetchStatus(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath,
        const wchar_t *fromFilePath,
        LONGLONG completed);

    DLL_EXP int __cdecl cfpCancelFetch(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl cfpGetPinState(
        const wchar_t *path,
        bool directory,
        CfpPinState *state);

    DLL_EXP int __cdecl cfpSetPinState(
        const wchar_t *path,
        bool directory,
        CfpPinState state);
}
