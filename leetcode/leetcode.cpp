#include <iostream>

#include "linklist/link_list.h"

int main(int argc, char* argv[]) {
    std::vector<int> nums;
    // nums.push_back(9);
    nums.push_back(9);
    nums.push_back(1);

    LinkList ls;
    ls.push_one(nums, 9);

    for (int row = 0; row < nums.size(); row++) {
        std::cout << nums[row];
    }

    return 0;
}