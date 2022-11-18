#include "termija.h"
#include <plog/Log.h>


#include <cstring>

namespace termija{


PaneFrame::PaneFrame() :
    beginning{0},
    end{0},
    localIndexStart{0},
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
    width{width},
    height{height},
    textMargin{1},
    fontSize{DEFAULT_FONT_SIZE}{

    //rope
    this->rope = rope_create_empty();

    //frame
    this->frame.nodeStart = this->rope.get();
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
    inserts given text inside frame, at cursor position
*/
RopeFlags* tra_insert_text_at_cursor(Pane& pane,const char* text){
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted";
        return nullptr;
    }
    //insert at local index inside frame
    size_t localIndex = pane.cursor.index - pane.frame.localIndexStart;
    size_t weight = rope_weight_measure(*(pane.frame.nodeStart));
    if(localIndex >= weight && weight != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return nullptr;
    }
    //insert at index, return inserted node
    rope_insert_at(pane.frame.nodeStart, localIndex, text);
    weight = rope_weight_measure_set(pane.rope.get());
    RopeNode *node = rope_node_at_index(*(pane.frame.nodeStart), localIndex + 1, NULL);
    if(node == nullptr){
        PLOG_ERROR << "couldn't find node at index.";
        return nullptr;
    }
    // cursor
    tra_move_cursor_up(&pane, strlen(text));
    tra_position_pane_frame(pane);
    return node->flags.get();
}

void tra_pane_destroy_rope(Pane& pane){
    rope_destroy(std::move(pane.rope));
}

/*
    positions pane frame so that cursor is positioned at it's pane coordinates 
*/
void tra_position_pane_frame(Pane &pane){
    size_t endRopeIndex = rope_weight_measure(*(pane.rope));
    size_t textWidth = pane.width - (2*pane.textMargin);
    size_t textHeight = pane.height - (2*pane.textMargin);
    endRopeIndex = endRopeIndex == 0 ? endRopeIndex : endRopeIndex - 1;
    //length to and from cursor
    uint16_t preLength = (textWidth * (pane.cursor.y - 1)) + pane.cursor.x;
    uint16_t postLength =  (textWidth * (textHeight - pane.cursor.y - 1)) + (textWidth - pane.cursor.x);
    //calculate beggining and end index
    pane.frame.beginning = std::min((pane.cursor.index + (size_t)preLength), endRopeIndex);
    pane.frame.end = std::min((pane.cursor.index + postLength), endRopeIndex);
    //find range in rope
    pane.frame.nodeStart = rope_range(*(pane.rope), pane.frame.beginning, pane.frame.end, &pane.frame.localIndexStart);
}

const Cursor& tra_get_cursor(Pane& pane){
    return pane.cursor;
}

/*
    move cursor index up
*/
void tra_move_cursor_up(Pane *pane, uint16_t  diff){
    if(pane == nullptr){
        PLOG_ERROR << "pane is NULL, aborted.";
        return;
    }
    if((pane->cursor.index==0?1:pane->cursor.index + diff) >=
        pane->rope->weight){
        PLOG_ERROR << "invalid index, aborted.";
        return;
    }
    size_t textWidth = pane->width - (2*(pane->textMargin));
    size_t textHeight = pane->height - (2*(pane->textMargin));
    //move up
    pane->cursor.index += diff;
    pane->cursor.y += (diff/textWidth);
    uint16_t leftover = diff%textWidth;
    if(leftover > (textWidth - pane->cursor.x)){
        pane->cursor.x = leftover - (textWidth - pane->cursor.x);
        pane->cursor.y ++;
    }else if(leftover < (textWidth - pane->cursor.x)){
        pane->cursor.x += leftover;
    }

    //out of bounds
    if(pane->cursor.y < 0){
        pane->cursor.y = 0;
    }else if(pane->cursor.y >= textHeight){
        pane->cursor.y = textHeight;
    }

}

/*
    move cursor index down
*/
void tra_move_cursor_down(Pane *pane, uint16_t  diff){
    if(pane == nullptr){
        PLOG_ERROR << "pane is NULL, aborted.";
        return;
    }
    if((pane->cursor.index==0?1:pane->cursor.index - diff) >= 
        pane->rope->weight){
        PLOG_ERROR << "invalid index, aborted.";
        return;
    }
    size_t textWidth = pane->width - (2*(pane->textMargin));
    size_t textHeight = pane->height - (2*(pane->textMargin));
    //move down
    diff = +diff;
    pane->cursor.index -= diff;
    pane->cursor.y -= (diff/textWidth);
    uint16_t leftover = diff%textWidth;
    if(leftover > pane->cursor.x){
        pane->cursor.x = textWidth - (leftover - pane->cursor.x);
        pane->cursor.y --;
    }else{
        pane->cursor.x -= leftover;
    }

    //out of bounds
    if(pane->cursor.y < 0){
        pane->cursor.y = 0;
    }else if(pane->cursor.y >= textHeight){
        pane->cursor.y = textHeight;
    }

}

/*
    positions cursor at coordinates
*/
void tra_position_cursor(Pane *pane, uint16_t x, uint16_t y){
    if(pane == nullptr){
        PLOG_ERROR << "pane is NULL, aborted.";
        return;
    }
    size_t textWidth = pane->width - (2*(pane->textMargin));
    size_t textHeight = pane->height - (2*(pane->textMargin));
    size_t avaliableLength = std::min(pane->cursor.index, textWidth * textHeight);

    if(((y * textWidth) + x) > avaliableLength){
        PLOG_ERROR << "invalid coordinates, aborted.";
        return;
    }
    pane->cursor.x = x;
    pane->cursor.y = y;
}

}