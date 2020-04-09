# Greenhouse

Apr 8 2020... This program does not run at my location because the WiFi drops. Otherwise it appears to work fine.

My project is to control a small greenhouse. 10ft x 10ft x 7.5 tall arch. 9 tempreature sensors. 5 sensors reading soil temp(1 through 5). Sensor 6 is Purge Water temp. Sensor 7 is GreenHouse temp. Sensor 8 is Outside Air temp. Sensor 9 is EXTRA. Watering occurs 2x/day. Based on NTP time.

THESE files are for the ADAFRUIT Metro M4 WiFi. This should work on other Arduino boards but may not compile without considerable changes to WiFi and NTP functions. I did not write any of the libraries. I just assembled the libraries and functions to suit my needs. I also used example code found on the net.

GreenHouseTesting.ino and FunctionTesting.ino are the Arduino files (used on Metro M4 Airlift instead).

UDP_Server.c is an NTP testing server that can run on an RaspberryPi. Time returned is static, but the UDP packet size is correct. This is being used for testing instead of banging away at the NTP servers on the net. Don't care if the time is correct. Only need a reply packet. I didn't write this but I modified it too serve my purposes.
Source https://www.geeksforgeeks.org/udp-server-client-implementation-c/

FUTURE work is to add aother timer to keep track of current time to reduce the frequency of NTP calls to get time. RTC.

FUTURE add a Web interface for displaying current envronment data (temp, humid, waterON, etc) instead of using serial monitor.
