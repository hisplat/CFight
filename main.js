

var net = require("net");
var game = require("./game.js");


var create_map = function(w, h) {
    var gamemap = [];
    for (var y = 0; y < h; y++) {
        gamemap[y] = [];
        for (var x = 0; x < w; x++) {
            gamemap[y][x] = 0;
        }
    }
    return gamemap;
};


var main = function() {
    var gamemap = create_map(20, 20);
    console.debug(gamemap);

    var server = net.createServer(function(connection) {
        console.log("client connected: ");
        console.debug(connection);
        connection.on("end", function() {
            console.debug("closed: " + this);
        });
        connection.on("data", function(d) {
            console.debug(d);
        });
        connection.pipe(connection);
    });

    server.listen("10801", function() {
        console.debug("server is listening on 10801.");
    });
}

main();


