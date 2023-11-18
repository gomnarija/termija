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
    this->widthTxt = width;
    this->heightTxt = height;
    //rope
    this->text = rope_create_empty();
    //frameCursor
    this->frameCursor.isDisplayed = false;
    this->frameCursorLine = 0;
}


TextBox::~TextBox(){
    rope_destroy(std::move(this->text));
}



void TextBox::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    //draw
    if(this->text->weight > 0)
        tra_draw_text(this->text.get(), 
                        startX, startY, 
                        this->x, this->y, 
                        std::min(textWidth, this->getTextWidth()), std::min(textHeight, this->getTextHeight()), 
                        this->frameCursor);

    tra_draw_cursor(startX+this->x, startY+this->y, this->cursor);

}

//TODO: redo
void TextBox::on_pane_resize(int16_t paneTextWidth, int16_t paneTextHeight){
    if(paneTextWidth < this->widthTxt){
        this->widthTxt = paneTextWidth;
    }
    if(paneTextHeight < this->heightTxt){
        this->heightTxt = paneTextHeight;
    }

    this->repositionFrameCursor();
    this->repositionCursor();
}

uint16_t TextBox::getWidth() const{
    return this->widthTxt;
}

uint16_t TextBox::getHeight() const{
    return this->heightTxt;
}

void TextBox::setSize(const uint16_t width,const uint16_t height){
    this->widthTxt = width;
    this->heightTxt = height;

    this->repositionFrameCursor();
    this->repositionCursor();
}

void
TextBox::setCursorIndex(size_t index){
    if(this->text->weight <= index){
        this->cursor.index = index;
        this->repositionCursor();
    }else{
        PLOG_ERROR << "index out of bounds, aborted. index : " << index;
    }
}



//TODO:margins, scrollbar
uint16_t TextBox::getTextWidth() const{
    return this->widthTxt;
}

//TODO:margins, scrollbar
uint16_t TextBox::getTextHeight() const{
    return this->heightTxt;
}


size_t 
TextBox::getCurrentIndex() const{
    return this->cursor.index;
}

size_t
TextBox::getTextLength() const{
    return this->text->weight;
}


std::string
TextBox::getText(size_t start, size_t end) const{
    if(end <= start ||
        this->text->weight == 0 ||
            this->text->weight <= start)
        return std::string();

    std::string rope_text;
    RopeLeafIterator litrope(this->text.get(), start);
        RopeNode *c;
        while((c = litrope.pop()) != nullptr && ustrlen(rope_text) < (end - start)){
            if( c->text != nullptr )
                rope_text += c->text.get();
        }

    return rope_text;
}
    
std::string
TextBox::getText() const{
    return this->getText(0, this->text->weight);
}

std::pair<uint16_t, uint16_t>
TextBox::getCursorPosition() const{
    return std::pair<uint16_t, uint16_t>(this->cursor.x, this->frameCursorLine + this->cursor.y);
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
    //cursor index goes from 0 to n
    size_t ropeIndex = this->cursor.index==this->text->weight?this->cursor.index-1:this->cursor.index;
    ///x
    bool        isOnNewLineAndEnd = this->cursor.index==this->text->weight && this->cursorIsOnNewLine();//rope_has_flag_at(*(this->text), this->cursor.index == 0 ? 0 : this->cursor.index-1, 1, FLAG_NEW_LINE);
    //if cursor index is at rope weight, and not on new line add 1 to point to the next avaliable
    size_t      weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), ropeIndex);
    uint8_t     endOfRopeAdd = this->cursor.index==this->text->weight && !this->cursorIsOnNewLine() ? 1:0;
    this->cursor.x = isOnNewLineAndEnd ? 0 : ((weightUntilPrevNewLine-1) + endOfRopeAdd) % this->getTextWidth();
    ///y relative to the frameCursor
    this->cursor.isDrawn = false;//not inside frame by default
    uint16_t    currentY=(this->cursor.x==0&&this->cursor.index==this->text->weight) || isOnNewLineAndEnd ?1:0;
    //starting index
    size_t      currentIndex = ropeIndex;
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

