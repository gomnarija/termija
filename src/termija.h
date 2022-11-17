#ifndef TERMIJA_H
#define TERMIJA_H

#include "rope.h"

#include <string>
#include <memory>
#include <stack>
#include <vector>

namespace termija{


//default values
inline const uint16_t            DEFAULT_SCREEN_WIDTH    = 800;
inline const uint16_t            DEFAULT_SCREEN_HEIGHT   = 600;
inline const char               *DEFAULT_WINDOW_NAME     = "termija";
inline const uint16_t            DEFAULT_TARGET_FPS      = 60;
inline const uint8_t             DEFAULT_PANE_MARGIN     = 30;

inline const uint8_t             DEFAULT_FONT_SIZE       = 20;



struct PaneFrame final{
    size_t          beginning;
    size_t          end;
    size_t          localIndexStart;
    RopeNode*       nodeStart;

    PaneFrame();
    PaneFrame(const PaneFrame&) = delete;
    PaneFrame& operator=(const PaneFrame&) = delete;
};

struct Cursor final{
    size_t          index;
    uint16_t        x;
    uint16_t        y;
    bool            isDrawn;

    Cursor();
    Cursor(const Cursor&) = delete;
    Cursor& operator=(const Cursor&) = delete;
};

struct Pane final{
private:

    std::unique_ptr<RopeNode>   rope;
    std::stack<char>            inputStack;
    Cursor                      cursor;
    PaneFrame                   frame;

public:
    Pane*                   top;
    Pane*                   bottom;
    Pane*                   left;
    Pane*                   right;
    
    uint16_t                topX;
    uint16_t                topY;
    uint16_t                width;
    uint16_t                height;
    uint8_t                 fontSize;


    Pane(uint16_t,uint16_t,uint16_t,uint16_t);
    Pane(const Pane&) = delete;
    Pane& operator=(const Pane&) = delete;
    
    
    friend Pane*            tra_split_pane_vertically(Pane &,uint16_t);
    friend Pane*            tra_split_pane_horizontally(Pane &,uint16_t);
    friend Pane*            tra_merge_panes(Pane &, Pane &);
    friend RopeNode*        tra_insertText(Pane&,const char*,uint16_t);
    friend void             tra_pane_destroy_rope(Pane &);
    friend void             tra_draw_pane(const Pane &);
    friend void             tra_draw_pane_border(const Pane &);
    friend void             tra_position_pane_frame(Pane &pane);

};

Pane*               tra_split_pane_vertically(Pane &);
Pane*               tra_split_pane_horizontally(Pane &);
Pane*               tra_split_pane_vertically(Pane &,uint16_t);
Pane*               tra_split_pane_horizontally(Pane &,uint16_t);
Pane*               tra_merge_panes(Pane &, Pane &);
RopeNode*           tra_insertText(Pane&,const char*,uint16_t);
void                tra_pane_destroy_rope(Pane &);
void                tra_position_pane_frame(Pane &pane);

//singleton
class Termija final{
    //window
    private:
        uint16_t           screenWidth;
        uint16_t           screenHeight;
        std::string        windowTitle;
        uint8_t            paneMargin;

    public:
        friend void             tra_set_window_size(const uint16_t, const uint16_t);
        friend size_t           tra_get_window_width();
        friend size_t           tra_get_window_height();
        friend void             tra_set_window_title(const char *);
        friend std::string      tra_get_window_title();
        friend void             tra_set_fps(uint16_t);
        friend void             tra_set_pane_margin(uint8_t);
        friend size_t           tra_get_pane_margin();

    //panes
    private:
        std::vector<std::unique_ptr<Pane>>      panes;
        Pane*                                   currentPane;

    public:
        friend void             tra_init_termija(uint16_t screenWidth, uint16_t screenHeight,const char * windowTitle, uint8_t paneMargin); 
        friend Pane*            tra_add_pane(uint16_t, uint16_t, uint16_t, uint16_t);
        friend void             tra_remove_pane(Pane *);
        friend void             tra_clear_panes();
        friend size_t           tra_get_pane_count();
        friend Pane*            tra_get_current_pane();
        friend void             tra_set_current_pane(Pane*);
        friend Pane*            tra_split_pane_vertically(Pane &);
        friend Pane*            tra_split_pane_horizontally(Pane &);
        friend Pane*            tra_split_pane_vertically(Pane &, uint16_t);
        friend Pane*            tra_split_pane_horizontally(Pane &, uint16_t);

    public:
        friend void             tra_draw();


    private:
        Termija(){}
        Termija(const Termija&)         = delete;
        void operator=(Termija const&)  = delete;

    public:
        static Termija& instance(){
            static Termija instance;
            return instance;
        }
};


//termija 
Termija&    tra_get_instance();
void        tra_terminate();
void        tra_init_termija();
void        tra_init_termija(uint16_t, uint16_t, const char *);
void        tra_init_termija(uint16_t, uint16_t, const char *, uint8_t);

//window
void        tra_set_window_size(const uint16_t, const uint16_t);
size_t      tra_get_window_width();
size_t      tra_get_window_height();
void        tra_set_window_title(const char *);
std::string tra_get_window_title();
void        tra_set_fps(uint16_t);
size_t      tra_get_fps();
void        tra_set_pane_margin(uint8_t);
size_t      tra_get_pane_margin();
bool        tra_should_close();

//panes
Pane*       tra_add_pane(uint16_t, uint16_t, uint16_t, uint16_t);
void        tra_remove_pane(Pane *);
void        tra_clear_panes();
size_t      tra_get_pane_count();
Pane*       tra_get_current_pane();
void        tra_set_current_pane(Pane*);

//drawing
void        tra_draw();
void        tra_draw_pane(const Pane&);
void        tra_draw_pane_border(const Pane&);

};


#endif