#ifndef LINK_LIST_H_
#define LINK_LIST_H_

#include <iostream>
#include <vector>

class LinkList {
public:
    LinkList();
    ~LinkList();

public:
    void rotate_image(std::vector<std::vector<int>>& img_mat);
    void push_one(std::vector<int>& nums, const int& digit);
    std::vector<int> graycode(const int& n);
};

#endif // !LINK_LIST_H_
