#include <Wire.h>
#include <ESP8266WiFi.h>              //Including the ESP8266 WiFi library in order to usm them
#include <U8x8lib.h>
#include <Adafruit_BMP085.h>          //Including the library for BMP180
#include <DHT.h>                      //Including the DHT library
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>
#include <Adafruit_Sensor.h>           //In order to use DHT sensor we have to include this library first 

Adafruit_BMP085 bmp;                  //Defining the object bmp
#define I2C_SCL 5                    //Connect SCL pin GPIO5(D1) of Nodemcu
#define I2C_SDA 4                    //Connect SDA pin GPIO4(D2) of Nodemcu

#define DHTPIN 14                                    //Connect the DHT22 sensor's data pin to GPIO14(D5) of Nodemcu    
#define DHTTYPE DHT22                               //Mention the type of sensor we are using, Here it it DHT22, for DHT11 just replace DHT22 with DHT11

#define ONE_WIRE_BUS 12  //Connect the DSB1820 sensor's data pin to GPIO12(D6) of Nodemcu
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
DHT dht(DHTPIN, DHTTYPE); //Defining the pin and the dhttype

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

Ticker flipper;

//BMP180 variables
float dst,bt,bp,ba;
bool bmp085_present=true;
//DHT22 Variables
float h,t;
//Pulse Sensor Variables
const int maxAvgSample = 20;
volatile int rate[maxAvgSample];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;
//DSB1820 Variable
float temp=0;
float prevTemp = 0;
//Wifi Variables
const char* ssid     = "arsalan-multilive-03219231052";
const char* password = "300saeed003";
//const char* ssid     = "Xperia";
//const char* password = "12345678";
const char* host = "ehealthssuet.000webhostapp.com";
const int httpPort = 80;
//Other Variables
unsigned char dly=0;
String BtempC;
String humC;
String TC;
String PC;
String aC;
String str ;
uint8_t heart_char[8] = { 0x0c,0x1e,0x3e,0x7c,0x7c,0x3e,0x1e,0x0c};
uint8_t wifi_char[8] = { 0x04, 0x0a, 0x25, 0xd5, 0xd5, 0x25, 0x0a, 0x04};
uint8_t server_char[8] = { 0x80, 0x4c, 0xbd, 0x92, 0x9a, 0xf6, 0x62, 0x01};
boolean cond_switch = true;
boolean Check = false;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Wire.begin(I2C_SDA, I2C_SCL); //Initializing the I2C connection
  u8x8.begin();
  dht.begin();  //Initializing the DHT sensor
  DS18B20.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);
  u8x8.drawTile(10, 0, 1, wifi_char);
  u8x8.setCursor(0, 0);
  if (!bmp.begin()){
    u8x8.print("fail");
    //Serial.println("fail");
    while(1);  }
  else
    u8x8.print("HOME");
  flipper.attach_ms(2, Test); // Test function will be call after every 2 milli seconds
}  
  
