#include "termija.h"
#include <plog/Log.h>


#include <cstring>
#include <iostream>//destroy

namespace termija{


PaneFrame::PaneFrame() :
    beginning{0},
    end{0},
    localIndexStart{0},
    localBeginning{0},
    nodeStart{nullptr}{};


Cursor::Cursor() :
    x{0},
    y{0},
    index{0},
    isDrawn{true}{};

Pane::Pane(uint16_t topX, uint16_t topY, uint16_t width, uint16_t height) :
    top{nullptr},
    bottom{nullptr},
    left{nullptr},
    right{nullptr},
    topX{topX},
    topY{topY},
    oldWidth{width},
    oldHeight{height},
    width{width},
    height{height},
    textMargin{1}{

    //rope
    this->ropes.push_back(std::move(rope_create_empty()));
    this->currentRopeIndex = 0;
    this->rope = this->ropes[this->currentRopeIndex].get();

    //frame
    this->frame.nodeStart = this->rope;
}

void tra_update_pane(Pane& pane){
    
    //check if resized
    if(pane.oldWidth != pane.width ||
        pane.oldHeight != pane.height){
        //if not init resize
        if(!(pane.oldHeight == 0 && pane.oldWidth == 0)){
            tra_pane_is_resized(pane, pane.width - pane.oldWidth, pane.height - pane.oldHeight);
        }
        //update old values
        pane.oldWidth = pane.width;
        pane.oldHeight = pane.height;
    }

    //update widgets
    for(size_t i = 0; i < pane.widgets.size(); i++){
        Widget *widget = pane.widgets[i].get();
        if(widget == nullptr){
            PLOG_ERROR << "widget at index: " << i << " is NULL, skipped.";
            continue;
        }
        widget->update();
    }
}


Pane* tra_split_pane_vertically(Pane &pane){
    return tra_split_pane_vertically(pane, (pane.width /2));
}
/*
    splits the given pane vertically, shortening it's width and creating new pane 
*/
Pane* tra_split_pane_vertically(Pane &pane, uint16_t width){
    Termija& termija = Termija::instance();

    if(pane.width <= width){
        PLOG_ERROR << "given width is bigger than the pane width, aborted";
        return nullptr;
    }
    //rescale given pane
    pane.width -= width + (termija.paneMargin/2);
    //create new pane
    Pane* new_pane = tra_add_pane(pane.topX + pane.width + termija.paneMargin, pane.topY, width - (termija.paneMargin/2), pane.height);
    //connect panes
    new_pane->left = &pane;
    new_pane->right = pane.right;
    new_pane->top = pane.top;
    new_pane->bottom = pane.bottom;
    pane.right = new_pane;
    return new_pane;
}

Pane* tra_split_pane_horizontally(Pane &pane){
    return tra_split_pane_horizontally(pane,(pane.height / 2));
}
/*
    splits the given pane horizontally, shortening it't height and creating new pane 
*/
Pane* tra_split_pane_horizontally(Pane &pane, uint16_t height){
    Termija& termija = Termija::instance();

    if(pane.height <= height){
        PLOG_ERROR << "given height is bigger than the pane height, aborted";
        return nullptr;
    }
    //rescale given pane
    pane.height -= height + (termija.paneMargin/2);
    //create new pane
    Pane* new_pane = tra_add_pane(pane.topX,pane.topY +  pane.height + termija.paneMargin, pane.width, height - (termija.paneMargin/2));
    //connect panes
    new_pane->left = pane.left;
    new_pane->right = pane.right;
    new_pane->top = &pane;
    new_pane->bottom = pane.bottom;
    pane.bottom = new_pane;
    return new_pane;
}

/*
    removes second pane, changing size of the first one;
        panes must be neighbours
*/
Pane* tra_merge_panes(Pane &pane, Pane &toMerge){
    size_t paneMargin = tra_get_pane_margin();

    //check if given panes are neighbours
    if(pane.left == &toMerge){
        pane.topX = toMerge.topX;
        pane.width = pane.width + paneMargin + toMerge.width;
    }else if(pane.right == &toMerge){
        pane.width = pane.width + paneMargin + toMerge.width;
    }else if(pane.top == &toMerge){
        pane.topY = toMerge.topY;
        pane.height = pane.height + paneMargin + toMerge.height;
    }else if(pane.bottom == &toMerge){
        pane.height = pane.height + paneMargin + toMerge.height;
    }else{
        PLOG_WARNING << "given panes must be neigbours, aborted";
        return nullptr;
    }
    tra_remove_pane(&toMerge);
    return &pane;
}

/*
    pane resized
*/
void tra_pane_is_resized(Pane &pane, int16_t widthDiff, int16_t heightDiff){

    //resize widgets
    for(size_t i = 0; i < pane.widgets.size(); i++){
        Widget *widget = pane.widgets[i].get();
        if(widget == nullptr){
            PLOG_ERROR << "widget at index: " << i << " is NULL, skipped.";
            continue;
        }
        widget->on_pane_resize(widthDiff, heightDiff);
    }
    //position cursor and frame
    rope_weight_measure_set(pane.rope);
    tra_position_cursor(pane, pane.rope->weight - 1);
    tra_position_pane_frame(pane);
}


/*
    inserts given text inside frame, at cursor position
*/
RopeFlags* tra_insert_text_at_cursor(Pane& pane,const char* text){
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted";
        return nullptr;
    }
    //local index inside frame
    size_t localIndex = pane.cursor.index - pane.frame.localIndexStart;
    size_t weight = rope_weight_measure(*(pane.frame.nodeStart));
    if(localIndex >= weight && weight != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return nullptr;
    }
    //insert at index, return inserted node
    rope_insert_at(pane.frame.nodeStart, localIndex, text);
    weight = rope_weight_measure_set(pane.rope);
    RopeNode *node = rope_node_at_index(*(pane.frame.nodeStart), localIndex + 1, NULL);
    if(node == nullptr){
        PLOG_ERROR << "couldn't find node at index.";
        return nullptr;
    }
    //rope rebalance
    if(!rope_is_balanced(*(pane.rope))){
        pane.ropes[pane.currentRopeIndex] = rope_rebalance(std::move(pane.ropes[pane.currentRopeIndex]));
        pane.rope = pane.ropes[pane.currentRopeIndex].get();
        pane.frame.nodeStart = pane.rope;
    }
    // cursor index at cursor index + inserted length
    // position frame
    tra_position_cursor(pane, pane.cursor.index + strlen(text));
    tra_position_pane_frame(pane);

