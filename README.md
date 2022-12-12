#Background.
I have an unused IoT device a ESP32 with a E-Paper and an accelerometer sensor, I will utelize this for my school project.  

###Use cases.
Use case 1 - Notify me to bring an umbrella in case of risk for rain
Use case 2 - Notify me of door events, when door is opened/closed
1. A simple counter that counts main door events in my house
2. Display local weather information for the next 12 hours on a screen at the main door

#####Project components.
Hardware.
Adafruit MagTag.
  ESP32-S2 240MHz Tensilica processor.
  E-Paper 2.9" grayscale display with 296x128 pixels.
  Triple-axis accelerometer (LIS3DH)


![Screenshot_1267](/assets/Screenshot_1267.png)
The flow can be described as follows.
#####Software.
1. The device, equipped with an accelerometer, wakes up when it senses movement and
generates an interrupt signal.

2. The device sends an MQTT message to AWS loT Hub, which acts as a broker between the
device and the cloud.

3. loT Hub triggers a Lambda function, which sends a question to OpenAl API.

4. The answer is received and stored in DynamoDB.

5. DynamoDB triggers an MQTT message to be sent, which is received by the device and displayed on its screen.

This flow allows the device to receive answers to questions posed by the user in real-time, leveraging the power of Al and cloud technology.