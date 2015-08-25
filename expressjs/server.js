// server.js
var express = require('express');
var app     = express();
var port    =   process.env.PORT || 8081;
var child_process = require('child_process');
var _ = require('underscore'); // for some utility goodness

app.get('/cache-bot/fullAnalysis/:user_id', function(req, res) {
    console.log('POST to /cache-bot/fullAnalysis/:user_id')

    var runFull = child_process.execFile(
        '../../expressjs/fullAnalysis.sh', [req.params.user_id], {
            cwd: '/home/ubuntu/Cache-Bot/bin/release',
            timeout: 0 },
        function(err, stdout, stderr) {
            console.log(err);
            console.log(stdout);
            console.log(stderr);
            var response = "ran fullAnalysis for user: " + req.params.user_id+'\n'
            + stdout + '\n' + stderr;
            res.status(200).send(response)
        }
    );
});

app.get('/cache-bot/extraCash/:user_id', function(req, res) {
    console.log('POST to /cache-bot/extraCash/:user_id('+req.params.user_id+')')

    var runExtra = child_process.execFile(
        '../../expressjs/extraCash.sh', [req.params.user_id], {
            cwd: '/home/ubuntu/Cache-Bot/bin/release',
            timeout: 30000 },
        function(err, stdout, stderr) {
            console.log(err);
            console.log(stdout);
            console.log(stderr);
            var response = "ran extraCache for user: " + req.params.user_id+'\n'
            + stdout + '\n' + stderr;
            res.status(200).send(response)
        }
    );
});

app.post('/cache-bot/fullAnalysis', function(req, res) {
    console.log('POST to /cache-bot/fullAnalysis/:user_id')

    var runFull = child_process.execFile(
        '../../expressjs/fullAnalysis.sh', [req.params.user_id], {
            cwd: '/home/ubuntu/Cache-Bot/bin/release',
            timeout: 0 },
        function(err, stdout, stderr) {
            console.log(err);
            console.log(stdout);
            console.log(stderr);
            var response = "ran fullAnalysis for user: " + req.params.user_id+'\n'
            + stdout + '\n' + stderr;
            res.status(200).send(response)
        }
    );
});

app.post('/cache-bot/extraCash', function(req, res) {
    console.log('POST to /cache-bot/extraCash/:user_id('+req.params.user_id+')')

    var runExtra = child_process.execFile(
        '../../expressjs/extraCash.sh', [req.params.user_id], {
            cwd: '/home/ubuntu/Cache-Bot/bin/release',
            timeout: 30000 },
        function(err, stdout, stderr) {
            console.log(err);
            console.log(stdout);
            console.log(stderr);
            var response = "ran extraCache for user: " + req.params.user_id+'\n'
            + stdout + '\n' + stderr;
            res.status(200).send(response)
        }
    );
});

// Start the server
app.listen(port);
console.log('Magic happens on port ' + port);
