#include "milknet_threadpool.h"
#include "milknet_error.h"

milk_thread_pool* milk_create_thread_pool(size_t thread_count, size_t queue_size)
{
    milk_thread_pool* thread_pool;

    if (thread_count == 0 || thread_count > MAX_THREADS || queue_size == 0 || queue_size > MAX_QUEUE)
    {
        return NULL;
    }

    thread_pool = (milk_thread_pool*)malloc(sizeof(milk_thread_pool));

    MILK_ASSERT(thread_pool != NULL, __FUNCTION__, __LINE__);

    thread_pool->head = 0;
    thread_pool->tail = 0;
    thread_pool->count = 0;
    thread_pool->thread_count = thread_count;
    thread_pool->queue_size = queue_size;
    thread_pool->shutdown = milk_false;
    thread_pool->started = 0;
    thread_pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_pool->thread_count);
    thread_pool->queue = (milk_task*)malloc(sizeof(milk_task) * thread_pool->queue_size);

    int mutex_init_result = pthread_mutex_init(&(thread_pool->lock), NULL);
    int cond_init_result = pthread_cond_init(&(thread_pool->notify), NULL);

    MILK_ASSERT(mutex_init_result != NULL, __FUNCTION__, __LINE__);
    MILK_ASSERT(cond_init_result != NULL, __FUNCTION__, __LINE__);

    MILK_ASSERT(thread_pool->threads != NULL, __FUNCTION__, __LINE__);
    MILK_ASSERT(thread_pool->queue != NULL, __FUNCTION__, __LINE__);

    for (int i = 0; i < thread_pool->thread_count; i++)
    {
        if (pthread_create(&(thread_pool->threads[i]), NULL, milk_thread_pool_thread, (void*)thread_pool) != 0)
        {
            milk_thread_pool_destroy(thread_pool, 0);
            return NULL;
        }

        milk_atomic_add(thread_pool->started);
    }

    return thread_pool;
}

milk_b8 milk_thread_pool_add(milk_thread_pool* thread_pool, milk_function function, void* argument)
{
    int next_index = 0;

    if (thread_pool == NULL || function == NULL)
    {
        return milk_false;
    }

    if (pthread_mutex_lock(&(thread_pool->lock)) != 0)
    {
        return milk_false;
    }

    next_index = (thread_pool->tail + 1) % thread_pool->queue_size;

    if (thread_pool->count == thread_pool->queue_size)
    {
        size_t new_queue_size = thread_pool->queue_size;
        if (new_queue_size > SIZE_MAX / 2 || new_queue_size > SIZE_MAX / sizeof(milk_task))
        {
            return milk_false;
        }
        new_queue_size *= 2;
        milk_task* queue_temp = (milk_task*)realloc(thread_pool->queue, sizeof(milk_task) * new_queue_size);
        MILK_ASSERT(queue_temp != NULL, __FUNCTION__, __LINE__);
        thread_pool->queue = queue_temp;
        thread_pool->queue_size = new_queue_size;
    }

    if (thread_pool->shutdown == milk_true)
    {
        pthread_mutex_unlock(&thread_pool->lock);
        return milk_false;
    }

    thread_pool->queue[thread_pool->tail].function = function;
    thread_pool->queue[thread_pool->tail].arguments = argument;
    thread_pool->tail = next_index;
    thread_pool->count++;

    if (pthread_cond_signal(&(thread_pool->notify)) != 0)
    {
        pthread_mutex_unlock(&thread_pool->lock);
        return milk_false;
    }

    if (pthread_mutex_unlock(&thread_pool->lock) != 0)
    {
        return milk_false;
    }

    return milk_true;
}

milk_b8 milk_thread_pool_destroy(milk_thread_pool* thread_pool, int flags)
{
    MILK_ASSERT(thread_pool != NULL, __FUNCTION__, __LINE__);

    if (pthread_mutex_lock(&(thread_pool->lock)) != 0)
    {
        return milk_false;
    }

    if (thread_pool->shutdown == milk_true)
    {
        pthread_mutex_unlock(&(thread_pool->lock));
        return milk_false;
    }

    thread_pool->shutdown = (flags & MILK_THREAD_POOL_GRACEFUL) ? MILK_GRACEFUL_SHUTDOWN : MILK_IMMEDIATE_SHUTDOWN;

    if ((pthread_cond_broadcast(&(thread_pool->notify)) != 0) || (pthread_mutex_unlock(&(thread_pool->lock)) != 0))
    {
        return milk_false;
    }

    for (int i = 0; i < thread_pool->thread_count; i++)
    {
        if (pthread_join(thread_pool->threads[i], NULL) != 0)
        {
            return milk_false;
        }
    }

    milk_thread_pool_free(thread_pool);

    return milk_true;
}

milk_b8 milk_thread_pool_free(milk_thread_pool* thread_pool)
{
    MILK_ASSERT(thread_pool != NULL, __FUNCTION__, __LINE__);

    if (thread_pool->threads)
    {
        free(thread_pool->threads);
        free(thread_pool->queue);

        pthread_mutex_destroy(&(thread_pool->lock));
        pthread_cond_destroy(&(thread_pool->notify));
    }

    free(thread_pool);

    return milk_true;
}

void* milk_thread_pool_thread(void* thread_pool_arg)
{
    milk_thread_pool* thread_pool = (milk_thread_pool*)thread_pool_arg;
    milk_task task;

    for (;;)
    {
        if (pthread_mutex_lock(&(thread_pool->lock)) != 0)
        {
            break;
        }

        while ((thread_pool->count == 0) && (!thread_pool->shutdown))
        {
            pthread_cond_wait(&(thread_pool->notify), &(thread_pool->lock));
        }

        if ((thread_pool->shutdown == MILK_IMMEDIATE_SHUTDOWN) ||
            ((thread_pool->shutdown == MILK_GRACEFUL_SHUTDOWN) && (thread_pool->count == 0)))
        {
            pthread_mutex_unlock(&(thread_pool->lock));
            break;
        }

        task.function = thread_pool->queue[thread_pool->head].function;
        task.arguments = thread_pool->queue[thread_pool->head].arguments;
        thread_pool->head = (thread_pool->head + 1) % thread_pool->queue_size;
        thread_pool->count--;

        pthread_mutex_unlock(&(thread_pool->lock));

        (*(task.function))(task.arguments);
    }

    milk_atomic_subtract(thread_pool->started);

    pthread_mutex_unlock(&(thread_pool->lock));
    pthread_exit(NULL);

    return NULL;
}