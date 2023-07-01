#ifndef WIDGET_H
#define WIDGET_H


#include "rope.h"

namespace termija{

struct Cursor final{
    size_t          index;
    uint16_t        x;
    uint16_t        y;
    bool            isDrawn;
    uint8_t         blinksPerSecond;
    float           blinkTimer;

    Cursor() : 
        index{0},
        x{0},
        y{0},
        isDrawn{true},
        blinksPerSecond{2},
        blinkTimer{0} {}
};

/*
    base Widget class
*/
class Widget{
protected:
    uint16_t                        x;
    uint16_t                        y;

    Widget() :
    x{0},
    y{0}{}

public:
    virtual ~Widget(){}

    virtual void update()=0;
    virtual void draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t)=0;
    virtual void on_pane_resize(const int16_t,const int16_t)=0;
};

/*
    Text Widget
*/
class Text : public Widget{
private:
    uint16_t                        textWidth;
    std::unique_ptr<RopeNode>       text;

public:
    Text(const uint16_t,const uint16_t,const char *);
    Text(const Text&)               = delete;
    void operator=(Text const&)     = delete;
    ~Text();

    void            update() override;
    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        length();//measure-sets rope weight
    uint16_t        lines();
    uint16_t        getX() const;
    uint16_t        getY() const;
    void            setPosition(const uint16_t,const uint16_t);
    void            setTextWidth(const uint16_t);
    uint16_t        getTextWidth();
    void            setText(const char *);
    void            insertAt(const char *, size_t);
    void            deleteAt(const size_t,const uint16_t);
    void            underline();

};

/*
    Text Box Widget
*/
//TODO:lines, cursor, scrolling
class TextBox : public Widget{
private:
    uint16_t                                    width;
    uint16_t                                    height;
    uint8_t                                     margin;
    std::unique_ptr<RopeNode>                   text;
    Cursor                                      cursor;
    Cursor                                      frameCursor;

    void            repositionCursor();
    bool            cursorIsOnNewLine() const;
    void            repositionFrameCursor();

public:
    TextBox(const uint16_t,const uint16_t,const uint16_t,const uint16_t);
    TextBox(const TextBox&)                 = delete;
    void operator=(TextBox const&)          = delete;
    ~TextBox();

    void            update() override;
    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        getWidth() const;
    uint16_t        getHeight() const;
    void            setSize(const uint16_t,const uint16_t);
    uint16_t        getTextWidth() const;
    uint16_t        getTextHeight() const;
    void            insertAtCursor(const char *);
    void            insertLineAtCursor(const char *);
    void            deleteAtCursor();
    void            backspaceAtCursor();
    void            cursorWalkLeft(uint16_t);
    void            cursorWalkRight(uint16_t);
    void            cursorWalkUp(uint16_t);
    void            cursorWalkDown(uint16_t);
    void            frameCursorMove(int16_t);


};

}



#endif



