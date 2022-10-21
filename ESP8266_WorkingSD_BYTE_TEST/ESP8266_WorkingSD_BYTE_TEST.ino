#include <ESP8266WiFi.h>
#include <SD.h>
#include <SPI.h>
#include "RTClib.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Arduino.h>

void SDSave();
void SDRead();

const long utcOffsetInSeconds = 10800;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

char buffS[75];

double avgSecV = 0;
double avgSecA = 0;
double avgSecSF = 0;
double avgSecCF = 0;
int countTicks = 0;

double avgMinV = 0;
double avgMinA = 0;
double avgMinSF = 0;
double avgMinCF = 0;
int minFileCheck = 0;
bool firstFileCheckMin = true;
int countSecs = 0;

double avgHourV = 0;
double avgHourA = 0;
double avgHourSF = 0;
double avgHourCF = 0;
int hourFileCheck = 0;
bool firstFileCheckHour = true;
int countMins = 0;

double avgDayV = 0;
double avgDayA = 0;
double avgDaySF = 0;
double avgDayCF = 0;
int dayFileCheck = 0;
bool firstFileCheckDay = true;
int countHours = 0;

double avgMonthV = 0;
double avgMonthA = 0;
double avgMonthSF = 0;
double avgMonthCF = 0;
int monthFileCheck = 0;
bool firstFileCheckMonth = true;
int countDays = 0;

unsigned long previousTime = 0;
unsigned long previousTimeS = 0;
unsigned long previousTimeM = 0;
unsigned long previousTimeH = 0;
unsigned long previousTimeD = 0;
unsigned long previousTimeMM = 0;

const unsigned long secCheck = 1000;
const unsigned long minCheck = 60000;
const unsigned long hourCheck = 120000;
const unsigned long dayCheck = 180000;
const unsigned long monthCheck = 240000;

RTC_Millis rtc;

String  ClientRequest;
IPAddress staticIP773_10(192, 168, 43, 10);
IPAddress gateway773_10(192, 168, 43, 1);
IPAddress subnet773_10(255, 255, 255, 0);

WiFiServer server(80);

