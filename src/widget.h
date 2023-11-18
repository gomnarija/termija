#ifndef WIDGET_H
#define WIDGET_H


#include "rope.h"
#include "string.h"


#include <map>

namespace termija{

struct Cursor final{
    size_t          index;
    uint16_t        x;
    uint16_t        y;
    bool            isDrawn;
    bool            isDisplayed;
    uint8_t         blinksPerSecond;
    float           blinkTimer;

    Cursor() : 
        index{0},
        x{0},
        y{0},
        isDrawn{true},
        isDisplayed{true},
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

    virtual void draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t)=0;
    virtual void on_pane_resize(const int16_t,const int16_t)=0;
};

/*
    Text Widget
*/
class Text : public Widget{
private:
    bool                            isActive;
    uint16_t                        textWidth;
    uint16_t                        textHeight;
    std::unique_ptr<RopeNode>       text;

public:
    Text(const uint16_t,const uint16_t,const char *);
    Text(const uint16_t,const uint16_t,const char *, const uint8_t);
    Text(const Text&)               = delete;
    void operator=(Text const&)     = delete;
    ~Text();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        length();//measure-sets rope weight
    uint16_t        lines();
    uint16_t        getX() const;
    uint16_t        getY() const;
    void            setPosition(const uint16_t,const uint16_t);
    void            setTextWidth(const uint16_t);
    void            setTextHeight(const uint16_t);
    uint16_t        getTextWidth();
    uint16_t        getTextHeight();
    void            setText(const char *);
    void            setText(const char *, const uint8_t);
    void            insertAt(const char *, size_t);
    void            insertFlagAt(const uint8_t, size_t, size_t);
    void            insertAt(const char *, size_t,const uint8_t);
    void            deleteAt(const size_t,const uint16_t);
    void            underline();
    void            activate(bool);

};

/*
    Text Box Widget
*/
class TextBox : public Widget{
private:
    uint16_t                                    widthTxt;
    uint16_t                                    heightTxt;
    uint8_t                                     margin;
    std::unique_ptr<RopeNode>                   text;
    Cursor                                      cursor;
    Cursor                                      frameCursor;
    uint16_t                                    frameCursorLine;

    void            repositionCursor();
    bool            cursorIsOnNewLine() const;
    bool            frameCursorIsOnNewLine() const;
    void            repositionFrameCursor();

public:
    TextBox(const uint16_t,const uint16_t,const uint16_t,const uint16_t);
    TextBox(const TextBox&)                 = delete;
    void operator=(TextBox const&)          = delete;
    ~TextBox();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        getWidth() const;
    uint16_t        getHeight() const;
    void            setSize(const uint16_t,const uint16_t);
    void            setCursorIndex(size_t);
    uint16_t        getTextWidth() const;
    uint16_t        getTextHeight() const;
    size_t          getCurrentIndex() const;
    size_t          getTextLength() const;
    std::string     getText(size_t, size_t) const;
    std::string     getText() const;
    std::pair<uint16_t, uint16_t>
                    getCursorPosition() const;
    void            insertAtCursor(const char *);
    void            insertLineAtCursor(const char *);
    void            insertAtCursor(const char *, const uint8_t);
    void            insertLineAtCursor(const char *, const uint8_t);
    void            insertFlagAtRange(size_t, size_t, uint8_t);
    void            deleteAtCursor();
    void            deleteAtRange(size_t, size_t);
    void            backspaceAtCursor();
    void            cursorWalkLeft(uint16_t);
    void            cursorWalkRight(uint16_t);
    void            cursorWalkUp(uint16_t);
    void            cursorWalkDown(uint16_t);
    void            frameCursorMove(int16_t);
    bool            isCursorVisible();
    void            displayCursor(bool);
    bool            isCursorDisplayed();
    void            findCursor();
    uint16_t        countLinesToCursorUp();
    uint16_t        countLinesToCursorDown();
    void            scrollToEnd();
    void            scrollToBeginning();
    void            clear();

    RopeLeafIterator    getRopeLeafIterator();


};

/*
    Box Widget
*/
class Box : public Widget{
private:

