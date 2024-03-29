#ifndef ROPE_H
#define ROPE_H


#include <memory>
#include <cstddef>
#include <stack>
#include <queue>
#include <string>
#include <bitset>



inline const size_t                 MAX_WEIGHT = 512;


//flags
inline const uint8_t                FLAG_NEW_LINE   = 0b00000001;
inline const uint8_t                FLAG_INVERT     = 0b00000010;

struct RopeNode;

struct RopeFlags final{
    std::bitset<8>              effects;


    RopeFlags();

    RopeFlags(const RopeFlags&) = delete;
    RopeFlags& operator=(const RopeFlags&) = delete;
};


struct RopeNode final{
    std::unique_ptr<char []>                    text;
    std::size_t                                 weight;
    std::unique_ptr<RopeFlags>                  flags;

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
    explicit RopeIteratorBFS(RopeNode *rope, size_t start = 0);

    RopeNode* next() override final;
    bool      hasNext() override final;
    RopeNode* pop() override final;
};

class RopeLeafIterator  :   public RopeIterator{
protected:
    std::stack<RopeNode*>   nodeStack;
    size_t                  localStartIndex;
    explicit                RopeLeafIterator() : localStartIndex{0}{}
public:


    explicit RopeLeafIterator(RopeNode *rope, size_t start = 0);

    RopeNode* next() override;
    bool      hasNext() override;
    RopeNode* pop() override;
    size_t    local_start_index();
};

class RopeLeafIteratorBack  :   public RopeLeafIterator{
public:
    explicit RopeLeafIteratorBack(RopeNode *rope, size_t start = 0);

    RopeNode* next() override final;
    bool      hasNext() override final;
    RopeNode* pop() override final;
};


size_t                          ustrlen(const char *);
size_t                          ustrlen(const std::string &);
size_t                          u_index_at(const char *, size_t );
bool                            has_flags(RopeFlags *, const uint8_t);


std::unique_ptr<RopeNode>       rope_create_empty();
std::unique_ptr<RopeNode>       rope_create_node(const char*);
std::unique_ptr<RopeNode>       rope_create_node(const char*, std::bitset<8>);
std::unique_ptr<RopeNode>       rope_create(const char*);
std::unique_ptr<RopeNode>       rope_create(const char*, std::bitset<8>);
void                            rope_destroy(std::unique_ptr<RopeNode>);
std::unique_ptr<RopeNode>       rope_concat(std::unique_ptr<RopeNode>,const char*);
std::unique_ptr<RopeNode>       rope_concat(std::unique_ptr<RopeNode>,std::unique_ptr<RopeNode>);
void                            rope_prepend(RopeNode*,const char*);
void                            rope_prepend(RopeNode*,std::unique_ptr<RopeNode>);
void                            rope_append(RopeNode*,const char*);
void                            rope_append(RopeNode*,std::unique_ptr<RopeNode>);
void                            rope_insert_at(RopeNode*,size_t,const char*);
void                            rope_insert_at(RopeNode*,size_t,std::unique_ptr<RopeNode>);
void                            rope_insert_flag_at(RopeNode*, size_t, size_t, uint8_t);
void                            rope_delete_at(RopeNode*, size_t, size_t);
size_t                          rope_weight_measure(const RopeNode&);
size_t                          rope_weight_measure_set(RopeNode*);
size_t                          rope_height_measure(const RopeNode&);
bool                            rope_is_balanced(const RopeNode&);
bool                            rope_has_flag_at(RopeNode&, size_t, size_t, uint8_t);
std::unique_ptr<RopeNode>       rope_rebalance(std::unique_ptr<RopeNode>);
std::unique_ptr<RopeNode>       rope_split_at(RopeNode*,size_t);
RopeNode*                       rope_node_at_index_trace(RopeNode&,size_t,std::stack<RopeNode*>*,size_t*);
RopeNode*                       rope_node_at_index(RopeNode&,size_t,size_t*);
RopeNode*                       rope_left_most_node_trace(RopeNode&,std::stack<RopeNode*>*);
RopeNode*                       rope_left_most_node(RopeNode&);
RopeNode*                       rope_right_most_node_trace(RopeNode&,std::stack<RopeNode*>*);
RopeNode*                       rope_right_most_node(RopeNode&);
RopeNode*                       rope_range(RopeNode&, size_t, size_t, size_t *);
std::string                     rope_dot(const RopeNode&);


#endif