#include "test_framework.h"
#include "../src/hal/async_worker.h"

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

TEST(async_worker_executes_task_and_drains_ui_callback) {
    hal::AsyncWorker worker;
    std::atomic<bool> taskExecuted{false};
    bool uiCallbackExecuted = false;

    worker.postTask(
        [&taskExecuted]() {
            taskExecuted = true;
        },
        [&uiCallbackExecuted]() {
            uiCallbackExecuted = true;
        }
    );

    // Wait for the background task to finish
    int attempts = 0;
    while (!taskExecuted && attempts < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        attempts++;
    }
    ASSERT_TRUE(taskExecuted.load());

    // UI callback should not have executed yet until drained
    ASSERT_FALSE(uiCallbackExecuted);

    // Drain UI callbacks on main thread
    std::size_t drained = worker.drainUiCallbacks();
    ASSERT_EQ(static_cast<std::size_t>(1), drained);
    ASSERT_TRUE(uiCallbackExecuted);
}

TEST(async_worker_handles_multiple_tasks_in_order) {
    hal::AsyncWorker worker;
    std::vector<int> executionOrder;
    std::vector<int> uiOrder;

    for (int i = 1; i <= 3; ++i) {
        worker.postTask(
            [i, &executionOrder]() {
                executionOrder.push_back(i);
            },
            [i, &uiOrder]() {
                uiOrder.push_back(i);
            }
        );
    }

    int attempts = 0;
    while (executionOrder.size() < 3 && attempts < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        attempts++;
    }
    ASSERT_EQ(static_cast<std::size_t>(3), executionOrder.size());

    worker.drainUiCallbacks();
    ASSERT_EQ(static_cast<std::size_t>(3), uiOrder.size());
    ASSERT_EQ(1, uiOrder[0]);
    ASSERT_EQ(2, uiOrder[1]);
    ASSERT_EQ(3, uiOrder[2]);
}

TEST(async_worker_shuts_down_cleanly) {
    hal::AsyncWorker worker;
    std::atomic<int> completedTasks{0};

    worker.postTask(
        [&completedTasks]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            completedTasks++;
        },
        nullptr
    );

    worker.stop();
    ASSERT_EQ(1, completedTasks.load());
}
