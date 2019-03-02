
var net = require("net");
var player = require("player");

exports = module.exports = {
    mNetServer: null,
    mClients: [],

    init: function() {
        var find_client = function(socket) {
            for (var k in mClients) {
                if (mClients[k].socket == socket) {
                    return mClients[k];
                }
            }
            return null;
        }

        var server = net.createServer(function(connection) {
            console.log("client connected: ");
            console.debug(connection);
            connection.on("end", function() {
                console.debug("closed: " + this);
            });
            connection.on("data", function(d) {
                console.debug(d);

                var client = find_client(connection);

                var op = d.op;
                if (op == "login") {
                    if (client == null) {
                        client = player.create();
                        client.socket = connection;
                        mClients.push(client);
                    } else {
                        connection.write("{op: 'fail'}");
                    }
                } else if (op == "observer") {
                } else if (op == "game") {
                }
            });
            connection.pipe(connection);
        });

        server.listen("10801", function() {
            console.debug("server is listening on 10801.");
        });
        mNetServer = server;
    }
};

