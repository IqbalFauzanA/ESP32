#include <WiFi.h>
#include <WiFiMulti.h>
#include<WiFiClientSecure.h>
int count=0,i,m,j,k;
int t;
int sensor_pin = A0;
int value ;


//ALL DECLARATIONS for CLOUD //////////////////////////////
const char* host = "api.thingsai.io";// OR host = devapi2.thethingscloud.com
const char* post_url = "/devices/deviceData";// OR /api/v2/thingscloud2/_table/data_ac
const char* time_server = "baas.thethingscloud.com";//convert timestamp
const int httpPort = 80;
const int httpsPort = 443;
const char*  server = "api.thingsai.io";  // Server URL
char timestamp[10];
WiFiMulti WiFiMulti;
// Use WiFiClient class to create TCP connections 
WiFiClient client;


//TIMESTAMP CALCULATION function///////////////////////////////////////
int GiveMeTimestamp()
{
	unsigned long timeout = millis();
	// WiFiClient client;
	while (client.available() == 0)
	{
		if (millis() - timeout > 50000)
		{
			client.stop();
			return 0;
		}
	}
	while (client.available())
    {
        String line = client.readStringUntil('\r');                   
		//indexOf() is a funtion to search for smthng , it returns -1 if not found
        int pos = line.indexOf("\"timestamp\"");                       
		//search for "\"timestamp\"" from beginning of response got and copy all data after that , it'll be the timestamp
        if (pos >= 0)                                                     
        {
			int j = 0;      
			for(j=0;j<10;j++)
			{
				timestamp[j] = line[pos + 12 + j];
			}
        }
    }
}  


////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{   
	Serial.begin(115200);
	Serial.println("Reading From the Sensor ...");
	delay(2000);
    // start by connecting to a WiFi network
    WiFiMulti.addAP("Wifi_name", "Wifi_Password");
    Serial.println();
    Serial.println();
	Serial.print("Wait for WiFi... ");
	while(WiFiMulti.run() != WL_CONNECTED) 
	{
		Serial.print(".");
        delay(500);
    }
	Serial.println("");
	Serial.println("WiFi connected");
    Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
    delay(500);
}


void loop() 
{
	// SEND THE QUERY AND RECEIVE THE RESPONSE/////////////////////// 
	value = analogRead(sensor_pin);
	value = map(value,550,0,0,100);		   
	Serial.print("Mositure : ");			
	Serial.print(value);			 
	Serial.println("%");			  
	delay(1000);	 
	Serial.print("connecting to ");	 
	Serial.println(host);                          //defined upside :-
	host = devapi2.thethingscloud.com or 139.59.26.117
	// TIMESTAMP CODE SNIPPET/////////////////////////
	Serial.println("inside get timestamp\n");
	if (!client.connect(time_server, httpPort)) 
	{
		return; //*-*-*-*-*-*-*-*-*-*
	}	 
	client.println("GET /api/timestamp HTTP/1.1");
	client.println("Host: baas.thethingscloud.com");	 
	client.println("Cache-Control: no-cache");	 
	client.println("Postman-Token: ea3c18c6-09ba-d049-ccf3-369a22a284b8");
	client.println();
	GiveMeTimestamp();//it'll call the function which will get the timestamp response from the server
	Serial.println("timestamp receieved");
	Serial.println(timestamp);	 
	Serial.println("inside ThingsCloudPost");
	String PostValue =	"{\"device_id\": 61121695918, \"slave_id\": 2";
	PostValue =	PostValue + ",\"dts\":" + timestamp;
	PostValue =	PostValue +",\"data\":{\"MOIST\":" + value +"}"+"}";	 
	Serial.println(PostValue);
	/* create an instance of WiFiClientSecure */
	WiFiClientSecure client;	   
	Serial.println("Connect to server via port 443");
	if	(!client.connect(server, 443))
	{		   
		Serial.println("Connection failed!");
	} else 
	{
		Serial.println("Connected to server!");
		/* create HTTP request */
		client.println("POST /devices/deviceData HTTP/1.1");
		client.println("Host: api.thingsai.io");
		//client.println("Connection: close");
		client.println("Content-Type: application/json");
		client.println("cache-control: no-cache");
		client.println("Authorization: Bearer eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.IjVhMzBkZDFkN2QwYjNhNGQzODkwYzQ4OSI.kaY6OMj5cYlWNqC2PNTkXs9PKy6_m9tdW5AG7ajfVlY");
		client.print("Content-Length: ");
		client.println(PostValue.length());
		client.println();
		client.println(PostValue);
		//POSTING the data on to the cloud is done and now get the response form cloud server//////////////////
		Serial.print("Waiting for response");
		while (!client.available())
		{
			delay(50);
			Serial.print(".");
		}  
		/* if data is available then receive and print to Terminal */
		while (client.available()) 
		{
			char c = client.read();
			Serial.write(c);
		}
		/* if the server disconnected, stop the client */
		if (!client.connected()) 
		{			   
			Serial.println();
			Serial.println("Server disconnected");
			client.stop();
		}
	}
	Serial.println("//////////////////////    THE END    /////////////////////");
	delay(3000);
}