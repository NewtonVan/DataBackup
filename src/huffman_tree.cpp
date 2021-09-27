#include "huffman_tree.h"

using namespace std;

/** 
 * 本哈夫曼树以byte词频为基础
 * 要求counts长度大于1
 */
TreeNode *TreeNode::BuildHuffman(const vector<TreeNode *> &counts) {
    // 将基础结点加入小顶堆
    function<bool(const TreeNode *x, const TreeNode *y)> cmp(TreeNode::Cmp);
    priority_queue<TreeNode *, vector<TreeNode *>, 
                   function<bool(const TreeNode *x, const TreeNode *y)>> my_heap(cmp);
    for(TreeNode *node : counts) {
        if(nullptr!=node) {
            my_heap.push(node);
        }
    }
    // 哈夫曼核心算法：构建树，并且进行叶节点编码
    if(my_heap.empty() || my_heap.size()==1) {
        return nullptr;
    }
    while(my_heap.size()>1) {
        TreeNode *left_node = my_heap.top();
        my_heap.pop();
        TreeNode *right_node = my_heap.top();
        my_heap.pop();
        TreeNode *root = new TreeNode;
        root->weight_ = left_node->weight_ + right_node->weight_;
        root->left_ = left_node;
        root->right_ = right_node;
        my_heap.push(root);
    }
    string code{""};
    GenerateCoding(my_heap.top(), code);
    return my_heap.top();
}

void TreeNode::GenerateCoding(TreeNode *root, string &code) {
    if(nullptr == root) return;
    if(nullptr==root->left_ && nullptr==root->right_) {
        root->code_=code;
    }
    code.push_back('0');
    GenerateCoding(root->left_, code);
    code.pop_back();
    code.push_back('1');
    GenerateCoding(root->right_, code);
    code.pop_back();
}