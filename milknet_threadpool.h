#ifndef _MILKNET_THREADPOOL_H
#define _MILKNET_THREADPOOL_H

#include "milknet_types.h"
#include "milknet_error.h"
#include <pthread.h>

#define MAX_THREADS 64
#define MAX_QUEUE 256

#if defined(_WIN32)
#include <Windows.h>
typedef LONG milk_atomic_int;
#define milk_atomic_add(x) InterlockedIncrement(&(x))
#define milk_atomic_subtract(x) InterlockedDecrement(&(x))
#elif defined(__unix__) || defined(__APPLE__)
#include <stdatomic.h>
typedef _Atomic int milk_atomic_int;
#define milk_atomic_add(x) atomic_fetch_add(&(x), 1)
#define milk_atomic_subtract(x) atomic_fetch_sub(&(x), 1)
#else 
#error "Platform isn't supported by milknet."
#endif 

typedef void (*milk_function)(void* arguments);

typedef struct milk_task
{
    milk_function function;
    void* arguments;
} milk_task;

typedef struct milk_thread_pool
{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t* threads;
    milk_task* queue;
    size_t thread_count;
    size_t queue_size;
    size_t count;
    int head;
    int tail;
    milk_b8 shutdown;
    milk_atomic_int started;
} milk_thread_pool;

typedef enum milk_shutdown
{
    MILK_IMMEDIATE_SHUTDOWN = 1,
    MILK_GRACEFUL_SHUTDOWN = 2
} milk_shutdown;

typedef enum milk_destroy_flags
{
    MILK_THREAD_POOL_GRACEFUL = 0
} milk_destroy_flags;

milk_b8 milk_thread_pool_free(milk_thread_pool* thread_pool);
milk_b8 milk_thread_pool_destroy(milk_thread_pool* thread_pool, int flags);
milk_b8 milk_thread_pool_add(milk_thread_pool* thread_pool, milk_function function, void* argument);
milk_thread_pool* milk_create_thread_pool(size_t thread_count, size_t queue_size);
void* milk_thread_pool_thread(void* thread_pool);

#endif // _MILKNET_THREADPOOL_H