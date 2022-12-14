const https = require('https');
const AWS = require('aws-sdk');

// Define the question and model to use
//const question = 'Hur stor är risken för regn i stockholm idag? Svara med 3 rader text och 30 bokstäver per rad samt avsluta raderna med \n';
const question = 'What is the risk of rain in Stockholm today? Answer with 3 lines of text and 30 characters per line, ending each line with \n.';
const model = 'text-davinci-003';

// Configure the DynamoDB client
const dynamoDB = new AWS.DynamoDB.DocumentClient();

exports.handler = function (event, context, callback) {
// Set the request body to send to OpenAI
  const body = JSON.stringify({
  'prompt': question,
  'model': model,
  'max_tokens': 256,
  'temperature': 0.5
});
// Set the request options
const options = {
  hostname: 'api.openai.com',
  path: '/v1/completions',
  port: 443,
  method: 'POST',
  headers: {
    'Authorization': 'Bearer xx-XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX',
    'Content-Type': 'application/json'
  },
};

// Send the request to OpenAI
const req = https.request(options, (res) => {
    // When the response is received, parse the response and extract the text
    res.on('data', (d) => {
        const data = JSON.parse(d.toString());

        // Print the response text to the console
        console.log(data.choices[0].text);

        // Store the response text in DynamoDB
        dynamoDB.put({
            TableName: 'openAI_chat',
            Item: {
                'timestamp': new Date().toString(),
                'payload': data.choices[0].text,
                'id': new Date().getTime().toString()
            }
        }, (err) => {
            if (err) {
                console.error(`Error storing item: ${err}`);
            } else {
                console.log('Item stored successfully');
            }
        });
    });
});
  
// Handle any errors that occur during the request
req.on('error', (e) => {
    console.error(e);
});

// Write the request body and send the request
req.write(body);
req.end();
};