void loop() {
  // Wait a few seconds between measurements.
  DS18B20.requestTemperatures();  // Send the command to get temperatures
  delay(500);
  temp = DS18B20.getTempCByIndex(0);
  temp= (temp*1.86)+32.0;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  if((dly%4)==0)
  {
    h = dht.readHumidity();
    t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)){
    return;
  }}
  if((dly%10)==0)
  {
    ba =  bmp.readAltitude();
    dst = bmp.readSealevelPressure()/100;
    if(dly==20)dly=0;
  }  
   
  if (QS == true) // A Heartbeat Was Found
    {
   u8x8.setCursor(0, 1);
   u8x8.print("                ");     
   u8x8.setCursor(0, 1);
   u8x8.print("BPM ");
   u8x8.print(BPM);
   //Serial.println(BPM);
   u8x8.drawTile(10, 1, 1, heart_char);
   QS = false; // reset the Quantified Self flag for next time    
    }
  u8x8.setCursor(0,2);
  u8x8.print("                ");     
  u8x8.setCursor(0,2);
  u8x8.print("BTemp ");
  u8x8.print(temp);
  //Serial.println(temp);
  u8x8.print("F");
  u8x8.setCursor(0,3);
  u8x8.print("                ");     
  u8x8.setCursor(0, 3);
  u8x8.print("IBI ");
  u8x8.print(IBI);
  //Serial.println(IBI);
  u8x8.drawTile(10, 3, 1, heart_char);
  u8x8.setCursor(0,4);
  u8x8.print("                ");     
  u8x8.setCursor(0, 4);
  u8x8.print("Hum. ");
  u8x8.print(h);
  //Serial.println(h);
  u8x8.print(" %");
  u8x8.setCursor(0,5);
  u8x8.print("                ");
  u8x8.setCursor(0,5);
  u8x8.print("A Temp ");
  u8x8.print(t);
  //Serial.println(t);
  u8x8.print(" C");
  u8x8.setCursor(0,6);
  u8x8.print("                ");
  u8x8.setCursor(0,6);
  u8x8.print("Pres ");
  u8x8.print(dst);
  //Serial.println(dst);
  u8x8.print(" mb");
  u8x8.setCursor(0,7);
  u8x8.print("                ");
  u8x8.setCursor(0,7);
  u8x8.print("Alti: ");
  u8x8.print(ba);
  //Serial.println(ba);
  u8x8.print(" m"); 
  if (dly==19){
  conv_data();
  strdev();
  send_sens();
  if ((cond_switch == false)&&(Check == false)) {
  u8x8.clear();
  u8x8.setCursor(0,3);
  u8x8.print("SHUTDOWN in 10s ");
  u8x8.setCursor(0,4);
  u8x8.print(" Contact Server ");
  delay(9000);
  u8x8.clear();
  u8x8.setPowerSave(1);
  Check=true;
  }  
  while((cond_switch == false)&&(Check == true))
  {
  str="/Success.php?D_ID=0001&Beats_Mins=0&Body_Temp=0&Beats_Interval=0&Humidity=0&Atm_Temprature=0&Atm_Pressure=0&Cal_Altitude=0&Guardian_Id=0001";
  send_sens();
  delay(10000);
  }
  if((cond_switch == true)&&(Check == true)) {
  u8x8.clear();
  u8x8.setPowerSave(0);
  u8x8.drawTile(0, 10, 1, wifi_char);
  Check=false;
  }
  }
  dly++;
}
void Test()
{
  
      Signal = analogRead(A0);              // read the Pulse Sensor 
        sampleCounter += 2;                         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

      if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
            T = Signal;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N > 250){                                   // avoid high frequency noise
  if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
    Pulse = true;                               // set the Pulse flag when we think there is a pulse
    IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
    lastBeatTime = sampleCounter;               // keep track of time for next pulse
         
         if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat = false;                 // clear secondBeat flag
               for(int i=0; i<=maxAvgSample-1; i++){         // seed the running total to get a realisitic BPM at startup
                    rate[i] = IBI;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=(maxAvgSample-2); i++){                // shift data in the rate array
          rate[i] = rate[i+1];              // and drop the oldest IBI value 
          runningTotal += rate[i];          // add up the 9 oldest IBI values
        }
        
    rate[maxAvgSample-1] = IBI;                          // add the latest IBI to the rate array
    runningTotal += rate[maxAvgSample-1];                // add the latest IBI to runningTotal
    runningTotal /= maxAvgSample;                     // average the last 10 IBI values 
    BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
    QS = true;                              // set Quantified Self flag 
    }                       
}

  if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
     }
  
  if (N > 2500){                             // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
      firstBeat = true;                      // set these to avoid noise
      secondBeat = true;                     // when we get the heartbeat back
     }
}


void conv_data(){
    char buffer[10];
  // there is a useful c function called dtostrf() which will convert a float to a char array 
  //so it can then be printed easily.  The format is: dtostrf(floatvar, StringLengthIncDecimalPoint, numVarsAfterDecimal, charbuf);
  BtempC = dtostrf(temp, 4, 1, buffer); 
  humC = dtostrf(h, 4, 1, buffer); 
  TC = dtostrf(t, 4, 1, buffer); 
  PC = dtostrf(dst, 4, 1, buffer); 
  aC = dtostrf(ba, 4, 1, buffer); 
}

void strdev(){
  str="/Success.php?D_ID=0001";
  str += "&Beats_Mins=";    //field 1 for BPM
  str += String(BPM);
  str += "&Body_Temp="; //field 2 for Body Temperature
  str += BtempC;
  str += "&Beats_Interval=";  //field 3 for IBI
  str += String(IBI);
  str += "&Humidity=";  //field 4 for humidity
  str += humC;
  str += "&Atm_Temprature=";  //field 5 for Atmospheric Temperature
  str += TC;
  str += "&Atm_Pressure=";  //field 6 for Atmospheric Preesure
  str += PC;
  str += "&Cal_Altitude=";  //field 7 for Calculated Altitude
  str += aC;
  str += "&Guardian_Id=0001";
}

void send_sens() {
    Serial.print(str);
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
      u8x8.drawTile(12, 0, 1, server_char);
      return;
    }
  
  // This will send the request to the server
        client.print(String("GET ") + str + " HTTP/1.1\r\n" +
                     "Host: " + host + "\r\n" + 
                     "Connection: close\r\n\r\n");
                     //connection time out
        unsigned long timeout = millis();
        while (client.available() == 0) {
          if (millis() - timeout > 5000) {
            client.stop();
            return;
          }
         }
         while(client.available()){
         str = client.readStringUntil('\n');
         }
        char* switchbit = new char[strlen(str.c_str())+1];
        strcpy(switchbit, str.c_str());
         if (switchbit[0] == '0')
              cond_switch == false;
         if (switchbit[0] == '1')
              cond_switch == true; 
  Serial.print(switchbit[0]);
  delete[] switchbit;
}

