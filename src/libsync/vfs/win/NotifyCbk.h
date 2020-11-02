#pragma once

typedef enum {
	NOTIFICATION_TYPE_FETCH_DATA = 0,
	NOTIFICATION_TYPE_CANCEL_FETCH_DATA
} NotificationType;

typedef enum {
	FETCH_STATUS_NONE,
	FETCH_STATUS_SYNC,
	FETCH_STATUS_WARNING,
	FETCH_STATUS_UP_TO_DATE,
	FETCH_STATUS_ERROR,
	FETCH_STATUS_EXCLUDED
} FetchStatus;

// Type definition for the debug callback function.
typedef void (NotifyCbk)(NotificationType type, const wchar_t *filePath);
