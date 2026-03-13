#include "milknet.h"

milk_b8 milk_set_array(void* array, size_t element_size, size_t element_count, void* value)
{
	MILK_ASSERT(element_size != 0, __FUNCTION__, __LINE__);
	MILK_ASSERT(element_count != 0, __FUNCTION__, __LINE__);

	for (size_t i = 0; i < element_count; i++) {
		void* target = (char*)array + (i * element_size);
		memcpy(target, value, element_size);
	}

	return milk_true;
}

milk_b8 milk_initialize_wsa(int version_param_one, int version_param_two)
{
	MILK_ASSERT(version_param_one != 0, __FUNCTION__, __LINE__);
	MILK_ASSERT(version_param_two != 0, __FUNCTION__, __LINE__);

	WSADATA data;

	int result = WSAStartup(MAKEWORD(version_param_one, version_param_two), &data);

	MILK_ASSERT(result == 0, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_initialize_context(milk_context* context, milk_socket_manager* socket_manager, size_t client_count, size_t server_count, size_t listening_count, int version_param_one, int version_param_two, milk_log_type log_type)
{
	MILK_ASSERT(client_count != 0, __FUNCTION__, __LINE__); 
	MILK_ASSERT(server_count != 0, __FUNCTION__, __LINE__);
	MILK_ASSERT(listening_count != 0, __FUNCTION__, __LINE__);

	milk_initialize_wsa(version_param_one, version_param_two);

	context->thread_pool = milk_create_thread_pool(MAX_THREADS, MAX_QUEUE);

	socket_manager->client_sockets->socket_count = client_count;
	socket_manager->server_sockets->socket_count = server_count;
	socket_manager->listening_sockets->socket_count = listening_count;

	socket_manager->client_sockets->sockets = malloc(socket_manager->client_sockets->socket_count * sizeof(SOCKET));
	socket_manager->server_sockets->sockets = malloc(socket_manager->server_sockets->socket_count * sizeof(SOCKET));
	socket_manager->listening_sockets->sockets = malloc(socket_manager->listening_sockets->socket_count * sizeof(SOCKET));

	MILK_ASSERT(socket_manager->listening_sockets->sockets != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(socket_manager->server_sockets->sockets != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(socket_manager->client_sockets->sockets != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(context->thread_pool != NULL, __FUNCTION__, __LINE__);

	static_log_type = log_type;

	return milk_true;
}

milk_b8 milk_initialize_socket(milk_socket* result, milk_socket_type type)
{
	switch (type)
	{
		case MILK_SOCKET_TYPE_DATAGRAM:
		{
			*result = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			MILK_ASSERT(*result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return milk_true;
		}
		case MILK_SOCKET_TYPE_STREAM:
		{
			*result = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			MILK_ASSERT(*result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return  milk_true;
		}
		case MILK_SOCKET_TYPE_RAW:
		{
			*result = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
			MILK_ASSERT(*result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return  milk_true;
		}
		default:
			return milk_false;
	}

	return milk_true;
}

milk_b8 milk_bind_socket(milk_socket socket, const unsigned short port, const char* ip)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);
	MILK_ASSERT(ip != NULL, __FUNCTION__, __LINE__);


	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
	{
		return milk_false;
	}

	int result = bind(socket, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_listen_socket(milk_socket socket, const unsigned int max_clients)
{
	MILK_ASSERT(max_clients != 0, __FUNCTION__, __LINE__);

	int result = listen(socket, max_clients);
	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_accept_socket(milk_socket* client_socket, milk_socket server_socket)
{
	*client_socket = accept(server_socket, NULL, NULL);
	MILK_ASSERT(*client_socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_shutdown_socket(milk_socket socket)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	int result = shutdown(socket, SD_SEND);
	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_close_socket(milk_socket socket)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	int result = closesocket(socket);
	MILK_ASSERT(result == 0, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_close_wsa()
{
	int result = WSACleanup();

	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_cleanup_context(milk_context* context, milk_socket_manager* socket_manager)
{
	milk_b8 is_successful = milk_true;

	MILK_ASSERT(socket_manager->client_sockets != INVALID_SOCKET, __FUNCTION__, __LINE__);
	MILK_ASSERT(context != NULL, __FUNCTION__, __LINE__);

	if (!milk_thread_pool_destroy(context->thread_pool, MILK_THREAD_POOL_GRACEFUL))
	{
		is_successful = milk_false;
		context->thread_pool = NULL;
	}

	for (int i = 0; i < socket_manager->client_sockets->socket_count; i++)
	{
		if (!milk_socket_free(&socket_manager->client_sockets->sockets[i]))
		{
			is_successful = milk_false;
			continue;
		}
	}

	for (int i = 0; i < socket_manager->server_sockets->socket_count; i++)
	{
		if (!milk_socket_free(&socket_manager->server_sockets->sockets[i]))
		{
			is_successful = milk_false;
			continue;
		}
	}

	for (int i = 0; i < socket_manager->listening_sockets->socket_count; i++)
	{
		if (!milk_socket_free(&socket_manager->listening_sockets->sockets[i]))
		{
			is_successful = milk_false;
			continue;
		}
	}

	free(socket_manager->client_sockets->sockets);
	free(socket_manager->server_sockets->sockets);
	free(socket_manager->listening_sockets->sockets);

	socket_manager->client_sockets->sockets = NULL;
	socket_manager->server_sockets->sockets = NULL;
	socket_manager->listening_sockets->sockets = NULL;

	socket_manager->client_sockets->socket_count = 0;
	socket_manager->server_sockets->socket_count = 0;
	socket_manager->listening_sockets->socket_count = 0;

	free(socket_manager);

	milk_close_wsa();

	return is_successful;
}

milk_b8 milk_send_socket(milk_socket socket, milk_dynamic_buffer* buffer, int flags)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);

	int total_sent = 0;
	int result = 0;

	while (total_sent < buffer->buffer_length)
	{
		result = send(socket, (const char*)buffer->buffer_data + total_sent, buffer->buffer_length - total_sent, flags);
		MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);
		total_sent += result;
	}

	return milk_true;
}

milk_b8 milk_connect_socket(milk_socket socket, const unsigned short port, const char* ip)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);
	MILK_ASSERT(port != 0, __FUNCTION__, __LINE__);
	MILK_ASSERT(ip != NULL, __FUNCTION__, __LINE__);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
	{
		return milk_false;
	}

	int result = connect(socket, (const struct sockaddr*)&addr, sizeof(struct sockaddr_in));
	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_socket_forward(milk_socket client_socket, milk_socket server_socket, milk_b8 from_client, fd_set* read_fds, fd_set* write_fds, milk_dynamic_buffer* buffer, int timeout_ms, size_t expected_receive_size)
{
	MILK_ASSERT(client_socket != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(server_socket != NULL, __FUNCTION__, __LINE__);

	SOCKET source_socket = from_client ? client_socket : server_socket;
	SOCKET destination_socket = from_client ? server_socket : client_socket;

	if (!milk_wait_socket(source_socket, MILK_SOCKET_WAIT_READ, timeout_ms))
	{
		return milk_true;
	}

	if (milk_receive_socket(source_socket, buffer, expected_receive_size))
	{
		if (!milk_send_socket(destination_socket, buffer, 0))
		{
			return milk_false;
		}
	}

	return milk_true;
}

milk_b8 milk_socket_set_unblocking(milk_socket socket)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	int result = milk_socket_change_control(socket, FIONBIO, 1);

	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_socket_change_control(milk_socket socket, long cmd, u_long argp)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	int result = ioctlsocket(socket, cmd, &argp);
	MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_socket_create_connect(milk_socket* server_socket, const unsigned short port, const char* ip)
{
	if (!milk_initialize_socket(server_socket, MILK_SOCKET_TYPE_STREAM))
	{
		return milk_false;
	}
	if (!milk_connect_socket(server_socket, port, ip))
	{
		return milk_false;
	}
	if (!milk_socket_set_unblocking(server_socket))
	{
		return milk_false;
	}

	return milk_true;
}

milk_b8 milk_add_new_client(milk_thread_args* thread_args)
{
	MILK_ASSERT(thread_args != NULL, __FUNCTION__, __LINE__);

	SOCKET* temp = NULL;

	if (!thread_args->socket_manager->client_sockets->socket_capacity)
	{
		thread_args->socket_manager->client_sockets->socket_capacity = MILK_DEFAULT_SOCKET_LIST_CAPACITY;
		thread_args->socket_manager->client_sockets->sockets = (milk_socket*)malloc(thread_args->socket_manager->client_sockets->socket_capacity * sizeof(milk_socket));

		MILK_ASSERT(thread_args->socket_manager->client_sockets->sockets != NULL, __FUNCTION__, __LINE__);
	}

	if (thread_args->socket_manager->client_sockets->socket_count >= thread_args->socket_manager->client_sockets->socket_capacity)
	{
		temp = (SOCKET*)realloc(thread_args->socket_manager->client_sockets->sockets, ((thread_args->socket_manager->client_sockets->socket_capacity * 2) * sizeof(SOCKET)));

		MILK_ASSERT(temp != NULL, __FUNCTION__, __LINE__);

		thread_args->socket_manager->client_sockets->sockets = temp;
	}

	thread_args->socket_manager->client_sockets->sockets[thread_args->socket_manager->client_sockets->socket_count] = thread_args->client_socket;

	(thread_args->socket_manager->client_sockets->socket_count)++;

	return milk_true;
}

milk_b8 milk_accept_new_client(milk_socket_manager* manager, milk_socket* listening_socket, fd_set* read_fds)
{
	MILK_ASSERT(read_fds != NULL, __FUNCTION__, __LINE__);

	if (FD_ISSET(*listening_socket, read_fds))
	{
		SOCKET incomingClient = INVALID_SOCKET;
		milk_accept_socket(&incomingClient, listening_socket);
		MILK_ASSERT(incomingClient != INVALID_SOCKET, __FUNCTION__, __LINE__);

		if (!milk_socket_set_unblocking(incomingClient))
		{
			milk_close_socket(incomingClient);
			return milk_false;
		}

		milk_thread_args* thread_args = (milk_thread_args*)malloc(sizeof(milk_thread_args));

		MILK_ASSERT(thread_args != NULL, __FUNCTION__, __LINE__);

		thread_args->client_socket = incomingClient;
		thread_args->socket_manager = manager;

		milk_add_new_client(thread_args);

		free(thread_args);
	}

	return milk_true;
}

milk_b8 milk_remove_client(milk_socket_manager* socket_manager, size_t index)
{
	MILK_ASSERT(socket_manager->client_sockets != NULL, __FUNCTION__, __LINE__);

	MILK_ASSERT(!(index > socket_manager->client_sockets->socket_count - 1), __FUNCTION__, __LINE__);

	for (size_t i = index; i < socket_manager->client_sockets->socket_count - 1; i++)
	{
		socket_manager->client_sockets->sockets[i] = socket_manager->client_sockets->sockets[i + 1];
	}

	(socket_manager->client_sockets->socket_count)--;

	return milk_true;
}

milk_b8 milk_serialize_data(void* element, size_t element_size, milk_dynamic_buffer* buffer)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(element != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(element_size != 0, __FUNCTION__, __LINE__);

	memcpy(buffer->buffer_data, element, element_size);

	MILK_ASSERT(buffer->buffer_data != NULL, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_deserialize_data(milk_dynamic_buffer* buffer, size_t element_size, void* element)
{
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(element != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(element_size != 0, __FUNCTION__, __LINE__);

	memcpy(element, buffer->buffer_data, element_size);

	MILK_ASSERT(element != NULL, __FUNCTION__, __LINE__);

	return milk_true;
}

milk_b8 milk_receive_socket(milk_socket socket, milk_dynamic_buffer* buffer, size_t expected_received_size)
{
	MILK_ASSERT(socket != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(expected_received_size != 0, __FUNCTION__, __LINE__);

	size_t total_received_size = 0;

	while (total_received_size < expected_received_size)
	{
		if (buffer->buffer_length + (expected_received_size - total_received_size) > buffer->buffer_capacity)
		{
			milk_reallocate_buffer(buffer, 1);
		}

		int result = recv(socket, (buffer->buffer_data + buffer->buffer_length), (expected_received_size - total_received_size), 0);
		MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

		buffer->buffer_length += result;
		total_received_size += result;
	}

	return milk_true;
}

milk_b8 milk_send_received_data_to_all_clients(void* data, size_t data_size, milk_socket_manager* socket_manager, milk_dynamic_buffer* buffer, int timeout_ms, size_t expected_receive_size)
{
	MILK_ASSERT(data != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(socket_manager->client_sockets->sockets != INVALID_SOCKET, __FUNCTION__, __LINE__);
	MILK_ASSERT(buffer != NULL, __FUNCTION__, __LINE__);
	MILK_ASSERT(data_size != 0, __FUNCTION__, __LINE__);

	size_t sender_index = 0;

	for (size_t i = 0; i < socket_manager->client_sockets->socket_count; i++)
	{
		if (milk_wait_socket(socket_manager->client_sockets->sockets[i], MILK_SOCKET_WAIT_READ, timeout_ms))
		{
			MILK_ASSERT(milk_receive_socket(socket_manager->client_sockets->sockets[i], buffer, expected_receive_size), __FUNCTION__, __LINE__);
			sender_index = i;
		}
	}

	MILK_ASSERT(milk_serialize_data(data, data_size, buffer), __FUNCTION__, __LINE__);

	for (size_t j = 0; j < socket_manager->client_sockets->socket_count; j++)
	{
		if (j == sender_index) { continue; }
		MILK_ASSERT(milk_send_socket(socket_manager->client_sockets->sockets[j], buffer, 0), __FUNCTION__, __LINE__);
	}

	return milk_true;
}

milk_b8 milk_wait_socket(milk_socket socket, milk_socket_wait option, int timeout_ms)
{
	int result;
	milk_b8 read_set;
	milk_b8 write_set;

	fd_set read_fds;
	fd_set write_fds;

	struct timeval tv;

	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	if (timeout_ms < 0)
	{
		return milk_false;
	}

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	read_set = milk_false;
	write_set = milk_false;

	switch (option)
	{
	case MILK_SOCKET_WAIT_READ:
	{
		FD_ZERO(&read_fds);
		FD_SET(socket, &read_fds);

		result = select(socket + 1, &read_fds, NULL, NULL, &tv);

		MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

		read_set = FD_ISSET(socket, &read_fds);

		return read_set;
	}
	case MILK_SOCKET_WAIT_WRITE:
	{
		FD_ZERO(&write_fds);
		FD_SET(socket, &write_fds);

		result = select(socket + 1, NULL, &write_fds, NULL, &tv);

		MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

		write_set = FD_ISSET(socket, &write_fds);

		return write_set;
	}
	case MILK_SOCKET_WAIT_READ_WRITE:
	{
		FD_ZERO(&write_fds);
		FD_ZERO(&read_fds);
		FD_SET(socket, &write_fds);
		FD_SET(socket, &read_fds);

		result = select(socket + 1, &read_fds, &write_fds, NULL, &tv);
		MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);

		write_set = FD_ISSET(socket, &write_fds);
		read_set = FD_ISSET(socket, &read_fds);

		return write_set || read_set;
	}
	default:
		return milk_false;
		break;
	}

	return milk_true;
}

milk_b8 milk_socket_free(milk_socket* socket)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	milk_socket_shutdown(socket, MILK_SOCKET_SHUTDOWN_READ_WRITE);
	milk_close_socket(*socket);

	return milk_true;
}

milk_b8 milk_socket_shutdown(milk_socket* socket, milk_shutdown how)
{
	MILK_ASSERT(socket != INVALID_SOCKET, __FUNCTION__, __LINE__);

	int result = 0;

	switch (how)
	{
		case MILK_SOCKET_SHUTDOWN_READ:
		{
			result = shutdown(*socket, 0);
			MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return milk_true;
		}
		case MILK_SOCKET_SHUTDOWN_WRITE:
		{
			result = shutdown(*socket, 1);
			MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return milk_true;
		}
		case MILK_SOCKET_SHUTDOWN_READ_WRITE:
		{
			result = shutdown(*socket, 2);
			MILK_ASSERT(result != SOCKET_ERROR, __FUNCTION__, __LINE__);
			return milk_true;
		}
		default:
			return milk_false;
	}

	return milk_true;
}
