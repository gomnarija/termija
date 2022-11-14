#include "termija.h"

#include <cstring>

namespace termija{

Pane::Pane(size_t topX, size_t topY, size_t width, size_t height) :
    top{nullptr},
    bottom{nullptr},
    left{nullptr},
    right{nullptr},
    topX{topX},
    topY{topY},
    width{width},
    height{height},
    fontSize{DEFAULT_FONT_SIZE}{

    //rope
    this->rope = rope_create_empty();

    //frame
    this->frame.nodeStart = this->rope.get();
    this->frame.cursor = 0;
}

Pane* tra_split_pane_vertically(Pane &pane){
    return tra_split_pane_vertically(pane, pane.width/2);
}
/*
    splits the given pane vertically, shortening it's width and creating new pane 
*/
Pane* tra_split_pane_vertically(Pane &pane, size_t width){
    Termija& termija = Termija::instance();

    if(pane.width <= width){
        //TODO:error
        return nullptr;
    }
    //rescale given pane
    pane.width -= width - termija.paneMargin;
    //create new pane
    Pane* new_pane = tra_add_pane(pane.width + termija.paneMargin + 1, pane.topY, width - termija.paneMargin, pane.height);
    //connect panes
    new_pane->left = pane.left;
    new_pane->right = &pane;
    new_pane->top = pane.top;
    new_pane->bottom = pane.bottom;
    pane.left = new_pane;
    return new_pane;
}

Pane* tra_split_pane_horizontally(Pane &pane){
    return tra_split_pane_horizontally(pane, pane.height/2);
}
/*
    splits the given pane horizontally, shortening it't height and creating new pane 
*/
Pane* tra_split_pane_horizontally(Pane &pane, size_t height){
    Termija& termija = Termija::instance();

    if(pane.height <= height){
        //TODO:error
        return nullptr;
    }
    //rescale given pane
    pane.height -= height - termija.paneMargin;
    //create new pane
    Pane* new_pane = tra_add_pane(pane.topX, pane.height + termija.paneMargin + 1, pane.width, height - termija.paneMargin);
    //connect panes
    new_pane->left = pane.left;
    new_pane->right = pane.right;
    new_pane->top = &pane;
    new_pane->bottom = pane.bottom;
    pane.bottom = new_pane;
    return new_pane;
}

//TODO
RopeNode* tra_insertText(Pane& pane,const char* text,size_t index){
    if(text == nullptr){
        //TODO:error
        return nullptr;
    }
    return nullptr;
}

void tra_pane_destroy_rope(Pane& pane){
    rope_destroy(std::move(pane.rope));
}
}