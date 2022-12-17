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


void tra_draw_cursor(uint16_t xPaneStart, uint16_t yPaneStart, Cursor &cursor){
    if(!cursor.isDrawn){
        return;
    }
    const Termija& termija = Termija::instance();
    //get font
    Font *font = tra_get_font();
    if(font == nullptr){
        PLOG_ERROR << "font not loaded, aborted.";
        return;
    }
    uint8_t thickness = 4;
    //draw if time allows
    if((int)(cursor.blinkTimer / (1.0 / cursor.blinksPerSecond)) % 2 == 0){
        //start position
        Vector2 position{(float)xPaneStart+(cursor.x*(termija.fontWidth+termija.fontSpacing)), (float)yPaneStart+(cursor.y*(termija.fontHeight))};
        //move to character bottom
        position.y += termija.fontHeight - thickness;
        DrawRectangle(position.x, position.y, termija.fontWidth+termija.fontSpacing, thickness, LIGHTGRAY);
    }
    cursor.blinkTimer += tra_delta_time();
    //reset timer
    if(cursor.blinkTimer > 1.0)
        cursor.blinkTimer = 0;
    
}

void _tra_draw_node_down(RopeNode *node, uint16_t &x, uint16_t &y, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight, size_t startIndex){
    const Termija& termija = Termija::instance();
    //get font
    Font *font = tra_get_font();
    if(font == nullptr){
        PLOG_ERROR << "font not loaded, aborted.";
        return;
    }
    if(startIndex >= node->weight){
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
    //new line, only if not already at the star
    if(x > xStart && (node->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE)){
        x=xStart;
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
    size_t initialStartIndex = litrope.local_start_index();
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
    measures weight up until next new line including cursor
*/
size_t weight_until_next_new_line(RopeNode *rope, size_t currentIndex){
    size_t weight = 0;
    RopeLeafIterator litrope(rope, currentIndex);
    size_t weightToNodeEnd = litrope.get() != nullptr ? litrope.get()->weight - litrope.local_start_index() : 0;
    RopeNode *current;
    do{
        current = litrope.pop();
        if(current == nullptr)
            break;        
        //add weight
        weight += weightToNodeEnd>0?weightToNodeEnd:current->weight;
        weightToNodeEnd = 0;
    }while(!(current->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE));
    return weight + weightToNodeEnd;
}

/*
    measures weight up until previous new line including cursor
*/
size_t weight_until_prev_new_line(RopeNode *rope, size_t currentIndex){
    size_t weight = 0;
    RopeLeafIteratorBack blitrope(rope, currentIndex);
    size_t weightToNodeStart = blitrope.local_start_index() + 1;
    RopeNode *current;
    blitrope.pop();
    //start from prev
    while((current = blitrope.pop()) != nullptr &&
        !(current->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE)){
        //add weight
        weight += current->weight;
    }
    return weight + weightToNodeStart;
}

void tra_draw_text(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,Cursor &cursor){
    _tra_draw_text_down(rope, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, cursor);
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