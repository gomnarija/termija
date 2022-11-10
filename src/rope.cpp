#include "rope.h"

#include <memory>
#include <cstring>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iostream>

/*

    Rope Data Structure Implementation;
    a binary tree where each leaf (end node) holds a string and a length (also known as a "weight"),
    and each node further up the tree holds the sum of the lengths of all the leaves in its left subtree.
    A node with two children thus divides the whole string into two parts: the left subtree stores the first part of the string,
    the right subtree stores the second part of the string, and a node's weight is the length of the first part.
    For rope operations, the strings stored in nodes are assumed to be constant immutable objects in the typical nondestructive case,
    allowing for some copy-on-write behavior.
*/




void                                            _split_node(RopeNode*, size_t, std::unique_ptr<RopeNode> &, std::unique_ptr<RopeNode> &);
std::vector<std::unique_ptr<RopeNode>>          _harvest(std::unique_ptr<RopeNode>);
std::unique_ptr<RopeNode>                       _merge(std::vector<std::unique_ptr<RopeNode>>*, size_t, size_t);



RopeNode::RopeNode()
: weight{0}, text{nullptr}, left{nullptr}, right{nullptr}
{}


RopeIteratorBFS::RopeIteratorBFS(RopeNode *rope, size_t start){
    if(rope == nullptr){
        //TODO:error
        return;
    }

    current = rope;
    if(current->left != nullptr)
        nodeQueue.push(current->left.get());
    if(current->right != nullptr)
        nodeQueue.push(current->right.get());
    //go to start
    while(start-- > 0){
        if(hasNext())
            next();
        else{
            //TODO:error
            current = nullptr;
            return;
        }
    }
}
RopeNode* RopeIteratorBFS::next(){
    if(!hasNext()){
        //TODO:error
        return nullptr;
    }

    current = nodeQueue.front();
    if(current->left != nullptr)
        nodeQueue.push(current->left.get());
    if(current->right != nullptr)
        nodeQueue.push(current->right.get());
    nodeQueue.pop();

    return current;
}
bool RopeIteratorBFS::hasNext(){
    return !nodeQueue.empty();
}
RopeNode* RopeIteratorBFS::pop(){
    if(!hasNext()){
        RopeNode *prev = current;
        current = nullptr;
        return prev;
    }

    RopeNode *prev = current;
    next();
    return prev;
}

RopeLeafIterator::RopeLeafIterator(RopeNode *rope, size_t start){
    if(rope == nullptr){
        //TODO:error
        return;
    }
    //find left-most leaf
    current = rope_left_most_node_trace(*rope, &nodeStack);
    //go to start
    while(start-- > 0){
        if(hasNext())
            next();
        else{
            //TODO:error
            current = nullptr;
            return;
        }
    }
}
RopeNode* RopeLeafIterator::next(){
    if(!hasNext()){
        //TODO:error
        return nullptr;
    }

    RopeNode *parent = nodeStack.top();
    nodeStack.pop();
    current = rope_left_most_node_trace(*(parent->right), &nodeStack);
    return current;
}
bool RopeLeafIterator::hasNext(){
    bool has_right_child = false;
    RopeNode *parent;
    while(!nodeStack.empty() && !has_right_child){
        parent = nodeStack.top();
        nodeStack.pop();
        has_right_child = parent->right != nullptr;
    }
    if(has_right_child)
        nodeStack.push(parent);
    return has_right_child;
}
RopeNode* RopeLeafIterator::pop(){
    if(!hasNext()){
        RopeNode *prev = current;
        current = nullptr;
        return prev;
    }
    RopeNode *prev = current;
    next();
    return prev;
}

/*
    creates node with the given text
*/
std::unique_ptr<RopeNode> rope_create_node(const char* text){
    std::unique_ptr<RopeNode> node = std::make_unique<RopeNode>();
    //add text
    node->text = std::make_unique<char[]>(strlen(text) + 1);
    node->weight = sprintf(node->text.get(), "%s", text);
    //cut while too long
    RopeNode *n  = node.get();
    while(n->weight > MAX_WEIGHT){
        _split_node(n, n->weight / 2, n->left, n->right);
        n = n->left.get();
    }

    return std::move(node);
}