void setup()
{
  pinMode(16, OUTPUT);
  digitalWrite(16, HIGH);
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  Serial.begin(250000);
  WiFi.disconnect();
  delay(3000);
  Serial.println("START");
  WiFi.begin("RouterName", "1234567890");
  
  while ((!(WiFi.status() == WL_CONNECTED)))
  {
    delay(300);
    Serial.print("..");
  }
  Serial.println("Connected");
  WiFi.config(staticIP773_10, gateway773_10, subnet773_10);
  Serial.println("Your IP is");
  Serial.println((WiFi.localIP().toString()));
  server.begin();

  timeClient.begin();
  timeClient.update();

  rtc.begin(DateTime(timeClient.getEpochTime()));
  delay(300);
  if (!SD.begin(4))
  {
    delayMicroseconds(10);
    Serial.println("Failed, check if the card is present.");
  }
  else {
    delayMicroseconds(10);
    Serial.println("Card initialized.");
  }
  /*
  while (!Serial)
  {
  }
  */
}
void loop()
{
  if (SD.begin(4))
  //if (true)
  {
    DateTime now = rtc.now();

    int timeYear = now.year();
    int timeMonth = now.month();
    int timeDay = now.day();
    int timeHour = now.hour();
    int timeMin = now.minute();
    int timeSec = now.second();

    String timeSecS;
    if (timeSec < 10) {
      timeSecS = '0' + (String)timeSec;
    }
    else {
      timeSecS = (String)timeSec;
    }

    String timeMinS;
    if (timeMin < 10) {
      timeMinS = '0' + (String)timeMin;
    }
    else {
      timeMinS = (String)timeMin;
    }

    String timeHS;
    if (timeHour < 10) {
      timeHS = '0' + (String)timeHour;
    }
    else {
      timeHS = (String)timeHour;
    }

    String sData = "";
    String volts = "";
    String amps = "";
    String cosf = "";
    String sinf = "";

    int count = 0;
    char ch = Serial.read();
    delayMicroseconds(1);

    if (ch == '|')  //  |/230.33:120.33;0.99<0.99=
    {
      while (count < 25)
      {
        count++;
        char sD = Serial.read();
        int check = sD;
        if (check >= 46 && check <= 62)
          sData.concat(sD);
        delayMicroseconds(100);
      }

      int startIdxV = sData.indexOf('/') + 1;
      int endIdxV = sData.indexOf(':');
      
      int startIdxA = endIdxV + 1;
      int endIdxA = sData.indexOf(';');
      
      int startIdxCF = endIdxA + 1;
      int endIdxCF = sData.indexOf('<');
      
      int startIdxSF = endIdxCF + 1;
      int endIdxSF = sData.indexOf('=');

      if (startIdxV != -1 && endIdxV != -1
          && startIdxA != -1 && endIdxA != -1
          && startIdxCF != -1 && endIdxCF != -1
          && startIdxSF != -1 && endIdxSF != -1)
      {
        volts = sData.substring(startIdxV, endIdxV);
        amps = sData.substring(startIdxA, endIdxA);
        cosf = sData.substring(startIdxCF, endIdxCF);
        sinf = sData.substring(startIdxSF, endIdxSF);
      }
      double voltsDouble = volts.toDouble();
      double ampsDouble = amps.toDouble();
      double cosfDouble = cosf.toDouble();
      double sinfDouble = sinf.toDouble();

      avgSecV += voltsDouble;
      avgSecA += ampsDouble;
      avgSecCF += cosfDouble;
      avgSecSF += sinfDouble;
      countTicks++;

      delay(80);

      unsigned long currentTimeS = millis();
      if (currentTimeS - previousTimeS >= secCheck)
      {
        avgSecV /= countTicks;
        avgSecA /= countTicks;
        avgSecCF /= countTicks;
        avgSecSF /= countTicks;
        
        avgMinV += avgSecV;
        avgMinA += avgSecA;
        avgMinCF += avgSecCF;
        avgMinSF += avgSecSF;

        String avgSecVS = String(avgSecV , 2);
        String avgSecAS = String(avgSecA , 2);
        String avgSecCFS = String(avgSecCF , 2);
        String avgSecSFS = String(avgSecSF , 2);
        
        sprintf(buffS, "%d/%d/%d|%s:%s:%s|%s|%s|%s|%s", timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgSecVS , avgSecAS, avgSecCFS , avgSecSFS);

        //Serial.println(buffS);

        avgSecV = 0;
        avgSecA = 0;
        avgSecCF = 0;
        avgSecSF = 0;
        countTicks = 0;

        previousTimeS = currentTimeS;

        delay(1);

        countSecs++;
      }

      unsigned long currentTimeM = millis();
      if (currentTimeM - previousTimeM >= minCheck)
      {

        avgMinV /= countSecs;
        avgMinA /= countSecs;
        avgMinCF /= countSecs;
        avgMinSF /= countSecs;
        
        if (minFileCheck <= 35)
        {
          minFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgMinV , avgMinA , avgMinCF , avgMinSF , "HourGrapth1_60Mins.txt");
          firstFileCheckMin = true;

          if (minFileCheck > 35)
          {
            SD.remove("HourGrapth2_60Mins.txt");
            delay(5);
            Serial.println("deleted HourGrapth2_60Mins.txt");
          }
        }

        if (minFileCheck > 35)
        {
          firstFileCheckMin = false;
          minFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgMinV , avgMinA , avgMinCF , avgMinSF , "HourGrapth2_60Mins.txt");
          if (minFileCheck > 70)
          {
            minFileCheck = 0;
            
            SD.remove("HourGrapth1_60Mins.txt");
            delay(5);
            Serial.println("deleted HourGrapth1_60Mins.txt");
          }
        }

        avgHourV += avgMinV;
        avgHourA += avgMinA;
        avgHourCF += avgMinCF;
        avgHourSF += avgMinSF;
        countMins++;

        avgMinV = 0;
        avgMinA = 0;
        avgMinCF = 0;
        avgMinSF = 0;
        countSecs = 0;

        previousTimeM = currentTimeM;
      }

      unsigned long currentTimeH = millis();
      if (currentTimeH - previousTimeH >= hourCheck)
      {

        avgHourV /= countMins;
        avgHourA /= countMins;
        avgHourCF /= countMins;
        avgHourSF /= countMins;

        if (hourFileCheck <= 24)
        {
          hourFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgHourV , avgHourA , avgHourCF , avgHourSF , "DayGraph1_24H.txt");
          firstFileCheckHour = true;

          if (hourFileCheck > 24)
          {
            SD.remove("DayGraph2_24H.txt");
            delay(5);
            Serial.println("deleted DayGraph2_24H.txt");
          }
        }

        if (hourFileCheck > 24)
        {
          firstFileCheckHour = false;
          hourFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgHourV , avgHourA , avgHourCF , avgHourSF , "DayGraph2_24H.txt");
          if (hourFileCheck > 48)
          {
            hourFileCheck = 0;
            SD.remove("DayGraph1_24H.txt");
            delay(5);
            Serial.println("deleted DayGraph1_24H.txt");
          }
        }

        avgDayV += avgHourV;
        avgDayA += avgHourA;
        avgDayCF += avgHourCF;
        avgDaySF += avgHourSF;
        countHours++;

        avgHourV = 0;
        avgHourA = 0;
        avgHourCF = 0;
        avgHourSF = 0;
        countMins = 0;

        previousTimeH = currentTimeH;
      }

      unsigned long currentTimeD = millis();
      if (currentTimeD - previousTimeD >= dayCheck)
      {

        avgDayV /= countHours;
        avgDayA /= countHours;
        avgDayCF /= countHours;
        avgDaySF /= countHours;

        if (dayFileCheck <= 31)
        {
          dayFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgDayV , avgDayA , avgDayCF , avgDaySF , "MonthGraph1_31D.txt");
          firstFileCheckDay = true;

          if (dayFileCheck > 31)
          {
            SD.remove("MonthGraph2_31D.txt");
            delay(5);
            Serial.println("deleted MonthGraph2_31D.txt");
          }
        }

        if (dayFileCheck > 31)
        {
          firstFileCheckDay = false;
          dayFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgDayV , avgDayA , avgDayCF , avgDaySF , "MonthGraph2_31D.txt");
          if (dayFileCheck > 62)
          {
            dayFileCheck = 0;
            SD.remove("MonthGraph1_31D.txt");
            delay(5);
            Serial.println("deleted MonthGraph1_31D.txt");
          }
        }

        avgMonthV += avgDayV;
        avgMonthA += avgDayA;
        avgMonthCF += avgDayCF;
        avgMonthSF += avgDaySF;
        countDays++;

        avgDayV = 0;
        avgDayA = 0;
        avgDayCF = 0;
        avgDaySF = 0;
        countHours = 0;

        previousTimeD = currentTimeD;
      }

      unsigned long currentTimeMM = millis();
      if (currentTimeMM - previousTimeMM >= monthCheck)
      {

        avgMonthV /= countDays;
        avgMonthA /= countDays;
        avgMonthCF /= countDays;
        avgMonthSF /= countDays;

        if (monthFileCheck <= 12)
        {
          monthFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgMonthV , avgMonthA , avgMonthCF , avgMonthSF , "YearGraph1_12M.txt");
          firstFileCheckMonth = true;
          
          if (monthFileCheck > 12)
          {
            SD.remove("YearGraph2_12M.txt");
            delay(5);
            Serial.println("deleted YearGraph2_12M.txt");
          }
        }

        if (monthFileCheck > 12)
        {
          firstFileCheckMonth = false;
          monthFileCheck++;
          SDSave(timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgMonthV , avgMonthA , avgMonthCF , avgMonthSF , "YearGraph2_12M.txt");
          if (monthFileCheck > 24)
          {
            monthFileCheck = 0;
            SD.remove("YearGraph1_12M.txt");
            delay(5);
            Serial.println("deleted YearGraph1_12M.txt");
          }
        }

        avgMonthV = 0;
        avgMonthA = 0;
        avgMonthCF = 0;
        avgMonthSF = 0;
        countDays = 0;

        previousTimeMM = currentTimeMM;
      }
    }
  }
  else if (!Serial.available()) {}

  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while (!client.available()) {
    delay(1);
  }

  unsigned long currentTime = millis();
  ClientRequest = (client.readStringUntil('\r'));
  ClientRequest.remove(0, 5);
  ClientRequest.remove(ClientRequest.length() - 9, 9);
  
  if (ClientRequest == "live") {
    if (currentTime - previousTime >= secCheck)
    {

      previousTime = currentTime;

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("");
      client.println("<!DOCTYPE HTML>");
      client.println("<html>");
      client.println(buffS);
      client.println("</html>");
      //Serial.println(buffS);
      delay(1);
    }
  }

  else if (ClientRequest == "hour")
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    if (firstFileCheckMin)
    {
      client.print(SDRead("HourGrapth2_60Mins.txt"));
      client.print(SDRead("HourGrapth1_60Mins.txt"));
    }
    else if (!firstFileCheckMin)
    {
      client.print(SDRead("HourGrapth1_60Mins.txt"));
      client.print(SDRead("HourGrapth2_60Mins.txt"));
    }
    client.println("</html>");
    delay(1);
  }

  else if (ClientRequest == "day")
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    if (firstFileCheckMin)
    {
      client.print(SDRead("DayGraph2_24H.txt"));
      client.print(SDRead("DayGraph1_24H.txt"));
    }
    else if (!firstFileCheckMin)
    {
      client.print(SDRead("DayGraph1_24H.txt"));
      client.print(SDRead("DayGraph2_24H.txt"));
    }
    client.println("</html>");
    delay(1);
  }

  else if (ClientRequest == "month")
  {

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    if (firstFileCheckMonth)
    {
      client.print(SDRead("MonthGraph2_31D.txt"));
      client.print(SDRead("MonthGraph1_31D.txt"));
    }
    else if (!firstFileCheckMonth)
    {
      client.print(SDRead("MonthGraph1_31D.txt"));
      client.print(SDRead("MonthGraph2_31D.txt"));
    }
    client.println("</html>");
    delay(1);
  }