    return node->flags.get();
}

/*
    inserts line of text
*/
RopeFlags* tra_insert_text_line_at_cursor(Pane& pane,const char* text){
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted";
        return nullptr;
    }
    //local index inside frame
    size_t localIndex = pane.cursor.index - pane.frame.localIndexStart;
    size_t weight = rope_weight_measure(*(pane.frame.nodeStart));
    if(localIndex >= weight && weight != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return nullptr;
    }
    //postWeight = weight until end of the line
    uint16_t textWidth = tra_get_text_width(pane);
    if(textWidth == 0){
        PLOG_ERROR << "couln't get textWidth, aborted.";
        return nullptr;
    }
    size_t weightUntilEnd = textWidth - ((pane.cursor.x + strlen(text)) % textWidth);
    //insert node at index
    rope_insert_at(pane.frame.nodeStart, localIndex, rope_create_node(text, 0, weightUntilEnd, 0));
    weight = rope_weight_measure_set(pane.rope);
    RopeNode *node = rope_node_at_index(*(pane.frame.nodeStart), localIndex + 1, NULL);
    if(node == nullptr){
        PLOG_ERROR << "couldn't find node at index.";
        return nullptr;
    }
    //rope rebalance
    if(!rope_is_balanced(*(pane.rope))){
        pane.ropes[pane.currentRopeIndex] = rope_rebalance(std::move(pane.ropes[pane.currentRopeIndex]));
        pane.rope = pane.ropes[pane.currentRopeIndex].get();
        pane.frame.nodeStart = pane.rope;
    }
    // cursor index at cursor index + inserted length
    // position frame
    tra_position_cursor(pane, pane.cursor.index + strlen(text) + weightUntilEnd);
    tra_position_pane_frame(pane);
    //create line widget
    pane.widgets.push_back(std::make_unique<Line>(node, strlen(text) + weightUntilEnd));
    //return created nodes flags
    return node->flags.get();
}


