#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <termija.h>


TEST_CASE( "Termija default init" ){
    termija::tra_init_termija();
}


TEST_CASE( "Pane splitted vertically", "[tra_split_pane_vertically]" ) {
    
    SECTION("splitting pane vertically"){
        termija::tra_set_pane_margin(3);
        termija::Pane *left = termija::tra_add_pane(3, 3, 497, 497);
        termija::Pane *right = termija::tra_split_pane_vertically(*left);

        REQUIRE( right != nullptr);
        REQUIRE( right->topX == 254);
        REQUIRE( right->topY == 3);
        REQUIRE( right->width == 247);
        REQUIRE( right->height == 497);

        REQUIRE( left != nullptr);
        REQUIRE( left->topX == 3);
        REQUIRE( left->topY == 3);
        REQUIRE( left->width == 248);
        REQUIRE( left->height == 497);
        
        termija::tra_clear_panes();
    }
}

TEST_CASE( "Pane splitted horizontally", "[tra_split_pane_horizontally]" ) {
    
    SECTION("splitting pane horizontally"){
        termija::tra_set_pane_margin(3);
        termija::Pane *top = termija::tra_add_pane(3, 3, 497, 497);
        termija::Pane *bottom = termija::tra_split_pane_horizontally(*top);

        REQUIRE( bottom != nullptr);
        REQUIRE( bottom->topX == 3);
        REQUIRE( bottom->topY == 254);
        REQUIRE( bottom->width == 497);
        REQUIRE( bottom->height == 247);

        REQUIRE( top != nullptr);
        REQUIRE( top->topX == 3);
        REQUIRE( top->topY == 3);
        REQUIRE( top->width == 497);
        REQUIRE( top->height == 248);

        termija::tra_clear_panes();
    }
}

TEST_CASE( "Pane merged", "[tra_merge_panes]" ) {
    
    SECTION("merging panes"){
        termija::tra_set_pane_margin(3);
        termija::Pane *top = termija::tra_add_pane(3, 3, 497, 497);
        termija::Pane *bottom = termija::tra_split_pane_horizontally(*top);
        termija::tra_merge_panes(*top, *bottom);

        REQUIRE( top != nullptr);
        REQUIRE( top->topX == 3);
        REQUIRE( top->topY == 3);
        REQUIRE( top->width == 497);
        REQUIRE( top->height == 498);

        termija::tra_clear_panes();
    }
}

TEST_CASE( "Cursor move", "[tra_move_cursor]" ) {
    
    SECTION("moving cursor"){
        //font size 8x16
        termija::tra_set_pane_margin(3);
        std::string text(3003, 'm');
        termija::Pane *top = termija::tra_add_pane(0, 0, 500, 500);
        termija::tra_insert_text_at_cursor(*top, text.c_str());
        const size_t textWidth = termija::tra_get_text_width(*top);
        const size_t textHeight = termija::tra_get_text_height(*top);


        const termija::Cursor &c = termija::tra_get_cursor(*top);

        REQUIRE(c.x == 3003%textWidth);
        REQUIRE(c.y == textHeight-1);

        termija::tra_move_cursor_down(top, 1000);

        REQUIRE(c.x == 2003%textWidth);
        REQUIRE(c.y == textHeight-1 - (1000/textWidth));

        termija::tra_clear_panes();
    }
}

TEST_CASE( "Cursor positioned", "[tra_move_cursor]" ) {
    
    SECTION("positions cursor"){
        //font size 8x16
        termija::tra_set_pane_margin(3);
        std::string text(3003, 'm');
        termija::Pane *top = termija::tra_add_pane(0, 0, 500, 500);
        termija::tra_insert_text_at_cursor(*top, text.c_str());
        const size_t textWidth = termija::tra_get_text_width(*top);
        const size_t textHeight = termija::tra_get_text_height(*top);
        const termija::Cursor &c = termija::tra_get_cursor(*top);

        REQUIRE(c.x == 3003%textWidth);
        REQUIRE(c.y == textHeight-1);

        termija::tra_position_cursor(top, 15, 5);

        REQUIRE(c.x == 15);
        REQUIRE(c.y == 5);

        termija::tra_clear_panes();
    }
    
    SECTION("positions cursor invalid coordinates"){
        //font size 8x16
        termija::tra_set_pane_margin(3);
        std::string text(64, 'm');
        termija::Pane *top = termija::tra_add_pane(0, 0, 500, 500);
        termija::tra_insert_text_at_cursor(*top, text.c_str());
        const size_t textWidth = termija::tra_get_text_width(*top);
        const size_t textHeight = termija::tra_get_text_height(*top);
        const termija::Cursor &c = termija::tra_get_cursor(*top);

        REQUIRE(c.x == 64%textWidth);
        REQUIRE(c.y == 64/textWidth);

        termija::tra_position_cursor(top, 13, 6);

        REQUIRE(c.x == 64%textWidth);
        REQUIRE(c.y == 64/textWidth);

        termija::tra_clear_panes();
    }
}

TEST_CASE( "Termija terminate" ){
    termija::tra_terminate();
}