bool TextBox::frameCursorIsOnNewLine() const{
    if(this->frameCursor.index > this->text->weight){
        PLOG_ERROR << "invalid frame cursor index";
        return false;
    }
    size_t localIndex = 0;
    size_t ropeIndex = this->frameCursor.index;
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
                }else{//start of rope
                    this->frameCursor.index = 0;
                    break;
                }
                weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), this->frameCursor.index);
                //place cursor at start of current line
                uint16_t leftover = (weightUntilPrevNewLine-1) % this->getTextWidth();
                this->frameCursor.index = this->frameCursor.index>leftover?this->frameCursor.index-leftover:0;
                weightUntilPrevNewLine -= leftover; 
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
    checks if cursor index is visible
*/
bool TextBox::isCursorVisible(){
    return this->cursor.isDrawn;
}

/*
    sets cursor visibility
*/
void TextBox::displayCursor(bool isDisplayed){
    this->cursor.isDisplayed = isDisplayed;
}


/*
    checks if cursor is being displayed
*/
bool TextBox::isCursorDisplayed(){
    return this->cursor.isDisplayed;
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


    //count at what line frameCursor is
    if(this->frameCursor.index == 0){
        this->frameCursorLine = 0;
        return;
    }

    size_t currentIndex = this->frameCursor.index - 1;
    uint16_t lines = 0;
    while(currentIndex >= 0){
        size_t weightUntilPreviousNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
        if(currentIndex < weightUntilPreviousNewLine)//uint overflow check
            weightUntilPreviousNewLine = currentIndex;

        //not inside this newlined block, continue
        if(currentIndex - weightUntilPreviousNewLine > 0){
            lines += 1 + (weightUntilPreviousNewLine / this->getTextWidth());
            currentIndex -= weightUntilPreviousNewLine;
        }else{//cursor is inside this newlined block
            lines += 1 + ((currentIndex - 0) / this->getTextWidth());
            break;
        }
    }

    this->frameCursorLine = lines;
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
        rope_insert_at(this->text.get(), this->cursor.index - 1, text);
    }
    //move cursor
    size_t iWeight = ustrlen(text);
    if(pWeight == this->text->weight - iWeight){
        cursorWalkRight(iWeight);
    }
}


void TextBox::insertAtCursor(const char *text, const uint8_t flags){
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
        rope_prepend(this->text.get(), rope_create(text, flags));
    }else if(this->cursor.index == this->text->weight){
        rope_append(this->text.get(), rope_create(text, flags));
    }
    else{
        rope_insert_at(this->text.get(), this->cursor.index - 1, rope_create(text, flags));
    }
    //move cursor
    size_t iWeight = ustrlen(text);
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
        rope_prepend(this->text.get(), rope_create_node(text, FLAG_NEW_LINE));
    }else if(this->cursor.index == this->text->weight){
        rope_append(this->text.get(), rope_create_node(text, FLAG_NEW_LINE));
    }
    else{
        rope_insert_at(this->text.get(), this->cursor.index - 1, rope_create_node(text, FLAG_NEW_LINE));
    }
    //move cursor to new line
    size_t iWeight = ustrlen(text);
    if(pWeight == this->text->weight - iWeight){
        cursorWalkRight(iWeight);
    }
}

void TextBox::insertLineAtCursor(const char *text, const uint8_t flags){
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
        rope_prepend(this->text.get(), rope_create_node(text, flags | FLAG_NEW_LINE));
    }else if(this->cursor.index == this->text->weight){
        rope_append(this->text.get(), rope_create_node(text, flags | FLAG_NEW_LINE));
    }
    else{
        rope_insert_at(this->text.get(), this->cursor.index - 1, rope_create_node(text, flags | FLAG_NEW_LINE));
    }
    //move cursor to new line
    size_t iWeight = ustrlen(text);
    if(pWeight == this->text->weight - iWeight){
        cursorWalkRight(iWeight);
    }
}

void TextBox::insertFlagAtRange(size_t index, size_t length, uint8_t flags){
    if((index+length) > this->text->weight || length <= 0){
        PLOG_ERROR << "invalid range, aborted.";
        return;
    }
    rope_insert_flag_at(this->text.get(), index, length, flags);

    if(flags & FLAG_NEW_LINE){
        this->repositionCursor();
    }
}

void TextBox::deleteAtCursor(){
    if(this->cursor.index > this->text->weight && this->cursor.index != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return;
    }
    //delete at cursor
    size_t ropeIndex = this->cursor.index==this->text->weight?this->cursor.index-1:this->cursor.index;
    if(this->text->weight > 0 
        && this->cursor.index+1 < this->text->weight)
        rope_delete_at(this->text.get(), ropeIndex, 1);
    //move back if somehow goes over
    if(this->cursor.index > this->text->weight){
        this->cursor.index = this->text->weight;//put at end
        this->repositionCursor();
    }
}

