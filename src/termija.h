#ifndef TERMIJA_H
#define TERMIJA_H

#include "rope.h"
#include "widget.h"
#include "raylib.h"

#include <string>
#include <memory>
#include <stack>
#include <vector>

namespace termija{


//default values
inline const uint16_t            DEFAULT_WINDOW_WIDTH               = 800;
inline const uint16_t            DEFAULT_WINDOW_HEIGHT              = 600;
inline const uint16_t            DEFAULT_WINDOW_MARGIN              = 40;
inline const char               *DEFAULT_WINDOW_NAME                = "termija";
inline const uint16_t            DEFAULT_TARGET_FPS                 = 60;
inline const uint8_t             DEFAULT_PANE_MARGIN                = 3;
//font
inline const char               *DEFAULT_FONT_PATH                  = "res/fonts/unscii-16-full.ttf";
inline const uint8_t             DEFAULT_FONT_WIDTH                 = 8;
inline const uint8_t             DEFAULT_FONT_HEIGHT                = 16;
inline const uint8_t             DEFAULT_FONT_SPACING               = 1;
inline const uint16_t            DEFAULT_TTF_GLYPH_COUNT            = 666;
//shaders
inline const uint16_t            GLSL_VERSION                       = 330;
inline const char               *DEFAULT_BASE_SHADER_PATH           = "res/shaders/base.vs";
inline const char               *DEFAULT_POST_SHADER_PATH           = "res/shaders/post.fs";
inline const char               *DEFAULT_ALPHA_DISCARD_SHADER_PATH  = "res/shaders/alpha_discard.fs";
inline const char               *DEFAULT_BLOOM_SHADER_PATH          = "res/shaders/bloom.fs";

inline Shader                   ALPHA_DISCARD_SHADER;
inline Shader                   BLOOM_SHADER;
inline Shader                   POST_SHADER;

inline const Color              TERMIJA_COLOR                       = (Color){ 238, 232, 213, 225};
inline const Color              ALPHA_DISCARD                       = (Color){ 26, 26, 26, 255 };
//res
inline const char               *DEFAULT_BACK_TEXTURE_PATH           = "res/screen.png";





extern bool configLoaded;



struct PaneFrame final{
    size_t          beginning;
    size_t          end;
    size_t          localIndexStart;//start of the rope to beggining
    size_t          localBeginning;
    RopeNode*       nodeStart;

    PaneFrame();
    PaneFrame(const PaneFrame&) = delete;
    PaneFrame& operator=(const PaneFrame&) = delete;
};

struct Pane final{
private:

    uint16_t                                        oldWidth;
    uint16_t                                        oldHeight;
    std::vector<std::unique_ptr<Widget>>            widgets;

public:
    Pane*                   top;
    Pane*                   bottom;
    Pane*                   left;
    Pane*                   right;
    uint16_t                topX;
    uint16_t                topY;
    uint16_t                width;
    uint16_t                height;
    uint8_t                 textMargin;//non-zero


    Pane(uint16_t,uint16_t,uint16_t,uint16_t);
    Pane(const Pane&) = delete;
    Pane& operator=(const Pane&) = delete;
    
    friend void             tra_update_pane(Pane&);
    friend Pane*            tra_split_pane_vertically(Pane &,uint16_t);
    friend Pane*            tra_split_pane_horizontally(Pane &,uint16_t);
    friend Pane*            tra_merge_panes(Pane &, Pane &);
    friend void             tra_pane_is_resized(Pane &, int16_t, int16_t);
    friend void             tra_draw_pane(const Pane &);
    friend void             tra_draw_pane_border(const Pane &);
    friend void             tra_set_font_size(Pane&, uint8_t, uint8_t);

    friend Widget*          tra_add_widget(Pane&, std::unique_ptr<Widget>);

};

void                        tra_update_pane(Pane&);
Pane*                       tra_split_pane_vertically(Pane &);
Pane*                       tra_split_pane_horizontally(Pane &);
Pane*                       tra_split_pane_vertically(Pane &,uint16_t);
Pane*                       tra_split_pane_horizontally(Pane &,uint16_t);
Pane*                       tra_merge_panes(Pane &, Pane &);
void                        tra_pane_is_resized(Pane &, int16_t, int16_t);
void                        tra_set_font_size(Pane&, uint8_t, uint8_t);


Widget*                     tra_add_widget(Pane&, std::unique_ptr<Widget>);
Text*                       tra_create_text(Pane &);


//singleton
class Termija final{
    //window
    private:
        uint16_t                            windowWidth;
        uint16_t                            windowHeight;
        uint16_t                            windowMargin;
        std::string                         windowTitle;
        uint8_t                             paneMargin;
        Font                                font;
        Texture2D                           backTexture;
        RenderTexture2D                     renderTexture;
        RenderTexture2D                     completeFrame;
        float                               time;
        std::stack<RenderTexture2D>         renderTextureGarbageStack;

    public:
        std::string                         fontPath;
        uint8_t                             fontWidth;
        uint8_t                             fontHeight;
        uint8_t                             fontSpacing;
        Vector4                             justLooking;

