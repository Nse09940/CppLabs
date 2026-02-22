#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <vector>
#include <list>

/*
    В данном файле представлен ряд тестов, где используются (вместе, раздельно и по-очереди):
        - push_back
        - push_front
        - insert
    Методы применяются на unrolled_list и на std::list.
    Ожидается, что в итоге порядок элементов в контейнерах будут идентичен
*/

TEST(unrolled_list, pushBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, pushFront) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_front(i);
        unrolled_list.push_front(i);
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, pushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 2 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, insertAndPushMixed) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.erase(std_it);
            unrolled_list.erase(unrolled_it);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}
TEST(unrolled_list, insertAndPushMixed2) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        if (i % 3 == 0) {
            std_list.push_front(i);
            unrolled_list.push_front(i);
        } else if (i % 3 == 1) {
            std_list.push_back(i);
            unrolled_list.push_back(i);
        } else {
            auto std_it = std_list.begin();
            auto unrolled_it = unrolled_list.begin();
            std::advance(std_it, std_list.size() / 2);
            std::advance(unrolled_it, std_list.size() / 2);
            std_list.insert(std_it,i);
            unrolled_list.insert(unrolled_it,i);
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, popFrontBack) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 1000; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    for (int i = 0; i < 500; ++i) {
        if (i % 2 == 0) {
            std_list.pop_back();
            unrolled_list.pop_back();
        } else {
            std_list.pop_front();
            unrolled_list.pop_front();
        }
    }

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));

    for (int i = 0; i < 500; ++i) {
        std_list.pop_back();
        unrolled_list.pop_back();
    }

    ASSERT_TRUE(unrolled_list.empty());
}



TEST(unrolled_list, insertAtDifferentPositions) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 100; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    auto std_it = std_list.begin();
    auto unrolled_it = unrolled_list.begin();
    std::advance(std_it, 50);
    std::advance(unrolled_it, 50);

    std_list.insert(std_it, 999);
    unrolled_list.insert(unrolled_it, 999);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, eraseAtDifferentPositions) {
    std::list<int> std_list;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 100; ++i) {
        std_list.push_back(i);
        unrolled_list.push_back(i);
    }

    auto std_it = std_list.begin();
    auto unrolled_it = unrolled_list.begin();
    std::advance(std_it, 20);
    std::advance(unrolled_it, 20);

    std_list.erase(std_it);
    unrolled_list.erase(unrolled_it);

    ASSERT_THAT(unrolled_list, ::testing::ElementsAreArray(std_list));
}

TEST(unrolled_list, supportsCopy) {
    unrolled_list<int> original_list;
    for (int i = 0; i < 50; ++i) {
        original_list.push_back(i);
    }

    unrolled_list<int> copied_list = original_list;
    ASSERT_THAT(copied_list, ::testing::ElementsAreArray(original_list));

    unrolled_list<int> moved_list = (original_list);
    ASSERT_THAT(moved_list, ::testing::ElementsAreArray(copied_list));
}

TEST(unrolled_list, interactsWithVector) {
    std::vector<int> vec;
    unrolled_list<int> unrolled_list;

    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
        unrolled_list.push_back(i);
    }

    ASSERT_TRUE(std::equal(vec.begin(), vec.end(), unrolled_list.begin()));
}
