#include "termija.h"
#include "widget.h"


#include <raylib.h>
#include <plog/Log.h>

#include <iostream>
#include <cstring>

namespace termija{

    

//raylib custom
void _DrawTextEx(Font, const char *, Vector2, float, float, Color, unsigned int);

void tra_draw_pane_border(const Pane& pane){
    DrawCircle(pane.topX, pane.topY, 4, RED);
    DrawRectangleLines(pane.topX, pane.topY, pane.width, pane.height, ORANGE);
}


void _tra_draw_node_down(RopeNode *node, uint16_t &x, uint16_t &y, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight, size_t startIndex){
    const Termija& termija = Termija::instance();
    //get font
    Font *font = tra_get_font();
    if(font == nullptr){
        PLOG_ERROR << "font not loaded, aborted.";
        return;
    }
    if(startIndex >= node->weight - 1){
        return;
    }
    //whole or until end of frame
    size_t left = startIndex;
    size_t right = std::min(strlen(node->text.get()), startIndex + (size_t)textWidth - x);
    //whole text or until textHeight
     //draw until right, excluding right
    while(left < right && y < textHeight){
        //draw
        Vector2 position{(float)xPaneStart+(x*(termija.fontWidth+termija.fontSpacing)), (float)yPaneStart+(y*(termija.fontHeight))};
        _DrawTextEx(*font, node->text.get() + left, position, (float)termija.fontHeight, (float)termija.fontSpacing, LIGHTGRAY, right-left);
        //move position
        x += (right - left);
        //next part
        left = right;
        right = std::min(strlen(node->text.get()), right+(size_t)(textWidth - xStart));
        //next line
        if(x >= textWidth){
            y++;
            x=xStart;
        }
    }
    //new line
    if(node->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE){
        y++;
    }
}

/*
    draw text down from cursor index;
        starts from cursor xy
*/
void _tra_draw_text_down(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,const Cursor &cursor){
    if(rope == nullptr){
        PLOG_ERROR << "rope is NULL, aborted.";
        return;
    }

    const Termija& termija = Termija::instance();
    //start iterating trough leaves
    RopeLeafIterator litrope(rope, cursor.index);
    RopeNode *current;//start from cursor xy
    uint16_t x=cursor.x,y=cursor.y;//initialStartIndex is length to starting index inside leaf
    size_t initialStartIndex = litrope.local_start_index()+1;
    //draw leaves
    while((current = litrope.pop()) != nullptr && y < textHeight){
        //draw node text
        if(current->text != nullptr){
            _tra_draw_node_down(current, x, y, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, initialStartIndex);
            initialStartIndex = 0;
        }
    }
}

/*
    measures weight up until previous new line or when weight is at limit
*/
uint16_t _weight_until_prev_new_line(RopeNode *rope, size_t currentIndex){
    uint16_t weight = 0;
    RopeLeafIteratorBack blitrope(rope, currentIndex);
    RopeNode *current;
    //start from prev
    blitrope.next();
    while((current = blitrope.pop()) != nullptr &&
        !(current->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE)){
        //add weight
        weight += current->weight;
    }
    return weight;
}

void _tra_draw_node_up(RopeNode *node, uint16_t &x, uint16_t &y, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight, size_t endIndex, uint16_t weightUntilPrevNewLine){
    const Termija& termija = Termija::instance();
    //get font
    Font *font = tra_get_font();
    if(font == nullptr){
        PLOG_ERROR << "font not loaded, aborted.";
        return;
    }
    bool isNewLine = node->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE;
    //prev line
    if(x <= xStart){
        y--;
        x= (xStart + textWidth);
    }
    //cut newlined, so that it fits right in the first line
    if(isNewLine && node->weight > (textWidth - (x - xStart))){
        //length of the first line of the newlined text
        uint16_t firstLineTextLength = textWidth - weightUntilPrevNewLine;
        x= (xStart + textWidth) - (firstLineTextLength - (node->weight % textWidth));
    }
    //whole or until start of frame
    size_t left = std::max(0, (int)endIndex - (x - xStart));
    size_t right = endIndex;
    //whole text or until yStart
     //draw until left, excluding left
    while(left < right && y >= yStart){
        //move position, make room for text
        if(left==0 && isNewLine){
            //whole text fits in line
            if(right == endIndex){//put the newlined text at the end of prev notnewlined block
                x = xStart + (weightUntilPrevNewLine>0?(weightUntilPrevNewLine % textWidth):0);
            }else{//it's been cut bcs it doesn't fit
                x = (xStart + textWidth) - (right-left);
            }
        }else{
            if(isNewLine){
                //newlined cut, put at start
                x = xStart;
            }else{//not newlined, just move so it fits
                x = x-(right - left);
            }
        }
        //draw
        Vector2 position{(float)xPaneStart+(x*(termija.fontWidth+termija.fontSpacing)), (float)yPaneStart+(y*(termija.fontHeight))};
        _DrawTextEx(*font, node->text.get() + left, position, (float)termija.fontHeight, (float)termija.fontSpacing, LIGHTGRAY, right-left);
        //prev part
        right = left;
        left = std::max(0, (int)left-(textWidth - xStart));
        //prev line
        if(x <= xStart){
            y--;
            x=xStart + textWidth;
        }
    }
}

/*
    draw text up from cursor index, including cursor index;
        starts from cursor xy - 1
*/
void _tra_draw_text_up(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,const Cursor &cursor){
    if(rope == nullptr){
        PLOG_ERROR << "rope is NULL, aborted.";
        return;
    }

    const Termija& termija = Termija::instance();
    //start iterating trough leaves
    RopeLeafIteratorBack blitrope(rope, cursor.index);
    if(blitrope.get() == nullptr){
        return;
    }
    RopeNode *current;//start from cursor xy-1
    uint16_t x=cursor.x,y=cursor.y;//initialEndIndex is length to end index inside leaf node
    size_t initialEndIndex = blitrope.local_start_index()+1;
    size_t currentIndex = (cursor.index - initialEndIndex) + blitrope.get()->weight;
    uint16_t weightUntilPrevNewLine = 0;
    //draw leaves
    while((current = blitrope.pop()) != nullptr && y >= yStart){
        //draw node text
        if(current->text != nullptr){
            if(blitrope.get() != nullptr){//calculate current rope index
                currentIndex -= current->weight - 1;//calculate distance to first previous new line
                weightUntilPrevNewLine = _weight_until_prev_new_line(rope, currentIndex);
            }else{
                currentIndex = 0;
                weightUntilPrevNewLine = 0;
            }
            //draw
            size_t endIndex = initialEndIndex>0?initialEndIndex:current->weight;
            _tra_draw_node_up(current, x, y, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, endIndex, weightUntilPrevNewLine);
            //used only on first call
            initialEndIndex = 0;
        }
    }
}

void tra_draw_text(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,const Cursor &cursor){
   //TODO:draw cursor
    _tra_draw_text_down(rope, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, cursor);
    _tra_draw_text_up(rope, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, cursor);

}






// DrawTextEx with size
// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
void _DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint, unsigned int size)
{
    if (font.texture.id == 0) font = GetFontDefault();  // Security check in case of not valid font

    size = std::min(size, TextLength(text));    // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += (int)((font.baseSize + font.baseSize/2.0f)*scaleFactor);
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                Vector2 v2;
                v2.x = position.x + textOffsetX;
                v2.y = position.y + textOffsetY;
                DrawTextCodepoint(font, codepoint, v2, fontSize, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

}