#include "gtest/gtest.h"

#include "../link_list.h"

class LinkListTest : public ::testing::Test {
protected:
    void SetUp() override { ls = std::make_unique<LinkList>(); }

    std::unique_ptr<LinkList> ls;
};

TEST_F(LinkListTest, rorate_img) {
    std::vector<int> row1;
    row1.push_back(1);
    row1.push_back(2);
    row1.push_back(3);

    std::vector<int> row2;
    row2.push_back(4);
    row2.push_back(5);
    row2.push_back(6);

    std::vector<int> row3;
    row3.push_back(7);
    row3.push_back(8);
    row3.push_back(9);

    std::vector<std::vector<int>> img;
    img.push_back(row1);
    img.push_back(row2);
    img.push_back(row3);

    ls->rotate_image(img);
}

TEST_F(LinkListTest, push_one) {
    std::vector<int> nums;
    nums.push_back(9);
    nums.push_back(9);
    nums.push_back(9);

    ls->push_one(nums, 1);

    // 999+1, 3位数变成4位数
    ASSERT_EQ(nums.size(), 4);

    // 1000 测试
    std::vector<int> nums_test;
    nums_test.push_back(1);
    nums_test.push_back(0);
    nums_test.push_back(0);
    nums_test.push_back(0);

    for (int i = 0; i < nums.size(); i++) {
        ASSERT_EQ(nums[i], nums_test[i]);
    }
}

TEST_F(LinkListTest, graycode) {
    int code_num = 3;

    auto codes = ls->graycode(code_num);

    // 编码个数是 2^n 个
    ASSERT_EQ(codes.size(), std::pow(2, code_num));
}