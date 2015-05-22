// server.js
var express = require('express');
var app     = express();
var port    =   process.env.PORT || 8081;
var spawn = require('child_process').spawn;

// sample route
app.get('/sample', function(req, res) {
    var _ = require('underscore'); // for some utility goodness
    var deploySh = spawn('sh', [ 'test.sh' ], {
      cwd: '/home/ubuntu/Cache-Bot/expressjs',
      env:_.extend(process.env, { PATH: process.env.PATH + ':/usr/local/bin' })
    });
    res.send('this is a sample route');  
});

app.get('/cache-bot/extraCash/:user_id', function(req, res) {
	console.log('POST to /cache-bot/extracash/:user_id')
	if (isCacheBot(req, res)) {
			if (req.params.user_id == null) {
				var response = {
					"error" : "Cannot find either req.params.user_id in POST URL"
				}
				res.send(200, response)
			} else {
				var response = {
					"success" : "Successfully parsed extra cash for user."
				}
				res.send(200, response)
			}
		}
	}
});

// Start the server
app.listen(port);
console.log('Magic happens on port ' + port);