    public:
        friend void             tra_set_window_size(const uint16_t, const uint16_t);
        friend uint16_t         tra_get_window_width();
        friend uint16_t         tra_get_window_height();
        friend void             tra_set_window_margin(uint16_t);
        friend uint16_t         tra_get_window_margin();
        friend void             tra_set_window_title(const char *);
        friend std::string      tra_get_window_title();
        friend void             tra_set_fps(uint16_t);
        friend void             tra_set_pane_margin(uint8_t);
        friend uint8_t          tra_get_pane_margin();

    //panes
    private:
        std::vector<std::unique_ptr<Pane>>      panes;
        Pane*                                   currentPane;

    public:
        friend void             tra_init_termija(uint16_t screenWidth, uint16_t screenHeight,const char * windowTitle, uint8_t paneMargin); 
        friend void             tra_terminate();
        friend void             tra_update();
        friend Pane*            tra_add_pane(uint16_t, uint16_t, uint16_t, uint16_t);
        friend Pane*            tra_duplicate_pane(Pane *);
        friend Pane*            tra_impose_duplicate_pane(Pane *);
        friend void             tra_remove_pane(Pane *);
        friend void             tra_clear_panes();
        friend size_t           tra_get_pane_count();
        friend Pane*            tra_get_current_pane();
        friend void             tra_set_current_pane(Pane*);
        friend Pane*            tra_split_pane_vertically(Pane &);
        friend Pane*            tra_split_pane_horizontally(Pane &);
        friend Pane*            tra_split_pane_vertically(Pane &, uint16_t);
        friend Pane*            tra_split_pane_horizontally(Pane &, uint16_t);
        friend void             tra_load_font(const char*, uint8_t, uint16_t);
        friend Font*            tra_get_font();
        friend void             tra_push_render_texture_to_garbage(RenderTexture2D);
        friend void             tra_unload_render_textures();
        friend RenderTexture2D  tra_get_render_texture();

    public:
        friend void             tra_draw();
        friend void             tra_draw_current();


    private:
        Termija();
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
void        tra_update();

//window
void        tra_set_window_size(const uint16_t, const uint16_t);
uint16_t    tra_get_window_width();
uint16_t    tra_get_window_height();
void        tra_set_window_margin(uint16_t);
uint16_t    tra_get_window_margin();
void        tra_set_window_title(const char *);
std::string tra_get_window_title();
uint16_t    tra_get_screen_width();
uint16_t    tra_get_screen_height();
void        tra_set_fps(uint16_t);
size_t      tra_get_fps();
void        tra_set_pane_margin(uint8_t);
uint8_t     tra_get_pane_margin();
bool        tra_should_close();
void        tra_look_around();
bool        tra_is_mouse_moving_away();

//panes
Pane*       tra_add_pane(uint16_t, uint16_t, uint16_t, uint16_t);
Pane*       tra_duplicate_pane(Pane *);
Pane*       tra_impose_duplicate_pane(Pane *);
void        tra_remove_pane(Pane *);
void        tra_clear_panes();
size_t      tra_get_pane_count();
Pane*       tra_get_current_pane();
void        tra_set_current_pane(Pane*);

//drawing
void        tra_draw();
void        tra_draw_current();
void        tra_draw_pane(const Pane&);
void        tra_draw_pane_border(const Pane&);
void        tra_draw_text(RopeNode *, uint16_t, uint16_t, uint16_t, uint16_t);
void        tra_draw_text(RopeNode *, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, size_t);
void        tra_draw_text(RopeNode *, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, Cursor &);
void        tra_draw_cursor(uint16_t, uint16_t, Cursor &);
void        tra_draw_back(uint16_t , uint16_t , const Texture2D *, const Shader *);
Texture2D   invert_font(Texture2D);
void        tra_push_render_texture_to_garbage(RenderTexture2D);
void        tra_unload_render_textures();
RenderTexture2D        tra_get_render_texture();
void        tra_draw_rectangle(uint16_t, uint16_t, uint16_t, uint16_t);


//font
void        tra_load_font();
void        tra_load_font(const char*, uint8_t, uint16_t);
Font*       tra_get_font();
Texture2D*  tra_get_font_inverted();

//config
void        tra_load_config(const char *);
void        tra_default_config();

//utils
size_t    weight_until_prev_new_line(RopeNode *, size_t);
size_t    weight_until_next_new_line(RopeNode *, size_t);
float     tra_delta_time();

inline const uint16_t tra_get_text_width(const Pane& pane){
    const Termija& termija = Termija::instance();
    return termija.fontWidth == 0 ? 0:((pane.width - (2*(pane.textMargin))) / (termija.fontWidth+termija.fontSpacing));
}

inline const uint16_t tra_get_text_height(const Pane& pane){
    const Termija& termija = Termija::instance();
    return termija.fontHeight == 0 ? 0:((pane.height - (2*(pane.textMargin))) / termija.fontHeight);
}

};


#endif