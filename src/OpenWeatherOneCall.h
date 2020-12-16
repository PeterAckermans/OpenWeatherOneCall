/*
   OpenWeatherOnecall.h
   Upgrade v3.0.0
   copyright 2020 - Jessica Hershey
   www.github.com/jHershey69

   WEATHER: Current, hourly, minutely, 8 day future, 5 day history
   REQUIRES: OpenWeatherMap.com API key
   INQUIRE BY: Latitude/Longitude, IP Address, CITY ID

   Please see manual for full instructions
*/

#ifndef _OPEN_WEATHER_ONECALL_H_FILE
#define _OPEN_WEATHER_ONECALL_H_FILE

#include <HTTPClient.h>         // Required but installed in the ESP32
#include <ArduinoJson.h>        // Version 6 Required

// For Historical Data - Need UNIX EPOCH TIMESTAMP
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Arduino.h>  // Basic Arduino Library
#include <string.h>
#include "errMsgs.h"

// Excludes
#define EXCL_C 1  //Exclude Current
#define EXCL_D 2  //Exclude Daily
#define EXCL_H 4  //Exclude Hourly
#define EXCL_M 8  //Exclude Minutely
#define EXCL_A 16 //Exclude Alerts

#define METRIC 1
#define IMPERIAL 2
#define KELVIN 3

//struct initializer
#define NEW_API {"",0.0f,0.0f,true,0,0,0}

class OpenWeatherOneCall
{
public:
    //Constructor
    OpenWeatherOneCall();

    //Methods
    int parseWeather(void);

    void initAPI(void);
    int setOpenWeatherKey(char* owKey);

    int setLatLon(float _LAT, float _LON);
    int setLatLon(int _CITY_ID);
    int setLatLon(void);

    int setExcl(int _EXCL);
    int setUnits(int _UNIT);
    int setHistory(int _HIS);
    char* getErrorMsgs(int errorMsg);

    //Legacy Method
    int parseWeather(char* DKEY, char* GKEY, float SEEK_LATITUDE, float SEEK_LONGITUDE, bool SET_UNITS, int CITY_ID, int API_EXCLUDES, int GET_HISTORY);

    //Destructor
    ~OpenWeatherOneCall();

    //Variables

    struct
    {
        char CITY[60] = {NULL};
        char STATE[10] = {NULL};
        char COUNTRY[10] = {NULL};
        char timezone[50];
        int timezoneOffset;
        float LATITUDE;
        float LONGITUDE;
    } location;

    struct nowData
    {
        long dayTime; // 1582151288
        float temperature; // 46.38
        float apparentTemperature; // 41.49
        float pressure; // 1026.4
        float humidity; // 0.31
        float dewPoint; // 17.18
        float uvIndex; // 0
        float cloudCover; // 0.02
        float visibility; // 10
        float windSpeed; // 10.22
        float windBearing; // 348
        float windGust;
        float id; //800
        char* main; //"Clear"
        char* summary; // "Clear Skies" - uses "description"
        char* icon; // "02d"
    } *current;


    struct futureData
    {
        char weekDayName[4];
        long dayTime; // 1582088400
        long sunriseTime; // 1582112760
        long sunsetTime; // 1582151880

        float temperatureDay; // 51.24
        float temperatureLow; // 30.17
        float temperatureHigh; // 51.24
        float temperatureNight; // 26.83
        float temperatureEve;
        float temperatureMorn;

        float apparentTemperatureHigh; // 50.76
        float apparentTemperatureLow; // 19.9
        float apparentTemperatureEve; // 50.76
        float apparentTemperatureMorn; // 19.9


        float pressure; // 1024.1
        float humidity; // 0.54
        float dewPoint; // 26.79
        float windSpeed; // 6.49
        float windBearing; // 324

        float id; //800
        char* main; // "rain" this is main
        char* summary; //description in json
        char* icon; //"02d"

        float cloudCover; // 0.53
        float pop;
        float uvIndex; // 3

    } *forecast = NULL; //[8]


    struct HOURLY
    {
        long dayTime; // 1604336400
        float temperature; // 46.58
        float apparentTemperature; // 28.54
        float pressure; // 1015
        float humidity; // 31
        float dewPoint; // 19.2
        float cloudCover; // 20
        float visibility; // 10000
        float windSpeed; // 22.77
        float windBearing; // 300

        float id; // 801
        char* main; // "Clouds"
        char* summary; // "few clouds"
        char* icon; // "02d"
        float pop; // 0
    } *hour = NULL;

    struct MINUTELY
    {
        long dayTime; // 1604341320
        float precipitation; // 0
    } *minute = NULL; //[61]


    struct ALERTS
    {
        char* senderName; //[30] = "No Alert"; // "NWS Philadelphia - Mount Holly (New Jersey, Delaware, Southeastern Pennsylvania)"
        char* event; //[50] = "No Event"; // "Gale Watch"
        long alertStart; // 1604271600
        long alertEnd;
        char *summary;
    } *alert = NULL;


    struct HISTORICAL
    {
        char weekDayName[4];
        long dayTime; // 1604242490
        long sunrise; // 1604230151
        long sunset; // 1604267932
        float temperature; // 285.9
        float apparentTemperature; // 283.42
        float pressure; // 1016
        float humidity; // 76
        float dewPoint; // 281.78
        float uvIndex; // 3.1
        float cloudCover; // 90
        float visibility; // 16093
        float windSpeed; // 3.1
        float windBearing; // 160
        float windGust;
        float id; // 804
        char* main; // "Clouds"
        char* summary; // "overcast clouds"
        char* icon; // "04d"

    } *history = NULL; //[25]

    const char* short_names[7] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
    char buffer[40];

private:

    //Methods

    int parseCityCoordinates(char* CTY_URL);
    int getIPLocation();
    int getIPAPILocation(char* URL);
    int createHistory(void);
    int createCurrent(int);
    int setExcludes(int EXCL);
    void setWeekdayName(long dayTime, int x);
    int getLocationInfo();

    void freeCurrentMem(void);
    void freeForecastMem(void);
    void freeAlertMem(void);
    void freeHourMem(void);
    void freeMinuteMem(void);




    //Variables


    // For eventual struct calls
    struct apiInfo
    {
        char OPEN_WEATHER_DKEY[100] = {NULL};
        float OPEN_WEATHER_LATITUDE = NULL;
        float OPEN_WEATHER_LONGITUDE = NULL;
        int OPEN_WEATHER_UNITS = 2;
        int OPEN_WEATHER_EXCLUDES = NULL;
        int OPEN_WEATHER_HISTORY = NULL;
    } USER_PARAM;


    char units[10] = "IMPERIAL";
    char _ipapiURL[38];
    int sumlen;


    int summary_len = 0;

    //BITFIELDS for exclude flags
    typedef union FLAGS
    {
        struct
        {
            uint8_t current : 1;
            uint8_t daily : 1;
            uint8_t hourly : 1;
            uint8_t minutely : 1;
            uint8_t alerts : 1;
            uint8_t Spare2 : 1;
            uint8_t Spare1 : 1;
        };
        uint8_t all_excludes;
    };

    FLAGS exclude; //<------- Declare for bitfield struct

};

#endif
