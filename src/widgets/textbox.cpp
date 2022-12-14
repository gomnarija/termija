#include "../widget.h"
#include "../rope.h"
#include "../termija.h"

#include <plog/Log.h>




namespace termija{

TextBox::TextBox(const uint16_t x,const uint16_t y,const uint16_t width,const uint16_t height):
margin{0}
{
    //coords
    this->x = x;
    this->y = y;
    //size
    this->width = width;
    this->height = height;
    //rope
    this->text = rope_create_empty();
    //frameCursor
    this->frameCursor.isDrawn = false;
}


TextBox::~TextBox(){
    rope_destroy(std::move(this->text));
}

void TextBox::update(){

}

void TextBox::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    //draw
    tra_draw_text(this->text.get(), 
                    startX, startY, 
                    this->x, this->y, 
                    std::min(textWidth, this->getTextWidth()), std::min(textHeight, this->getTextHeight()), 
                    this->frameCursor);

    tra_draw_cursor(startX, startY, this->cursor);

}

//TODO: redo
void TextBox::on_pane_resize(int16_t paneTextWidth, int16_t paneTextHeight){
    if(paneTextWidth < this->width){
        this->width = paneTextWidth;
    }
    if(paneTextHeight < this->height){
        this->height = paneTextHeight;
    }

    this->repositionFrameCursor();
    this->repositionCursor();
}

uint16_t TextBox::getWidth() const{
    return this->width;
}

uint16_t TextBox::getHeight() const{
    return this->height;
}

void TextBox::setSize(const uint16_t width,const uint16_t height){
    this->width = width;
    this->height = height;

    this->repositionFrameCursor();
    this->repositionCursor();
}

//TODO:margins, scrollbar
uint16_t TextBox::getTextWidth() const{
    return this->width;
}

//TODO:margins, scrollbar
uint16_t TextBox::getTextHeight() const{
    return this->height;
}

/*
    positions cursor inside textbox, based on rope index, relative to the frameCursor
*/
void TextBox::repositionCursor(){
    if(this->cursor.index > this->text->weight){
        PLOG_ERROR << "invalid cursor index, set to rope end.";
        this->cursor.index = this->text->weight>0?this->text->weight-1:0;
        return;
    }
    size_t localIndex = 0;//cursor index goes from 0 to n
    size_t ropeIndex = this->cursor.index==this->text->weight?this->cursor.index-1:this->cursor.index;
    ///x
    //if cursor index is at rope weight
    uint8_t endOfRopeAdd = this->cursor.index==this->text->weight?1:0;
    this->cursor.x = ((weight_until_prev_new_line(this->text.get(), ropeIndex)-1) + endOfRopeAdd) % this->getTextWidth();
    ///y relative to the frameCursor
    this->cursor.isDrawn = false;//not inside frame by default
    uint16_t    currentY=(this->cursor.x==0&&this->cursor.index==this->text->weight)?1:0;
    //starting index
    size_t      currentIndex = ropeIndex;
    size_t      weightUntilPrevNewLine = 0;
    //count y distance from cursor to frameCursor
    while(currentY <= this->getTextHeight() && currentIndex >= this->frameCursor.index){
        weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
        //frame cursor inside this newlined block
        if(currentIndex - (weightUntilPrevNewLine-1) <= this->frameCursor.index){
            currentY += ((currentIndex - this->frameCursor.index)/this->getTextWidth()) + 1;
            break;
        }else{//continue to the next newlined block
            currentY += ((weightUntilPrevNewLine-1)/this->getTextWidth()) + 1;
            currentIndex = currentIndex>weightUntilPrevNewLine?(currentIndex - weightUntilPrevNewLine):0;
        }
    }
    //cursor inside frame
    if(currentY>0 && currentY <= this->getHeight()){
        this->cursor.y = currentY-1;//starts from 0
        this->cursor.isDrawn = true;
    }
}

