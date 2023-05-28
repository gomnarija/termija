#include "rope.h"
#include <plog/Log.h>


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
RopeNode*                                       _rope_node_at_index_trace_right(RopeNode &,size_t, std::stack<RopeNode*> *, size_t *);
size_t                                          _rope_weight_measure_node(const RopeNode&);

RopeFlags::RopeFlags()
: effects{0}
{}

RopeNode::RopeNode()
: weight{0}, text{nullptr}, left{nullptr}, right{nullptr}
{
    this->flags = std::make_unique<RopeFlags>();
}


RopeIteratorBFS::RopeIteratorBFS(RopeNode *rope, size_t start){
    if(rope == nullptr){
        PLOG_ERROR << "given rope is NULL, aborted.";
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
            PLOG_ERROR << "given start point is bigger than rope size, aborted.";
            current = nullptr;
            return;
        }
    }
}
RopeNode* RopeIteratorBFS::next(){
    if(!hasNext()){
        PLOG_WARNING << "there is no next element, aborted.";
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
        PLOG_ERROR << "given rope is NULL, aborted.";
        return;
    }
    //find node at index
    localStartIndex = 0;
    current = _rope_node_at_index_trace_right(*rope, start, &nodeStack, &localStartIndex);
}
RopeNode* RopeLeafIterator::next(){
    if(!hasNext()){
        PLOG_WARNING << "there is no next element, aborted.";
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

RopeLeafIteratorBack::RopeLeafIteratorBack(RopeNode *rope, size_t start){
    if(rope == nullptr){
        PLOG_ERROR << "given rope is NULL, aborted.";
        return;
    }
    //find node at index
    localStartIndex = 0;
    current = rope_node_at_index_trace(*rope, start, &nodeStack, &localStartIndex);
}
RopeNode* RopeLeafIteratorBack::next(){
    if(!hasNext()){
        PLOG_WARNING << "there is no previous element, aborted.";
        return nullptr;
    }

    RopeNode *parent = nodeStack.top();
    nodeStack.pop();
    current = rope_node_at_index_trace(*(parent), parent->weight - 1, &nodeStack, NULL);
    return current;
}
bool RopeLeafIteratorBack::hasNext(){
    bool has_left_child = false;
    RopeNode *parent;
    RopeNode *prev=current;
    while(!nodeStack.empty() && !(has_left_child)){
        parent = nodeStack.top();
        nodeStack.pop();
        has_left_child = parent->left != nullptr && (parent->left.get() != prev);
        prev = parent;
    }
    if(has_left_child)
        nodeStack.push(parent);
    return has_left_child;
}
RopeNode* RopeLeafIteratorBack::pop(){
    if(!hasNext()){
        RopeNode *prev = current;
        current = nullptr;
        return prev;
    }
    RopeNode *prev = current;
    next();
    return prev;
}


size_t RopeLeafIterator::local_start_index(){
    return this->localStartIndex;
}

/*
    creates empty rope
*/
std::unique_ptr<RopeNode> rope_create_empty(){
    std::unique_ptr<RopeNode> rope = std::make_unique<RopeNode>();
    //rope->left = std::make_unique<RopeNode>();

    return std::move(rope);
}

std::unique_ptr<RopeNode> rope_create_node(const char* text){
    return rope_create_node(text, 0);
}

/*
    creates node with the given text and flags
*/
std::unique_ptr<RopeNode> rope_create_node(const char* text, std::bitset<8> effects){
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted.";
        return nullptr;
    }
    std::unique_ptr<RopeNode> node = std::make_unique<RopeNode>();
    //add text
    node->text = std::make_unique<char[]>(strlen(text) + 1);
    node->weight = sprintf(node->text.get(), "%s", text);
    //flags
    node->flags->effects = effects;
    //cut while too long
    RopeNode *n  = node.get();
    while(n->weight > MAX_WEIGHT){
        _split_node(n, std::max((n->weight / 2), n->weight-MAX_WEIGHT), n->left, n->right);
        n = n->left.get();
    }

    return std::move(node);
}

/*
    creates rope with the given text
*/
std::unique_ptr<RopeNode> rope_create(const char* text){
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted.";
        return nullptr;
    }
    //create new rope, and it's left node
    std::unique_ptr<RopeNode> rope = std::make_unique<RopeNode>();
    rope->left = std::make_unique<RopeNode>();
    RopeNode *n = rope->left.get();
    //add text to the left node
    rope->left->text = std::make_unique<char[]>(strlen(text)+1);
    rope->weight = rope->left->weight = sprintf(rope->left->text.get(), "%s", text);

    //cut while too long
    while(n->weight > MAX_WEIGHT){
        _split_node(n, std::max((n->weight / 2), n->weight-MAX_WEIGHT), n->left, n->right);
        n = n->left.get();
    }

    return std::move(rope);
}

