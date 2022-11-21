#include <memory>
#include <iostream>
#include <string>
#include <cstring>  //strcpy
#include <stack> //destroy

#include "termija.h"
#include <raylib.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

namespace termija{

Termija::Termija() :
    screenWidth{0},
    screenHeight{0},
    paneMargin{0},
    currentPane{nullptr}{}



Termija& tra_get_instance(){
    return Termija::instance();
}

void tra_terminate(){
    const Termija& termija = Termija::instance();

    //delete panes
    tra_clear_panes();

    //raylib
    if(termija.font != nullptr)
        UnloadFont(*(termija.font));
    CloseWindow();
}

void tra_init_termija(){
    tra_init_termija(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, DEFAULT_WINDOW_NAME, DEFAULT_PANE_MARGIN);
}

void tra_init_termija(uint16_t screenWidth,uint16_t screenHeight,const char * windowTitle){
    tra_init_termija(screenWidth, screenHeight, windowTitle, DEFAULT_PANE_MARGIN);
}

void tra_init_termija(uint16_t screenWidth,uint16_t screenHeight,const char * windowTitle,uint8_t paneMargin){
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

void tra_set_window_size(uint16_t width,uint16_t height){
    Termija& termija = Termija::instance();

    termija.screenWidth = width;
    termija.screenHeight = height;
    SetWindowSize(width, height);
}

size_t tra_get_window_width(){
    const Termija& termija = Termija::instance();

    return termija.screenWidth;
}

size_t tra_get_window_height(){
    const Termija& termija = Termija::instance();

    return termija.screenHeight;
}

void tra_set_window_title(const char *windowTitle){
    Termija& termija = Termija::instance();

    termija.windowTitle = std::string(windowTitle);
    SetWindowTitle(windowTitle);
}

std::string tra_get_window_title(){
    const Termija& termija = Termija::instance();

    return termija.windowTitle;
}

void tra_set_fps(uint16_t targetFPS){
    SetTargetFPS(targetFPS);
}

void tra_set_pane_margin(uint8_t paneMargin){
    Termija& termija = Termija::instance();

    termija.paneMargin = paneMargin;
    //TODO: rescale panes
}

size_t tra_get_pane_margin(){
    const Termija& termija = Termija::instance();

    return termija.paneMargin;
}

bool tra_should_close(){
    return WindowShouldClose();
}

Pane* tra_add_pane(uint16_t topX, uint16_t topY, uint16_t width, uint16_t height){
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
    for(size_t i = 0;i<termija.panes.size();i++){
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
    for(size_t i = 0;i<termija.panes.size();i++){
        tra_pane_destroy_rope(*(termija.panes[i]));
    }
    //clear vector
    termija.panes.clear();
}

size_t tra_get_pane_count(){
    const Termija& termija = Termija::instance();
    
    return termija.panes.size();
}

Pane* tra_get_current_pane(){
    const Termija& termija = Termija::instance();

    return termija.currentPane;
}

void tra_set_current_pane(Pane* pane){
    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return;
    }
    Termija& termija = Termija::instance();
    bool has_pane = false;

    for(size_t i=0;i<termija.panes.size();i++){
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

void tra_load_font(const char *fontPath, uint8_t fontSize, uint16_t glyphCount){
    if(fontPath == nullptr){
        PLOG_ERROR << "given path is NULL, aborted.";
        return;
    }
    Termija& termija = Termija::instance();
    termija.font = std::make_unique<Font>(LoadFontEx(fontPath, fontSize, NULL, glyphCount));

    if(termija.font->glyphCount == 0){
        PLOG_ERROR << "failed to load font: " << fontPath;
        return;
    }
    for(int i =0;i<termija.font->glyphCount;i++){
        std::cout << termija.font->recs[i].width << "\n";
    }
}

Font* tra_get_font(){
    Termija& termija = Termija::instance();
    return termija.font.get();
}


}