bool TextBox::cursorIsOnNewLine() const{
    if(this->cursor.index > this->text->weight){
        PLOG_ERROR << "invalid cursor index";
        return false;
    }
    size_t localIndex = 0;
    size_t ropeIndex = this->cursor.index==this->text->weight?this->cursor.index-1:this->cursor.index;
    RopeNode *nodeAtCursor = rope_node_at_index(*(this->text), ropeIndex, &localIndex);
    if(nodeAtCursor == nullptr){
        return false;
    }
    //if node is newlined, and cursor is at the end
    return (nodeAtCursor->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE) &&
            (localIndex == nodeAtCursor->weight - 1);  
}

/*
    moves frame cursor up/down based on the number of given lines
*/
void TextBox::frameCursorMove(int16_t diff){
    //down
    if(diff > 0){
        size_t      weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), this->frameCursor.index);
        //count how much should index be moved
        while(diff > 0 && this->frameCursor.index < this->text->weight){
            uint16_t yDiff = ((weightUntilNextNewLine)/this->getTextWidth());
            if(yDiff >= diff){//cursor goes inside this newlined block
                this->frameCursor.index += (diff * this->getTextWidth());
                break;
            }else{//continue to the next newlined block
                if((this->frameCursor.index+weightUntilNextNewLine)<this->text->weight){
                    this->frameCursor.index = this->frameCursor.index+weightUntilNextNewLine;
                }else{//end of rope
                    this->frameCursor.index = this->text->weight-1;
                    break;
                }
                weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), this->frameCursor.index);
                yDiff++;//next newlined block
                diff = diff>yDiff?diff-yDiff:0;
            }
        }
    }//up
    else if(diff < 0){
        diff = diff*-1;
        size_t      weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), this->frameCursor.index);
        //count how much should index be moved
        while(diff > 0 && this->frameCursor.index > 0){
            uint16_t yDiff = ((weightUntilPrevNewLine - 1)/this->getTextWidth());
            if(yDiff >= diff){//cursor goes inside this newlined block
                uint16_t added = (diff * this->getTextWidth());
                this->frameCursor.index = this->frameCursor.index>added?this->frameCursor.index-added:0;
                break;
            }else{//continue to the next newlined block
                size_t prevWeightUntilPrevNewLine = weightUntilPrevNewLine;
                if((this->frameCursor.index>weightUntilPrevNewLine)){
                    this->frameCursor.index -= weightUntilPrevNewLine;
                }else{//start of ope
                    this->frameCursor.index = 0;
                    break;
                }
                weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), this->frameCursor.index);
                //place cursor at start of current
                uint16_t leftover = (weightUntilPrevNewLine-1) % this->getTextWidth();
                this->frameCursor.index = this->frameCursor.index>leftover?this->frameCursor.index-leftover:0; 
                yDiff++;//prev newlined block
                diff = diff>yDiff?diff-yDiff:0;
            }
    }

    }
    //out of bounds
    if(this->frameCursor.index >= this->text->weight){
        this->frameCursor.index = this->text->weight - 1;
    }

    this->repositionFrameCursor();
    this->repositionCursor();
}

/*
    moves back frameCursor index so that weightUntilPrevNewLine % textWidth = 0
*/
void TextBox::repositionFrameCursor(){
    if(this->frameCursor.index >= this->text->weight){
        PLOG_ERROR << "invalid frame cursor index, set to 0.";
        this->frameCursor.index = 0;
        return;
    }
    size_t localIndex = 0;
    RopeNode *nodeAtCursor = rope_node_at_index(*(this->text), this->frameCursor.index, &localIndex);
    if(nodeAtCursor == nullptr){
        PLOG_ERROR << "couldn't find node at index, aborted.";
        return;
    }
    uint16_t weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), this->frameCursor.index);
    this->frameCursor.index -= (weightUntilPrevNewLine-1) % this->getTextWidth();
}


void TextBox::insertAtCursor(const char *text){
    if(text == nullptr){
        PLOG_ERROR << "text is NULL, aborted.";
        return;
    }
    else if(this->cursor.index > this->text->weight && this->cursor.index != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return;
    }
    size_t pWeight = this->text->weight;
    //insert given text at cursor
    if(this->cursor.index == 0){
        rope_prepend(this->text.get(), text);
    }else if(this->cursor.index == this->text->weight){
        rope_append(this->text.get(), text);
    }
    else{
        rope_insert_at(this->text.get(), this->cursor.index, text);
    }
    //move cursor
    size_t iWeight = strlen(text);
    if(pWeight == this->text->weight - iWeight){
        cursorWalkRight(iWeight);
    }
}

