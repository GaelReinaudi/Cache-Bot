// server.js
var express = require('express');
var app     = express();
var port    =   process.env.PORT || 8081;
var spawn = require('child_process').spawn;
var _ = require('underscore'); // for some utility goodness

// sample route
app.get('/sample', function(req, res) {
    var deploySh = spawn('sh', [ 'test.sh' ], {
      cwd: '/home/ubuntu/Cache-Bot/expressjs',
      env:_.extend(process.env, { PATH: process.env.PATH + ':/usr/local/bin' })
    });
    res.send('this is a sample route');  
});

app.get('/cache-bot/extraCash/:user_id', function(req, res) {
	console.log('POST to /cache-bot/extracash/:user_id')
	var response = {
		"success" : "Successfully parsed extra cash for user: " + req.params.user_id
	}
	res.send(200, response)

    var deploySh = spawn('./test', [ 'extraCash:' + req.params.user_id ], {
      cwd: '/home/ubuntu/Cache-Bot/tests',
      env:_.extend(process.env, { PATH: process.env.PATH + ':/usr/local/bin' })
    });

});

// Start the server
app.listen(port);
console.log('Magic happens on port ' + port);