void tra_pane_destroy_rope(Pane& pane){
   for(size_t i=0;i<pane.ropes.size();i++) 
        rope_destroy(std::move(pane.ropes[i]));
}

/*
    positions pane frame so that cursor is positioned at it's pane coordinates 
*/
void tra_position_pane_frame(Pane &pane){
    size_t endRopeIndex = rope_weight_measure(*(pane.rope));
    size_t textWidth = tra_get_text_width(pane);
    size_t textHeight = tra_get_text_height(pane);
    if(textWidth == 0 || textHeight == 0){
        PLOG_ERROR << "invalid textWidth|textHeight, aborted";
        return;
    }
    endRopeIndex = endRopeIndex == 0 ? endRopeIndex : endRopeIndex - 1;
    //length to and from cursor
    uint16_t preLength = (textWidth * (pane.cursor.y)) + pane.cursor.x;
    preLength = preLength>0?preLength-1:preLength;//index from 0
    uint16_t postLength =  (textWidth * (textHeight - pane.cursor.y)) + (textWidth - pane.cursor.x);
    postLength = postLength>0?postLength-1:postLength;//index from 0
    //calculate beggining and end index
    pane.frame.beginning = std::max(((int32_t)pane.cursor.index - (int32_t)preLength ), (int32_t)0);
    pane.frame.end = std::min((pane.cursor.index + postLength), endRopeIndex);
    //find range in rope
    pane.frame.nodeStart = rope_range(*(pane.rope), pane.frame.beginning, pane.frame.end, &pane.frame.localIndexStart);
    //local beggining index
    pane.frame.localBeginning = pane.frame.beginning - pane.frame.localIndexStart;
}

const Cursor& tra_get_cursor(Pane& pane){
    return pane.cursor;
}

/*
    positions cursor at index
*/
void tra_position_cursor(Pane &pane, size_t index){
    size_t textWidth = tra_get_text_width(pane);
    size_t textHeight = tra_get_text_height(pane);
    if(textWidth == 0 || textHeight == 0){
        PLOG_ERROR << "invalid textWidth|textHeight, aborted";
        return;
    }
    pane.cursor.index = std::min(index, pane.rope->weight - 1);
    index = pane.cursor.index + 1; //index from 0
    //put at coordinates starting from 0,0
    pane.cursor.y = index / textWidth;
    pane.cursor.x = index % textWidth;

    //out of bounds
    if(pane.cursor.y >= textHeight){
        pane.cursor.y = textHeight - 1;
    }
}

/*
    positions cursor at coordinates
*/
void tra_position_cursor(Pane &pane, uint16_t x, uint16_t y){
    size_t textWidth = tra_get_text_width(pane);
    size_t textHeight = tra_get_text_height(pane);
    if(textWidth == 0 || textHeight == 0){
        PLOG_ERROR << "invalid textWidth|textHeight, aborted";
        return;
    }
    uint16_t preLength = (textWidth * (pane.cursor.y)) + pane.cursor.x;
    size_t startIndex = pane.cursor.index - preLength;
    size_t index = std::min(pane.rope->weight - 1, startIndex + ((y*textWidth) + x));
    tra_position_cursor(pane, index);
}
}