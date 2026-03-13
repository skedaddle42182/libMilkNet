#ifndef _MILKNET_FILE_H
#define _MILKNET_FILE_H

#include "milknet_error.h"
#include "milknet_types.h"

milk_b8 milk_write_to_file(const char* path, const char* buffer);
milk_b8 milk_write_to_file_binary(const char* path, size_t element_size, size_t element_count, void* element);
milk_b8 milk_read_file(const char* path, const char* buffer, size_t buffer_size);
milk_b8 milk_read_file_binary(const char* path, size_t element_size, size_t element_count, void* element);

#endif // _MILKNET_FILE_H

