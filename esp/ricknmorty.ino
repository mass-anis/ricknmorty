#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "1does not simply connect to wifi"
#define STAPSK "freckles1"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = "192.168.0.17";
const uint16_t port = 3001;
const int SENS_PIN = 0;
const int DEBOUNCE = 150;

void setup()
{
  Serial.begin(115200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(SENS_PIN, INPUT_PULLUP);
}

WiFiClient client;
int prev_state = 0;
int edge;
void loop()
{
  if (!client.connected())
  {
    Serial.print("connecting to ");
    Serial.print(host);
    Serial.print(':');
    Serial.println(port);

    // Use WiFiClient class to create TCP connections
    if (!client.connect(host, port))
    {
      Serial.println("connection failed");
      delay(5000);
      return;
    }
    else
    {
      Serial.println("connected to server");
      prev_state = digitalRead(SENS_PIN);
    }
  }
  else
  {
    int hi = 0;
    int lo = 0;
    boolean rising = false;
    boolean falling = false;
    //debounce
    if (digitalRead(SENS_PIN) != prev_state)
    {
      while (!rising && !falling)
      {
        if (digitalRead(SENS_PIN))
        {
          hi++;
        }
        else
        {
          lo++;
        }
        delay(1);

        if (hi > DEBOUNCE)
        {
          Serial.print("rising edge");
          rising = true;
          prev_state = 1;
        }
        else
        {
          if (lo > DEBOUNCE)
          {
            Serial.print("falling edge");
            falling = true;
            prev_state = 0;
          }
        }
      }
    }

    if (rising)
    {
      // This will send a string to the server
      Serial.println("sending data to server");
      if (client.connected())
      {
        client.println("open");
      }
    }
    else
    {
      if (falling)
      {
        // This will send a string to the server
        Serial.println("sending data to server");
        if (client.connected())
        {
          client.println("close");
        }
      }
    }
  }
}