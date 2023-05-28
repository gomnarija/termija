#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <rope.h>

#include <memory>
#include <string>
#include <utility>

TEST_CASE( "Rope Node is created", "[rope_create_node]" ) {
    
    SECTION("creating rope node with valid text"){
        const std::unique_ptr<RopeNode> rope = rope_create_node("some_text");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left == nullptr );
        REQUIRE( rope->right == nullptr );
        REQUIRE( rope->weight == 9 );
        REQUIRE( rope->text != nullptr );
        REQUIRE( strcmp(rope->text.get(), "some_text") == 0 );
    }
    
    SECTION("creating rope node with text length bigger than MAX_WEIGHT"){
        const size_t length = MAX_WEIGHT*3;
        const std::string text(length, 'm');
        const std::unique_ptr<RopeNode> rope = rope_create_node(text.c_str());
     
        REQUIRE( rope != nullptr );
        REQUIRE( rope->left != nullptr );
        REQUIRE( rope->right != nullptr );
        REQUIRE( rope_weight_measure(*rope) + rope_weight_measure(*(rope->right)) == length );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == text );

    }

    SECTION("creating rope node with NULL"){
        const std::unique_ptr<RopeNode> rope = rope_create_node(nullptr);

        REQUIRE( rope == nullptr );
    }
}

TEST_CASE( "Rope is created", "[rope_create]" ) {
    
    SECTION("creating rope with valid text"){
        const std::unique_ptr<RopeNode> rope = rope_create("some_text");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left != nullptr );
        REQUIRE( rope->right == nullptr );
        REQUIRE( rope->weight == 9 );
        REQUIRE( rope->text == nullptr );
        REQUIRE( rope->left->weight == 9 );
        REQUIRE( rope->left->text != nullptr );
        REQUIRE( strcmp(rope->left->text.get(), "some_text") == 0 );
    }
    
    SECTION("creating rope with text length bigger than MAX_WEIGHT"){
        const size_t length = MAX_WEIGHT*3;
        const std::string text(length, 'm');
        const std::unique_ptr<RopeNode> rope = rope_create(text.c_str());
     
        REQUIRE( rope != nullptr );
        REQUIRE( rope->left != nullptr );
        REQUIRE( rope->right == nullptr );
        REQUIRE( rope_weight_measure(*rope) == length );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == text );

    }

    SECTION("creating rope node with NULL"){
        std::unique_ptr rope = rope_create_node(nullptr);

        REQUIRE( rope == nullptr );
    }
}


TEST_CASE( "Rope is destroyed", "[rope_destroy]" ) {
    
    SECTION("destroy rope"){
        const size_t length = MAX_WEIGHT*3;
        const std::string text(length, 'm');
        std::unique_ptr<RopeNode> rope = rope_create(text.c_str());
        rope_destroy(std::move(rope));

        REQUIRE( rope == nullptr );
    }
}

TEST_CASE( "Rope is concatenated", "[rope_concat]" ) {
    
    SECTION("concatenate two ropes"){
        std::unique_ptr<RopeNode> left_rope = rope_create("some_left_text_");
        std::unique_ptr<RopeNode> right_rope = rope_create("some_right_text");
        std::unique_ptr<RopeNode> concat_rope = rope_concat(std::move(left_rope), std::move(right_rope));

        REQUIRE( left_rope == nullptr );
        REQUIRE( right_rope == nullptr );
        REQUIRE( concat_rope != nullptr );
        REQUIRE( concat_rope->weight == 15 );

        std::string rope_text;
        RopeLeafIterator litrope(concat_rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_left_text_some_right_text" );
    }

    SECTION("concatenate rope and text"){
        std::unique_ptr<RopeNode> left_rope = rope_create("some_left_text_");
        std::unique_ptr<RopeNode> concat_rope = rope_concat(std::move(left_rope), "some_right_text");

        REQUIRE( left_rope == nullptr );
        REQUIRE( concat_rope != nullptr );
        REQUIRE( concat_rope->weight == 15 );

        std::string rope_text;
        RopeLeafIterator litrope(concat_rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_left_text_some_right_text" );
    }

    SECTION("concatenate rope and nullptr"){
        std::unique_ptr<RopeNode> left_rope = rope_create("some_left_text_");
        std::unique_ptr<RopeNode> concat_rope = rope_concat(std::move(left_rope), nullptr);

        REQUIRE( left_rope == nullptr );
        REQUIRE( concat_rope != nullptr );
        REQUIRE( concat_rope->right == nullptr );

    }
}

TEST_CASE( "Rope is prepended", "[rope_prepend]" ) {
    
    SECTION("prepend rope"){
        std::unique_ptr<RopeNode> pre_rope = rope_create("some_pre_text_");
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), std::move(pre_rope));

        REQUIRE( pre_rope == nullptr );
        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 23 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_pre_text_some_text" );
    }

    SECTION("prepend rope multiple"){
        std::unique_ptr<RopeNode> pre_rope = rope_create("some_pre_text_");
        rope_prepend(pre_rope.get(), "pre_pre");
        rope_prepend(pre_rope.get(), "pre_pre");
        rope_prepend(pre_rope.get(), "pre_pre");
        rope_prepend(pre_rope.get(), "pre_pre");
        rope_prepend(pre_rope.get(), "pre_pre");
        rope_prepend(pre_rope.get(), "pre_pre");

        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), std::move(pre_rope));

        REQUIRE( pre_rope == nullptr );
        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 65 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "pre_prepre_prepre_prepre_prepre_prepre_presome_pre_text_some_text" );
    }

    SECTION("prepend text"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), "some_pre_text_");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 23 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_pre_text_some_text" );
    }

    SECTION("prepend text multiple"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), "some_pre_text_");
        rope_prepend(rope.get(), "some_more_pre_");
        rope_prepend(rope.get(), "again_");
        rope_prepend(rope.get(), "with_");
        rope_prepend(rope.get(), "the_");
        rope_prepend(rope.get(), "pre_");


        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 56 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "pre_the_with_again_some_more_pre_some_pre_text_some_text" );
    }

    SECTION("prepend empty text"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), "");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left->left == nullptr ); //nothing added
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );
    }

    SECTION("prepend nullptr"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_prepend(rope.get(), nullptr);

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left->left == nullptr ); //nothing added
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );

    }
}

