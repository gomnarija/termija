#include "../widget.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

ScrollBar::ScrollBar(const uint16_t x,const uint16_t y, const uint16_t height){
    this->heightTxt         = height;
    this->x                 = x;
    this->y                 = y;
    _isActive               = true;
}



void ScrollBar::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(this->getTextHeight() == 0 || !_isActive){
        return;
    }

    //back
    tra_draw_rectangle_fill_char(startX + this->x, startY + this->y, tra_get_font_width(), this->getTextHeight() * tra_get_font_height(), back);
    //cursor ( see setSegments for explanation of how segmetation works )
    tra_draw_rectangle_fill(startX + this->x, startY + this->cursorY, tra_get_font_width(), this->cursorHeight);
}

void ScrollBar::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

ScrollBar::~ScrollBar(){
}


uint16_t
ScrollBar::getX(){
    return this->x;
}

uint16_t
ScrollBar::getY(){
    return this->y;
}


uint16_t
ScrollBar::getTextHeight(){
    return this->heightTxt;
}

void
ScrollBar::resize(uint16_t width, uint16_t height){
    this->heightTxt = width;
    this->heightTxt = height;
}

void
ScrollBar::reposition(uint16_t x, uint16_t y){
    this->x = x;
    this->y = y;
}

void
ScrollBar::activate(bool isActive){
    this->_isActive = isActive;
}

bool
ScrollBar::isActive() const{
    return this->_isActive;
}

void
ScrollBar::setSegments(uint8_t numberOfSegments, uint8_t currentSegment){
    this->numberOfSegments  = std::min((uint16_t)numberOfSegments, this->getTextHeight());
    this->currentSegment    = std::max(currentSegment, (uint8_t)1);
    if(this->currentSegment > this->numberOfSegments)
        this->currentSegment = this->numberOfSegments;
    /*
        to make cursor fit exactly into textHeight for given numberOfSegments,
        we need to add the remainder evenly accross, starting from segment number 1
        we extend each segment by 1, up to remainder.
    */
    uint8_t cursorHeightTxt = (this->getTextHeight()/this->numberOfSegments);
    uint16_t baseCursorHeight = std::max(cursorHeightTxt * tra_get_font_height(), (int)tra_get_font_height()); 
    uint8_t remainder = this->getTextHeight() % numberOfSegments;
    this->cursorHeight = baseCursorHeight;
    if(remainder >= currentSegment){//segment is inside remainder range, extend by 1
        this->cursorHeight += tra_get_font_height();
    }
    //calculate starting cursorY, accounting for previously added remainder
    uint8_t prevRemainder = std::min(remainder, (uint8_t)(this->currentSegment - 1));//number of segments that got extended
    this->cursorY = std::min((baseCursorHeight * (this->currentSegment-1)) + (prevRemainder * tra_get_font_height()) , (this->getTextHeight() * tra_get_font_height()) - this->cursorHeight);
}

}