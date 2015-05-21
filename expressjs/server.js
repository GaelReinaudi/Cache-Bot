// server.js
var express = require('express');
var app     = express();
var port    =   process.env.PORT || 8081;

// sample route
app.get('/sample', function(req, res) {
   res.send('this is a sample route');  
});

// Start the server
app.listen(port);
console.log('Magic happens on port ' + port);