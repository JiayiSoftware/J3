#include "global_layer.hpp"

std::string_view global_layer::id() {
    return "global_layer";
}

std::string_view global_layer::layout() {
    return R"(
        <rml>
        <head>
            <title>J3 global layer</title>
        </head>
        <body>
            <div>
                <h1>J3</h1>
            </div>
        </body>
        </rml>
    )";
}

std::string_view global_layer::styles() {
    return R"(
        div {
            background-color: #1E1E1E;
            color: #FFFFFF;
            width: 4rem;
            height: 100vh;
        }
    )";
}
