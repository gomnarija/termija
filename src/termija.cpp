#include <memory>
#include <iostream>
#include <string>
#include <cstring>  //strcpy

#include "termija.h"
#include <raylib.h>
#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

namespace termija{

Termija::Termija() :
    windowWidth{0},
    windowHeight{0},
    windowMargin{0},
    paneMargin{0},
    currentPane{nullptr},
    fontWidth{0},
    fontHeight{0},
    fontSpacing{0},
    time{0}{}



Termija& tra_get_instance(){
    return Termija::instance();
}

void tra_terminate(){
    const Termija& termija = Termija::instance();

    //delete panes
    tra_clear_panes();

    //font
    if(termija.font.glyphCount > 0)
        UnloadFont(termija.font);
    //shaders
    UnloadShader(BLOOM_SHADER);
    UnloadShader(POST_SHADER);
    UnloadShader(ALPHA_DISCARD_SHADER);


    //render textures
    UnloadRenderTexture(termija.renderTexture);
    UnloadRenderTexture(termija.completeFrame);

    CloseWindow();
}

void tra_init_termija(){
    //config
    if(!configLoaded){
        tra_default_config();
    }

    tra_init_termija(tra_get_window_width(), tra_get_window_height(), tra_get_window_title().c_str(), tra_get_pane_margin());
}

void tra_init_termija(uint16_t windowWidth,uint16_t windowHeight,const char * windowTitle){
    //config
    if(!configLoaded){
        tra_default_config();
    }

    tra_init_termija(windowWidth, windowHeight, windowTitle, tra_get_pane_margin());
}

void tra_init_termija(uint16_t windowWidth,uint16_t windowHeight,const char * windowTitle,uint8_t paneMargin){
    Termija& termija = tra_get_instance();
    std::string workingDirectory = GetWorkingDirectory() + std::string("\\");

    //plog
    plog::init(plog::debug, "termija.log");

    //config
    if(!configLoaded){
        tra_default_config();
    }

    //raylib
    // SetConfigFlags(FLAG_WINDOW_UNDECORATED);//remove title bar
    InitWindow(windowWidth, windowHeight, windowTitle);

    //font
    tra_load_font();

    //textures
    termija.backTexture         = LoadTexture(DEFAULT_BACK_TEXTURE_PATH);
    termija.backTexture.width   = windowWidth;
    termija.backTexture.height  = windowHeight;
    
    //shaders
    ALPHA_DISCARD_SHADER        = LoadShader(NULL, TextFormat((workingDirectory+std::string(DEFAULT_ALPHA_DISCARD_SHADER_PATH)).c_str(), GLSL_VERSION));
    BLOOM_SHADER                = LoadShader(NULL, TextFormat((workingDirectory+std::string(DEFAULT_BLOOM_SHADER_PATH)).c_str(), GLSL_VERSION));
    POST_SHADER                 = LoadShader(TextFormat((workingDirectory+std::string(DEFAULT_BASE_SHADER_PATH)).c_str()), 
                                                TextFormat((workingDirectory+std::string(DEFAULT_POST_SHADER_PATH)).c_str(), GLSL_VERSION));

    termija.renderTexture       = LoadRenderTexture(termija.windowWidth, termija.windowHeight);
    termija.completeFrame       = LoadRenderTexture(termija.windowWidth, termija.windowHeight);

    //mouse
    SetMousePosition(windowWidth/2, windowHeight/2);
    HideCursor();


    //window
    tra_set_window_size(windowWidth, windowHeight);
    tra_set_window_title(windowTitle);
    tra_set_pane_margin(paneMargin);
    termija.windowMargin = DEFAULT_WINDOW_MARGIN;

    //panes
    termija.currentPane = tra_add_pane(paneMargin + termija.windowMargin, paneMargin + termija.windowMargin, tra_get_screen_width() - 2*paneMargin, tra_get_screen_height() - 2*paneMargin);
}

void tra_update(){
    Termija& termija = Termija::instance();


    if(IsKeyPressed(KEY_Q)){
        CloseWindow();
    }


    //update panes
    for(size_t i=0;i<termija.panes.size();i++){
        Pane *pane = termija.panes[i].get();
        if(pane == nullptr){
            PLOG_ERROR << "pane at index: " << i << " is NULL, skipped.";
            continue;
        }
        tra_update_pane(*pane);
    }

    //look around, my little babe
    tra_look_around();
}

void tra_draw(){
    Termija &termija = Termija::instance();
    termija.time += GetFrameTime();
    //update shader uniforms
    SetShaderValue(POST_SHADER, GetShaderLocation(POST_SHADER, "time"), &termija.time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(POST_SHADER, GetShaderLocation(POST_SHADER, "justLooking"), &termija.justLooking, SHADER_UNIFORM_VEC4);
    //render onto texture
    BeginTextureMode(termija.renderTexture);
        //draw panes
            ClearBackground(BLANK);
            for(size_t i=0;i<termija.panes.size();i++){
                Pane *pane = termija.panes[i].get();
                if(pane == nullptr){
                    PLOG_ERROR << "pane is NULL at index: " << i << ", skipped.";
                    continue;
                }
                tra_draw_pane(*pane);

            }
    EndTextureMode();
    //draw renderedTexture onto background
    BeginTextureMode(termija.completeFrame);
        ClearBackground(BLANK); 
        tra_draw_back(termija.windowWidth, termija.windowHeight,  &(termija.backTexture), nullptr);
        if(BLOOM_SHADER.id > 0)
            BeginShaderMode(BLOOM_SHADER);
                DrawTextureRec(termija.renderTexture.texture, {0,0,(float)termija.windowWidth, (float)-termija.windowHeight}, {0,0}, RAYWHITE);   
        if(BLOOM_SHADER.id > 0);
            EndShaderMode();
    EndTextureMode();
    //draw
    BeginDrawing();
        BeginShaderMode(POST_SHADER);
            ClearBackground(BLACK); 
            DrawTextureRec(termija.completeFrame.texture, {0,0,(float)termija.windowWidth, (float)-termija.windowHeight}, {0,0}, RAYWHITE);   
        EndShaderMode();
    EndDrawing();


    tra_unload_render_textures();
}




void tra_draw_current(){
    Termija &termija = Termija::instance();
    termija.time += GetFrameTime();
    //update shader uniforms
    SetShaderValue(POST_SHADER, GetShaderLocation(POST_SHADER, "time"), &termija.time, SHADER_UNIFORM_FLOAT);
    SetShaderValue(POST_SHADER, GetShaderLocation(POST_SHADER, "justLooking"), &termija.justLooking, SHADER_UNIFORM_VEC4);
    //render onto texture
    BeginTextureMode(termija.renderTexture);
        //draw current pane
            ClearBackground(BLANK);
            if(termija.currentPane == nullptr){
                PLOG_ERROR << "current pane is NULL, aborted.";
            }else{
                tra_draw_pane(*(termija.currentPane));
            }
    EndTextureMode();
    //draw renderedTexture onto background
    BeginTextureMode(termija.completeFrame);
        ClearBackground(BLANK); 
        tra_draw_back(termija.windowWidth, termija.windowHeight,  &(termija.backTexture), nullptr);
        if(BLOOM_SHADER.id > 0)
            BeginShaderMode(BLOOM_SHADER);
                DrawTextureRec(termija.renderTexture.texture, {0,0,(float)termija.windowWidth, (float)-termija.windowHeight}, {0,0}, RAYWHITE);   
        if(BLOOM_SHADER.id > 0);
            EndShaderMode();
    EndTextureMode();
    //draw
    BeginDrawing();
        BeginShaderMode(POST_SHADER);
            ClearBackground(BLACK); 
            DrawTextureRec(termija.completeFrame.texture, {0,0,(float)termija.windowWidth, (float)-termija.windowHeight}, {0,0}, RAYWHITE);   
        EndShaderMode();
    EndDrawing();


    tra_unload_render_textures();
}




void tra_set_window_size(uint16_t width,uint16_t height){
    Termija& termija = Termija::instance();

    termija.windowWidth = width;
    termija.windowHeight = height;

    if(GetWindowHandle() != nullptr){
        SetWindowSize(width, height);
    }
}

uint16_t tra_get_window_width(){
    const Termija& termija = Termija::instance();

    return termija.windowWidth;
}

uint16_t tra_get_window_height(){
    const Termija& termija = Termija::instance();

    return termija.windowHeight;
}

void tra_set_window_margin(uint16_t windowMargin){
    Termija& termija = Termija::instance();
    termija.windowMargin = windowMargin;
    //todo:resize screen
}

uint16_t tra_get_window_margin(){
    const Termija& termija = Termija::instance();
    return termija.windowMargin;
}


void tra_set_window_title(const char *windowTitle){
    Termija& termija = Termija::instance();

    termija.windowTitle = std::string(windowTitle);

    if(GetWindowHandle() != nullptr){
        SetWindowTitle(windowTitle);
    }
}

std::string tra_get_window_title(){
    const Termija& termija = Termija::instance();

    return termija.windowTitle;
}

void tra_look_around(){
    Termija& termija = Termija::instance();
    uint16_t mouseX = GetMouseX();
    uint16_t mouseY = GetMouseY();//moves faster the further you are away from center
    uint16_t xSpeed = 3 + (abs((tra_get_window_width()/2) - mouseX)/13);
    uint16_t ySpeed = 3 + (abs((tra_get_window_height()/2) - mouseY)/13);

    //center if outside center area
    if(mouseX < tra_get_window_width()/2.3){
        mouseX = tra_get_window_width()/2.3;
    }
    if(mouseX > tra_get_window_width()-(tra_get_window_width()/2.3)){
        mouseX = tra_get_window_width()-(tra_get_window_width()/2.3);
    }
    if(mouseY < tra_get_window_height()/2.3){
        mouseY = tra_get_window_height()/2.3;
    }
    if(mouseY > tra_get_window_height()-(tra_get_window_height()/2.3)){
        mouseY = tra_get_window_height()-(tra_get_window_height()/2.3);
    }

    //move towards center of the window
    if(!tra_is_mouse_moving_away()){
        if(mouseX < tra_get_window_width()/2)
            mouseX += std::max(1, (int)(((tra_get_window_width()/2) - mouseX) * 0.123));
        if(mouseX > tra_get_window_width()/2)
            mouseX -= std::max(1, (int)((mouseX - (tra_get_window_width()/2)) * 0.123));
        if(mouseY < tra_get_window_height()/2)
            mouseY += std::max(1, (int)(((tra_get_window_height()/2) - mouseY) * 0.123));
        if(mouseY > tra_get_window_height()/2)
            mouseY -= std::max(1, (int)((mouseY - (tra_get_window_height()/2)) * 0.123));
    }
    SetMousePosition(mouseX, mouseY);
    //set uniform
    termija.justLooking.x = ((tra_get_window_width()/2) - GetMouseX()) / (float)(tra_get_window_width()/2);
    termija.justLooking.y = (GetMouseY() - (tra_get_window_height()/2)) / (float)(tra_get_window_height()/2);
}

bool tra_is_mouse_moving_away(){
    static uint16_t mouseX, mouseY;
    static float time=0.23;

    if((mouseX>GetMouseX() && mouseX<(tra_get_window_width()/2) ||
            mouseX<GetMouseX() && mouseX>(tra_get_window_width()/2) ||
                mouseY>GetMouseY() && mouseY<(tra_get_window_height()/2) ||
                    mouseY<GetMouseY() && mouseY>(tra_get_window_height()/2))){
        time = 0.23;
        mouseX = GetMouseX();
        mouseY = GetMouseY();
        return true;
    }
    else if(time > 0.023){
        time -= tra_delta_time();
        mouseX = GetMouseX();
        mouseY = GetMouseY();
        return true;
    }
    else{
        mouseX = GetMouseX();
        mouseY = GetMouseY();
        return false;
    }

}


void tra_set_fps(uint16_t targetFPS){
    if(GetWindowHandle() != nullptr){
        SetTargetFPS(targetFPS);
    }
}

uint16_t tra_get_screen_width(){
    return tra_get_window_width() - (2*tra_get_window_margin());
}

uint16_t    tra_get_screen_height(){
    return tra_get_window_height() - (2*tra_get_window_margin());
}

void tra_set_pane_margin(uint8_t paneMargin){
    Termija& termija = Termija::instance();

    termija.paneMargin = paneMargin;
    //TODO: rescale panes
}

uint8_t tra_get_pane_margin(){
    const Termija& termija = Termija::instance();

    return termija.paneMargin;
}

bool tra_should_close(){
    return WindowShouldClose();
}

Pane* tra_add_pane(uint16_t topX, uint16_t topY, uint16_t width, uint16_t height){
    Termija& termija = Termija::instance();

    termija.panes.push_back(std::make_unique<Pane>(topX, topY, width, height));
    return termija.panes.back().get();
}

Pane* tra_duplicate_pane(Pane *pane){
    Termija& termija = Termija::instance();

    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return nullptr;
    }

    termija.panes.push_back(std::make_unique<Pane>(pane->topX, pane->topY, pane->width, pane->height));
    return termija.panes.back().get();
}

Pane* tra_impose_duplicate_pane(Pane *pane){
    Termija& termija = Termija::instance();

    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return nullptr;
    }

