#include "test_place.hpp"

std::string_view test_place::id() {
    return "test-place";
}

std::string_view test_place::layout() {
    return R"(
        <rml>
        <head>
            <title>J3 - test place</title>
        </head>
        <body>
            <h1>J3</h1>
            <p>I wrote this text using harfbuzz, and you can tell because these characters I just wrote are overhanging</p>
        </body>
        </rml>
    )";
}

std::string_view test_place::styles() {
    return R"(
        body {
            font-family: Montserrat;
            background-color: #0F0F0F;
            color: #FFFFFF;
            padding: 2em 1em;
            position: absolute;
            width: 900px;
            height: 550px;
        }

        h1 {
            font-size: 32px;
            font-weight: bold;
        }

        p {
            font-size: 16px;
        }
    )";
}
