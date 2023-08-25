#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

Box::Box(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height){
    this->width     = width;
    this->height    = height;
    this->x         = x;
    this->y         = y;
    isActive        = true;
}


void Box::update(){

}

void Box::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(width == 0 || height == 0){
        return;
    }else if(!isActive){
        return;
    }

    tra_draw_rectangle(startX + this->x, startY + this->y, this->width, this->height);
}

void Box::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

Box::~Box(){
}

uint16_t
Box::getWidth(){
    return this->width;
}

uint16_t
Box::getHeight(){
    return this->height;
}

uint16_t
Box::getX(){
    return this->x;
}

uint16_t
Box::getY(){
    return this->y;
}

void
Box::resize(uint16_t width, uint16_t height){
    this->width     = width;
    this->height    = height;
}

void
Box::activate(bool isActive){
    this->isActive = isActive;
}


}