/*
    creates rope with the given text
*/
std::unique_ptr<RopeNode> rope_create(const char* text){
    //create new rope, and it's left node
    std::unique_ptr<RopeNode> rope = std::make_unique<RopeNode>();
    rope->left = std::make_unique<RopeNode>();
    RopeNode *n = rope->left.get();
    //add text to the left node
    rope->left->text = std::make_unique<char[]>(strlen(text)+1);
    rope->weight = rope->left->weight = sprintf(rope->left->text.get(), "%s", text);

    //cut while too long
    while(n->weight > MAX_WEIGHT){
        _split_node(n, n->weight / 2, n->left, n->right);
        n = n->left.get();
    }

    return std::move(rope);
}

/*
    destroys the given rope
*/
void rope_destroy(std::unique_ptr<RopeNode> rope){
    std::stack<RopeNode *> nodeStack;
    RopeNode *n, *p;
    n = rope_left_most_node_trace(*rope, &nodeStack);
    while(!nodeStack.empty()){
        p = nodeStack.top();
        nodeStack.pop();
        if(p->left != nullptr)
            p->left.release();
        if(p->right != nullptr){
            n = rope_left_most_node_trace(*(p->right), &nodeStack);
            //if left-most node has the right child
            nodeStack.push(n);
        }
    }
    if(rope->left != nullptr)
        rope->left.release();
}

std::unique_ptr<RopeNode> rope_concat(std::unique_ptr<RopeNode> left, const char *text){
    return std::move(rope_concat(std::move(left), rope_create_node(text)));
}
/*
    concatenates two ropes to a new root node;
        measures it's weight;
            consumes ownership
*/
std::unique_ptr<RopeNode> rope_concat(std::unique_ptr<RopeNode> left,std::unique_ptr<RopeNode> right){
    std::unique_ptr<RopeNode> rope = std::make_unique<RopeNode>();
    rope->left.swap(left);
    rope->right.swap(right);
    rope->weight = rope_weight_measure(*(rope));
    return std::move(rope);
}

void rope_prepend(RopeNode *rope, const char *text){
    rope_prepend(rope, rope_create_node(text));
}
/*
    connects the given rope before the beggining;
        consumes the ownership of the given rope
*/
void rope_prepend(RopeNode *rope,std::unique_ptr<RopeNode> prope){
    std::stack<RopeNode*> nodeStack;
    size_t prope_weight = prope->weight;
    //get left-most leaf
    RopeNode *left_most = rope_left_most_node_trace(*rope, &nodeStack);
    //prepend the given rope
    left_most->left.swap(prope);
    left_most->weight = prope_weight;
    //push text to the new right node
    left_most->right = rope_create_node(left_most->text.get());
    left_most->text.release();
    //go up the stack changing weight
    RopeNode *current;
    while(!nodeStack.empty()){
        current = nodeStack.top();
        current->weight += prope_weight;
        nodeStack.pop();
    }
}

void rope_append(RopeNode *rope, const char *text){
    rope_append(rope, rope_create_node(text));
}
/*
    connects the given rope to the end;
            consumes the ownership of the given rope

*/
void rope_append(RopeNode *rope,std::unique_ptr<RopeNode> prope){
    std::stack<RopeNode*> nodeStack;
    size_t prope_weight = prope->weight;
    //get right-most leaf
    RopeNode *right_most = rope_node_at_index_trace(*rope, rope->weight-1, &nodeStack, NULL);
    //append the given rope
    right_most->right.swap(prope);
    //push text to the new left node
    right_most->left = rope_create_node(right_most->text.get());
    right_most->text.release();
    //go up the stack changing weight
    RopeNode *current, *prev=right_most;
    while(!nodeStack.empty()){
        current = nodeStack.top();
        //add weight only if the previous node is current nodes right childs
        if(current->left.get() == prev)
            current->weight += prope_weight;
        nodeStack.pop();
        prev = current;
    }
}

