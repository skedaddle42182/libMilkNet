#ifndef _MILKNET_BUFFER_H
#define _MILKNET_BUFFER_H

#include <stdlib.h>
#include "milknet_error.h"
#include "milknet_types.h"

typedef struct _milk_dynamic_buffer
{
	size_t buffer_length;
	size_t buffer_capacity;
	uint8_t* buffer_data;
} milk_dynamic_buffer;

milk_b8 milk_initialize_buffer(milk_dynamic_buffer* buffer, size_t buffer_capacity);
milk_b8 milk_buffer_add_element(milk_dynamic_buffer* buffer, size_t element_size, void* element);
milk_b8 milk_reallocate_buffer(milk_dynamic_buffer* buffer, size_t element_size);
milk_b8 milk_clear_buffer(milk_dynamic_buffer* buffer, size_t element_size);
milk_b8 milk_remove_buffer_element(milk_dynamic_buffer* buffer, size_t index, size_t element_size);

#endif // _MILKNET_BUFFER_H