/*
    destroys the given rope
*/
void rope_destroy(std::unique_ptr<RopeNode> rope){
    if(rope == nullptr){
        PLOG_ERROR << "given rope is NULL, aborted.";
        return;
    }

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
    if(prope == nullptr || rope == nullptr || prope->weight == 0){
        PLOG_ERROR << "given rope is NULL or empty, aborted.";
        return;
    }
    std::stack<RopeNode*> nodeStack;
    uint16_t prope_weight = prope->weight;

    //head
    if(rope->left == nullptr)
        rope->weight += prope_weight;
    
    //get left-most leaf
    RopeNode *left_most = rope_left_most_node_trace(*rope, &nodeStack);
    if(left_most == nullptr)
        return;

    //prepend the given rope
    left_most->left.swap(prope);

    //push text to the new right node
    left_most->right = left_most->text==nullptr?nullptr:rope_create_node(left_most->text.get());
    left_most->text.release();

    //move flags to the new right node
    if(left_most->right != nullptr)
        left_most->right->flags.swap(left_most->flags);

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
    if(prope == nullptr || rope == nullptr || prope->weight == 0){
        PLOG_ERROR << "given rope is NULL or empty, aborted.";
        return;
    }

    std::stack<RopeNode*> nodeStack;
    size_t prope_weight = _rope_weight_measure_node(*prope);

    //head
    if(rope->right == nullptr)
        rope->weight += prope_weight;

    //go left
    if(rope->left != nullptr){
        rope = rope->left.get();
    }

    //get right-most leaf
    RopeNode *right_most = rope_right_most_node_trace(*rope, &nodeStack);
    if(right_most == nullptr)
        return;

    //append the given rope
    if(right_most->left == nullptr && right_most->text == nullptr)
        right_most->left.swap(prope);
    else
        right_most->right.swap(prope);

    //push text to the new left node
    if(right_most->text != nullptr){
        right_most->left = rope_create_node(right_most->text.get());
        right_most->text.release();
        //move flags to the created node
        right_most->left->flags.swap(right_most->flags);
    }

    //go up the stack changing weight
    RopeNode *current, *prev=right_most;
    while(!nodeStack.empty()){
        current = nodeStack.top();
        //add weight only if the previous node is current nodes right childs
        if(current->left != nullptr &&
            current->left.get() == prev)
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
    if(rope == nullptr || prope == nullptr || prope->weight == 0){
        PLOG_ERROR << "given rope is NULL or empty, aborted.";
        return;
    }
    if(index >= rope->weight && rope->weight != 0){
        PLOG_ERROR << "given index is bigger than rope weight, aborted. index: " << index;
        return;
    }

    if(false){
        rope_prepend(rope, std::move(prope));
    }else{
        std::unique_ptr<RopeNode> right_side = rope_split_at(rope, index);
        
        rope_append(rope, std::move(prope));

        if(right_side != nullptr)
            rope_append(rope, std::move(right_side));
    }

}

/*
    removes the given number of characters at the given index
*/
void rope_delete_at(RopeNode *rope, size_t index, size_t length){
    if(rope == nullptr){
        PLOG_ERROR << "given rope is NULL, aborted.";
        return;
    }
    if(length == 0 ||
         index+length >= rope->weight){
        PLOG_ERROR << "index/length combination invalid, aborted." << "index: " << index << " length: " << length;
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
    size_t total_weight = 0;
    std::stack<const RopeNode*> node_stack;
    const RopeNode* current_node = &rope;
    
    //head
    if (current_node->left == nullptr && current_node->right == nullptr) {
        return current_node->text != nullptr ? strlen(current_node->text.get()) : 0;
    }

    //go left
    current_node = current_node->left.get();

    while (current_node != nullptr || !node_stack.empty()) {
        //find left-most
        while (current_node != nullptr) {
            node_stack.push(current_node);
            current_node = current_node->left.get();
        }

        current_node = node_stack.top();
        node_stack.pop();
        
        //leaf
        if (current_node->left == nullptr && current_node->right == nullptr) {
            total_weight += current_node->text != nullptr ? strlen(current_node->text.get()) : 0;
        }

        current_node = current_node->right.get();
    }

    return total_weight;
}

/*
    go trough tree measuring actual weight, including starting node;
        kinda like making a new root node and starting from there...
*/
size_t _rope_weight_measure_node(const RopeNode &rope){
    size_t total_weight = 0;
    std::stack<const RopeNode*> node_stack;
    const RopeNode* current_node = &rope;
    
    while (current_node != nullptr || !node_stack.empty()) {
        //find left-most
        while (current_node != nullptr) {
            node_stack.push(current_node);
            current_node = current_node->left.get();
        }

        current_node = node_stack.top();
        node_stack.pop();
        
        //leaf
        if (current_node->left == nullptr && current_node->right == nullptr) {
            total_weight += current_node->text != nullptr ? strlen(current_node->text.get()) : 0;
        }

        current_node = current_node->right.get();
    }

    return total_weight;
}

/*
    go trough tree measuring and setting actual weight,
        returns total weight
*/
size_t rope_weight_measure_set(RopeNode *rope) {
    if (rope == nullptr) {
        PLOG_ERROR << "given rope is NULL, aborted.";
        return 0;
    }

    size_t total_weight = 0;
    std::stack<RopeNode*> node_stack;
    node_stack.push(rope);

    while (!node_stack.empty()) {
        RopeNode* current_node = node_stack.top();
        node_stack.pop();

        if (current_node->left == nullptr && current_node->right == nullptr) {
            current_node->weight = current_node->text != nullptr ? std::strlen(current_node->text.get()) : 0;
            total_weight += current_node->weight;
        }
        else {
            if (current_node->left != nullptr) {
                node_stack.push(current_node->left.get());
            }
            if (current_node->right != nullptr) {
                node_stack.push(current_node->right.get());
            }
        }
    }

    return rope->weight;
}

/*
    iterate trough nodes measuring height
*/
size_t rope_height_measure(const RopeNode &rope){
    if(rope.left == nullptr && rope.right == nullptr)
        return 0;

    size_t left_height = rope.left != nullptr ?  rope_height_measure(*(rope.left)) : 0;
    size_t right_height = rope.right != nullptr ?  rope_height_measure(*(rope.right)) : 0;
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
        PLOG_ERROR << "given rope is NULL, aborted.";
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
        PLOG_ERROR << "given rope is NULL, aborted.";
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
void _split_flags(RopeFlags *flags, RopeFlags *left, RopeFlags *right){
    //new line goes to the right
    if(flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE){
        flags->effects ^= FLAG_NEW_LINE;
        right->effects |= FLAG_NEW_LINE;
    }
}


/*
    helper
*/
void _split_node(RopeNode *node, size_t index, std::unique_ptr<RopeNode> &left,  std::unique_ptr<RopeNode> &right){
    if(node == nullptr || node->text == nullptr ||
        !(node->left == nullptr && node->right == nullptr)){
        PLOG_ERROR << "given node not a leaf, aborted.";
        return;
    }

    size_t length = strlen(node->text.get());
    if(index >= length){
        PLOG_ERROR << "index bigger than length, aborted. index : " << index;
        return;
    }

    //new split nodes
    left = std::make_unique<RopeNode>();
    right = std::make_unique<RopeNode>();

    //left, up to index, including index
    left->weight = index+1;        
    {    
        //split text
        left->text = std::make_unique<char []>(left->weight+1);
        snprintf(left->text.get(), (left->weight)  + 1, "%s", node->text.get());//The generated string has a length of at most n-1    
    }                                       

    //right, from index                            
    right->weight = (length) - (index+1);
    {
        //split text
        right->text = std::make_unique<char []>(right->weight+1);
        snprintf(right->text.get(), (right->weight) + 1, "%s", node->text.get() + ((index)+1));
    }

    //split flags
    _split_flags(node->flags.get(), left->flags.get(), right->flags.get());
    //delete text from parent; set weight to left childs weight; connect left child
    node->text.release();
    node->weight = left->weight;
}

/*
    cuts rope so that it contains characters up to index, including index;
        returns new rope containing characters from index;
            on error returns nullptr
*/
std::unique_ptr<RopeNode> rope_split_at(RopeNode *rope,size_t index){
    if(rope == nullptr){
        PLOG_ERROR << "given rope is NULL, aborted.";
        return nullptr;
    }
    std::unique_ptr<RopeNode> new_rope = rope_create_empty();
    std::stack<RopeNode*> nodeStack;
    size_t local_index = 0;
    size_t removed_weight = 0;


    RopeNode *leaf = _rope_node_at_index_trace_right(*rope, index, &nodeStack, &local_index);
    if(leaf==nullptr)
        return nullptr;

    //split leaf, if index is not on the end
    if((leaf->weight - (local_index+1)) > 0){
        std::unique_ptr<RopeNode> split_left, split_right;
        _split_node(leaf, local_index, split_left, split_right);
        leaf->left.swap(split_left);
        
        if(split_right != nullptr){
            new_rope->weight = split_right->weight;
            removed_weight = new_rope->weight;
            new_rope->left.swap(split_right);
        }
    }

    //backtrace nodeStack and split, changing weight
    RopeNode *current;
    while(!nodeStack.empty()){
        current = nodeStack.top();
        if(current == nullptr)
            continue;

        current->weight -= removed_weight;

        if(current->right != nullptr){
            removed_weight += _rope_weight_measure_node(*(current->right));
            rope_append(new_rope.get(), std::move(current->right));
        }


        nodeStack.pop();
    }

    return new_rope->weight > 0 ? 
        std::move(new_rope) : nullptr;
}

/*
    returns node that contains character at the given index,
        putting nodes on the way in the stack,
            on error returns null
*/
RopeNode* rope_node_at_index_trace(RopeNode &rope,size_t index, std::stack<RopeNode*> *nodeStack, size_t *local_index){
    RopeNode *current = &rope;
    //find leaf
    while(current->left != nullptr || current->right != nullptr){
        if(index == 0){
        return rope_left_most_node_trace(*current, nodeStack);
        }
        //right
        if(index + 1 > current->weight){
            if(current->right != nullptr){
                nodeStack->push(current);//stack it
                index -= current->weight;
                current = current->right.get();
            }
            else{
                PLOG_ERROR << "index bigger than weight but right node is null, aborted.";
                return nullptr;
            }
        }else{//left
            if(current->left != nullptr){
                nodeStack->push(current);//stack it
                current = current->left.get();
            }else{
                PLOG_ERROR << "left node is null, aborted.";
                return nullptr;
            }
            
        }
    }

    //leaf
    if(current != nullptr &&
        current->left == nullptr && current->right == nullptr){
        if(index+1 > current->weight){
            PLOG_ERROR << "index bigger than weight, aborted. index : " << index;
            return nullptr;
        }
        if(local_index != nullptr)
            *local_index = index;
        return current;
    }

    return nullptr;
}

/*
    returns node that contains character at the given index,
        putting nodes on the way in the stack, only when moving left
            on error returns null
*/
RopeNode* _rope_node_at_index_trace_right(RopeNode &rope,size_t index, std::stack<RopeNode*> *nodeStack, size_t *local_index){
    RopeNode *current = &rope;
    //find leaf
    while(current->left != nullptr || current->right != nullptr){
        if(index == 0){
        return rope_left_most_node_trace(*current, nodeStack);
        }
        //right
        if(index + 1 > current->weight){
            if(current->right != nullptr){
                index -= current->weight;
                current = current->right.get();
            }
            else{
                PLOG_ERROR << "index bigger than weight but right node is null, aborted.";
                return nullptr;
            }
        }else{//left
            if(current->left != nullptr){
                nodeStack->push(current);//stack it
                current = current->left.get();
            }else{
                PLOG_ERROR << "left node is null, aborted.";
                return nullptr;
            }
        }
    }

    //leaf
    if(current != nullptr &&
        current->left == nullptr && current->right == nullptr){
        if(index+1 > current->weight){
            PLOG_ERROR << "index bigger than weight, aborted. index : " << index;
            return nullptr;
        }
        if(local_index != nullptr)
            *local_index = index;
        return current;
    }

    return nullptr;
}

/*
    returns node that contains character at the given index;
            on error returns null
*/
RopeNode* rope_node_at_index(RopeNode &rope,size_t index, size_t *local_index){
    RopeNode *current = &rope;
    //find leaf
    while(current->left != nullptr || current->right != nullptr){
        if(index == 0){
        return rope_left_most_node(*current);
        }
        //right
        if(index + 1 > current->weight){
            if(current->right != nullptr){
                index -= current->weight;
                current = current->right.get();
            }
            else{
                PLOG_ERROR << "index bigger than weight but right node is null, aborted.";
                return nullptr;
            }
        }else{//left
            if(current->left != nullptr){
                current = current->left.get();
            }else{
                PLOG_ERROR << "left node is null, aborted.";
                return nullptr;
            }
            
        }
    }

    //leaf
    if(current != nullptr &&
        current->left == nullptr && current->right == nullptr){
        if(index+1 > current->weight){
            PLOG_ERROR << "index bigger than weight, aborted. index : " << index;
            return nullptr;
        }
        if(local_index != nullptr)
            *local_index = index;
        return current;
    }

    return nullptr;
}

/*
    returns left-most node in the given rope,
        putting nodes on the way in the stack,
            on error returns null
*/
RopeNode* rope_left_most_node_trace(RopeNode &rope, std::stack<RopeNode*> *nodeStack){
    RopeNode *current = &rope;
    while(current->left != nullptr){
        nodeStack->push(current);
        current = current->left.get();
    }

    return current;
}

/*
    returns left-most node in the given rope;
            on error returns null
*/
RopeNode* rope_left_most_node(RopeNode &rope){
    RopeNode *current = &rope;
    while(current->left != nullptr){
        current = current->left.get();
    }

    return current;
}

/*
    returns right-most node in the given rope,
        putting nodes on the way in the stack,
            on error returns null
*/
RopeNode* rope_right_most_node_trace(RopeNode &rope, std::stack<RopeNode*> *nodeStack){
    RopeNode *current = &rope;
    while(current->right != nullptr){
        nodeStack->push(current);
        current = current->right.get();
    }

    return current;
}

/*
    returns right-most node in the given rope;
            on error returns null
*/
RopeNode* rope_right_most_node(RopeNode &rope){
    RopeNode *current = &rope;
    while(current->right != nullptr){
        current = current->right.get();
    }

    return current;
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
            PLOG_ERROR << "current node is NULL, skipping";
            continue;
        }
        if(c->left != nullptr)
            nodeQueue.push(c->left.get());
        if(c->right != nullptr)
            nodeQueue.push(c->right.get());

        //
        DOT << "\"" << c << "\"" << "[label=\"" + std::to_string(c->weight)+"\n"+((c->text==nullptr)||true?"":c->text.get())+"\"];\n";
        if(c->left)
            DOT << "\"" << c << "\"" << "--" << "\"" << c->left.get() << "\"" << "\n";
        if(c->right)
            DOT << "\"" << c << "\"" << "--" <<  "\"" << c->right.get() << "\"" << "\n";
        
        nodeQueue.pop();
    }
    DOT << "}";
    return DOT.str();
}

