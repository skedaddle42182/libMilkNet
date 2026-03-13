#ifndef _MILKNET_H
#define _MILKNET_H

#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include "milknet_types.h"
#include "milknet_error.h"
#include "milknet_file.h"
#include "milknet_threadpool.h"
#include "milknet_buffer.h"

#pragma comment (lib, "Ws2_32.lib")

typedef SOCKET milk_socket;

#define MILK_HOST_TO_NET_16 htons
#define MILK_HOST_TO_NET_32 htonl
#define MILK_NET_TO_HOST_16 ntohs
#define MILK_NET_TO_HOST_32 ntohl

#define MILK_DEFAULT_SOCKET_LIST_CAPACITY 256

typedef struct _milk_socket_list
{
    size_t socket_count;
    size_t socket_capacity;
    milk_socket* sockets;
} milk_socket_list;

typedef struct _milk_socket_manager
{
    milk_socket_list* listening_sockets;
    milk_socket_list* server_sockets;
    milk_socket_list* client_sockets;
} milk_socket_manager;

typedef struct _milk_address
{
    const char* ip;
    const char* port;
} milk_address;

typedef struct _milk_address_list
{
    milk_address* addresses;
} milk_address_list;

typedef struct _milk_context
{
    milk_thread_pool* thread_pool;
    fd_set write_fds;
    fd_set read_fds;
    pthread_mutex_t lock;
} milk_context;

typedef enum _milk_socket_type
{
    MILK_SOCKET_TYPE_DATAGRAM = 1,
    MILK_SOCKET_TYPE_STREAM = 2,
    MILK_SOCKET_TYPE_RAW = 3
} milk_socket_type;

typedef enum _milk_socket_option
{
    MILK_SOCKET_OPTION_NONBLOCK = 1,
    MILK_SOCKET_OPTION_BROADCAST = 2,
    MILK_SOCKET_OPTION_RCVBUF = 3,
    MILK_SOCKET_OPTION_SNDBUF = 4,
    MILK_SOCKET_OPTION_REUSEADDR = 5,
    MILK_SOCKET_OPTION_RCVTIMEO = 6,
    MILK_SOCKET_OPTION_SNDTIMEO = 7,
    MILK_SOCKET_OPTION_ERROR = 8,
    MILK_SOCKET_OPTION_NODELAY = 9,
    MILK_SOCKET_OPTION_TTL = 10,
    MILK_SOCKET_OPTION_KEEPALIVE = 11,
    MILK_SOCKET_OPTION_DONTROUTE = 12
} milk_socket_option;

typedef enum _milk_shutdown
{
    MILK_SOCKET_SHUTDOWN_READ = 1,
    MILK_SOCKET_SHUTDOWN_WRITE = 2,
    MILK_SOCKET_SHUTDOWN_READ_WRITE = 3
} milk_shutdown;

typedef enum _milk_socket_wait
{
    MILK_SOCKET_WAIT_NONE = 0,
    MILK_SOCKET_WAIT_READ = 1,
    MILK_SOCKET_WAIT_WRITE = 2,
    MILK_SOCKET_WAIT_READ_WRITE = 3
} milk_socket_wait;

typedef struct _milk_thread_args
{
    milk_socket_manager* socket_manager;
    milk_socket client_socket;
} milk_thread_args;

milk_b8 milk_initialize_wsa(int version_major, int version_minor);
milk_b8 milk_initialize_context(milk_context* context, milk_socket_manager* socket_manager, size_t client_count, size_t server_count, size_t listening_count, int version_param_one, int version_param_two, milk_log_type log_type);
milk_b8 milk_initialize_socket(milk_socket* result, milk_socket_type type);
milk_b8 milk_bind_socket(milk_socket socket_fd, const unsigned short port, const char* ip);
milk_b8 milk_listen_socket(milk_socket socket_fd, const unsigned int max_clients);
milk_b8 milk_accept_socket(milk_socket* client_socket, milk_socket server_socket);
milk_b8 milk_socket_shutdown(milk_socket* socket, milk_shutdown how);
milk_b8 milk_close_socket(milk_socket socket_fd);
milk_b8 milk_close_wsa();
milk_b8 milk_cleanup_context(milk_context* context, milk_socket_manager* socket_manager);
milk_b8 milk_send_socket(milk_socket socket, milk_dynamic_buffer* buffer, int flags);
milk_b8 milk_connect_socket(milk_socket socket_fd, const unsigned short port, const char* ip);
milk_b8 milk_socket_forward(milk_socket client_socket, milk_socket server_socket, milk_b8 from_client, fd_set* read_fds, fd_set* write_fds, milk_dynamic_buffer* buffer, int timeout_ms, size_t expected_receive_size);
milk_b8 milk_socket_set_unblocking(milk_socket socket_fd);
milk_b8 milk_socket_change_control(milk_socket socket_fd, long cmd, u_long argp);
milk_b8 milk_accept_new_client(milk_socket_manager* manager, milk_socket* listening_socket, fd_set* read_fds);
milk_b8 milk_remove_client(milk_socket_manager* manager, size_t index);
milk_b8 milk_log_wsa_error(const char* error_message);
milk_b8 milk_add_new_client(milk_thread_args* thread_args);
milk_b8 milk_set_array(void* array, size_t element_size, size_t element_count, void* value);
milk_b8 milk_serialize_data(void* element, size_t element_size, milk_dynamic_buffer* buffer);
milk_b8 milk_deserialize_data(milk_dynamic_buffer* buffer, size_t element_size, void* element);
milk_b8 milk_send_received_data_to_all_clients(void* data, size_t data_size, milk_socket_manager* socket_manager, milk_dynamic_buffer* buffer, int timeout_ms, size_t expected_receive_size);
milk_b8 milk_wait_socket(milk_socket socket, milk_socket_wait option, int timeout_ms);
milk_b8 milk_socket_free(milk_socket* socket);
milk_b8 milk_receive_socket(milk_socket socket, milk_dynamic_buffer* buffer, size_t expected_received_size);

#endif // _MILKNET_H