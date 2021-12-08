#include <NTPClient.h>
#include <WiFiUdp.h>
#include <string>
#include "datum.h"
#define utcOffsetInSeconds 3600
#define dstOffsetInSeconds 3600

Datum::Datum() {
};
    
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void Datum::update() {
    timeClient.update();
    }

String Datum::getFormattedTime() {
    return timeClient.getFormattedTime();
}
int Datum::getHours() {
    return timeClient.getHours();
}
int Datum::getMinutes() {
    return timeClient.getMinutes();
}

int Datum::getDay() {
    return timeClient.getDay();
}

int Datum::getHour(String s, char delimiter) {
    int i = s.length();
    while (i >= 0 && s[i] != delimiter) {
        s[i] = ' ';
        i--;
    }
    if (i >= 0)
        s[i] = ' '; // delimiter
    return s.toInt();
}

int Datum::getMinute(String s, char delimiter) {
    int i = 0;
    while (i <= s.length() && s[i] != delimiter) {
        s[i] = ' ';
        i++;
    }
    if (i <= s.length())
        s[i] = ' '; // delimiter
    return s.toInt();
}

/**
 * getDate
 * -------
 * calculate date and time from epoche time
 */

Datum::t_date Datum::getDate(double dSeconds) {
    // double dSeconds = timeClient.getEpochTime(); //time(NULL);
    long int liElapsedHours, liElapsedDays, liElapsedYears, liElapsedLeapYears, liRemainingDays, liCurrentYear,
        liCurrentMonth, liCurrentDay, liCurrentHours, liCurrentMinutes, liCurrentSeconds;

    char bCurrentYearIsLeapYear;
    int iMonthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    liElapsedHours = (long int)trunc(dSeconds / 3600);
    liElapsedDays = (long int)trunc(dSeconds / (3600 * 24));
    liElapsedYears = (long int)trunc(dSeconds / (3600 * 24 * 365));
    liElapsedLeapYears = liElapsedYears / 4 + liElapsedYears / 400 - liElapsedYears / 100;
    liRemainingDays = liElapsedDays - liElapsedYears * 365 - liElapsedLeapYears;

    liCurrentHours = ((long int)dSeconds - liElapsedDays * 3600 * 24) / 3600;
    liCurrentMinutes = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 / 60;
    liCurrentSeconds = ((long int)dSeconds - liElapsedDays * 3600 * 24) % 3600 % 60;

    liCurrentYear = 1970 + liElapsedYears;
    bCurrentYearIsLeapYear = (liCurrentYear % 4 == 0 && (liCurrentYear % 100 != 0 || liCurrentYear % 400 == 0));
    if (bCurrentYearIsLeapYear)
        iMonthDays[1]++;

    int i = 0;
    int j = 0;
    while (i < 12 && j < liRemainingDays) {
        j += iMonthDays[i];
        i++;
    }
    liCurrentMonth = i;

    j = 0;
    liCurrentDay = liRemainingDays;
    while (j < i - 1) {
        liCurrentDay = liCurrentDay - iMonthDays[j];
        j++;
    }

    Datum::t_date st; // = (t_date *)malloc( sizeof(t_date ));
    st.year = liCurrentYear;
    st.month = liCurrentMonth;
    st.day = liCurrentDay;
    st.hour = liCurrentHours;
    st.minute = liCurrentMinutes;
    st.second = liCurrentSeconds;
    return (st);
}

/**
 * initTimeServer
 * --------------
 * String getFormattedTime() const;
 * int getDay() const;
 * int getHours() const;
 * int getMinutes() const;
 * int getSeconds() const;
 */
Datum::t_date Datum::initTimeServer(t_date lastDate) {
    Serial.println("initTimeServer");
    timeClient.begin();
    delay(100);
    timeClient.update();
    lastDate = getDate(timeClient.getEpochTime());
    lastDate.day = 0;
    Serial.println("Time initialized");
    return lastDate;
}

/**
 * updateTimeServer
 * --------------
 */
Datum::t_date Datum::updateTimeServer(t_date lastDate) {
    timeClient.update();
    Datum::t_date d = getDate(timeClient.getEpochTime());
    if (d.day != lastDate.day) {
        lastDate = d;
        //  int[] DST_start = {28, 27, 26, 31};
        //  int[] DST_end   = {31, 30, 29, 27};
        if (d.month > 3 && d.month < 11)
            timeClient.setTimeOffset(utcOffsetInSeconds + dstOffsetInSeconds);
        else
            timeClient.setTimeOffset(utcOffsetInSeconds);
    }
    return lastDate;
}