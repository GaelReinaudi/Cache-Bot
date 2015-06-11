// server.js
var express = require('express');
var app     = express();
var port    =   process.env.PORT || 8081;
var child_process = require('child_process');
var _ = require('underscore'); // for some utility goodness

// sample route
app.get('/sample', function(req, res) {
    var deploySh = child_process.spawn('sh', 
        [ 'test.sh' ], {
        cwd: '/home/ubuntu/Cache-Bot/expressjs',
        env:_.extend(process.env, { PATH: process.env.PATH + ':/usr/local/bin' })
    });
    res.send('this is a sample route');
});

app.get('/cache-bot/fullAnalysis/:user_id', function(req, res) {
    console.log('POST to /cache-bot/extracash/:user_id')

    var deploySh = child_process.spawn('sh',
        ['fullAnalysis.sh', req.params.user_id], {
        cwd: '/home/ubuntu/Cache-Bot/bin/release',
        env:_.extend(process.env, { PATH: process.env.PATH + ':./' })
    });
    console.log(deploySh)

    var response = {
        "success" : "Successfully parsed extra cash for user: " + req.params.user_id,
        "return" : deploySh
    }
    res.send(200, response)
});

// Start the server
app.listen(port);
console.log('Magic happens on port ' + port);
