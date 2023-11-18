#include "../widget.h"
#include "../rope.h"
#include "../termija.h"
#include <plog/Log.h>


namespace termija{


List::List(const uint16_t x,const uint16_t y,const uint16_t width, const uint16_t height, Pane *pane){
    this->pane = pane;
    this->x = x;
    this->y = y;
    this->widthTxt = width;
    this->heightTxt = height;
}



List::~List(){
    //TODO
}


void List::draw(const uint16_t startX,const uint16_t startY,const uint16_t textWidth,const uint16_t textHeight){
    termija::Termija &termija = termija::tra_get_instance();
    uint16_t fontWidth = (termija.fontWidth+termija.fontSpacing);
    uint16_t fontHeight = (termija.fontHeight+termija.fontSpacing);
    //draw selected empty rows
    for(auto &column : this->table){
        //empty
        if(column.rows.find(this->selectedIndex) == column.rows.end()){
            uint16_t index =  (this->isShowColumnNames?1:0) + this->selectedIndex - this->startingIndex;
            tra_draw_rectangle_fill(startX + column.x, startY + (column.y + (index * fontHeight) + this->rowSpacing), 
                fontWidth * column.width, fontHeight);
        }
    }
}

void List::on_pane_resize(const int16_t paneTextWidth,const int16_t paneTextHeight){
    //TODO
}


uint16_t
List::getWidth(){
    return this->widthTxt * tra_get_font_width();
}

uint16_t
List::getHeight(){
    return this->heightTxt * tra_get_font_height();
}

uint16_t
List::getX(){
    return this->x;
}

uint16_t
List::getY(){
    return this->y;
}


/*
    tries to find column with given name inside table,
        on fail returns nullptr
*/
ListColumn*
List::findColumn(const std::string &columnName){
    for(auto &column : this->table){
        if(column.name == columnName){
            return &column;
        }
    }

    return nullptr;
}

/*
    counts width of all columnds
*/
uint16_t
List::countActualWidth(){
    Termija &termija = tra_get_instance();
    uint16_t textWidth = (termija.fontWidth+termija.fontSpacing);
    uint16_t width = 0;
    for(const auto listColumn : this->table){
        width += (textWidth * listColumn.width) + this->columnSpacing;
    }

    return width;
}

/*
    finds highes row index in columns
*/
uint16_t
List::getHighestRowIndex(){
    uint16_t highest = 0;
    for(const auto &lineColumn : this->table){
        if(!lineColumn.rows.empty() && highest < lineColumn.rows.rbegin()->first){//keys are sorted in map
            highest = lineColumn.rows.rbegin()->first;
        }
    }

    return highest;
}



/*
    update table Text positions
    show/hide headers
*/
void
List::updateTable(){
    Termija &termija = tra_get_instance();
    uint16_t textHeight = (termija.fontHeight+termija.fontSpacing);
    //go over table updating text y positions,
    //  sets active / inactive
    for(auto &column : this->table){
        for(auto &entry : column.rows){
            uint16_t index = (this->isShowColumnNames?1:0) + entry.first - this->startingIndex;
            Text* textWidget = entry.second;
            //inside list
            if(index < this->heightTxt && (index > 0 || !this->isShowColumnNames) && textWidget->getX() < this->getWidth()){
                //y position based on index and startingIndex
                textWidget->setPosition(textWidget->getX(), column.y + (index * textHeight) + this->rowSpacing);
                textWidget->activate(true);
            }else{//outside, set to inactive
                textWidget->activate(false);
            }
            //highlight selected
            if(entry.first == this->selectedIndex){
                this->invertText(entry.second, true);
            }else{
                this->invertText(entry.second, false);
            }
        }
    }
    //show/hide headers
    for(auto &header : headerColumns){
        if(header->getX() < this->getWidth()){
            header->activate(this->isShowColumnNames);
        }else{
            //outside
            header->activate(false);
        }
    }
}

/*
    inserts given column;
        NOTE: sets column position
*/
void 
List::insertColumn(ListColumn column){
    if(!this->findColumn(column.name)){
        column.x = this->x+(this->countActualWidth()+this->columnSpacing);
        column.y = this->y;
        this->table.emplace_back(column);
        headerColumns.push_back((termija::Text*)termija::tra_add_widget(*(this->pane),
                                            std::make_unique<termija::Text>(column.x, column.y, column.name.c_str())));
        headerColumns.at(headerColumns.size()-1)->activate(this->isShowColumnNames);
        headerColumns.at(headerColumns.size()-1)->setTextWidth(column.width);
        headerColumns.at(headerColumns.size()-1)->setTextHeight(1);//TODO: maybe add multiple lines in the future
    }else{
        PLOG_WARNING << "column with name, " << column.name << " already exists, aborted.";
    }
}


/*
    inserts row from vector at the end
*/
void
List::insertRow(std::vector<std::string> &entries){
    //TODO    
}

/*
    inserts row from vector at the given index,
        inserting from column 0 going up while there are entries

*/
void
List::insertRow(std::vector<std::string> &entries, uint16_t index){
    Termija &termija = tra_get_instance();
    uint16_t textHeight = (termija.fontHeight+termija.fontSpacing);
    for(int i=0;i<entries.size();i++){
        if(i>=this->table.size())
            break;
        
        ListColumn &listColumn = this->table.at(i);
        //if there is no Text widget at index, create new one
        if(listColumn.rows.find(index) == listColumn.rows.end()){
            listColumn.rows[index] = (termija::Text*)termija::tra_add_widget(*(this->pane),
                                            std::make_unique<termija::Text>(listColumn.x, 
                                                listColumn.y + (index * textHeight) + this->rowSpacing + (this->isShowColumnNames?textHeight+this->rowSpacing:0), 
                                                    entries.at(i).c_str()));
            listColumn.rows[index]->setTextWidth(listColumn.width);
            listColumn.rows[index]->setTextHeight(1);//TODO:maybe add multiple lines in the future
            //insert whitespace for highlighting if text is not filling whole row width
            if(listColumn.rows[index]->length() < listColumn.width){
                std::string whitespace(listColumn.width - listColumn.rows[index]->length(), ' ');
                size_t textIndex = listColumn.rows[index]->length()>0?listColumn.rows[index]->length()-1:0;
                listColumn.rows[index]->insertAt(whitespace.c_str(), textIndex, FLAG_INVERT);
            }
        }else{//...if there is, change it's text
            listColumn.rows[index]->setText(entries.at(i).c_str());
        }
    }
}

/*
    moves startingIndex down
*/
void
List::scrollDown(uint16_t diff){
    if((this->startingIndex+diff) < this->getHighestRowIndex()){
        this->startingIndex += diff;
        this->updateTable();
    }

}

/*
    moves startingIndex up
*/
void
List::scrollUp(uint16_t diff){
    if((this->startingIndex-diff) >= 0){
        this->startingIndex -= diff;
        this->updateTable();
    }
}


/*
    moves selectedIndex down
*/
void
List::selectDown(uint16_t diff){
    if((this->selectedIndex+diff) <= this->getHighestRowIndex()){
        this->selectedIndex += diff;
        PLOG_DEBUG << this->selectedIndex;
        PLOG_DEBUG << (this->startingIndex + this->heightTxt - (this->isShowColumnNames?1:0));
        if(this->selectedIndex >= (this->startingIndex + this->heightTxt - (this->isShowColumnNames?1:0))){
            this->startingIndex = this->selectedIndex - (this->heightTxt - (this->isShowColumnNames?2:1));//1 because it starts from 0, and another if there is headers
            PLOG_DEBUG << "teraj";
        }
        this->updateTable();
    }

}

/*
    moves selectedIndex up
*/
void
List::selectUp(uint16_t diff){
    if((this->selectedIndex-diff) >= 0){
        this->selectedIndex -= diff;
        if(this->selectedIndex < this->startingIndex){
            this->startingIndex = this->selectedIndex;
        }
        this->updateTable();
    }
}



void
List::showColumNames(bool isShowColumNames){
    this->isShowColumnNames = isShowColumNames;
}


void
List::invertText(Text *text, bool isInverted){
    if(isInverted){
        text->insertFlagAt(FLAG_INVERT, 0, text->length());
    }else{
        text->insertFlagAt(0, 0, text->length());
    }
}



//TODOR
/*
    selected
        highlighted
        bar < 

*/

}