void TextBox::insertLineAtCursor(const char *text){
    if(text == nullptr){
        PLOG_ERROR << "text is NULL, aborted.";
        return;
    }
    else if(this->cursor.index > this->text->weight && this->cursor.index != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return;
    }
    size_t pWeight = this->text->weight;
    //insert given text, with new line flag, at cursor
    if(this->cursor.index == 0){
        rope_prepend(this->text.get(), rope_create_node(text, 0, 0, FLAG_NEW_LINE));
    }else if(this->cursor.index == this->text->weight){
        rope_append(this->text.get(), rope_create_node(text, 0, 0, FLAG_NEW_LINE));
    }
    else{
        rope_insert_at(this->text.get(), this->cursor.index, rope_create_node(text, 0, 0, FLAG_NEW_LINE));
    }
    //move cursor to new line
    size_t iWeight = strlen(text);
    if(pWeight == this->text->weight - iWeight){
        cursorWalkRight(iWeight);
    }
}

void TextBox::cursorWalkLeft(uint16_t diff){
    if(this->cursor.index >= diff){
        this->cursor.index -= diff;
        this->repositionCursor();
    }
}

void TextBox::cursorWalkRight(uint16_t diff){
    if(this->cursor.index + diff <= this->text->weight){
        this->cursor.index += diff;
        this->repositionCursor();
    }
}

void TextBox::cursorWalkUp(uint16_t diff){
    uint16_t    leftDiff = 0;
    size_t      currentIndex = this->cursor.index==this->text->weight?this->cursor.index-1:this->cursor.index;
    size_t      weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
    //count how much should index be moved
    while(diff > 0 && currentIndex > 0){
        uint16_t yDiff = ((weightUntilPrevNewLine - 1)/this->getTextWidth());
        if(yDiff >= diff){//cursor goes inside this newlined block
            leftDiff += (diff * this->getTextWidth());
            break;
        }else{//continue to the next newlined block
            uint16_t leftover = ((weightUntilPrevNewLine) % this->getTextWidth());
            size_t prevWeightUntilPrevNewLine = weightUntilPrevNewLine;
            currentIndex = currentIndex>weightUntilPrevNewLine?currentIndex-weightUntilPrevNewLine:0;
            weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
            leftDiff += prevWeightUntilPrevNewLine;
            //try placing cursor at previous x, if theres space
            if((leftover > 0) && ((weightUntilPrevNewLine) % this->getTextWidth() > leftover))
                leftDiff += ((weightUntilPrevNewLine) % this->getTextWidth()) - leftover;
            yDiff++; 
            diff = diff>yDiff?diff-yDiff:0;
        }
    }
    //move back
    this->cursorWalkLeft(leftDiff);
}

void TextBox::cursorWalkDown(uint16_t diff){
    uint16_t    rightDiff = 0;
    size_t      currentIndex = this->cursor.index;
    size_t      weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), currentIndex);
    //count how much should index be moved
    while(diff > 0 && currentIndex <= this->text->weight){
        uint16_t yDiff = ((weightUntilNextNewLine)/this->getTextWidth());
        if(yDiff >= diff){//cursor goes inside this newlined block
            rightDiff += (diff * this->getTextWidth());
            break;
        }else{//continue to the next newlined block
            uint16_t leftover = ((weight_until_prev_new_line(this->text.get(), currentIndex)-1) % this->getTextWidth());
            size_t prevWeightUntilNextNewLine = weightUntilNextNewLine;
            currentIndex = (currentIndex+prevWeightUntilNextNewLine+1)<this->text->weight?(currentIndex+prevWeightUntilNextNewLine+1):this->text->weight-1;
            weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), currentIndex);
            rightDiff += prevWeightUntilNextNewLine;
            //try placing cursor at previous x, if theres space
            if((leftover > 0) && (weightUntilNextNewLine) >= leftover)
                rightDiff += leftover;
            yDiff++; 
            diff = diff>yDiff?diff-yDiff:0;
        }
    }
    //move back
    this->cursorWalkRight(rightDiff);
}

}