else if (ClientRequest == "year")
  {

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    if (firstFileCheckMonth)
    {
      client.print(SDRead("YearGraph2_12M.txt"));
      client.print(SDRead("YearGraph1_12M.txt"));
    }
    else if (!firstFileCheckMonth)
    {
      client.print(SDRead("YearGraph1_12M.txt"));
      client.print(SDRead("YearGraph2_12M.txt"));
    }
    client.println("</html>");
    delay(1);
  }
  
  if (ClientRequest == "delete")
  {
    SD.remove("HourGrapth1_60Mins.txt");
    SD.remove("HourGrapth2_60Mins.txt");
    SD.remove("DayGraph1_24H.txt");
    SD.remove("DayGraph2_24H.txt");
    SD.remove("MonthGraph1_31D.txt");
    SD.remove("MonthGraph2_31D.txt");
    SD.remove("YearGraph1_12M.txt");
    SD.remove("YearGraph2_12M.txt");
    Serial.println("SD Data Deleted!");
    delay(10);
  }

  if (ClientRequest == "lamp1ON")
  {
    digitalWrite(16, LOW);
    Serial.println("Lamp1 ON");
    delay(1);
  }
  else if (ClientRequest == "lamp1OFF")
  {
    digitalWrite(16, HIGH);
    Serial.println("Lamp1 OFF");
    delay(1);
  }

  if (ClientRequest == "lamp22ON")
  {
    digitalWrite(5, LOW);
    Serial.println("Lamp2 ON");
    delay(1);
  }
  else if (ClientRequest == "lamp22OFF")
  {
    digitalWrite(5, HIGH);
    Serial.println("Lamp2 OFF");
    delay(1);
  }

  client.flush();

}
