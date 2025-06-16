#include "global_layer.hpp"

std::string_view global_layer::id() { return "global_layer"; }

std::string_view global_layer::layout() {
    return R"(
        <rml>
        <head>
            <title>J3 global layer</title>
        </head>
        <body>
            <div>
                <h1>J3</h1>

                <label>
                    <input type="radio"/>
                    &#xe88a;
                </label>
            </div>
        </body>
        </rml>
    )";
}

std::string_view global_layer::styles() {
    return R"(
        body {
            height: 100vh;
        }

        div {
            background-color: #FFFFFF0C; /* white with 5% alpha for overlay effect */
            color: #D9D9D9;
            width: 64px;
            border-radius: 4px;
            padding: 0;
            position: fixed;
            top: 15px;
            left: 15px;
            bottom: 15px;
            display: flex;
            flex-direction: column;
            align-items: center;
            backdrop-filter: drop-shadow(#000000 0 4px 2px);
        }

        h1 {
            color: #CE3A3A;
            margin: 15px 15px 15px 14px;
            font-weight: 900;
            font-size: 32px;
            font-style: italic;
            filter: drop-shadow(#0000003F 0 4px 2px);
        }
        
        label {
            width: 64px;
            height: 64px;
            font-size: 24px;
            font-weight: normal;
        }
    )";
}
