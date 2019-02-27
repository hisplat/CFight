
var helper = require("./helper.js");


// init -> addplayer ->  grow -> attack(player, position) -> grou -> attach -> grou -> attack ...
//
// status:
//      0. offline
//      1. gaming 

exports = module.exports = {
    parties: [],
    gamemap: [],
    m_w: 0,
    m_h: 0,

    init: function(w, h) {
        for (var y = 0; y < h; y++) {
            gamemap[y] = [];
            for (var x = 0; x < w; x++) {
                gamemap[y][x] = {
                    uid: 0,
                    hp: 0,
                };
            }
        }
    },

    findparty: function(name) {
        for (var k in parties) {
            if (parties[k].name == name) {
                return k;
            }
        }
        return -1;
    },

    findempty: function() {
        var empty = [];
        for (var y in gamemap) {
            for (var x in gamemap[y]) {
                if (gamemap[y][x].uid == 0) {
                    empty.push({x: x, y: y});
                }
            }
        }
        if (empty.length == 0) {
            return null,
        }
        var index = helper.rand(0, empty.length - 1);
        return empty[index];
    },

    addparty: function(name, option) {
        if (option.speed + option.attack + option.hit != 100) {
            return -1;
        }
        var pk = findparty(name);
        if (party == -1) {
            var loc = findemtpy();
            if (loc == null) {
                return -2;
            }

            var newparty = {
                name: name,
                speed: option.speed,
                attack: option.attack,
                hit: option.hit,
                home: loc,
                stat: 1,
            };
            parties.push(party);

            return parties.length - 1;
        } else {
            parties[pk].stat = 1;
            return pk;
        }
        return 0;
    },

    grow: function() {
    },

    attack: function() {
    },

    place: function(name, loc) {
        var uid = findparty(name);
        if (uid == -1) {
            return;
        }
        gamemap[loc.y][loc.x] = {
        };
    },
};


