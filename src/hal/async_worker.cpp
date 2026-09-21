#include "async_worker.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace hal {

namespace {

struct WorkItem {
    AsyncTask task;
    UiCallback callback;
};

class CallbackQueue {
public:
    void enqueue(UiCallback callback) {
        if (!callback) {
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.push_back(std::move(callback));
    }

    std::vector<UiCallback> drain() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<UiCallback> ready(callbacks_.begin(), callbacks_.end());
        callbacks_.clear();
        return ready;
    }

private:
    std::mutex mutex_;
    std::deque<UiCallback> callbacks_;
};

} // namespace

struct AsyncWorker::Impl {
    CallbackQueue uiQueue;
    std::deque<WorkItem> tasks;
    std::mutex taskMutex;
    std::condition_variable cv;
    std::thread thread;
    bool running{true};

    Impl() {
        thread = std::thread([this]() {
            workerLoop();
        });
    }

    ~Impl() {
        stop();
    }

    void stop() {
        std::unique_lock<std::mutex> lock(taskMutex);
        if (!running) {
            return;
        }
        running = false;
        cv.notify_all();
        lock.unlock();

        if (thread.joinable()) {
            thread.join();
        }
    }

    void workerLoop() {
        while (true) {
            WorkItem item;
            if (!waitForWork(item)) {
                return;
            }
            executeWork(item);
        }
    }

    bool waitForWork(WorkItem& item) {
        std::unique_lock<std::mutex> lock(taskMutex);
        cv.wait(lock, [this]() {
            return !running || !tasks.empty();
        });
        if (!running && tasks.empty()) {
            return false;
        }
        item = std::move(tasks.front());
        tasks.pop_front();
        return true;
    }

    void executeWork(WorkItem& item) {
        if (item.task) {
            item.task();
        }
        uiQueue.enqueue(std::move(item.callback));
    }
};

AsyncWorker::AsyncWorker() : impl_(std::make_unique<Impl>()) {}

AsyncWorker::~AsyncWorker() = default;

void AsyncWorker::postTask(AsyncTask task, UiCallback onComplete) {
    if (!impl_) {
        return;
    }
    std::lock_guard<std::mutex> lock(impl_->taskMutex);
    impl_->tasks.push_back(WorkItem{std::move(task), std::move(onComplete)});
    impl_->cv.notify_one();
}

std::size_t AsyncWorker::drainUiCallbacks() {
    if (!impl_) {
        return 0;
    }
    auto callbacks = impl_->uiQueue.drain();
    for (const auto& callback : callbacks) {
        callback();
    }
    return callbacks.size();
}

void AsyncWorker::stop() {
    if (!impl_) {
        return;
    }
    impl_->stop();
}

} // namespace hal
