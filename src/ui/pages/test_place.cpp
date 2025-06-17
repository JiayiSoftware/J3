#include "test_place.hpp"

std::string_view test_place::id() { return "test_place"; }

std::string_view test_place::layout() {
    return R"(
        <rml>
        <head>
            <title>J3 - test place</title>
        </head>
        <body>
            <h1>Header 1</h1>
            <h2>Header 2</h2>
            <h3>Header 3</h3>

            <p>Regular text <i>Subtext </i> <em>Less important text</em></p>

            <div class="flex-row">
                <button>Regular button</button>
                <button class="less">Less important</button>
                <button class="disabled">Disabled button</button>
                <button class="progress">With progress</button>
            </div>
        </body>
        </rml>
    )";
}

std::string_view test_place::styles() {
    return R"(
        .flex-row {
            display: flex;
            flex-direction: row;
            gap: 15px;
        }
    )";
}

void test_place::bind_data(Rml::DataModelConstructor& dmc) {
    
}
