#ifndef KINDLE_HAL_ASYNC_WORKER_H
#define KINDLE_HAL_ASYNC_WORKER_H

#include <cstddef>
#include <functional>
#include <memory>

namespace hal {

/** @brief Callable work item executed on the worker thread. */
using AsyncTask = std::function<void()>;

/** @brief Callable completion handler invoked by the UI thread. */
using UiCallback = std::function<void()>;

/**
 * @brief Thread-safe asynchronous worker for background operations.
 *
 * Blocking tasks are dispatched to a dedicated background thread. Completion
 * callbacks are queued until the UI thread explicitly drains them.
 */
class AsyncWorker {
public:
    /** @brief Starts the worker thread. */
    AsyncWorker();

    /**
     * @brief Stops the worker and releases its resources.
     *
     * Queued and active tasks are allowed to finish before the worker thread
     * is joined.
     */
    ~AsyncWorker();

    /** @brief Disables copying of the worker thread and task queues. */
    AsyncWorker(const AsyncWorker&) = delete;

    /** @brief Disables assignment of the worker thread and task queues. */
    AsyncWorker& operator=(const AsyncWorker&) = delete;

    /**
     * @brief Posts a background task with an optional UI completion callback.
     * @param task Work to execute on the background thread.
     * @param onComplete Callback queued for the UI thread after the task ends.
     */
    void postTask(AsyncTask task, UiCallback onComplete);

    /**
     * @brief Drains and invokes pending UI callbacks on the caller's thread.
     * @return Number of callbacks executed.
     */
    std::size_t drainUiCallbacks();

    /**
     * @brief Requests shutdown and waits for the worker thread to finish.
     */
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hal

#endif // KINDLE_HAL_ASYNC_WORKER_H
