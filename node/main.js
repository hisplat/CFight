

var game = require("./game.js");
var webserver = require("./webserver.js");


var main = function() {
    game.connect(function(gameinfo) {
        console.debug("gameinfo updated.");
        // console.debug(gameinfo.gamemap);
    });
    webserver.init(game);
}

main();


