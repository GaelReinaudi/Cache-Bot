var child_process = require('child_process');
console.log('Loading function');

exports.handler = function(event, context) {
    //console.log('Received event:', JSON.stringify(event, null, 2));
    jsonStr = JSON.stringify(event);
    console.log('extraCash  body: '+ jsonStr);
    console.log('user: '+ event.user_id);
    var folder = '../../expressjs';
    var fileBase = 'extraCash';
    if (event.cmd) {
        fileBase = event.cmd;
    }

    console.log("starting " + fileBase);
    var start = new Date().getTime();
    var runExtra = child_process.execFile(
        folder + '/' + fileBase + '.sh', [jsonStr], {
            cwd: './bin/release',
            timeout: 550 * 1000 ,
            maxBuffer: 16 * 1024 * 1024},
        function(err, stdout, stderr) {
            console.log(err);
            console.log(stdout);
            console.log(stderr);
            var response = "ran " + fileBase + " for user: " + event.user_id+'\n'
            + stdout + '\n' + stderr;
            if (err)
                context.fail(err);  // Echo back the response
            context.succeed(response);  // Echo back the response
        }
    );
    var end = new Date().getTime();
    var time = end - start;
    console.log('Execution time: ' + time);
};
