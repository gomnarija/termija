#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

Bar::Bar(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height){
    this->widthPx         = width;
    this->heightPx        = height;
    this->x             = x;
    this->y             = y;
    isActive            = true;
}



void Bar::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(this->getWidth() == 0 || this->getHeight() == 0){
        return;
    }else if(!isActive){
        return;
    }

    tra_draw_rectangle_fill(startX + this->x, startY + this->y, this->getWidth(), this->getHeight());
}

void Bar::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

Bar::~Bar(){
}


uint16_t
Bar::getX(){
    return this->x;
}

uint16_t
Bar::getY(){
    return this->y;
}

uint16_t
Bar::getWidth(){
    return this->widthPx;
}

uint16_t
Bar::getHeight(){
    return this->heightPx;
}


void
Bar::resize(uint16_t width, uint16_t height){
    this->widthPx = width;
    this->heightPx = height;
}

void
Bar::reposition(uint16_t x, uint16_t y){
    this->x = x;
    this->y = y;
}

void
Bar::activate(bool isActive){
    this->isActive = isActive;
}


}