/*
    from startIndex until endIndex, not including endIndex
*/
void TextBox::deleteAtRange(size_t startIndex, size_t endIndex){
    if(startIndex >= endIndex || endIndex > this->text->weight){
        PLOG_ERROR << "invalid range, aborted.";
        return;
    }
    rope_delete_at(this->text.get(), startIndex, (endIndex - startIndex - 1));
    if(this->cursor.index > this->text->weight){
        this->cursor.index = this->text->weight;//put at end
        this->repositionCursor();
    }
    //NOTE: can frameCursor also go out of scope ?
}

void TextBox::backspaceAtCursor(){
    if(this->cursor.index > this->text->weight && this->cursor.index != 0){
        PLOG_ERROR << "invalid cursor index, aborted.";
        return;
    }
    /*
        if cursor is at index 1,
            text becomes split text 1-weight;
            cursor moves to 0
    */
    if(this->cursor.index == 1){
        if(this->text->weight > 0){
            std::unique_ptr<RopeNode> new_rope = rope_split_at(this->text.get(), 0);
            if(new_rope != nullptr)
                std::swap(this->text, new_rope);
            else
                this->text = rope_create_empty();
            
            this->cursor.index = this->cursor.x = this->cursor.y = 0;
        }
    }else if(this->cursor.index > 1){//delete at index - 2
        rope_delete_at(this->text.get(), this->cursor.index-2, 1);
        cursorWalkLeft(1);
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
    diff = 1;//limit it for now, should work with bigger diff but I'm not completely sure
    uint16_t    leftDiff = 0;
    size_t      currentIndex = this->cursor.index==this->text->weight&&!this->cursorIsOnNewLine()?this->cursor.index-1:this->cursor.index;
    size_t      weightUntilPrevNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
    //count how much should index be moved
    while(diff > 0 && currentIndex >= 0){
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
    diff = 1;//limit it for now, should work with bigger diff but I'm not completely sure
    if(this->cursor.index == this->text->weight)
        return;//cursor at rope end
    uint16_t    rightDiff = 0;
    size_t      currentIndex = this->cursor.index;
    size_t      weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), currentIndex);
    //count how much should index be moved
    while(diff > 0 && currentIndex <= this->text->weight){
        uint16_t yDiff = ((weightUntilNextNewLine)/this->getTextWidth());
        if(yDiff >= diff){//cursor can be placed inside this newlined block
            rightDiff += (diff * this->getTextWidth());
            break;
        }else{//continue to the next newlined block
            //count length from start
            uint16_t leftover = ((weight_until_prev_new_line(this->text.get(), currentIndex)-1) % this->getTextWidth());
            size_t prevWeightUntilNextNewLine = weightUntilNextNewLine;//jump to next line
            currentIndex = (currentIndex+prevWeightUntilNextNewLine)<this->text->weight?(currentIndex+prevWeightUntilNextNewLine):this->text->weight-1;
            weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), currentIndex);
            rightDiff += prevWeightUntilNextNewLine;//count how many lines it passed
            yDiff+=((leftover + prevWeightUntilNextNewLine + 1)/this->getTextWidth())+1;
            //it's on the right line, try placing it on right place inside the line
            if(yDiff==diff){
                //try adding leftover, which is the length from the start before moving to the next line
                if((leftover > 0) && (weightUntilNextNewLine) > leftover){
                    rightDiff += leftover;
                }//if there is no space for full leftover, place it at the end of the line
                else if((weightUntilNextNewLine) <= leftover){
                    rightDiff += weightUntilNextNewLine-1;
                }
                break;//done
            }
            //this happens when cursor gets placed at newline and goes over to the next line
            //  in that case just return before new line
            else if(yDiff>diff){
                rightDiff--;
                break;//done
            }
            else{
                //there is still some lines to go, continue
                diff = diff>yDiff?diff-yDiff:0;
            }
        }
    }
    //move back
    this->cursorWalkRight(rightDiff);
}

