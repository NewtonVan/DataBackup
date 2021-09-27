#ifndef HUFFMAN_TREE_H_
#define HUFFMAN_TREE_H_

#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <stack>

class TreeNode {
public:
    TreeNode() 
        : left_(nullptr), right_(nullptr), ch_('\0'), weight_(0), code_("") {}
    TreeNode(unsigned char ch) 
        : left_(nullptr), right_(nullptr), ch_(ch), weight_(0), code_("") {}
    static TreeNode *BuildHuffman(const std::vector<TreeNode *> &counts);
    static bool Cmp(const TreeNode *x, const TreeNode *y) { // 用于优先队列
        return x->weight_ > y->weight_;
    }
private:
    static void GenerateCoding(TreeNode *root, std::string &code);
public:
    TreeNode *left_;
    TreeNode *right_;
    unsigned char ch_;
    uint64_t weight_;
    std::string code_;
};

#endif