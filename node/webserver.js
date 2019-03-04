
var fs = require("fs");
var express = require("express");
var http = require("http");
var socketio = require("socket.io");


exports = module.exports = {
    init: function(game) {
        // var readfile = function(filename, res) {
        //     fs.readFile("./webapp/" + filename, function(err, data) {
        //         if (err) {
        //             console.error(err);
        //             res.writeHeader(404);
        //             res.end();
        //         } else {
        //             console.debug("send file " + filename);
        //             res.writeHeader(200);
        //             res.write(data.toString());
        //             res.end();
        //         }
        //     });
        // };

        var app = express();
        var server = http.createServer(app);
        var io = socketio.listen(server);


        var log_request = function(res) {
            console.log("[%s:%s]", res.socket.remoteAddress, res.socket.remotePort);
        };
        app.get("/", function(req, res) {
            log_request(res);
            // readfile("index.htm", res);
            res.sendFile(__dirname + "/webapp/index.htm");
        });

        app.use("/socket.io/", express.static(__dirname + "/node_modules/socket.io-client/dist"));
        app.use(express.static("webapp"));

        server.listen(8080, function() {
            var host = server.address().address;
            var port = server.address().port;
            console.log("express server listening on %s:%s", host, port);
        });

        io.on("connection", function(socket) {
            // console.debug(socket);
            socket.on("start", function(obj) {
                console.debug("start game.");
            });
            socket.emit("info", game.gameinfo);
            // console.debug("info emited.");
        });
        game.listen(function(gameinfo) {
            io.emit("info", gameinfo);
        });
    },
};

