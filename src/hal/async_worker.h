#ifndef KINDLE_HAL_ASYNC_WORKER_H
#define KINDLE_HAL_ASYNC_WORKER_H

#include <cstddef>
#include <functional>
#include <memory>

namespace hal {

using AsyncTask = std::function<void()>;
using UiCallback = std::function<void()>;

/**
 * @brief Thread-safe asynchronous worker for background operations.
 *
 * Dispatches blocking tasks (like network I/O) onto a dedicated background thread
 * and enqueues completion callbacks to be drained on the main UI thread.
 */
class AsyncWorker {
public:
    AsyncWorker();
    ~AsyncWorker();

    AsyncWorker(const AsyncWorker&) = delete;
    AsyncWorker& operator=(const AsyncWorker&) = delete;

    /**
     * @brief Posts a background task with an optional UI completion callback.
     * @param task Work to execute on background thread.
     * @param onComplete Callback to enqueue for UI thread after task finishes.
     */
    void postTask(AsyncTask task, UiCallback onComplete);

    /**
     * @brief Drains and invokes all pending UI completion callbacks on caller thread.
     * @return Number of callbacks executed.
     */
    std::size_t drainUiCallbacks();

    /**
     * @brief Stops the worker thread and waits for active tasks to complete.
     */
    void stop();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace hal

#endif // KINDLE_HAL_ASYNC_WORKER_H
