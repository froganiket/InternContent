// #include <ESP8266WiFi.h>
// #include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h> // Required for generating the JSON payload

#define SAMPLE_RATE 500
#define BAUD_RATE 115200
#define INPUT_PIN_CLOSE 34
#define BUFFER_SIZE 128
#define EMG_MIN 0
#define EMG_MAX 600

const char* ssid = "Aniket iPhone";
const char* password = "aniket11";
const char* serverAddress = "172.20.10.6";
const int serverPort = 3000; // Replace with your server port

int circular_buffer[BUFFER_SIZE];
int data_index, sum;
int prevPosition = 0;

// Function to generate the dynamic payload with real-time values
String generatePayload() {

  // // Calculate elapsed time
	static unsigned long past = 0;
	unsigned long present = micros();
	unsigned long interval = present - past;
	past = present;

	// Run timer
	static long timer = 0;
	timer -= interval;

  int envelop = 0;
	// Sample and get envelop
	if(timer < 0) {
		timer += 1000000 / SAMPLE_RATE;
		int sensor_value = analogRead(INPUT_PIN_CLOSE);
		int signal = EMGFilter(sensor_value);
		envelop = getEnvelop(abs(signal));
		// Serial.print(signal);
		// Serial.print(",");
		// Serial.println(envelop);
	}

  float normalizedEnvelop = map(envelop, EMG_MIN, EMG_MAX, 0, 100) / 100.0;
  Serial.print("Close: ");
  Serial.println(normalizedEnvelop);

   // Create a JSON object
  StaticJsonDocument<128> jsonDocClose;
  jsonDocClose["ClosePositionRef"] = normalizedEnvelop;

  // Serialize the JSON object to a string
  String payloadClose;
  serializeJson(jsonDocClose, payloadClose);

  return payloadClose;
}



void setup() {
  Serial.begin(BAUD_RATE);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}



// Declare a variable to store the previous envelope value
int previousClosePosition = 0;
// Declare a flag variable to track the state
bool aboveThreshold = false;

void loop() {
  // Create a WiFiClient object
  WiFiClient client;

  String payload = generatePayload();

// Parse the JSON payload to extract the envelope value
  DynamicJsonDocument jsonDoc(128);
  deserializeJson(jsonDoc, payload);
  float currentPosition = jsonDoc["ClosePositionRef"];

  // Create an HTTPClient object with the WiFiClient
  HTTPClient http;
  String endpoint = "/close";
  http.begin(client, "http://" + String(serverAddress) + ":" + String(serverPort) + endpoint);

  // Set the content type header
  http.addHeader("Content-Type", "application/json");

   // Check if the current envelope value is above the threshold
  if (currentPosition > 0.5 && !aboveThreshold) {
    // Set the flag to true
    aboveThreshold = true;

   // Create a JSON object for the desired payload
    StaticJsonDocument<128> desiredPayload;
    desiredPayload["ClosePositionRef"] = 1; // Set the value to 1

     // Serialize the JSON object to a string
    String desiredPayloadString;
    serializeJson(desiredPayload, desiredPayloadString);

    // Send the POST request with the desired payload
    int httpResponseCode = http.POST(desiredPayloadString);

    // Check the response code
    if (httpResponseCode == 200) {
      Serial.println("Data sent successfully: " + desiredPayloadString);
    } else {
      Serial.print("Error sending data. HTTP response code: ");
      Serial.println(httpResponseCode);
    }
  }
  // Check if the current position is below the threshold
  else if (currentPosition < 0.5 && aboveThreshold) {
    // Set the flag to false
    aboveThreshold = false;
  }

  // End the HTTP request
  http.end();

  //Delay before sending the next request
  // delay(1000);




  //same things as above but for Opening arm


  // // Create an HTTPClient object with the WiFiClient
  // HTTPClient httpOpen;
  // httpOpen.begin(client, "http://" + String(serverAddress) + ":" + String(serverPort) + "/open");

  // // Set the content type header
  // httpOpen.addHeader("Content-Type", "application/json");


  //  // Check if the current envelope value is above the threshold
  // if (currentOpenPosition > 0.5 && !aboveOpenThreshold && currentClosePosition < 0.5) {
  //   // Set the flag to true
  //   aboveOpenThreshold = true;

  //   // Create a JSON object for the desired payload
  //   StaticJsonDocument<128> desiredOpenPayload;
  //   desiredOpenPayload["OpenPositionRef"] = 1; // Set the value to 1

  //   // Serialize the JSON object to a string
  //   String desiredOpenPayloadString;
  //   serializeJson(desiredOpenPayload, desiredOpenPayloadString);

  //   // Send the POST request with the desired payload
  //   int httpResponseCodeOpen = httpOpen.POST(desiredOpenPayloadString);

  //   // Check the response code
  //   if (httpResponseCodeOpen == 200) {
  //     Serial.println("Data sent successfully: " + desiredOpenPayloadString);
  //   } else {
  //     Serial.print("Error sending data. HTTP response code: ");
  //     Serial.println(httpResponseCodeOpen);
  //   }
  // } okay generate Belur close karita no Raju Bhaiya
  // // Check if the current envelope value is below the threshold
  // else if (currentOpenPosition < 0.5 && aboveOpenThreshold && currentClosePosition > 0.5) {
  //   // Set the flag to false
  //   aboveOpenThreshold = false;
  // }

  // // Update the previous envelope value
  // previousOpenPosition = currentOpenPosition;

  // // End the HTTP request
  // httpOpen.end();

  // Delay before sending the next request
  // delay(1000);
}




// Envelop detection algorithm
int getEnvelop(int abs_emg){
	sum -= circular_buffer[data_index];
	sum += abs_emg;
	circular_buffer[data_index] = abs_emg;
	data_index = (data_index + 1) % BUFFER_SIZE;
	return (sum/BUFFER_SIZE) * 2;
}

// Band-Pass Butterworth IIR digital filter, generated using filter_gen.py.
// Sampling rate: 500.0 Hz, frequency: [74.5, 149.5] Hz.
// Filter is order 4, implemented as second-order sections (biquads).
// Reference: 
// https://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.butter.html
// https://courses.ideate.cmu.edu/16-223/f2020/Arduino/FilterDemos/filter_gen.py
float EMGFilter(float input)
{
  float output = input;
  {
    static float z1, z2; // filter section state
    float x = output - 0.05159732*z1 - 0.36347401*z2;
    output = 0.01856301*x + 0.03712602*z1 + 0.01856301*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -0.53945795*z1 - 0.39764934*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - 0.47319594*z1 - 0.70744137*z2;
    output = 1.00000000*x + 2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  {
    static float z1, z2; // filter section state
    float x = output - -1.00211112*z1 - 0.74520226*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1;
    z1 = x;
  }
  return output;
}
