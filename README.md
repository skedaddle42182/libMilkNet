# MilkNet Simple TCP Socket Abstraction Library.

This is a basic TCP socket abstraction library for windows that can perform any simple tasks like sending, recieving, or even serializing data through the network etc...  

**DISCLAIMER:** This library is **NOT** intended for production purposes, however it can still be very educational and great for hobbyists.

## Basic Functions

milk_b8 milk_initialize_wsa(int version_major, int version_minor);

milk_b8 milk_initialize_globals(milk_globals* globals, const char* port, const char* ip, milk_client_list* client_list, size_t client_count, size_t server_count);

milk_b8 milk_initialize_socket(milk_socket* result_socket, int af, int type, int protocol);

milk_b8 milk_bind_socket(milk_socket socket_fd, const unsigned short port, const char* ip);

milk_b8 milk_listen_socket(milk_socket socket_fd, const unsigned int max_clients);

milk_b8 milk_accept_socket(milk_socket* client_socket, milk_socket server_socket);

milk_b8 milk_shutdown_socket(milk_socket socket_fd);

milk_b8 milk_close_socket(milk_socket socket_fd);

milk_b8 milk_close_wsa();

milk_b8 milk_cleanup_globals(milk_globals* globals, milk_client_list* client_list);

milk_b8 milk_send_socket(milk_socket socket_fd, const char* buffer, int length, int flags);

milk_b8 milk_connect_socket(milk_socket socket_fd, const unsigned short port, const char* ip);

milk_b8 milk_socket_forward(milk_socket client_socket, milk_socket server_socket, milk_b8 from_client, fd_set* read_fds, fd_set* write_fds, const char* buffer);

milk_b8 milk_socket_set_unblocking(milk_socket socket_fd);

milk_b8 milk_socket_change_control(milk_socket socket_fd, long cmd, u_long argp);

milk_b8 milk_accept_new_client(milk_thread_pool* thread_pool, milk_client_list* client_list, milk_socket* listening_socket, fd_set* write_fds, fd_set* read_fds);

milk_b8 milk_remove_client(milk_thread_pool* thread_pool, milk_client_list* client_list, size_t index);

milk_b8 milk_log_invalid_data_error(void* data, milk_error_type error_type);

milk_b8 milk_log_wsa_error(const char* error_message);

milk_b8 milk_add_new_client(milk_thread_args* thread_args);

milk_b8 milk_set_array(void* array, size_t element_size, size_t element_count, void* value);

milk_b8 milk_serialize_data(void* element, size_t element_size, uint8_t* buffer);

milk_b8 milk_deserialize_data(uint8_t* buffer, size_t element_size, void* element);

milk_b8 milk_send_data_to_all_clients(void* data, size_t data_size, milk_client_list* client_list, milk_socket server_socket_fd, fd_set* write_fds, fd_set* read_fds, const char* buffer);

milk_b8 milk_wait_socket(milk_socket socket, milk_socket_wait option, int timeout_ms);

milk_b8 milk_socket_free(milk_socket* socket);

## Missing features which should be there but I am too lazy to add them.

Server Socket lists.

Listening Socket Lists.

A recv function.

My own proper assert implementation.

## This is the command to compile it using bash **WINDOWS ONLY**.

```bash
gcc -o program.exe program.c -lws2_32 -lpthread
