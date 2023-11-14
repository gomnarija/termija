#include "termija.h"
#include <plog/Log.h>


#include <fstream>
#include <map>
#include <algorithm>

namespace termija{

void _parse_font_path(const std::string);
void _parse_font_width(const std::string);
void _parse_font_height(const std::string);
void _parse_font_color(const std::string);
void _parse_window_width(const std::string);
void _parse_window_height(const std::string);
void _parse_window_title(const std::string);
void _parse_pane_margin(const std::string);
void _parse_back_color(const std::string);

Color parse_color(const std::string &, Color);

bool configLoaded = false;

std::map<std::string, void (*)(const std::string)> configFields{
    {"fontPath",        &_parse_font_path},
    {"fontWidth",       &_parse_font_width},
    {"fontHeight",      &_parse_font_height},
    {"fontColor",       &_parse_font_color},
    {"windowWidth",     &_parse_window_width},
    {"windowHeight",    &_parse_window_height},
    {"windowTitle",     &_parse_window_title},
    {"paneMargin",      &_parse_pane_margin},
    {"backColor",       &_parse_back_color}
};



/*
    field parsers
*/

void _parse_font_path(const std::string field){
    Termija& termija = Termija::instance();
    termija.fontPath = field;
}

void _parse_font_width(const std::string field){
    Termija& termija = Termija::instance();
    termija.fontWidth = std::stoi(field);
}


void _parse_font_height(const std::string field){
    Termija& termija = Termija::instance();
    termija.fontHeight = std::stoi(field);
}

void _parse_font_color(const std::string field){
    Termija& termija = Termija::instance();
    termija.fontColor = parse_color(field, TERMIJA_COLOR);
}

void _parse_back_color(const std::string field){
    Termija& termija = Termija::instance();
    termija.backColor = parse_color(field, TERMIJA_COLOR);
}

void _parse_window_width(const std::string field){
    tra_set_window_size(std::stoi(field), tra_get_window_height());
}

void _parse_window_height(const std::string field){
    tra_set_window_size(tra_get_window_width(), std::stoi(field));
}

void _parse_window_title(const std::string field){
    tra_set_window_title(field.c_str());
}

void _parse_pane_margin(const std::string field){
    tra_set_pane_margin(std::stoi(field));
}

void _parse(const std::string field,const std::string value){
    //parse field
    if(configFields.find(field) != configFields.end()){
        configFields[field](value);
    }else{
        PLOG_WARNING << "unknown config field: " << field;
    }
}

/*
    helpers
*/


Color parse_color(const std::string &str, Color defaultColor){
    if(std::count(str.begin(), str.end(), ',') != 2){
        PLOG_ERROR << "invalid RGB value: " << str;
        return defaultColor;
    }

    std::string valueToken;
    std::stringstream ss(str);
    try{
        std::getline(ss, valueToken, ',');
        unsigned char redValue = std::stoi(valueToken);
        std::getline(ss, valueToken, ',');
        unsigned char greenValue = std::stoi(valueToken);
        std::getline(ss, valueToken, ',');
        unsigned char blueValue = std::stoi(valueToken);

       return {redValue, greenValue, blueValue, 255};
    }catch(const std::exception& e){
        PLOG_ERROR << "invalid RGB value: " << str;
        return defaultColor;
    }
}


/*
    parse config field value pairs from the given file
*/
void tra_load_config(const char *configPath){
    //set defaults
    tra_default_config();
    //open file
    std::ifstream configFile(configPath);
    if(!configFile.is_open()){
        PLOG_ERROR << "failed to open config file: " << configPath;
        return;
    }
    //parse file
    std::string line, field, value;
    while(std::getline(configFile, line)){
        //empty lines
        if(std::all_of(line.begin(), line.end(), ::isspace))
            continue;

        //parse line, field=value
        std::istringstream    is_line(line);
        if(std::getline(is_line, field, '=')){
            //comments are line that start with #
            if(line[0]=='#')
                continue;
            if( std::getline(is_line, value)){
                //parse
                _parse(field, value);
            }else{
                PLOG_ERROR << "value missing at line: " << line;
            }
        }else{
            PLOG_ERROR << "invalid config line: " << line;
        }
    }
    //done
    configFile.close();
}

void tra_default_config(){
    Termija& termija = Termija::instance();
    configLoaded = true;

    //window
    tra_set_window_size(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    tra_set_window_title(DEFAULT_WINDOW_NAME);
    tra_set_pane_margin(DEFAULT_PANE_MARGIN);

    //font
    termija.fontPath        = DEFAULT_FONT_PATH;
    termija.fontWidth       = DEFAULT_FONT_WIDTH;
    termija.fontHeight      = DEFAULT_FONT_HEIGHT;
    termija.fontSpacing     = DEFAULT_FONT_SPACING;
    termija.fontColor       = TERMIJA_COLOR;
    termija.backColor       = TERMIJA_COLOR;

}

}