void rope_insert_at(RopeNode *rope, size_t index, const char *text){
    rope_insert_at(rope, index, rope_create_node(text));
}
/*
    inserts the given rope at the given index;
            consumes the ownership of the given rope
*/
void rope_insert_at(RopeNode *rope, size_t index, std::unique_ptr<RopeNode> prope){
    if(rope == nullptr){
        //TODO:error
        return;
    }
    if(index >= rope->weight){
        //TODO:error
        return;
    }

    if(index == 0){
        rope_prepend(rope, std::move(prope));
    }else if(index == rope->weight - 1){
        rope_append(rope, std::move(prope));
    }else{
        std::unique_ptr<RopeNode> right_side = rope_split_at(rope, index);
        rope_append(rope, std::move(prope));
        rope_append(rope, std::move(right_side));
    }

}

/*
    removes the given number of characters at the given index
*/
void rope_delete_at(RopeNode *rope, size_t index, size_t length){
    if(rope == nullptr){
        //TODO:error
        return;
    }
    if(length < 0 ||
         index+length >= rope->weight){
        //TODO:error
        return;
    }
    std::unique_ptr<RopeNode> deleted_rope = rope_split_at(rope, index);
    std::unique_ptr<RopeNode> right_side = rope_split_at(deleted_rope.get(), length-1);
    rope_append(rope, std::move(right_side));
}

/*
    go trough tree measuring actual weight
*/
size_t rope_weight_measure(const RopeNode &rope){
    //leaf, measure it's weight
    if(rope.left == nullptr && rope.right == nullptr)
        return rope.text != nullptr ? strlen(rope.text.get()):0;
    //weight = weight(left) + weight(rightside)
    RopeNode *curr_right = rope.left->right.get();
    size_t right_side = 0;
    while(curr_right != nullptr){
        right_side += rope_weight_measure(*curr_right);
        curr_right = curr_right->right.get();
    }
    
    return rope_weight_measure(*(rope.left)) + right_side;
    
}

/*
    go trough tree measuring and setting actual weight,
        returns total weight
*/
size_t rope_weight_measure_set(RopeNode *rope){
    if(rope==nullptr){
        //TODO:error
        return 0;
    }
    //leaf, measure it's weight
    if(rope->left == nullptr && rope->right == nullptr){
        rope->weight = rope->text != nullptr ? strlen(rope->text.get()):0;
        return rope->weight;
    }
    //weight = weight(left) + weight(rightside)
    RopeNode *curr_right = rope->left->right.get();
    size_t right_side = 0;
    while(curr_right != nullptr){
        right_side += rope_weight_measure_set(curr_right);
        curr_right = curr_right->right.get();
    }
    rope->left->weight = rope_weight_measure_set(rope->left.get());
    rope->weight = rope->left->weight + right_side;
    return rope->weight;
}

/*
    iterate trough nodes measuring height
*/
size_t rope_height_measure(const RopeNode &rope){
    if(rope.left == nullptr && rope.right == nullptr)
        return 0;

    size_t left_height = rope.left != nullptr ?  rope_height_measure(*(rope.left)) : 0;
    size_t right_height = rope.left != nullptr ?  rope_height_measure(*(rope.left)) : 0;
    return 1 + std::max(left_height, right_height);
}

/*
    the height of the left and right subtree of any node differ by not more than 1
*/
bool rope_is_balanced(const RopeNode &rope){
    long left_height = rope.left != nullptr ? static_cast<long>(rope_height_measure(*(rope.left))) : 0;
    long right_height = rope.right != nullptr ? static_cast<long>(rope_height_measure(*(rope.right))) : 0;
    return abs(left_height - right_height)<=1;
}

