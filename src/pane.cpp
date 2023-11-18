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
    textMargin{1}{}

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

    //update widgets DEPRECATED
    // for(size_t i = 0; i < pane.widgets.size(); i++){
    //     Widget *widget = pane.widgets[i].get();
    //     if(widget == nullptr){
    //         PLOG_ERROR << "widget at index: " << i << " is NULL, skipped.";
    //         continue;
    //     }
    //     widget->update();
    // }
}

void tra_draw_pane(const Pane& pane){
    const Termija& termija = Termija::instance();
    uint16_t textWidth = tra_get_text_width(pane);
    uint16_t textHeight = tra_get_text_height(pane);

    if(textWidth == 0 || textHeight == 0){
        PLOG_ERROR << "couldn't calculate text area size, aborted,";
        return;
    }

    //draw widgets
    for(size_t i = 0; i < pane.widgets.size(); i++){
        Widget *widget = pane.widgets[i].get();
        if(widget == nullptr){
            PLOG_ERROR << "widget at index: " << i << " is NULL, skipped.";
            continue;
        }
        widget->draw(pane.topX + pane.textMargin, pane.topY + pane.textMargin, textWidth, textHeight);
    }

    //draw borders
    //tra_draw_pane_border(pane);
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
        //TODO
        //widget->on_pane_resize(widthDiff, heightDiff);
    }
}

Widget* tra_add_widget(Pane& pane, std::unique_ptr<Widget> widget){
    if(widget == nullptr){
        PLOG_ERROR << "given widget is nullptr, aborted.";
        return nullptr;
    }
    //add to widget vector
    pane.widgets.push_back(std::move(widget));
    return pane.widgets[pane.widgets.size() - 1].get();
}


}