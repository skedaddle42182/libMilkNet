#ifndef _MILKNET_ERROR_H
#define _MILKNET_ERROR_H

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include "milknet_types.h"

#define MILK_DEFAULT_ERROR_BUFFER_SIZE 256

typedef enum _milk_log_type
{
    MILK_LOG_TYPE_WRITE_TO_CONSOLE = 1,
    MILK_LOG_TYPE_WRITE_TO_FILE = 2,
    MILK_LOG_TYPE_NONE = 3
} milk_log_type;

static milk_log_type static_log_type;

#define MILK_ASSERT(x, function_name, line_number)                                 \
    do                                                                             \
    {                                                                              \
        if (!(x))                                                                  \
        {                                                                          \
            milk_log_error((#x), function_name, line_number, static_log_type);     \
            return milk_false;                                                     \
        }                                                                          \
    } while (0)

milk_b8 milk_log_wsa_error(const char* error_message);

#endif // _MILKNET_ERROR_H