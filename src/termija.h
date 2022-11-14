#ifndef TERMIJA_H
#define TERMIJA_H

#include "rope.h"

#include <string>
#include <memory>
#include <stack>
#include <vector>

namespace termija{


//default values
inline const int            DEFAULT_SCREEN_WIDTH    = 800;
inline const int            DEFAULT_SCREEN_HEIGHT   = 600;
inline const char          *DEFAULT_WINDOW_NAME     = "termija";
inline const int            DEFAULT_TARGET_FPS      = 60;
inline const int            DEFAULT_PANE_MARGIN     = 3;

inline const int            DEFAULT_FONT_SIZE       = 20;



struct PaneFrame final{
    size_t          cursor;
    RopeNode*       nodeStart;
};

struct Pane final{
private:

    std::unique_ptr<RopeNode>   rope;
    std::stack<char>            inputStack;
    PaneFrame                   frame;

public:
    Pane*              top;
    Pane*              bottom;
    Pane*              left;
    Pane*              right;
    
    size_t             topX;
    size_t             topY;
    size_t             width;
    size_t             height;
    size_t             fontSize;


    Pane(size_t,size_t,size_t,size_t);
    Pane(const Pane&) = delete;
    Pane& operator=(const Pane&) = delete;
    
    
    friend Pane*            tra_split_pane_vertically(Pane &,size_t);
    friend Pane*            tra_split_pane_horizontally(Pane &,size_t);
    friend RopeNode*        tra_insertText(Pane&,const char*,size_t);
    friend void             tra_pane_destroy_rope(Pane&);
    friend void             tra_draw_pane(const Pane&);
    friend void             tra_draw_pane_border(const Pane&);

};

Pane*               tra_split_pane_vertically(Pane &);
Pane*               tra_split_pane_horizontally(Pane &);
Pane*               tra_split_pane_vertically(Pane &,size_t);
Pane*               tra_split_pane_horizontally(Pane &,size_t);
RopeNode*           tra_insertText(Pane&,const char*,size_t);
void                tra_pane_destroy_rope(Pane&);

//singleton
class Termija final{
    //window
    private:
        size_t             screenWidth;
        size_t             screenHeight;
        std::string        windowTitle;
        size_t             paneMargin;

    public:
        friend void             tra_set_window_size(const size_t, const size_t);
        friend size_t           tra_get_window_width();
        friend size_t           tra_get_window_height();
        friend void             tra_set_window_title(const char *);
        friend std::string      tra_get_window_title();
        friend void             tra_set_fps(size_t);
        friend void             tra_set_pane_margin(size_t);
        friend size_t           tra_get_pane_margin();

    //panes
    private:
        std::vector<std::unique_ptr<Pane>>      panes;
        Pane*                                   currentPane;

    public:
        friend void             tra_init_termija(int screenWidth,int screenHeight,const char * windowTitle, int paneMargin); 
        friend Pane*            tra_add_pane(size_t, size_t, size_t, size_t);
        friend void             tra_remove_pane(Pane *);
        friend void             tra_clear_panes();
        friend size_t           tra_get_pane_count();
        friend Pane*            tra_get_current_pane();
        friend Pane*            tra_split_pane_vertically(Pane &);
        friend Pane*            tra_split_pane_horizontally(Pane &);
        friend Pane*            tra_split_pane_vertically(Pane &,size_t);
        friend Pane*            tra_split_pane_horizontally(Pane &,size_t);

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
void        tra_init_termija(int,int,const char *);
void        tra_init_termija(int,int,const char *,int);

//window
void        tra_set_window_size(const size_t, const size_t);
size_t      tra_get_window_width();
size_t      tra_get_window_height();
void        tra_set_window_title(const char *);
std::string tra_get_window_title();
void        tra_set_fps(size_t);
size_t      tra_get_fps();
void        tra_set_pane_margin(size_t);
size_t      tra_get_pane_margin();
bool        tra_should_close();

//panes
Pane*       tra_add_pane(size_t, size_t, size_t, size_t);
void        tra_remove_pane(Pane *);
void        tra_clear_panes();
size_t      tra_get_pane_count();
Pane*       tra_get_current_pane();

//drawing
void        tra_draw();
void        tra_draw_pane(const Pane&);
void        tra_draw_pane_border(const Pane&);

};


#endif