TEST_CASE( "Rope is appended", "[rope_append]" ) {
    
    SECTION("append rope"){
        std::unique_ptr<RopeNode> post_rope = rope_create("_some_post_text");
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), std::move(post_rope));

        REQUIRE( post_rope == nullptr );
        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 24 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text_some_post_text" );
    }

    SECTION("append rope multiple"){
        std::unique_ptr<RopeNode> post_rope = rope_create("_some_post_text");
        rope_append(post_rope.get(), "_");
        rope_append(post_rope.get(), "p");
        rope_append(post_rope.get(), "o");
        rope_append(post_rope.get(), "s");
        rope_append(post_rope.get(), "t");
        rope_append(post_rope.get(), "_and_some");
        rope_append(post_rope.get(), "_");
        rope_append(post_rope.get(), "more");

        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), std::move(post_rope));

        REQUIRE( post_rope == nullptr );
        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 43 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text_some_post_text_post_and_some_more" );
    }

    SECTION("append text"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), "_some_post_text");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 24 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text_some_post_text" );
    }

    SECTION("append text multiple"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), "_some_post_text");
        rope_append(rope.get(), "_ya_");
        rope_append(rope.get(), "ba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dibba_");
        rope_append(rope.get(), "dum");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 70 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text_some_post_text_ya_ba_dibba_dibba_dibba_dibba_dibba_dibba_dum" );
    }

    SECTION("append empty"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), "");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left->left == nullptr ); //nothing added
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );

    }

    SECTION("append nullptr"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_append(rope.get(), nullptr);

        REQUIRE( rope != nullptr );
        REQUIRE( rope->left->left == nullptr ); //nothing added
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );

    }
}


TEST_CASE( "Inserted at index inside of rope", "[rope_insert_at]" ) {
    
    SECTION("insert rope at index inside rope"){
        std::unique_ptr<RopeNode> in_rope = rope_create("_some_text");
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_insert_at(rope.get(), 3, std::move(in_rope));
        rope_insert_at(rope.get(), 9, "_umetak_jos_jedan");
        rope_insert_at(rope.get(), 0, "pre_");
        rope_insert_at(rope.get(), 39, "post_");



        REQUIRE( in_rope == nullptr );
        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 45 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "spre_ome_some__umetak_jos_jedantext_textpost_" );
    }

    SECTION("insert text at index inside rope"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_insert_at(rope.get(), 3, "_some_text");

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 19 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_some_text_text" );
    }

    SECTION("insert nullptr at index inside rope"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_insert_at(rope.get(), 3, nullptr);

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );
    }

    SECTION("insert at invalid index inside rope"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_insert_at(rope.get(), -1, "_some_text");
        rope_insert_at(rope.get(), 9, "_some_text");
        rope_insert_at(rope.get(), MAX_WEIGHT, "_some_text");


        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );

    }
}

