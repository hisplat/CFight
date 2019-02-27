
var net = require("net");


var socket = new net.Socket();
socket.connect(10801, "localhost", function() {
    console.debug("connected.");
});

socket.on("data", function(data) {
    console.debug(data);
});

socket.on("end", function() {
    console.debug("server down.");
});

console.debug("bye.");

