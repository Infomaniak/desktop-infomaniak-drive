/*
Infomaniak Drive Windows Explorer Extension DLL
Copyright (C) 2021 christophe.larchier@infomaniak.com
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
        VFS_PIN_STATE_UNSPECIFIED = 0,
        VFS_PIN_STATE_PINNED = 1,
        VFS_PIN_STATE_UNPINNED = 2,
        VFS_PIN_STATE_EXCLUDED = 3,
        VFS_PIN_STATE_INHERIT = 4
    } VfsPinState;

    DLL_EXP int __cdecl vfsInit(
        TraceCbk *traceCbk,
        const wchar_t *appName,
        const wchar_t* version,
        const wchar_t* trashURI);

    DLL_EXP int __cdecl vfsStart(
        const wchar_t *driveId,
        const wchar_t *userId,
        const wchar_t *folderId,
        const wchar_t *folderName,
        const wchar_t *folderPath,
        wchar_t *namespaceCLSID,
        DWORD *namespaceCLSIDSize);

    DLL_EXP int __cdecl vfsStop(
        const wchar_t *driveId,
        const wchar_t *folderId);

    DLL_EXP int __cdecl vfsGetPlaceHolderStatus(
        const wchar_t *filePath, 
        bool *isPlaceholder, 
        bool *isDehydrated,
        bool *isSynced);

    DLL_EXP int __cdecl vfsSetPlaceHolderStatus(
        const wchar_t *path,
        bool directory,
        bool inSync);

    DLL_EXP int __cdecl vfsDehydratePlaceHolder(
        const wchar_t *path);

    DLL_EXP int __cdecl vfsHydratePlaceHolder(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *path);

    DLL_EXP int __cdecl vfsCreatePlaceHolder(
        const wchar_t *fileId,
        const wchar_t *relativePath,
        const wchar_t *destPath,
        const WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl vfsConvertToPlaceHolder(
        const wchar_t *fileId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl vfsUpdatePlaceHolder(
        const wchar_t *filePath,
        const WIN32_FIND_DATA *findData);

    DLL_EXP int __cdecl vfsUpdateFetchStatus(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath,
        const wchar_t *fromFilePath,
        LONGLONG completed,
        bool *canceled,
        bool *finished);

    DLL_EXP int __cdecl vfsCancelFetch(
        const wchar_t *driveId,
        const wchar_t *folderId,
        const wchar_t *filePath);

    DLL_EXP int __cdecl vfsGetPinState(
        const wchar_t *path,
        bool directory,
        VfsPinState *state);

    DLL_EXP int __cdecl vfsSetPinState(
        const wchar_t *path,
        bool directory,
        VfsPinState state);
}