TEST_CASE( "Delete at index inside of rope", "[rope_delete_at]" ) {

    SECTION("delete text at index inside rope"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text_to_delete_text");
        rope_delete_at(rope.get(), 3, 15);

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );
    }

    SECTION("delete at invalid index inside rope"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        rope_delete_at(rope.get(), 7, 15);
        rope_delete_at(rope.get(), 4, 0);
        rope_delete_at(rope.get(), MAX_WEIGHT, MAX_WEIGHT);

        REQUIRE( rope != nullptr );
        REQUIRE( rope->weight == 9 );

        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text" );
    }
}

TEST_CASE( "Measure rope weight", "[rope_weight_measure]" ) {

    SECTION("measure weight of the rope"){
        const size_t size = MAX_WEIGHT * 8;
        const std::string text(size, 'm');
        const std::unique_ptr<RopeNode> rope = rope_create(text.c_str());

        REQUIRE( rope_weight_measure(*rope) == size );
    }
}

TEST_CASE( "Measure rope set weight", "[rope_weight_measure_set]" ) {

    SECTION("measure set weight of the rope"){
        const size_t size = MAX_WEIGHT * 8;
        const std::string text(size, 'm');
        std::unique_ptr<RopeNode> rope = rope_create(text.c_str());

        REQUIRE( rope_weight_measure_set(rope.get()) == size );
    }
}

TEST_CASE( "Rebalanced rope", "[rope_rebalance]" ) {

    SECTION("rebalance the given rope"){
        const size_t size = MAX_WEIGHT * 8;
        const std::string text(size, 'm');
        std::unique_ptr<RopeNode> rope = rope_create(text.c_str());
        std::unique_ptr<RopeNode> new_rope = rope_rebalance(std::move(rope)); 

        REQUIRE( rope == nullptr );
        REQUIRE( new_rope->weight == size );
        REQUIRE( rope_is_balanced(*(new_rope->left)) );

        std::string rope_text;
        RopeLeafIterator litrope(new_rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == text );

    }
    SECTION("rebalance the nullptr"){
        std::unique_ptr<RopeNode> new_rope = rope_rebalance(nullptr); 

        REQUIRE( new_rope == nullptr );
    }
}

TEST_CASE( "Rope splited at index", "[rope_split_at]" ) {

    SECTION("splits the rope at the given index"){
        const size_t size = MAX_WEIGHT * 8;
        const size_t new_size = size/3;
        const std::string text(size, 'm');
        std::unique_ptr<RopeNode> rope = rope_create(text.c_str());
        std::unique_ptr<RopeNode> new_rope = rope_split_at(rope.get(), size - new_size -1); 

        REQUIRE( rope != nullptr );
        REQUIRE( new_rope != nullptr );
        REQUIRE( rope->weight == size - new_size );
        REQUIRE( new_rope->weight == new_size);


        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == text.substr(0, size - new_size ) );

        std::string new_rope_text;
        RopeLeafIterator litnrope(new_rope.get());

        while((c = litnrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            new_rope_text += c->text.get();
        }
        REQUIRE( new_rope_text == text.substr(size - new_size  - 1, new_size) );

    }


    SECTION("splits the rope at the given index"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text_some_more_text");
        std::unique_ptr<RopeNode> new_rope = rope_split_at(rope.get(), 10);


        REQUIRE( rope != nullptr );
        REQUIRE( new_rope != nullptr );
        REQUIRE( rope->weight == 11 );
        REQUIRE( new_rope->weight == 13);


        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_text_s" );

        std::string new_rope_text;
        RopeLeafIterator litnrope(new_rope.get());

        while((c = litnrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            new_rope_text += c->text.get();
        }
        REQUIRE( new_rope_text == "ome_more_text" );

    }

    
    SECTION("splits the rope at the given index"){
        std::unique_ptr<RopeNode> rope = rope_create("some_text");
        std::unique_ptr<RopeNode> new_rope = rope_split_at(rope.get(), 4);


        REQUIRE( rope != nullptr );
        REQUIRE( new_rope != nullptr );
        REQUIRE( rope->weight == 5 );
        REQUIRE( new_rope->weight == 4);


        std::string rope_text;
        RopeLeafIterator litrope(rope.get());
        RopeNode *c;

        while((c = litrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            rope_text += c->text.get();
        }
        REQUIRE( rope_text == "some_" );

        std::string new_rope_text;
        RopeLeafIterator litnrope(new_rope.get());

        while((c = litnrope.pop()) != nullptr){
            REQUIRE( c->text != nullptr );
            new_rope_text += c->text.get();
        }
        REQUIRE( new_rope_text == "text" );

    }
}
