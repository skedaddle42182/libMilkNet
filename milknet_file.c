#include "milknet_file.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

milk_b8 milk_write_to_file(const char* path, const char* buffer)
{
	if (!path)
	{
		return milk_false;
	}

	FILE* ptr;

	ptr = fopen(path, "w");

	MILK_ASSERT(ptr != NULL, __FUNCTION__, __LINE__);

	fputs(buffer, ptr);

	fclose(ptr);

	return milk_true;
}

milk_b8 milk_write_to_file_binary(const char* path, size_t element_size, size_t element_count, void* element)
{
	if (!path || !element_size || !element_count || !element)
	{
		return milk_false;
	}

	FILE* ptr;

	ptr = fopen(path, "wb");

	MILK_ASSERT(ptr != NULL, __FUNCTION__, __LINE__);

	fwrite(element, element_size, element_count, ptr);

	fclose(ptr);

	return milk_true;
}

milk_b8 milk_read_file(const char* path, const char* buffer, size_t buffer_size)
{
	if (!path || !buffer_size)
	{
		return milk_false;
	}

	FILE* ptr;

	ptr = fopen(path, "r");

	MILK_ASSERT(ptr != NULL, __FUNCTION__, __LINE__);

	if (fgets(buffer, buffer_size, ptr) != NULL)
	{
		puts(buffer);
	}

	fclose(ptr);

	return milk_true;
}

milk_b8 milk_read_file_binary(const char* path, size_t element_size, size_t element_count, void* element)
{
	if (!path || !element_size || !element_count)
	{
		return milk_false;
	}

	FILE* ptr;

	ptr = fopen(path, "rb");

	MILK_ASSERT(ptr != NULL, __FUNCTION__, __LINE__);

	fread(element, element_size, element_count, ptr);

	fclose(ptr);

	return milk_true;
}

