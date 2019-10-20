#include <ESP8266WiFi.h>
#include <Ticker.h>

#ifndef STASSID
#define STASSID "1does not simply connect to wifi"
#define STAPSK "freckles1"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

const char *host = "192.168.0.17";
const uint16_t port = 3001;

const int LED_PIN  = 2; //On board LED
const int SENS_PIN = 12; //reed switch
const int ZC_PIN = 4; //Zero crossing pin
const int TRIAC_PIN = 5; //PWM pin

const int TMR1_US = 5; // timer count for 1us
const int SIG_PERIOD = (TMR1_US*16666); // 60Hz period in timer ticks
const int TRIG_PERIOD = (TMR1_US*100); //width of the triac trigger signal 
const int DEBOUNCE = 150;
int dimTmrPeriod = 0;



void ICACHE_RAM_ATTR onTimerISR();
void ICACHE_RAM_ATTR onZeroCrosssingISR();


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
  pinMode(ZC_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIAC_PIN, OUTPUT);

  //Initialize Ticker every 0.5s
  timer1_attachInterrupt(onTimerISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);

  attachInterrupt(ZC_PIN, onZeroCrosssingISR, RISING);
}

/**
 * @brief calculates the timer offset period from the ZC signal that corresponds to the 
 * % level. the % is mapped to the signal period not the light inrensity
 * 
 * @param level %
 * @return int timer offset that corresponds to the % level
 */
int calcDimTimerPeriod(int level)
{
  int tmr;
  tmr = level*SIG_PERIOD/100;
  return tmr;
}

/**
 * @brief ZC interrupt used to sync the triac control signal to the main AC
 * in this function start the timer to do the main delay from the ZC start
 */
void ICACHE_RAM_ATTR onZeroCrosssingISR()
{
  timer1_write(dimTmrPeriod);
}

/**
 * @brief timer ISR, this function generate the trigger signal for the triac
 * it rearms the timer for the trigger width then stops
 * the timer will get started again on the next ZC interrupt
 */
void ICACHE_RAM_ATTR onTimerISR()
{
  static boolean trig = true;
  if(trig)
  {
    digitalWrite(LED_PIN, 1); 
    timer1_write(TRIG_PERIOD);
    trig = false;
  }else{
    digitalWrite(LED_PIN, 0);
    trig = true;
  }
  // digitalWrite(LED_PIN, !digitalRead(LED_PIN)); 
  //   timer1_write(TRIG_PERIOD);
}

WiFiClient client;
int prev_state = 0;
int edge;
void loop()
{
  dimTmrPeriod = calcDimTimerPeriod(20);
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
  else //client connected
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