    termija.panes.push_back(std::make_unique<Pane>(pane->topX, pane->topY, pane->width, pane->height));
    tra_set_current_pane(termija.panes.back().get());
    return termija.currentPane;
}

void tra_remove_pane(Pane *pane){
    Termija& termija = Termija::instance();

    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return;
    }

    bool        pane_removed = false;
    for(size_t i = 0;i<termija.panes.size();i++){
        if(termija.panes[i].get() == pane){
            pane_removed = true;
            //disconnect
            Pane &to_remove = *(termija.panes[i]);
            if(to_remove.left != nullptr)
                to_remove.left->right = to_remove.right;
            if(to_remove.right != nullptr)
                to_remove.right->left = to_remove.left;
            if(to_remove.top != nullptr)
            to_remove.top->bottom = to_remove.bottom;
            if(to_remove.bottom != nullptr)
                to_remove.bottom->top = to_remove.top;        
            //TODO:destroy widgets
            //rescale neighbours //TODO
            termija.panes[i].swap(termija.panes[termija.panes.size()-1]);
            termija.panes.pop_back();
            break;
        }
    }

    if(!pane_removed){
        PLOG_WARNING << "no panes match the given pointer, none were removed.";
    }
}

void tra_clear_panes(){
    Termija& termija = Termija::instance();

    //destroy ropes
    for(size_t i = 0;i<termija.panes.size();i++){
        //TODO:destroy widgets
    }
    //clear vector
    termija.panes.clear();
}

