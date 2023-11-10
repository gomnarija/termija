#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

Bar::Bar(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height){
    this->width         = width;
    this->height        = height;
    this->x             = x;
    this->y             = y;
    isActive            = true;
}


void Bar::update(){

}

void Bar::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(width == 0 || height == 0){
        return;
    }else if(!isActive){
        return;
    }

    tra_draw_rectangle_fill(startX + this->x, startY + this->y, this->width, this->height);
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

bool
Bar::getIsActive(){
    return this->isActive;
}

void
Bar::resize(uint16_t width, uint16_t height){
    this->width = width;
    this->height = height;
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

