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
                <button disabled>Disabled button</button>
                <button class="progress">With progress</button>
            </div>

            <div class="flex-row">
                <div class="flex-column half-gap">
                    <label>Text field</label>
                    <input type="text"/>
                </div>

                <div class="flex-column half-gap">
                    <label>With text</label>
                    <input type="text" value="This text field has text"/>
                </div>

                <div class="flex-column half-gap">
                    <label>Disabled text field</label>
                    <input disabled type="text" value="No input allowed"/>
                </div>
            </div>

            <div class="flex-column third-width">
                <div class="flex-column half-gap">
                    <label>Slider</label>
                    <input type="range" min="0" max="100" step="1" orientation="horizontal"/>
                </div>
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

        .flex-column {
            display: flex;
            flex-direction: column;
            gap: 15px;
        }

        .half-gap {
            gap: 7px;
        }

        .third-width {
            width: 33%;
        }
    )";
}

void test_place::bind_data(Rml::DataModelConstructor& dmc) {
    
}