void
TextBox::findCursor(){
    //needs to go up
    if(this->cursor.index < this->frameCursor.index){
        uint16_t lines = this->countLinesToCursorUp();
        if(lines > 0)
            this->frameCursorMove(lines * -1);
    }
    //needs to go down
    else if(!this->cursor.isDrawn){
        uint16_t lines = this->countLinesToCursorDown();
        if(lines > 0)
            this->frameCursorMove(lines);
    }

}

/*
    count how many lines there are from frameCursor to cursor going up,
        only applicable if cursor is out of frame, up
            if cursor is not out of frame up it will return 0
*/
uint16_t 
TextBox::countLinesToCursorUp(){
    if(this->cursor.index >= this->frameCursor.index || this->frameCursor.index == 0)
        return 0;

    size_t currentIndex = this->frameCursor.index - 1;
    uint16_t lines = 0;
    while(currentIndex >= this->cursor.index){
        size_t weightUntilPreviousNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
        if(currentIndex < weightUntilPreviousNewLine)//uint overflow check
            weightUntilPreviousNewLine = currentIndex;

        //not inside this newlined block, continue
        if(currentIndex - weightUntilPreviousNewLine > this->cursor.index){
            lines += 1 + (weightUntilPreviousNewLine / this->getTextWidth());
            currentIndex -= weightUntilPreviousNewLine;
        }else{//cursor is inside this newlined block
            lines += 1 + ((currentIndex - this->cursor.index) / this->getTextWidth());
            break;
        }
        
    }
    return lines;
}

/*
    count how many lines there are from end of textBox to cursor going down,
        only applicable if cursor is out of frame down
            if cursor is not out of frame down it will return 0
*/
uint16_t 
TextBox::countLinesToCursorDown(){
    if(this->cursor.index <= this->frameCursor.index)
        return 0;

    size_t currentIndex = this->frameCursor.index;
    uint16_t lines = 0;
    while(currentIndex <= this->cursor.index){
        size_t weightUntilNextNewLine = weight_until_next_new_line(this->text.get(), currentIndex);
        //not inside this newlined block, continue
        if(currentIndex + weightUntilNextNewLine < this->cursor.index){
            lines += 1 + (weightUntilNextNewLine / this->getTextWidth());
            currentIndex += weightUntilNextNewLine;
        }else{//cursor is inside this newlined block
            lines += 1 + ((this->cursor.index - currentIndex) / this->getTextWidth());
            break;
        }
        
    }
    //substract whole frame so that cursor is on bottom
    // if it's less that whole frame then something is wrong because cursor should be bellow
    //  frame
    return lines > (this->getTextHeight()-1) ? lines - (this->getTextHeight()-1) : 0;
}

/*
    move frameCursor all the way down
*/
void 
TextBox::scrollToEnd(){
    if(this->text->weight == 0)
        return;

    //count how many lines from end of rope to start of textBox height, 
    //  or whole rope if number of lines is smaller than textBox height
    size_t currentIndex = this->text->weight - 1;
    uint16_t lines = 0;
    while(true){
        size_t weightUntilPreviousNewLine = weight_until_prev_new_line(this->text.get(), currentIndex);
        lines += 1 + (weightUntilPreviousNewLine / this->getTextWidth());
        if(currentIndex < weightUntilPreviousNewLine)
            return;//smaller than textBox height, do nothing
        currentIndex -= weightUntilPreviousNewLine;

        if(lines > this->getTextHeight() - 1)
            break;
    }

    //place frameCursor index at the end of rope
    // and then go up number of lines needed so that end of rope is inside frame
    this->frameCursor.index = this->text->weight - 1;
    this->frameCursorMove((this->getTextHeight()-1) * -1);
}

/*
    move frameCursor all the way up
*/
void 
TextBox::scrollToBeginning(){
    this->frameCursor.index = 0;
    this->repositionFrameCursor();
}

/*
    empties textBox
*/
void 
TextBox::clear(){
    //destroy current rope
    rope_destroy(std::move(this->text));
    //create new rope
    this->text = rope_create_empty();
    //frameCursor
    this->frameCursor.index = 0;
    this->frameCursor.x = 0;
    this->frameCursor.y = 0;
    this->frameCursorLine = 0;
    //cursor
    this->cursor.index = 0;
    this->cursor.x = 0;
    this->cursor.y = 0;
}



RopeLeafIterator 
TextBox::getRopeLeafIterator(){
    return RopeLeafIterator(this->text.get(), 0);
}



}

