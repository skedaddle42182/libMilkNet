#include "milknet_error.h"
#include "milknet_file.h" 

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

milk_b8 milk_log_error(const char* expression, const char* function_name, const char* line_number, milk_log_type log_type)
{
    char milk_error_message[MILK_DEFAULT_ERROR_BUFFER_SIZE];

    sprintf(milk_error_message, sizeof(milk_error_message), "[+] LOG: function_name: %s, line_number: %s, expression: %s", function_name, line_number, expression);

    switch (log_type)
    {
        case MILK_LOG_TYPE_NONE:
            break;
        case MILK_LOG_TYPE_WRITE_TO_FILE:
            if (!milk_write_to_file("milk_error_log.txt", milk_error_message))
            {
                return milk_false;
            }
            break;
        case MILK_LOG_TYPE_WRITE_TO_CONSOLE:
            printf("[+] LOG: function_name: %s, line_number: %s, expression: %s", function_name, line_number, expression);
            break;
    }

    return milk_true;
}

milk_b8 milk_log_wsa_error(const char* error_message)
{
    printf("%s: %ld\n", error_message, WSAGetLastError());
    return milk_true;
}