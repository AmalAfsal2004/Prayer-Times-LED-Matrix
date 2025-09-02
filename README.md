# Islamic Prayer Times Display
Muslims have to peform five prayers each day, with the times of these prayers being determined by the position of the sun (or times of day). For example, there are prayers just at sunrise, noon, and sunset. However, because sunrise and sunset times are constantly changing due to the tilt of the Earth, it maybe difficult to keep track of the times of prayer. Luckily, mathmatical equations and astronomical data (not discovered by me) are being used to calculate the prayer times, and there are websites and applications that help Muslims determine when it is time to pray. I decided to take a different approach to these conventional methods, and use an ESP32 and a 64x64 RGB LED Matrix to display these times

## Physical Components
- ESP-WROOM-32 ESP32 ESP-32S Development Board 2.4GHz Dual-Mode WiFi + Bluetooth (Brand: AITRIP)
- 64x64 HUB 75 RGB MATRIX
- Two 5v Power Supplies to power Display and ESP32 (Will most likely switch to one 5V Power Supply with more amps in the future)
- Power Supply Breadboard (Connect one of the power supplies to this, which then powers the ESP32)
- Breadboard (For the Power Supply Breadboard to connect to)
- Jumper Cables, female to and female to male


<div>
<img src = "https://github.com/AmalAfsal2004/Prayer-Times-LED-Matrix/blob/main/images/prayer_clk1.jpeg?raw=true" align ="left"/>

<img src = "https://github.com/AmalAfsal2004/Prayer-Times-LED-Matrix/blob/main/images/prayer_clock_gif.gif?raw=true" align = "right"/>
</div>




## Operations
Please install the essential libraries in the PrayerTime.ino file. I will cover why I included the non-essential libraries later. For this project to work, you will need access to a WiFi network, as the program utilizes an NTP Server, and the Islamic 
Finder API.

### Clock
Instead of using an RTC module, this project utilzes an NTP Server to display the time. At the top of PrayerTime.ino, you will see information about the clock such as server link, gmtOffset in seconds, and daylight offset in seconds (daylight savings). You must change this depending on your time zone. For example, Arizona has an offset of -7 hours, so the GMT offset in seconds is
-25200. 

By default, the time is in 24 hour format, so the function, get_n_display_clk(), in functions.ino converts it back to 12 hour time and displays it on the RGB Matrix.

### Prayer Times
This project utilizes the Islamic Finder API which I will leave a link for here:
https://www.islamicfinder.us/index.php/api .
There are various paramaters that can be used for a get request, but I simply used country and zipcode as shown in PrayerTime.ino .
The function get_n_display_time() in functions.ino gets a JSON response from the API, and displays the prayer times to the RGB matrix.

### (Optional) ElegantOTA and WebSerial
The non-essential libraries that I have used allow me to upload updates to the ESP32 over the air, and also looking at the serial monitor over the air. If you noticed a lack of Serial.print() commands, it is because they have been replaced by my Serial_n_Web() commands. These commands print to the Serial Monitor, and also the Web Serial Monitor.
