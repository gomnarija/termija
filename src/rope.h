#ifndef ROPE_H
#define ROPE_H


#include <memory>
#include <cstddef>
#include <stack>
#include <queue>
#include <string>
#include <bitset>



inline const size_t                MAX_WEIGHT = 512;



struct RopeNode final{
    std::bitset<7>                              flags;
    std::unique_ptr<char []>                    text;
    std::size_t                                 weight;

    std::unique_ptr<RopeNode>                   left;
    std::unique_ptr<RopeNode>                   right;

    RopeNode();

    RopeNode(const RopeNode&) = delete;
    RopeNode& operator=(const RopeNode&) = delete;
};


class RopeIterator{
protected:
    RopeNode                *current;
    RopeIterator() 
            : current{nullptr}{};

public:
    virtual RopeNode* get() const {return current;}
    virtual RopeNode* next() = 0;
    virtual bool      hasNext() = 0;
    virtual RopeNode* pop() = 0;
};

class RopeIteratorBFS   :   public RopeIterator{
private:
    std::queue<RopeNode*>   nodeQueue;

public:
    RopeIteratorBFS(RopeNode *rope, size_t start = 0);

    RopeNode* next() override final;
    bool      hasNext() override final;
    RopeNode* pop() override final;
};

class RopeLeafIterator  :   public RopeIterator{
private:
    std::stack<RopeNode*>   nodeStack;

public:
    RopeLeafIterator(RopeNode *rope, size_t start = 0);

    RopeNode* next() override final;
    bool      hasNext() override final;
    RopeNode* pop() override final;
};


std::unique_ptr<RopeNode>       rope_create_empty();
std::unique_ptr<RopeNode>       rope_create_node(const char*);
std::unique_ptr<RopeNode>       rope_create(const char*);
void                            rope_destroy(std::unique_ptr<RopeNode>);
std::unique_ptr<RopeNode>       rope_concat(std::unique_ptr<RopeNode>,const char*);
std::unique_ptr<RopeNode>       rope_concat(std::unique_ptr<RopeNode>,std::unique_ptr<RopeNode>);
void                            rope_prepend(RopeNode*,const char*);
void                            rope_prepend(RopeNode*,std::unique_ptr<RopeNode>);
void                            rope_append(RopeNode*,const char*);
void                            rope_append(RopeNode*,std::unique_ptr<RopeNode>);
void                            rope_insert_at(RopeNode*,size_t,const char*);
void                            rope_insert_at(RopeNode*,size_t,std::unique_ptr<RopeNode>);
void                            rope_delete_at(RopeNode*, size_t, size_t);
size_t                          rope_weight_measure(const RopeNode&);
size_t                          rope_weight_measure_set(RopeNode*);
size_t                          rope_height_measure(const RopeNode&);
bool                            rope_is_balanced(const RopeNode&);
std::unique_ptr<RopeNode>       rope_rebalance(std::unique_ptr<RopeNode>);
std::unique_ptr<RopeNode>       rope_split_at(RopeNode*,size_t);
RopeNode*                       rope_node_at_index_trace(RopeNode&,size_t,std::stack<RopeNode*>*,size_t*);
RopeNode*                       rope_node_at_index(RopeNode&,size_t,size_t*);
RopeNode*                       rope_left_most_node_trace(RopeNode&,std::stack<RopeNode*>*);
RopeNode*                       rope_left_most_node(RopeNode&);
std::string                     rope_dot(const RopeNode&);


#endif