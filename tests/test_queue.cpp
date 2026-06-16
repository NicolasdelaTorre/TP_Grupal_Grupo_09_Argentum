#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "../common/queue.h"

TEST(QueueTest, pushPopMantieneFifo) {
    Queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    EXPECT_EQ(q.pop(), 1);
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
}

TEST(QueueTest, tryPopEnVaciaDevuelveFalse) {
    Queue<int> q;
    int v = 999;
    EXPECT_FALSE(q.try_pop(v));
    EXPECT_EQ(v, 999);
}

TEST(QueueTest, tryPushRespetaCapacidad) {
    Queue<int> q(2);
    EXPECT_TRUE(q.try_push(10));
    EXPECT_TRUE(q.try_push(20));
    EXPECT_FALSE(q.try_push(30));
    int v = 0;
    EXPECT_TRUE(q.try_pop(v));
    EXPECT_EQ(v, 10);
    EXPECT_TRUE(q.try_push(30));
}

TEST(QueueTest, popEnQueueCerradaTiraClosedQueue) {
    Queue<int> q;
    q.close();
    EXPECT_THROW(q.pop(), ClosedQueue);
}

TEST(QueueTest, pushEnQueueCerradaTiraClosedQueue) {
    Queue<int> q;
    q.close();
    EXPECT_THROW(q.push(7), ClosedQueue);
}

TEST(QueueTest, closeDesbloqueaConsumidorEsperando) {
    Queue<int> q;
    std::atomic<bool> threw{false};
    std::thread consumer([&] {
        try {
            q.pop();
        } catch (const ClosedQueue&) {
            threw = true;
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    q.close();
    consumer.join();
    EXPECT_TRUE(threw);
}

TEST(QueueTest, closeNoDescartaItemsPendientes) {
    Queue<int> q;
    q.push(42);
    q.close();
    int v = 0;
    EXPECT_TRUE(q.try_pop(v));
    EXPECT_EQ(v, 42);
    EXPECT_THROW(q.try_pop(v), ClosedQueue);
}

TEST(QueueTest, multipleProducersConsumersNoSePierdenItems) {
    constexpr int N_PRODUCERS = 4;
    constexpr int N_CONSUMERS = 4;
    constexpr int ITEMS_PER_PRODUCER = 500;
    Queue<int> q;

    std::vector<std::thread> producers;
    for (int i = 0; i < N_PRODUCERS; ++i) {
        producers.emplace_back([&] {
            for (int j = 0; j < ITEMS_PER_PRODUCER; ++j) {
                q.push(1);
            }
        });
    }

    std::atomic<int> total{0};
    std::vector<std::thread> consumers;
    for (int i = 0; i < N_CONSUMERS; ++i) {
        consumers.emplace_back([&] {
            try {
                while (true) {
                    total += q.pop();
                }
            } catch (const ClosedQueue&) {}
        });
    }

    for (auto& t: producers)
        t.join();
    q.close();
    for (auto& t: consumers)
        t.join();

    EXPECT_EQ(total.load(), N_PRODUCERS * ITEMS_PER_PRODUCER);
}
