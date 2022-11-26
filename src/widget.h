#ifndef WIDGET_H
#define WIDGET_H


#include "rope.h"


namespace termija{

/*
    base Widget class
*/
class Widget{

public:
    virtual ~Widget(){}

    virtual void update()=0;
    virtual void draw(uint16_t, uint16_t, uint16_t, uint16_t)=0;
    virtual void on_pane_resize(int16_t, int16_t)=0;
};

/*
    Text Widget
*/
class Text : public Widget{
private:
    uint16_t                        x;
    uint16_t                        y;
    uint16_t                        textWidth;
    std::unique_ptr<RopeNode>       text;

public:
    Text(uint16_t,uint16_t,const char *);
    Text(const Text&)               = delete;
    void operator=(Text const&)     = delete;
    ~Text();

    void            update() override;
    void            draw(uint16_t, uint16_t, uint16_t, uint16_t) override;
    void            on_pane_resize(int16_t, int16_t) override;
    uint16_t        length();
    uint16_t        lines();
    void            setTextWidth(uint16_t);
    uint16_t        getTextWidth();
    void            setText(const char *);
    void            insertAt(const char *, size_t);
    void            deleteAt(size_t, uint16_t);
    void            underline();

};

/*
    Text Box Widget
*/
//TODO:lines, cursor, scrolling
class TextBox : public Widget{
private:
    uint16_t                                    x;
    uint16_t                                    y;
    uint16_t                                    width;
    uint16_t                                    height;
    std::vector<std::unique_ptr<Text>>          lines;

public:
    TextBox(uint16_t,uint16_t,const char *);
    TextBox(const TextBox&)                 = delete;
    void operator=(TextBox const&)          = delete;
    ~TextBox();

    void            update() override;
    void            draw(uint16_t, uint16_t, uint16_t, uint16_t) override;
    void            on_pane_resize(int16_t, int16_t) override;
};

}



#endif



