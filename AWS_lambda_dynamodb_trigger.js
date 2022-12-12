const AWS = require('aws-sdk');

// Create an IotData object that can be used to communicate with the AWS IoT service
let IotData = new AWS.IotData({
  endpoint: 'a2hh4nrz69x658-ats.iot.us-east-1.amazonaws.com'
});

exports.handler = async (event) => {
  // Log the event that was passed to this Lambda function
  console.log(JSON.stringify(event));

  // Use the DynamoDB Converter to unmarshall the 'NewImage' attribute of each record
  // in the event and store the result in the 'items' array
  const items = event.Records.map((record) => AWS.DynamoDB.Converter.unmarshall(record.dynamodb.NewImage));

  // Extract the 'payload' attribute from each item and store it in the 'answer' array
  const answer = items.map(item => item.payload);

  // Use the IotData object to publish the 'answer' array to the 'esp32/sub' topic
  // on the AWS IoT service
  const response = await IotData.publish({
    topic: 'esp32/sub',
    payload: JSON.stringify({
      "payload": answer
    })
  }).promise();

  // Log the response that was received from the AWS IoT service
  console.log("Resp---> ", response);
}