    bool                        isActive;
    uint16_t                    widthPx;
    uint16_t                    heightPx;
    bool                        fillBack;

public:
    Box(const uint16_t,const uint16_t, const uint16_t, const uint16_t);
    Box(const uint16_t,const uint16_t, const uint16_t, const uint16_t, bool);
    Box(const Box&)                 = delete;
    void operator=(Box const&)      = delete;
    ~Box();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        getWidth();
    uint16_t        getHeight();
    uint16_t        getX();
    uint16_t        getY();
    void            resize(uint16_t, uint16_t);
    void            activate(bool);

};

/*
    Bar Widget
*/
class Bar : public Widget{
private:

    bool                        isActive;
    uint16_t                    heightPx;
    uint16_t                    widthPx;

public:
    Bar(const uint16_t,const uint16_t, const uint16_t, const uint16_t);
    Bar(const Bar&)                 = delete;
    void operator=(Bar const&)      = delete;
    ~Bar();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        getX();
    uint16_t        getY();
    uint16_t        getWidth();
    uint16_t        getHeight();
    bool            getIsActive();
    void            resize(uint16_t, uint16_t);
    void            reposition(uint16_t, uint16_t);
    void            activate(bool);

};



struct ListColumn{
    std::string         name;
    uint16_t            x, y;
    uint16_t            width;
    std::map<uint16_t, Text*>
                        rows;
};


//fwd declaration
struct Pane;

/*
    List Widget
*/
class List : public Widget{
private:
    Pane                        *pane;
    uint16_t                    widthTxt;
    uint16_t                    heightTxt;
    uint16_t                    startingIndex=0;
    uint16_t                    selectedIndex=0;

    //TODO: remove this
    uint8_t                     columnSpacing=0;//DO NOT CHANGE THIS!!
    uint8_t                     rowSpacing=0;// STARTED GOING FOR CHAR PERFECT!!

    std::vector<ListColumn>     table;
    std::vector<Text*>          headerColumns;
    bool                        isShowColumnNames;


    ListColumn*                 findColumn(const std::string &);
    uint16_t                    countActualWidth();
    uint16_t                    getHighestRowIndex();
    void                        invertText(Text*, bool);

public:
    List(const uint16_t,const uint16_t, const uint16_t, const uint16_t, Pane*);
    List(const List&)               = delete;
    void operator=(List const&)     = delete;
    ~List();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    void            updateTable();
    uint16_t        getHeight();
    uint16_t        getX();
    uint16_t        getY();
    uint16_t        getWidth();
    void            insertColumn(ListColumn);
    void            insertRow(std::vector<std::string>&, uint16_t);
    void            insertRow(std::vector<std::string>&);
    void            scrollUp(uint16_t);
    void            scrollDown(uint16_t);
    void            selectUp(uint16_t);
    void            selectDown(uint16_t);
    void            showColumNames(bool);

};


/*
    PopUp Widget
*/
class PopUp : public Widget{
private:
    const char*                 shadow = "░";

    bool                        isActive;
    uint16_t                    widthPx;
    uint16_t                    heightPx;

    std::unique_ptr<Box>        backBox;
    std::unique_ptr<Bar>        titleBar;
    std::unique_ptr<Text>       titleText;
    std::vector<std::unique_ptr<Widget>>
                                widgets;

public:
    PopUp(const uint16_t,const uint16_t, const uint16_t, const uint16_t);
    PopUp(const uint16_t,const uint16_t, const uint16_t, const uint16_t, const char *);
    PopUp(const PopUp&)                 = delete;
    void operator=(PopUp const&)        = delete;
    ~PopUp();

    void            draw(const uint16_t,const uint16_t,const uint16_t,const uint16_t) override;
    void            on_pane_resize(const int16_t,const int16_t) override;
    uint16_t        getWidth();
    uint16_t        getHeight();
    uint16_t        getX();
    uint16_t        getY();
    void            resize(uint16_t, uint16_t);
    void            activate(bool);
    Widget*         addWidget(std::unique_ptr<Widget> widget);

};




}

#endif



