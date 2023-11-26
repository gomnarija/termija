#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

PopUp::PopUp(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height){
    PopUp(x, y, width, height, nullptr);
}


PopUp::PopUp(const uint16_t x,const uint16_t y, const uint16_t width, const uint16_t height, const char *title){
    this->widthPx     = width;
    this->heightPx    = height;
    this->x         = x;
    this->y         = y;
    isActive        = true;

    //children widgets
    this->backBox =     std::make_unique<termija::Box>(x, y, width, height, true);
    if(title != nullptr){
        //title text widget, with maximum length of popUp width
        uint16_t maxTitleLength = width / tra_get_font_width();
        this->titleText = std::make_unique<termija::Text>(x, y, title, FLAG_INVERT);
        this->titleText->setTextWidth(maxTitleLength);this->titleText->setTextHeight(1);
        uint16_t titleLengthPx = ustrlen(title) * tra_get_font_width();
        if(titleLengthPx < width){
               this->titleBar = std::make_unique<termija::Bar>(x + titleLengthPx, y, width - titleLengthPx, tra_get_font_height());
        }
    }else{
        this->titleBar =    std::make_unique<termija::Bar>(x, y, width, tra_get_font_height());
    }
}

void PopUp::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(this->getWidth() == 0 || this->getHeight() == 0){
        return;
    }else if(!isActive){
        return;
    }

    //draw shadow
    tra_draw_rectangle_fill_transparent(startX + this->getX() + (tra_get_font_width() * 2), startY + this->getY() + (tra_get_font_height()),
        this->getWidth(), this->getHeight());
    tra_draw_rectangle_fill_char(startX + this->getX() + (tra_get_font_width() * 2), startY + this->getY() + (tra_get_font_height()),
        this->getWidth(), this->getHeight(), this->shadow); 
    //draw widgets
    this->backBox->draw(startX, startY, textWidth, textHeight);
    if(this->titleBar != nullptr){
        this->titleBar->draw(startX, startY, textWidth, textHeight);
    }
    if(this->titleText != nullptr){
        this->titleText->draw(startX, startY, textWidth, textHeight);
    }

    //draw widgets
    uint16_t childX = startX + this->getX() + tra_get_font_width();//box line is counted as one character
    uint16_t childY = startY + this->getY() + tra_get_font_height(); //start from end of bar, bar is always 1 char
    uint16_t childTextWidth = (this->getWidth() / tra_get_font_width()) - 2;//2 for box lines
    uint16_t childTextHeight = (this->getHeight() / tra_get_font_height()) - (1 + 1);//bar and bottom box line
    for(size_t i = 0; i < this->widgets.size(); i++){
        Widget *widget = this->widgets[i].get();
        if(widget == nullptr){
            PLOG_ERROR << "widget at index: " << i << " is NULL, skipped.";
            continue;
        }
        widget->draw(childX, childY, childTextWidth, childTextHeight);
    }
}

void PopUp::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

PopUp::~PopUp(){
}

uint16_t
PopUp::getWidth(){
    return this->widthPx;
}

uint16_t
PopUp::getHeight(){
    return this->heightPx;
}

uint16_t
PopUp::getX(){
    return this->x;
}

uint16_t
PopUp::getY(){
    return this->y;
}

void
PopUp::resize(uint16_t width, uint16_t height){
    this->widthPx     = width;
    this->heightPx    = height;
}

void
PopUp::activate(bool isActive){
    this->isActive = isActive;
}


Widget*
PopUp::addWidget(std::unique_ptr<Widget> widget){
    if(widget == nullptr){
        PLOG_ERROR << "given widget is nullptr, aborted.";
        return nullptr;
    }
    //add to widget vector
    this->widgets.push_back(std::move(widget));
    return this->widgets.back().get();
}


}