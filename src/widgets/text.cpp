#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>




namespace termija{

Text::Text(const uint16_t x,const uint16_t y, const char *text):
textWidth{0}{
    if(text == nullptr){
        PLOG_ERROR << "given text is NULL, aborted.";
        return;
    }  
    //coords
    this->x = x;
    this->y = y;
    //create rope
    this->text = rope_create(text);
}


void Text::update(){

}

void Text::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    if(this->text == nullptr){
        PLOG_ERROR << "text is NULL, aborted.";
        return;
    }else if(this->text->weight == 0){
        return;
    }
    // else if(this->x >= textWidth ||
    //     this->y >= textHeight){
    //     PLOG_WARNING << "text is outside of text area bounds, aborted.";
    //     return;
    // }
    uint16_t actualTextWidth = this->textWidth==0?this->length():this->textWidth;
    //draw text
    tra_draw_text(this->text.get(), startX, startY, this->x, this->y, 
                std::min((this->x + actualTextWidth), (int)textWidth), std::min((uint16_t)(this->y + this->lines()),textHeight), 0);
}

void Text::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}

Text::~Text(){
    if(this->text != nullptr){
        rope_destroy(std::move(this->text));
    }
}

uint16_t Text::getX() const{
    return this->x;
}

uint16_t Text::getY() const{
    return this->y;
}

void Text::setPosition(const uint16_t x,const uint16_t y){
    this->x = x;
    this->y = y;
}

uint16_t Text::length(){
    return this->text == nullptr ? 0 : rope_weight_measure_set(this->text.get());
}

uint16_t Text::lines(){
    uint16_t actualTextWidth = textWidth==0?this->length():textWidth;
    return (this->length() / actualTextWidth) + ((this->length()%actualTextWidth)>0?1:0);
}

void Text::setTextWidth(const uint16_t textWith){
    this->textWidth = textWidth;
}

uint16_t Text::getTextWidth(){
    return textWidth==0?this->length():textWidth;
}

void Text::setText(const char *text){
    if(text == nullptr){
        PLOG_ERROR << "text is NULL, aborted.";
        return;
    }
    //destroy current rope, and create new one
    rope_destroy(std::move(this->text));
    this->text = rope_create(text);
}

void Text::insertAt(const char *text,const size_t index){
    if(index > 0 && index >= this->text->weight)
        return;

    //insert given text at index
    if(index == 0){
        rope_prepend(this->text.get(), text);
    }else if(index == this->text->weight - 1){
        rope_append(this->text.get(), text);
    }
    else{
        rope_insert_at(this->text.get(), index, text);
    }
}

void Text::insertAt(const char *text,const size_t index,const uint8_t flags){
    if(index > 0 && index >= this->text->weight)
        return;

    //insert given text at index
    if(index == 0){
        rope_prepend(this->text.get(), rope_create(text, flags));
    }else if(index == this->text->weight - 1){
        rope_append(this->text.get(), rope_create(text, flags));
    }
    else{
        rope_insert_at(this->text.get(), index, rope_create(text, flags));
    }
}

void Text::deleteAt(const size_t index,const uint16_t length){
    //delete text of given length at given index
    rope_delete_at(this->text.get(), index, length);
}

void Text::underline(){
    if(this->text == nullptr){
        PLOG_ERROR << "text is NULL, aborted.";
        return;
    }

    RopeLeafIterator litrope(this->text.get());
    RopeNode *current;
    //add underline flags
    while((current = litrope.pop()) != nullptr){
        //TODO: add UNDERLINE_FLAG
        current->flags->effects;
    }
}

}


