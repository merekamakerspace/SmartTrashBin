#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define MQTT_VERSION MQTT_VERSION_3_1_1

//Time in seconds the board will be in deep sleep mode. 
const unsigned long int sleepTimeSeconds = 300; //5 minutes
//Max sleep time is 4,294,967,295 µs, which is about ~71 minutes

//Ultrasonic sensor
const int minDistance = 10; //Distance from trash to sensor set for full trash bin
const int trigPin = D7;
const int echoPin = D4;

//Network values
const char* ssid = "MeReka Wifi";
const char* password = "whatwillyoumake?";
const char* mqtt_server = "192.168.0.210";
const char* outTopic = "bin/recycle";

char msg[50];

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client 
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");             
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }    
  }  
}

void sendMessage(const char* content){
      char msg[50];
      snprintf (msg, 75, content);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish(outTopic, msg);
}

int detectDistance(){
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;  
  return distance;
}

void sleep(){
  unsigned long int sleepTimeMicroSeconds = 1000000*sleepTimeSeconds;
  delay(100);  //wait for publishing messages
  Serial.print("Going into deep sleep for ");
  Serial.print(sleepTimeSeconds);
  Serial.print(" seconds (");
  Serial.print(sleepTimeMicroSeconds);
  Serial.println(" miliseconds)");
  ESP.deepSleep(sleepTimeMicroSeconds);  
}

void setup_wifi() {
  delay(100);
  // We start by connecting to a WiFi network
  Serial.println(" ");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);  
    
  Serial.begin(115200);
  Serial.setTimeout(2000);
    
  // Wait for serial to initialize.
  while(!Serial) { }
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  
  //Begin sensor  
  if (!client.connected()) {
    reconnect();    
  }
  client.loop();    

  //Check trash level
  long distance = detectDistance();
      
  delay(100);  //wait for publishing messages
  char cstr[16];
  if (distance < minDistance) {  // This is where the LED On/Off happens
    Serial.println("FULL");
    sendMessage ("FULL");          
  }else{  
    if (distance >= 200 || distance <= 0){
      Serial.println("OUT of range");
      sendMessage ("OUT");    
    }else{    
      //"Trash bin OK"      
      Serial.print(distance);
      Serial.println(" cm");     
      
      itoa(distance, cstr, 10);
      sendMessage (cstr);          
    }
  }        
  sleep(); 
}

void loop() {    
}
