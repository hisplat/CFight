
$(document).ready(function() {
    var color_table = [];
    color_table.push("#ffffff");
    color_table.push("#ff0000");
    color_table.push("#00ff00");
    color_table.push("#0000ff");
    color_table.push("#ff00ff");
    color_table.push("#ffff00");
    color_table.push("#00ffff");

    var find_color = function(id) {
        if (typeof(color_table[id]) == "undefined") {
            return "#000000";
        }
        return color_table[id];
    }

    var page = new Vue({
        el: "#page-content",
        data: {
            gameinfo: null,
            makecolor: function(id) {
                return {
                    "background-color": find_color(id)
                }
            }
        },
        methods: {
            update: function(event) {
            },
        }
    });

    var socket = io();
    socket.on("info", function(o) {
        console.debug(o);
        page.gameinfo = o;
    });

});

