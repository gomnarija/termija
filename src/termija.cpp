#include <memory>
#include <iostream>
#include <string>
#include <cstring>  //strcpy
#include <stack> //destroy

#include "raylib.h"
#include "termija.h"
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

namespace termija{


Termija& tra_get_instance(){
    return Termija::instance();
}

void tra_terminate(){
    Termija& termija = Termija::instance();

    //delete panes
    tra_clear_panes();

    //raylib
    CloseWindow();
}

void tra_init_termija(){
    tra_init_termija(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, DEFAULT_WINDOW_NAME, DEFAULT_PANE_MARGIN);
}

void tra_init_termija(int screenWidth,int screenHeight,const char * windowTitle){
    tra_init_termija(screenWidth, screenHeight, windowTitle, DEFAULT_PANE_MARGIN);
}

void tra_init_termija(int screenWidth,int screenHeight,const char * windowTitle, int paneMargin){
    Termija& termija = tra_get_instance();

    //plog
    plog::init(plog::debug, "termija.log");

    //raylib
    InitWindow(screenWidth, screenHeight, windowTitle);

    //window
    tra_set_window_size(screenWidth, screenHeight);
    tra_set_window_title(windowTitle);
    tra_set_pane_margin(paneMargin);

    //panes
    termija.currentPane = tra_add_pane(paneMargin, paneMargin, screenWidth - 2*paneMargin, screenHeight - 2*paneMargin);
}

void tra_set_window_size(size_t width, size_t height){
    Termija& termija = Termija::instance();

    termija.screenWidth = width;
    termija.screenHeight = height;
    SetWindowSize(width, height);
}

size_t tra_get_window_width(){
    Termija& termija = Termija::instance();

    return termija.screenWidth;
}

size_t tra_get_window_height(){
    Termija& termija = Termija::instance();

    return termija.screenHeight;
}

void tra_set_window_title(const char *windowTitle){
    Termija& termija = Termija::instance();

    termija.windowTitle = std::string(windowTitle);
    SetWindowTitle(windowTitle);
}

std::string tra_get_window_title(){
    Termija& termija = Termija::instance();

    return termija.windowTitle;
}

void tra_set_fps(size_t targetFPS){
    SetTargetFPS(targetFPS);
}

void tra_set_pane_margin(size_t paneMargin){
    Termija& termija = Termija::instance();

    termija.paneMargin = paneMargin;
    //TODO: rescale panes
}

size_t tra_get_pane_margin(){
    Termija& termija = Termija::instance();

    return termija.paneMargin;
}

bool tra_should_close(){
    return WindowShouldClose();
}

Pane* tra_add_pane(size_t topX, size_t topY, size_t width, size_t height){
    Termija& termija = Termija::instance();

    termija.panes.push_back(std::make_unique<Pane>(topX, topY, width, height));
    return termija.panes.back().get();
}

void tra_remove_pane(Pane *pane){
    Termija& termija = Termija::instance();

    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return;
    }

    bool        pane_removed = false;
    for(int i = 0;i<termija.panes.size();i++){
        if(termija.panes[i].get() == pane){
            pane_removed = true;
            //disconnect
            Pane &to_remove = *(termija.panes[i]);
            if(to_remove.left != nullptr)
                to_remove.left->right = to_remove.right;
            if(to_remove.right != nullptr)
                to_remove.right->left = to_remove.left;
            if(to_remove.top != nullptr)
            to_remove.top->bottom = to_remove.bottom;
            if(to_remove.bottom != nullptr)
                to_remove.bottom->top = to_remove.top;        
            //destroy rope
            tra_pane_destroy_rope(to_remove);
            //rescale neighbours //TODO
            termija.panes[i].swap(termija.panes[termija.panes.size()-1]);
            termija.panes.pop_back();
            break;
        }
    }

    if(!pane_removed){
        PLOG_WARNING << "no panes match the given pointer, none were removed.";
    }
}

void tra_clear_panes(){
    Termija& termija = Termija::instance();

    //destroy ropes
    for(int i = 0;i<termija.panes.size();i++){
        tra_pane_destroy_rope(*(termija.panes[i]));
    }
    //clear vector
    termija.panes.clear();
}

size_t tra_get_pane_count(){
    Termija& termija = Termija::instance();
    
    return termija.panes.size();
}

Pane* tra_get_current_pane(){
    Termija& termija = Termija::instance();

    return termija.currentPane;
}

void tra_set_current_pane(Pane* pane){
    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return;
    }
    Termija& termija = Termija::instance();
    bool has_pane = false;

    for(int i=0;i<termija.panes.size();i++){
        if (termija.panes[i].get() == pane){
            has_pane = true;
            break;
        }
    }
    if(has_pane){
        termija.currentPane = pane;
    }else{
        PLOG_WARNING << "given pane wasn't found inside panes, aborted.";
    }
}
}