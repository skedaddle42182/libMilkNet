#include "milknet_buffer.h"

milk_b8 milk_initialize_buffer(milk_dynamic_buffer* buffer, size_t buffer_capacity)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	if (!buffer_capacity)
	{
		return milk_false;
	}

	buffer->buffer_capacity = buffer_capacity;
	buffer->buffer_data = (uint8_t*)malloc(buffer_capacity * sizeof(uint8_t));

	return milk_true;
}

milk_b8 milk_buffer_add_element(milk_dynamic_buffer* buffer, size_t element_size, void* element)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	if (!element_size || !element)
	{
		return milk_false;
	}

	if (buffer->buffer_length >= buffer->buffer_capacity)
	{
		if (!milk_reallocate_buffer(buffer, element_size))
		{
			return milk_false;
		}
	}

	memcpy(buffer->buffer_data + (buffer->buffer_length * element_size), element, element_size);
	buffer->buffer_length++;

	return milk_true;
}

milk_b8 milk_reallocate_buffer(milk_dynamic_buffer* buffer, size_t element_size)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	if (!element_size)
	{
		return milk_false;
	}

	buffer->buffer_capacity *= 2;

	void* temp = realloc(buffer->buffer_data, (element_size * buffer->buffer_capacity));

	MILK_ASSERT(temp != NULL, __FUNCTION__, __LINE__);

	buffer->buffer_data = (uint8_t*)temp;

	return milk_true;
}

milk_b8 milk_clear_buffer(milk_dynamic_buffer* buffer, size_t element_size)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	if (!element_size)
	{
		return milk_false;
	}

	memset(buffer->buffer_data, 0, buffer->buffer_length * element_size);

	return milk_true;
}

milk_b8 milk_remove_buffer_element(milk_dynamic_buffer* buffer, size_t index, size_t element_size)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	if (!index || index >= buffer->buffer_length || !element_size)
	{
		return milk_false;
	}

	memmove(buffer->buffer_data + index, buffer->buffer_data + index + 1, ((buffer->buffer_length - index - 1) * element_size));
	buffer->buffer_length--;

	return milk_true;
}