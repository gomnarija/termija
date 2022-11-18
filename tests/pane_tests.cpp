#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <termija.h>


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