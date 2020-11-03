/*
   OpenWeatherOneCall.cpp v1.3.1 (Added data variables)
   copyright 2020 - Jessica Hershey
   www.github.com/JHershey69

   Open Weather Map - Weather Conditions
   Viva La Resistance

   Version upgrade 1.2.0 - Added CITY ID option
   Version upgrade 1.3.0 - Added exclude values for API call
*/

#include "Arduino.h"            // Basic Arduino Library
#include "OpenWeatherOneCall.h"
#include <HTTPClient.h>         // Required
#include <ArduinoJson.h>        // Version 6 Required
#include "NAComplete.h"         // Must include for City ID search North America ONLY CA/US/MX

OpenWeatherOneCall::OpenWeatherOneCall()
{

}


#define DS_URL1 "https://api.openweathermap.org/data/2.5/onecall"
char DS_URL2[100];
#define DS_URL3 "&appid="

#define GEOLOCATIONURL "https://www.googleapis.com/geolocation/v1/geolocate"



String short_names[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};


int OpenWeatherOneCall::getCoordinates(String googleKey) //<------------------ USED ONLY WITH GOOGLE KEY SUPPLIED TO FIND LOCATION otherwise ignored
{

    HTTPClient http;

    String jsonSend;    // This will be the json we send to Google with nearby MAC ADDRESSES

    // Scan for nearby WiFi networks

    int numNetworks = WiFi.scanNetworks();

    /*
     * Add 3 nearest WiFi MAC Addresses in this JSON form:
      {
      "wifiAccessPoints": [
      "macAddress":"4A:5D:36:35:BA:6D",
      "macAddress":"48:5D:36:35:BA:6C",
      "macAddress":"B0:B9:8A:EF:D2:D8"
      ]
      }
    */

    // Creating the json with some string operations

    jsonSend = "{\n";
    jsonSend += "\"wifiAccessPoints\": [\n";
    for (int j = 0; j < numNetworks; ++j)
    {
        jsonSend += "{\n";
        jsonSend += "\"macAddress\" : \"";
        jsonSend += (WiFi.BSSIDstr(j));
        jsonSend += "\",\n";
        jsonSend += "\"signalStrength\": ";
        jsonSend += WiFi.RSSI(j);
        jsonSend += "\n";
        if (j < numNetworks - 1)
        {
            jsonSend += "},\n";
        }
        else
        {
            jsonSend += "}\n";
        }
    }
    jsonSend += ("]\n");
    jsonSend += ("}\n");


    // Setting up the url for the POST

    String geoLocURL = GEOLOCATIONURL;
    geoLocURL.concat("?key=");
    geoLocURL.concat(googleKey);

    // Lets send the json in the POST and get back an answer with getString(),
    // add header for application/json

    http.begin(geoLocURL);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(jsonSend);

    if (httpCode > 399)
    {
        return httpCode;

    }
    // Setting the parsing routine

    const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + 30;
    DynamicJsonDocument geo(capacity);


    deserializeJson(geo, http.getString());     // parsing the return from Google into geo Document

    // Variables in parse

    latitude = geo["location"]["lat"];
    longitude = geo["location"]["lng"];

    // int accuracy = geo["accuracy"]; // 37

    http.end();
    return 0;
} // End of Coordinates Routine




