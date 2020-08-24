#include "gtest/gtest.h"
#include "merge_utils.h"

#include <vector>
#include "utils.h"

using namespace std;

namespace test {

    const size_t TESTD = 2;
    class MergeUtilsTest : public ::testing::Test {
        public:
        template <typename T>
        bool ArrayEqual(const std::vector<T>& got, const std::vector<T>& want) {
            bool match = (got.size() == want.size());
            if (match) {
                for (size_t i = 0; i < got.size(); i++) {
                    match &= (got[i] == want[i]);
                }
            }
            if (!match) {
                std::cout << "Got " << VectorToString(got) << " but wanted "
                   << VectorToString(want) << std::endl; 
            }
            return match;
        }
    };

    TEST_F(MergeUtilsTest, TestMerge) {
        std::vector<PhysicalIndexRange> ranges = {{1, 5}, {10, 12}, {15, 18}, {22, 25}};
        std::vector<size_t> idxs = {0, 1, 3, 4, 6, 9, 16, 18, 22, 23, 24};
        auto got = MergeUtils::Merge(ranges, idxs);
        IndexList want = {1, 3, 4, 16, 22, 23, 24};
        EXPECT_TRUE(ArrayEqual(got, want));
    }

    TEST_F(MergeUtilsTest, TestIntersect) {
        std::vector<size_t> list1 = {0, 3, 5, 6, 7, 10, 20, 21, 23, 25};
        std::vector<size_t> list2 = {0, 1, 4, 5, 7, 8, 10};
        std::vector<size_t> list3 = {2, 6, 20, 21, 23, 25};

        std::vector<size_t> want12 = {0, 5, 7, 10};
        std::vector<size_t> want13 = {6, 20, 21, 23, 25};
        std::vector<size_t> want23 = {};

        auto got12 = MergeUtils::Intersect(list1, list2);
        auto got13 = MergeUtils::Intersect(list1, list3);
        auto got23 = MergeUtils::Intersect(list2, list3);
        // Make sure they work in reverse
        auto got21 = MergeUtils::Intersect(list2, list1);
        auto got31 = MergeUtils::Intersect(list3, list1);
        auto got32 = MergeUtils::Intersect(list3, list2);

        EXPECT_TRUE(ArrayEqual(got12, want12));
        EXPECT_TRUE(ArrayEqual(got21, want12));
        EXPECT_TRUE(ArrayEqual(got13, want13));
        EXPECT_TRUE(ArrayEqual(got31, want13));
        EXPECT_TRUE(ArrayEqual(got23, want23));
        EXPECT_TRUE(ArrayEqual(got32, want23));
    }

    TEST_F(MergeUtilsTest, TestIntersectPhysicalIndexSets) {
        IndexRangeList r1 = {{5, 10}, {15, 20}, {50, 60}};
        IndexList l1 = {2, 25, 26, 31, 36, 42};
        IndexRangeList r2 = {{8, 18}, {25, 30}, {50, 55}};
        IndexList l2 = {2, 5, 20, 42, 59, 60};

        IndexRangeList want_range = {{8, 10}, {15, 18}, {50, 55}};
        IndexList want_list = {2, 5, 25, 26, 42, 59};

        auto got_set = MergeUtils::Intersect({r1, l1}, {r2, l2});
        EXPECT_TRUE(ArrayEqual(got_set.ranges, want_range));
        EXPECT_TRUE(ArrayEqual(got_set.list, want_list));
    }
} // namespace test