size_t tra_get_pane_count(){
    const Termija& termija = Termija::instance();
    
    return termija.panes.size();
}

Pane* tra_get_current_pane(){
    const Termija& termija = Termija::instance();

    return termija.currentPane;
}

void tra_set_current_pane(Pane* pane){
    if(pane == nullptr){
        PLOG_ERROR << "given pane is NULL, aborted.";
        return;
    }
    Termija& termija = Termija::instance();
    bool has_pane = false;

    for(size_t i=0;i<termija.panes.size();i++){
        if (termija.panes[i].get() == pane){
            has_pane = true;
            break;
        }
    }
    if(has_pane){
        termija.currentPane = pane;
    }else{
        PLOG_WARNING << "given pane wasn't found inside panes, aborted.";
    }
}

void tra_load_font(){
    const Termija& termija = Termija::instance();
    tra_load_font(termija.fontPath.c_str(), termija.fontHeight, DEFAULT_TTF_GLYPH_COUNT);
}
void tra_load_font(const char *fontPath, uint8_t fontSize, uint16_t glyphCount){
    if(fontPath == nullptr){
        PLOG_ERROR << "given path is NULL, aborted.";
        return;
    }
    Termija& termija = Termija::instance();
    termija.font = LoadFontEx(fontPath, fontSize, NULL, glyphCount);

    if(termija.font.glyphCount == 0){
        PLOG_ERROR << "failed to load font: " << fontPath;
        return;
    }
}

Font* tra_get_font(){
    Termija& termija = Termija::instance();
    return &(termija.font);
}

uint16_t tra_get_font_width(){
    Termija& termija = Termija::instance();
    return (termija.fontWidth+termija.fontSpacing);
}
uint16_t tra_get_font_height(){
    Termija& termija = Termija::instance();
    return (termija.fontHeight);
}

void tra_push_render_texture_to_garbage(RenderTexture2D renderTexture2D){
    Termija& termija = Termija::instance();
    termija.renderTextureGarbageStack.push(renderTexture2D);
}

void tra_unload_render_textures(){
    Termija& termija = Termija::instance();
    while(!termija.renderTextureGarbageStack.empty()){
        UnloadRenderTexture(termija.renderTextureGarbageStack.top());
        termija.renderTextureGarbageStack.pop();
    }
}

RenderTexture2D tra_get_render_texture(){
    Termija& termija = Termija::instance();
    return termija.renderTexture;
}


float tra_delta_time(){
    //raylib
    return GetFrameTime();
}


}