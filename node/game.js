

var net = require("net");
const { StringDecoder } = require("string_decoder");

exports = module.exports = {
    gameinfo: {
        gamemap: null,
        players: [],
        currentplayer: 0,
        attack: {
            x: -1,
            y: -1,
        },
    },
    listener: null,

    listen: function(callback) {
        this.listener = callback;
    },

    init_gamemap: function(w, h) {
        this.gameinfo.gamemap = [];
        for (var y = 0; y < h; y++) {
            this.gameinfo.gamemap[y] = [];
            for (var x = 0; x < w; x++) {
                this.gameinfo.gamemap[y].push({
                    p: 0,
                    h: 0,
                });
            }
        }
    },

    update_gameinfo: function(content) {
        var data = content.slice(4);
        data = data.trim();

        var arrs = data.split("\n");

        // console.debug(arrs);
        const kInit = 0;
        const kInfo = 1;
        const kMap = 2;
        const kHit = 3;
        const kAttack = 4;
        const kPlayers = 5;

        var current = kInit;
        var current_row = 0;
        for (var k in arrs) {
            var line = arrs[k];
            line = line.trim();
            if (line == "") {
                continue;
            }
            // console.debug('line = ' + line);

            if (line == "[info]") {
                current = kInfo;
                console.debug("parse info");
            } else if (line == "[map]") {
                current = kMap;
                current_row = 0;
                console.debug("parse map");
            } else if (line == "[hit]") {
                current = kHit;
                current_row = 0;
                console.debug("parse hit");
            } else if (line == "[attack]") {
                current = kAttack;
            } else if (line == "[players]") {
                current = kPlayers;
                this.gameinfo.players = [];
                console.debug("parse players");
            } else {
                if (current == kInfo) {
                    var scs = line.split(" ");
                    var w = scs[0];
                    var h = scs[1];
                    this.init_gamemap(w, h);
                } else if (current == kMap) {
                    var ids = line.split(" ");
                    for (var k in ids) {
                        this.gameinfo.gamemap[current_row][k].p = ids[k];
                    }
                    current_row++;
                } else if (current == kHit) {
                    var ids = line.split(" ");
                    for (var k in ids) {
                        this.gameinfo.gamemap[current_row][k].h = ids[k];
                    }
                    current_row++;
                } else if (current == kPlayers) {
                    // console.debug(line);

                    var pss = line.split(":");
                    if (pss.length != 2) {
                        continue;
                    }

                    this.gameinfo.players.push({
                        playerid: pss[0],
                        name: pss[1].trim(),
                    });
                } else if (current == kAttack) {
                    var scs = line.split(" ");
                    var x = scs[0];
                    var y = scs[1];
                    this.gameinfo.attack.x = x;
                    this.gameinfo.attack.y = y;
                } else {
                }
            }
        }
    },

    update_gameturn: function(content) {
    },

    connect: function(callback) {
        socket = new net.Socket();
        socket.connect(11233, 'localhost');

        socket.on("connect", function() {
            // socket.write("LOGIN TOKEN_BOXER");
        });

        socket.on("end", function() {
        });

        var that = this;
        socket.on("data", function(data) {
            const decoder = new StringDecoder("utf8");
            var content = decoder.write(data);
            // console.debug(content);

            var command = content.split(/\s/, 1);
            console.debug(command);
            if (command[0] == "GAME") {
                that.update_gameinfo(content);
            } else if (command[0] == "TURN") {
                that.update_gameturn(content);
            }
            callback(that.gameinfo);
            if (that.listener != null) {
                that.listener(that.gameinfo);
            }
        });
    },
}


