#include "termija.h"
#include "widget.h"


#include <raylib.h>
#include "rlgl.h"               // OpenGL abstraction layer to OpenGL 1.1, 3.3 or ES2
#include <plog/Log.h>

#include <iostream>
#include <cstring>

namespace termija{

    

//raylib custom
void _DrawTextEx(Font, const char *, Vector2, float, float, Color, unsigned int);
void _DrawInvertedTextEx(Font, const char *, Vector2, float, float, Color, unsigned int);

void tra_draw_pane_border(const Pane& pane){
    const Termija& termija = Termija::instance();
    DrawRectangleLines(pane.topX, pane.topY, pane.width, pane.height, termija.fontColor);
}


void tra_draw_rectangle(uint16_t topX, uint16_t topY, uint16_t width, uint16_t height){
    const Termija& termija = Termija::instance();
    DrawRectangleLines(topX, topY, width, height, termija.fontColor);
}

void tra_draw_rectangle_fill(uint16_t topX, uint16_t topY, uint16_t width, uint16_t height){
    const Termija& termija = Termija::instance();
    DrawRectangle(topX, topY, width, height, termija.fontColor);
}


void tra_draw_back(uint16_t width, uint16_t height, const Texture2D *backTexture, const Shader *backShader){
    const Termija& termija = Termija::instance();
    if(backTexture == NULL){
        PLOG_ERROR << "backTexture is NULL, aborted.";
        return;
    }
    //draw
    // if(backShader != NULL)
    //     BeginShaderMode(*backShader);
            DrawTextureRec(*backTexture, {0,0,(float)width, (float)height}, {0,0}, termija.fontColor);//TODOR: maybe add separate background color
    // if(backShader != NULL)
    //     EndShaderMode();
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
        DrawRectangle(position.x, position.y, termija.fontWidth+termija.fontSpacing, thickness, termija.fontColor);
    }
    cursor.blinkTimer += tra_delta_time();
    //reset timer
    if(cursor.blinkTimer > 1.0)
        cursor.blinkTimer = 0;
    
}


