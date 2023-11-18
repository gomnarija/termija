#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

Box::Box(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height)
:Box(x, y, width, height, false){}

Box::Box(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height, bool fillBack){
    this->widthPx     = width;
    this->heightPx    = height;
    this->x         = x;
    this->y         = y;
    isActive        = true;
    this->fillBack = fillBack;
}




void Box::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(widthPx == 0 || heightPx == 0){
        return;
    }else if(!isActive){
        return;
    }

    if(fillBack)
        tra_draw_rectangle_fill_transparent(startX + this->x, startY + this->y, this->widthPx, this->heightPx);
    tra_draw_rectangle(startX + this->x, startY + this->y, this->widthPx, this->heightPx);
}

void Box::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

Box::~Box(){
}

uint16_t
Box::getWidth(){
    return this->widthPx;
}

uint16_t
Box::getHeight(){
    return this->heightPx;
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
    this->widthPx     = width;
    this->heightPx    = height;
}

void
Box::activate(bool isActive){
    this->isActive = isActive;
}


}