/*
    helper
*/
void _harvest(std::unique_ptr<RopeNode> rope, std::vector<std::unique_ptr<RopeNode>> *nodeVector){
    if(rope == nullptr){
        //TODO:error
        return;
    }
    //if leaf, collect
    if(rope->left == nullptr && rope->right == nullptr){
        (*nodeVector).push_back(std::move(rope));
        return;
    }
    //if children are leaves, collect them
    if(rope->left != nullptr &&
        rope->left->left == nullptr && rope->left->right == nullptr){
        (*nodeVector).push_back(std::move(rope->left));
    }
    if(rope->right != nullptr &&
        rope->right->left == nullptr && rope->right->right == nullptr){
        (*nodeVector).push_back(std::move(rope->right)); 
    }
    std::stack<RopeNode*> nodeStack;
    //get left-most; collect parent->left
    if(rope->left != nullptr){
        RopeNode *lm = rope_left_most_node_trace(*rope, &nodeStack);
        RopeNode *p = nodeStack.top();
        nodeVector->push_back(std::move(p->left));
        while(!nodeStack.empty()){
            //if some of the parents have the right child, harvest it
            p = nodeStack.top();
            if(p->right != nullptr)
                _harvest(std::move(p->right), nodeVector);
        
            nodeStack.pop();
        }
    }else if(rope->right != nullptr){
         _harvest(std::move(rope->right), nodeVector);
    }
    //destroy old rope
    rope_destroy(std::move(rope));
}
/*
    helper
*/
std::unique_ptr<RopeNode> _merge(std::vector<std::unique_ptr<RopeNode>> *nodeVector, size_t left, size_t right){
    size_t range = right - left;
    if(range==0){
        return std::move((*nodeVector)[left]);
    }else if(range == 1){
        return rope_concat(std::move((*nodeVector)[left]), std::move((*nodeVector)[right]));
    }
    size_t mid = left + (range/2);
    return rope_concat(_merge(nodeVector, left, mid), _merge(nodeVector, mid+1, right));
}

/*
    collect all of the leaves, and build a new tree from the bottom up,
        on error return nullptr;
            consumes ownership
*/
std::unique_ptr<RopeNode> rope_rebalance(std::unique_ptr<RopeNode> rope){
    if(rope == nullptr){
        //TODO:error
        return nullptr;
    }
    std::vector<std::unique_ptr<RopeNode>> nodeVector;
    //collect leaves, destroy rope
    _harvest(std::move(rope), &nodeVector);
    //merge collected leaves into a new balanced rope
    std::unique_ptr<RopeNode> left_sub = _merge(&nodeVector,0, nodeVector.size()-1);
    std::unique_ptr<RopeNode> right_sub;
    return std::move(rope_concat(std::move(left_sub), std::move(right_sub)));
}
/*
    helper
*/
void _split_node(RopeNode *node, size_t index, std::unique_ptr<RopeNode> &left,  std::unique_ptr<RopeNode> &right){
    if(node == nullptr || node->text == nullptr ||
        !(node->left == nullptr && node->right == nullptr)){
                    //TODO:error
        return;
    }
    size_t length = strlen(node->text.get());
    if(index >= length){
        //TODO:error
        return;
    }
    left = std::make_unique<RopeNode>();
    right = std::make_unique<RopeNode>();
    //left, up to index, including index
    left->weight = index+1;                   
    left->text = std::make_unique<char []>(left->weight+1);
    snprintf(left->text.get(), left->weight + 1, "%s", node->text.get());//The generated string has a length of at most n-1    
    //right, from text index                            //leaving space for the additional terminating null character.
    right->weight = length - (index+1);
    right->text = std::make_unique<char []>(right->weight+1);
    snprintf(right->text.get(), right->weight + 1, "%s", node->text.get() + (index+1));
    //delete text from parent; set weight to left childs weight; connect left child
    node->text.release();
    node->weight = left->weight;
}