void _draw(uint8_t flags, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint, unsigned int size){
    if(flags & FLAG_INVERT){
        _DrawInvertedTextEx(font, text, position, fontSize, spacing, tint, size);
    }else{
        _DrawTextEx(font, text, position, fontSize, spacing, tint, size);
    }
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
    size_t right = std::min(ustrlen(node->text.get()), startIndex + (size_t)textWidth - x);
    //whole text or until textHeight
     //draw until right, excluding right
    while(left < right && y < textHeight){
        //draw
        Vector2 position{(float)xPaneStart+xStart+(x*(termija.fontWidth+termija.fontSpacing)), (float)yPaneStart+yStart+(y*(termija.fontHeight))};
        _draw(node->flags->effects.to_ullong(),*font, node->text.get() + u_index_at(node->text.get(), left), position, (float)termija.fontHeight, (float)termija.fontSpacing, termija.fontColor, right-left);
        //move position
        x += (right - left);
        //next part
        left = right;
        right = std::min(ustrlen(node->text.get()), right+(size_t)(textWidth - 0));
        //next line
        if(x >= textWidth){
            y++;
            x=0;
        }
    }
    //new line, only if not already at the star
    if(x > 0 && (node->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE)){
        x=0;
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
    current = blitrope.pop();

    //start from prev
    while((current = blitrope.pop()) != nullptr &&
        !(current->flags->effects.to_ulong() & (uint64_t)FLAG_NEW_LINE)){
        //add weight
        weight += current->weight;
    }
    return weight + weightToNodeStart;
}

void tra_draw_text(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,size_t index){
    Cursor c;
    c.index = index;
    _tra_draw_text_down(rope, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, c);
}

void tra_draw_text(RopeNode *rope, uint16_t xPaneStart, uint16_t yPaneStart, uint16_t xStart, uint16_t yStart, uint16_t textWidth, uint16_t textHeight,Cursor &cursor){
    _tra_draw_text_down(rope, xPaneStart, yPaneStart, xStart, yStart, textWidth, textHeight, cursor);
}

// Draw inverted character (codepoint)
void _DrawInvertedTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font.glyphs[index].offsetX*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      position.y + font.glyphs[index].offsetY*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      (font.recs[index].width + 2.0f*font.glyphPadding)*scaleFactor,
                      (font.recs[index].height + 2.0f*font.glyphPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    //Draw back

    // Draw the character texture on the screen
    Texture2D texture = font.texture;
    Texture2D tx;
    Vector2 origin = (Vector2){ 0, 0 };
    if (texture.id > 0)
    {


        //ClearBackground(tint); 

        float width = (float)texture.width;
        float height = (float)texture.height;

        bool flipX = false;

        if (srcRec.width < 0) { flipX = true; srcRec.width *= -1; }
        if (srcRec.height < 0) srcRec.y -= srcRec.height;

        Vector2 topLeft = { 0 };
        Vector2 topRight = { 0 };
        Vector2 bottomLeft = { 0 };
        Vector2 bottomRight = { 0 };

        if (true)
        {
            float x = dstRec.x - origin.x;
            float y = dstRec.y - origin.y;
            topLeft = (Vector2){ x, y };
            topRight = (Vector2){ x + dstRec.width, y };
            bottomLeft = (Vector2){ x, y + dstRec.height };
            bottomRight = (Vector2){ x + dstRec.width, y + dstRec.height };
        }

        rlCheckRenderBatchLimit(4);     // Make sure there is enough free space on the batch buffer

        rlSetTexture(texture.id);
        rlBegin(RL_QUADS);

            rlColor4ub(ALPHA_DISCARD.r, ALPHA_DISCARD.g, ALPHA_DISCARD.b, ALPHA_DISCARD.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);                          // Normal vector pointing towards viewer

            // Top-left corner for texture and quad
            if (flipX) rlTexCoord2f((srcRec.x + srcRec.width)/width, srcRec.y/height);
            else rlTexCoord2f(srcRec.x/width, srcRec.y/height);
            rlVertex2f(topLeft.x, topLeft.y);

            // Bottom-left corner for texture and quad
            if (flipX) rlTexCoord2f((srcRec.x + srcRec.width)/width, (srcRec.y + srcRec.height)/height);
            else rlTexCoord2f(srcRec.x/width, (srcRec.y + srcRec.height)/height);
            rlVertex2f(bottomLeft.x, bottomLeft.y);

            // Bottom-right corner for texture and quad
            if (flipX) rlTexCoord2f(srcRec.x/width, (srcRec.y + srcRec.height)/height);
            else rlTexCoord2f((srcRec.x + srcRec.width)/width, (srcRec.y + srcRec.height)/height);
            rlVertex2f(bottomRight.x, bottomRight.y);

            // Top-right corner for texture and quad
            if (flipX) rlTexCoord2f(srcRec.x/width, srcRec.y/height);
            else rlTexCoord2f((srcRec.x + srcRec.width)/width, srcRec.y/height);
            rlVertex2f(topRight.x, topRight.y);

        rlEnd();
        rlSetTexture(0);

    }
}

// DrawTextEx with size
// Draw text using Font
// NOTE: chars spacing is NOT proportional to fontSize
void _DrawInvertedTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint, unsigned int size)
{
    if (font.texture.id == 0) font = GetFontDefault();  // Security check in case of not valid font

    size = std::min(size, TextLength(text));    // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor
    size_t rSize = TextLength(text);
    EndTextureMode();//end parent's textureMode before starting childs...
    RenderTexture2D renderTexture = LoadRenderTexture(tra_get_screen_width(), fontSize+spacing);
    BeginTextureMode(renderTexture);
    ClearBackground(tint);
    for (int i = 0, j=0; j < size && i < rSize;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (false)
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
                v2.x = textOffsetX;
                v2.y = textOffsetY;

                DrawTextCodepoint(font, codepoint, v2, fontSize, ALPHA_DISCARD);


            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else textOffsetX += ((float)font.glyphs[index].advanceX*scaleFactor + spacing);
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
        j ++;
    }

    EndTextureMode();//end childs and
    BeginTextureMode(tra_get_render_texture());//start parent's textureMode again...
    if(ALPHA_DISCARD_SHADER.id > 0)
        BeginShaderMode(ALPHA_DISCARD_SHADER);
    DrawTextureRec(renderTexture.texture, {0,0,(float)textOffsetX, (float)-renderTexture.texture.height}, {position.x,position.y}, tint);
    if(ALPHA_DISCARD_SHADER.id > 0)
        EndShaderMode();
    tra_push_render_texture_to_garbage(renderTexture);
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
    size_t rSize = TextLength(text);
    for (int i = 0, j=0; j < size && i < rSize;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (false)
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
        j ++;
    }
}

}