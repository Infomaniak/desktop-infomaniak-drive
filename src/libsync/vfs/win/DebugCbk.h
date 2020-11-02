#pragma once

typedef enum {
	TRACE_LEVEL_INFO = 0,
	TRACE_LEVEL_DEBUG,
	TRACE_LEVEL_WARNING,
	TRACE_LEVEL_ERROR
} TraceLevel;

// Type definition for the debug callback function.
typedef void (TraceCbk)(TraceLevel level, const wchar_t *msg);