/*
    cuts rope so that it contains characters up to index; 
        returns new rope containing characters from index;
            on error returns nullptr
*/
std::unique_ptr<RopeNode> rope_split_at(RopeNode *rope,size_t index){
    if(rope == nullptr){
        //TODO:error
        return nullptr;
    }
     //split node; weight = index;delete text;new rope is split_right
    if(rope->left == nullptr && rope->right == nullptr){
        std::unique_ptr<RopeNode> split_left, split_right;
        _split_node(rope, index, split_left, split_right);
        rope->left.swap(split_left);
        return std::move(split_right);
    }//go left; cut right; new rope is split(left) and right
    else if(index+1 < rope->weight){
        rope->weight = index+1;
        std::unique_ptr<RopeNode> left_split = rope_split_at(rope->left.get(), index);
        std::unique_ptr<RopeNode> right;
        rope->right.swap(right);
        return std::move(rope_concat(std::move(left_split), std::move(right)));
    }//index-=weight;go right;new rope is split(right)
    else if(index+1 > rope->weight){
        return rope_split_at(rope->right.get(), index-rope->weight);
    }//cut right; new rope is right node
    else if(index+1 == rope->weight){
        std::unique_ptr<RopeNode> right;
        rope->right.swap(right);
        return std::move(right);
    }

    //shouldn't happen
    return nullptr;
}

/*
    returns node that contains character at the given index,
        putting nodes on the way in the stack,
            on error returns null
*/
RopeNode* rope_node_at_index_trace(RopeNode &rope,size_t index, std::stack<RopeNode*> *nodeStack, size_t *local_index){
    //leaf
    if(rope.left == nullptr && rope.right == nullptr){
        if(index+1 > rope.weight){
            //TODO:error
            return nullptr;
        }
        if(local_index != nullptr)
            *local_index = index;
        return &rope;
    }
    if(index + 1 > rope.weight){
        nodeStack->push(&rope);
        return rope_node_at_index_trace(*(rope.right), index-rope.weight, nodeStack, local_index);
    }else{
        nodeStack->push(&rope);
        return rope_node_at_index_trace(*(rope.left), index, nodeStack, local_index);
    }
}

/*
    returns node that contains character at the given index;
            on error returns null
*/
RopeNode* rope_node_at_index(RopeNode &rope,size_t index, size_t *local_index){
    //leaf
    if(rope.left == nullptr && rope.right == nullptr){
        if(index+1 > rope.weight){
            //TODO:error
            return nullptr;
        }
        if(local_index != nullptr)
            *local_index = index;
        return &rope;
    }
    if(index + 1 > rope.weight){
        return rope_node_at_index(*(rope.right), index-rope.weight, local_index);
    }else{
        return rope_node_at_index(*(rope.left), index, local_index);
    }
}

/*
    returns left-most node in the given rope,
        putting nodes on the way in the stack,
            on error returns null
*/
RopeNode* rope_left_most_node_trace(RopeNode &rope, std::stack<RopeNode*> *nodeStack){
    if(rope.left == nullptr){
        return &rope;
    }
    else{
        nodeStack->push(&rope);
        return rope_left_most_node_trace(*(rope.left), nodeStack);
    }
}

/*
    returns left-most node in the given rope;
            on error returns null
*/
RopeNode* rope_left_most_node(RopeNode &rope){
    if(rope.left == nullptr)
        return &rope;
    else
        return rope_left_most_node(*(rope.left));
}

/*
    generates DOT for a given rope
*/
std::string rope_dot(const RopeNode &rope){
    std::ostringstream DOT;
    std::queue<const RopeNode*> nodeQueue;
    nodeQueue.push(&rope);
    const RopeNode *c;

    DOT<<("graph rope{\n");
    while(!nodeQueue.empty()){
        c = nodeQueue.front();
        if(c == nullptr){
            //TODO:error
            continue;
        }
        if(c->left != nullptr)
            nodeQueue.push(c->left.get());
        if(c->right != nullptr)
            nodeQueue.push(c->right.get());

        //
        DOT << "\"" << c << "\"" << "[label=\"" + std::to_string(c->weight)+"\n"+(c->text==nullptr?"":c->text.get())+"\"];\n";
        if(c->left)
            DOT << "\"" << c << "\"" << "--" << "\"" << c->left.get() << "\"" << "\n";
        if(c->right)
            DOT << "\"" << c << "\"" << "--" <<  "\"" << c->right.get() << "\"" << "\n";
        
        nodeQueue.pop();
    }
    DOT << "}";
    return DOT.str();
}

