# Background
I have an unused IoT device a ESP32 with a E-Paper and an accelerometer sensor, 
I will utelize this for my school project

### Use cases
1. A simple counter that counts main door events in my house
2. Display local weather information for the next 12 hours on a screen at the main door by asking AI from OpenAI

### Project components
Hardware.
Adafruit MagTag.
 * ESP32-S2 240MHz Tensilica processor
 * E-Paper 2.9" grayscale display with 296x128 pixels
 * Triple-axis accelerometer (LIS3DH)
  <hr>

#### This flow allows the device to receive answers to questions posed by the user in real-time, leveraging the power of IoT with Al and cloud technology
![Screenshot_1267](/assets/Screenshot_1267.png)

### Software.
The flow can be described as follows

1. The accelerometer on the device wakes it up when movement is detected, causing it to generate an interrupt signal. ( *Interrupt not yet implemented in the* [**magtag_aws_openai.ino**](https://github.com/Oscar-Tideback/IoT_Porter/blob/main/magtag_aws_openai.ino) )

2. The device then sends an MQTT message to AWS loT Hub, which acts as a broker
between the device and the cloud.

3. IoT Hub triggers a Lambda function, which sends a question to the OpenAl API.

4. The response is received and stored in DynamoDB.

5. DynamoDB triggers an MQTT message to be sent, which is received by the device and displayed on its screen.

6. DataSet loT Analytics can also be used to display graphical representations of the data.


### Some example of working proof of concept.
![Screenshot_1267](/assets/Screenshot_1271.png)
![Screenshot_1267](/assets/Screenshot_1.png)
![Screenshot_1267](/assets/Screenshot_2.png)

### Graphical representation via Quicksight
*Well not much to display only one day of data*
![Screenshot_1267](/assets/Screenshot_1265.png)
