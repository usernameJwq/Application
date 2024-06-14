#include "link_list.h"

#include <iostream>

LinkList::LinkList() {}

LinkList::~LinkList() {}

/*
 * 翻转图像
 *
 * 步骤:
 *	1.从左上角向右下角方向 进行 对角线翻转
 *	2.再左右翻转
 *
 * 时间复杂度：O(n^2)
 * 空间复杂度：O(1)
 */
void LinkList::rotate_image(std::vector<std::vector<int>>& img_mat) {
    std::vector<std::vector<int>> rotate_img_mat;
    if (img_mat.empty()) {
        return;
    }

    for (int row = 0; row < img_mat.size(); row++) {
        for (int col = 0; col < row; col++) {
            std::swap(img_mat[row][col], img_mat[col][row]);
        }
    }

    for (int row = 0; row < img_mat.size(); row++) {
        for (int col = 0; col < img_mat[row].size() / 2; col++) {
            std::swap(img_mat[row][col], img_mat[row][img_mat[row].size() - 1 - col]);
        }
    }
}

/*
 * 高精度加法
 *
 * 步骤:
 *  1.求和除以10获取进位的数字
 *  2.求和取余更新数组中的数字
 *
 * 时间复杂度: O(n)
 * 空间复杂度: O(1)
 */
void LinkList::push_one(std::vector<int>& nums, const int& digit) {
    if (nums.empty()) {
        return;
    }

    int carry = digit;
    for (auto it = nums.rbegin(); it != nums.rend(); it++) {
        int sum = *it + carry;
        carry = sum / 10;
        *it = sum % 10;
    }

    if (carry > 0) {
        nums.insert(nums.begin(), 1);
    }
}

/*
 * 格雷编码
 *
 * 步骤:
 *
 * 时间复杂度:
 * 空间复杂度:
 */
std::vector<int> LinkList::graycode(const int& n) {}