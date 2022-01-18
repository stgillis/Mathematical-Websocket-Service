var WebSocketClient = require('websocket').client;

var client = new WebSocketClient();

// Create callback that runs if the connection fails
client.on('connectFailed', function(error)
{
    // Print the error message
    console.log('Connect Error: ' + error.toString());
});

// Create callback that runs if the connection is succesful
client.on('connect', function(connection)
{
    console.log('WebSocket Client Connected');

    // Create callback that runs if an error occurs
    connection.on('error', function(error)
    {
        console.log("Error: " + error.toString());
    });

    // Create callback that runs when the connection is closed
    connection.on('close', function()
    {
        console.log('Connection Closed');
    });

    // Create callback that runs when a message is received
    connection.on('message', function(message)
    {
        if (message.type === 'utf8')
        {
            var json_obj = JSON.parse(message.utf8Data);

            var type = json_obj["type"];
            var payload = json_obj["payload"];

            console.log(type + ": " + payload);

            if (connection.connected)
            {
                //connection.sendUTF("stop-listening");
                connection.close();
            }
        }
    });

    var type = myArgs[0];
    var a = parseFloat(myArgs[1], 10);
    var b = parseFloat(myArgs[2], 10);

    // Create new message
    var message =
    {
        "type": type,
        "payload":
        {
            "a": a,
            "b": b
        }
    }

    if (connection.connected)
    {
        //console.log("Sending: " + JSON.stringify(message))
        connection.sendUTF(JSON.stringify(message));
    }    
});

const myArgs = process.argv.slice(2);

if (myArgs.length == 3 && (myArgs[0] == "add" || myArgs[0] == "subtract") && (parseFloat(myArgs[1], 10) != NaN) && (parseFloat(myArgs[2], 10) != NaN))
{
    // Request connection to the server
    client.connect('ws://localhost:8080/');
}
else
{
    console.log("Invalid parameters");
}