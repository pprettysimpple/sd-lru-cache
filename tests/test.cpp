#include <lru.h>
#include <gtest/gtest.h>

struct TMoveOnly {
    explicit TMoveOnly(size_t id) noexcept
        : Id(id)
    {
    }

    TMoveOnly(const TMoveOnly&) = delete;
    TMoveOnly(TMoveOnly&&) = default;
    TMoveOnly& operator=(const TMoveOnly&) = delete;
    TMoveOnly& operator=(TMoveOnly&&) = default;
    size_t Id;
};

struct TCopyCounter {
    explicit TCopyCounter(size_t id, const std::shared_ptr<size_t>& init)
        : Id(id)
        , Counter(init)
    {
    }

    TCopyCounter(const TCopyCounter& rhs)
        : Id(rhs.Id)
        , Counter(rhs.Counter)
    {
        (*Counter)++;
    }

    TCopyCounter& operator=(const TCopyCounter& rhs) {
        Id = rhs.Id;
        Counter = rhs.Counter;

        (*Counter)++;
        return *this;
    }

    bool operator<(const TCopyCounter& rhs) const {
        return Id < rhs.Id;
    }

    size_t Id;
    std::shared_ptr<size_t> Counter;
};

TEST(LRUCache, Init) {
    TLRUCache<int, int, 10> cache;
    cache.Put(10, 228);
    ASSERT_FALSE(cache.Get(10) == nullptr);
    ASSERT_EQ(*cache.Get(10), 228);
}

TEST(LRUCache, LessThenCapacity) {
    TLRUCache<int, int, 30> cache;

    for (size_t i = 1; i <= 10; i++) {
        cache.Put(i, 10 * i);
    }

    for (size_t i = 1; i <= 10; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(*cache.Get(i), 10 * i);
    }
}

TEST(LRUCache, MoreThenCapacity) {
    TLRUCache<int, int, 3> cache;

    for (size_t i = 1; i <= 10; i++) {
        cache.Put(i, 10 * i);
    }

    for (size_t i = 1; i <= 7; i++) {
        ASSERT_TRUE(cache.Get(i) == nullptr);
    }

    for (size_t i = 8; i <= 10; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(*cache.Get(i), 10 * i);
    }
}

TEST(LRUCache, CapacityOne) {
    TLRUCache<int, int, 1> cache;

    for (size_t i = 1; i <= 10; i++) {
        cache.Put(i, 10 * i);
    }

    for (size_t i = 1; i <= 9; i++) {
        ASSERT_TRUE(cache.Get(i) == nullptr);
    }

    for (size_t i = 10; i <= 10; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(*cache.Get(i), 10 * i);
    }
}

TEST(LRUCache, FitsCapacity) {
    TLRUCache<int, int, 5> cache;

    for (size_t i = 1; i <= 5; i++) {
        cache.Put(i, 10 * i);
    }

    for (size_t i = 1; i <= 5; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(*cache.Get(i), 10 * i);
    }
}

TEST(LRUCache, SameKeys) {
    TLRUCache<int, int, 5> cache;

    for (size_t i = 1; i <= 5; i++) {
        cache.Put(i, 10 * i);
    }

    cache.Put(3, 228);
    cache.Put(3, 30);

    for (size_t i = 1; i <= 5; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(*cache.Get(i), 10 * i);
    }
}

TEST(LRUCache, ValueMoveOnly) {
    TLRUCache<int, TMoveOnly, 5> cache;

    for (size_t i = 1; i <= 5; i++) {
        cache.Put(i, TMoveOnly(i * 10));
    }

    for (size_t i = 1; i <= 5; i++) {
        ASSERT_FALSE(cache.Get(i) == nullptr);
        ASSERT_EQ(cache.Get(i)->Id, i * 10);
    }
}

TEST(LRUCache, KeyCopies) {
    size_t n = 5;
    TLRUCache<TCopyCounter, TMoveOnly, 5> cache;

    std::vector<std::shared_ptr<size_t>> copyCounters;
    copyCounters.reserve(n);
    for (size_t i = 1; i <= n; i++) {
        copyCounters.push_back(std::make_shared<size_t>(0));
        auto key = TCopyCounter(i, copyCounters[i-1]);

        cache.Put(key, TMoveOnly(i * 10));
    }

    for (size_t i = 1; i <= n; i++) {
        auto key = TCopyCounter(i, copyCounters[i-1]);

        ASSERT_FALSE(cache.Get(key) == nullptr);
        ASSERT_EQ(cache.Get(key)->Id, i * 10);

        ASSERT_EQ(*copyCounters[i-1], 3);
    }
}
