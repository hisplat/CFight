

var net = require("net");
var game = require("./game.js");

var players = [];


var find_player = function(socket) {
    for (var k in players) {
        if (players[k].socket == socket) {
            return players[k];
        }
    }
    return null;
}

var main = function() {
    var server = net.createServer(function(connection) {
        console.log("client connected: ");
        console.debug(connection);
        connection.on("end", function() {
            console.debug("closed: " + this);
        });
        connection.on("data", function(d) {
            console.debug(d);
            var op = d.op;
            if (op == "login") {
            } else if (op == "observer") {
            } else if (op == "game") {
            }
        });
        connection.pipe(connection);
    });

    server.listen("10801", function() {
        console.debug("server is listening on 10801.");
    });
}

main();