int OpenWeatherOneCall::parseWeather(char* DKEY, char* GKEY, float SEEK_LATITUDE, float SEEK_LONGITUDE, bool SET_UNITS, int CITY_ID, int API_EXCLUDES)
{

    // Name the exclude options
    char* EXCL_NAMES[] = {"current", "daily", "hourly", "minutely", "alerts"};

    // Allocate mem requirements for parser
    unsigned int EXCL_SIZES[] = {485, 15197, 2967, 2546};

    // Maximum MEM if no excludes
    unsigned int SIZE_CAPACITY = 21309+9160;

    FLAGS exclude; //<------- Declare for bitfield struct

    // Clear the current struct for current weather
    memset(&current, 0, sizeof current);

    char getURL[200] = {0}; //<--------------- Clear API URL

    //************** BUILD EXCLUDE URL AND CAPACITY MEMORY VALUE **********

    if(API_EXCLUDES > 0)    //<-------------- Zero is NO EXCLUDES
    {
        exclude.all_excludes = API_EXCLUDES; //<--- Sets the bits to 1 for exclude
        strcpy(DS_URL2,"&exclude=");

        // Add exclude names to url
        for (int x = 0; x < (sizeof(EXCL_NAMES) / sizeof(EXCL_NAMES[0])); x++)
        {
            if (exclude.all_excludes >> x & 1)
            {
                strcat(DS_URL2,EXCL_NAMES[x]);
                strcat(DS_URL2,",");

                if (x < (sizeof(EXCL_NAMES) / sizeof(EXCL_NAMES[0])) - 1)
                {
                    SIZE_CAPACITY -= EXCL_SIZES[x]; //<-------- sets size for JSON const char*
                }
            }
        }

        // Add units declaration to url
        strcat(DS_URL2,"&units=");

    }


    //************** Metric/Imperial Units ************

    if(SET_UNITS)
    {
        strcpy(units,"metric");
    }
    else
        strcpy(units,"imperial");

    //**************************************************


    HTTPClient http; //<----------- Declare HTTP Client


    // **** The following routines set Lat and Lon coords

    if((!GKEY)&&(!CITY_ID)) //<------ For when latitude and longitude locations are known
    {
        latitude = SEEK_LATITUDE;
        longitude = SEEK_LONGITUDE;
    }
    else if (CITY_ID)       //<------- If City ID is used we search the supplied library for North America Coordinates
    {

        // Binary search the database ht[TABLE_SIZE]
        int first = 1;
        int last = TABLE_SIZE-1;
        int middle = (first + last)/2;

        while(first <= last)
        {
            if(ht[middle].city_code < CITY_ID)
            {
                first = middle+1;
            }
            else if ( ht[middle].city_code == CITY_ID)
            {
                latitude = ht[middle].lat;
                longitude = ht[middle].lon;
                break;
            }
            else
                last = middle -1;
            middle = (first+last)/2;
        }

        if(first > last)
        {
            latitude = ht[last].lat;
            longitude = ht[last].lon;

        }

    }
    else //<-------------- Get Lat and Lon Coordinates from Google GEOlocate if we haven't supplied a CITY ID or LAT AND LON
    {
        OpenWeatherOneCall::getCoordinates(GKEY);
    }

    // ****** URL for API Call is finished being created
    sprintf(getURL,"%s?lat=%.6f&lon=%.6f%s%s%s%s",DS_URL1,latitude,longitude,DS_URL2,units,DS_URL3,DKEY);

    http.begin(getURL);             //<------------ Connect to OpenWeatherMap
    int httpCode = http.GET();

    if (httpCode > 399)             //<------------ Check for connect errors
    {
        return httpCode;
    }

    // Setting MEM requirement for parser based on excludes
    const size_t capacity = SIZE_CAPACITY;

    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, http.getString()); // <<---------------- - Streams JSON


    //************ IF CURRENT NOT EXCLUDED *****************
    if(!exclude.current)
    {
        JsonObject currently = doc["current"];
        current.dayTime = currently["dt"]; // 1586781931
        current.temperature = currently["temp"]; // 287.59
        current.apparentTemperature = currently["feels_like"]; // 281.42
        current.pressure = currently["pressure"]; // 1011
        current.humidity = currently["humidity"]; // 93
        current.dewPoint = currently["dew_point"]; // 286.47
        current.uvIndex = currently["uvi"]; // 6.31
        current.cloudCover = currently["clouds"]; // 90
        current.visibility = currently["visibility"]; // 8047
        current.windSpeed = currently["wind_speed"]; // 10.3
        current.windBearing = currently["wind_deg"]; // 170
        current.windGust = currently["wind_gust"]; // 14.4
        current.icon = currently["weather"][0]["main"];
    }

    //************ IF DAILY NOT EXCLUDED *****************
    if(!exclude.daily)
    {

        JsonArray daily = doc["daily"];
        for (int x = 0; x < (sizeof(forecast) / sizeof(forecast[0])) - 1; x++)
        {
            forecast[x].dayTime = daily[x]["dt"]; // 1586793600
            forecast[x].sunriseTime = daily[x]["sunrise"]; // 1586773262
            forecast[x].sunsetTime = daily[x]["sunset"]; // 1586820773

            forecast[x].temperatureHigh = daily[x]["temp"]["day"]; // 288.74
            forecast[x].temperatureMin = daily[x]["temp"]["min"]; // 286.56
            forecast[x].temperatureMax = daily[x]["temp"]["max"]; // 293.23
            forecast[x].temperatureLow = daily[x]["temp"]["night"]; // 286.56

            forecast[x].apparentTemperatureHigh = daily[x]["feels_like"]["day"]; // 280.11
            forecast[x].apparentTemperatureLow = daily[x]["feels_like"]["night"]; // 280.29

            forecast[x].pressure = daily[x]["pressure"]; // 1006
            forecast[x].humidity = daily[x]["humidity"]; // 91
            forecast[x].dewPoint = daily[x]["dew_point"]; // 287.28
            forecast[x].windSpeed = daily[x]["wind_speed"]; // 14.2
            forecast[x].windBearing = daily[x]["wind_deg"]; // 180

            forecast[x].precipType = daily[x]["weather"][0]["main"]; // "Rain"
            forecast[x].icon = daily[x]["weather"][0]["icon"]; // "10d"

            forecast[x].cloudCover = daily[x]["clouds"]; // 95
            forecast[x].precipIntensity = daily[x]["rain"]; // 23.31
            forecast[x].uvIndex = daily[x]["uvi"]; // 6.31
        }
    }
    //****************** IF ALERTS NOT EXCLUDED **********************

    if(!exclude.alerts)
    {

        JsonObject alerts= doc["alerts"][0];
        alert[0].sender_name = alerts["sender_name"];
        alert[0].event = alerts["event"];
        alert[0].start = alerts["start"];
        alert[0].alert_end = alerts["end"];
        alert[0].description = alerts["description"];

        JsonObject alerts_1= doc["alerts"][1];
        alert[1].sender_name = alerts_1["sender_name"];
        alert[1].event = alerts_1["event"];
        alert[1].start = alerts_1["start"];
        alert[1].alert_end = alerts_1["end"];
        alert[1].description = alerts_1["description"];
    }

    //******************* IF HOURLY NOT EXCLUDED ***********
    if(!exclude.hourly)
    {

        JsonArray hourly = doc["hourly"];
        for(int h = 0; h < 48; h++)
        {

            JsonObject hourly_0 = hourly[h];
            hour[h].dt = hourly_0["dt"]; // 1604336400
            hour[h].temp = hourly_0["temp"]; // 46.58
            hour[h].feels_like = hourly_0["feels_like"]; // 28.54
            hour[h].pressure = hourly_0["pressure"]; // 1015
            hour[h].humidity = hourly_0["humidity"]; // 31
            hour[h].dew_point = hourly_0["dew_point"]; // 19.2
            hour[h].clouds = hourly_0["clouds"]; // 20
            hour[h].visibility = hourly_0["visibility"]; // 10000
            hour[h].wind_speed = hourly_0["wind_speed"]; // 22.77
            hour[h].wind_deg = hourly_0["wind_deg"]; // 300

            JsonObject hourly_0_weather_0 = hourly_0["weather"][0];
            hour[h].id = hourly_0_weather_0["id"]; // 801
            hour[h].main = hourly_0_weather_0["main"]; // "Clouds"
            hour[h].description = hourly_0_weather_0["description"]; // "few clouds"
            hour[h].icon = hourly_0_weather_0["icon"]; // "02d"

            hour[h].pop = hourly_0["pop"]; // 0
        }
        //******************************************************

        //**********************IF MINUTELY IS NOT EXCLUDED ***********

        if(!exclude.minutely)
        {

            JsonArray minutely = doc["minutely"];

            for(int x = 0; x<61; x++)
            {
                minute[x].dt = minutely[x]["dt"]; // 1604341320
                minute[x].precipitation = minutely[x]["precipitation"]; // 0
            }
        }
    }
    http.end();
    return 0;
}
