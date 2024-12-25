# LilyGo T5 4.7 ePaper display for Todoist.com

## Features
<li>A charming desktop display designed to keep you on top of your daily activities effortlessly!
<li>This ESP32-powered device syncs with ToDoist.com every 15 minutes to retrieve your pending tasks.
<li>Power it via USB-C or a LiPo battery for flexibility.
<li>Leveraging ePaper and ESP32's deep sleep mode, a 1000mAh battery can last up to a month.
<li>Perfect for your office desk, kitchen, or living room—this stand-alone device fits anywhere.
<li>A computer is required only during the initial setup. Simply edit the settings file with your timezone, Wi-Fi details, and API key to get started.

## Install Instructions:

1.) Buy yoursellf LilyGo T5 4.7 EPD47 ePaper display.<br>
2.) Install Arduino and related libraries. See  Xinyuan-LilyGO /LilyGo-EPD47.<br>
3.) Clone this repo (ESP32-EPD47-ToDoist-Display)<br>
4.) Edit owm_credentials.h <br>
&nbsp;&nbsp;&nbsp;&nbsp;a.) with your Wifi and password<br>
&nbsp;&nbsp;&nbsp;&nbsp;b.) API integration key from ToDoist.com<br>
&nbsp;&nbsp;&nbsp;&nbsp;c.) Preffered Project and Section to show on the left side.<br>
5.) Upload to device and enJoy!!<br>

Some code has been reused from Xinyuan-LilyGO/ LilyGo-EPD-4-7-OWM-Weather-Display

## Support ToDoist.com to help keep these APIs running!

(https://developer.todoist.com/rest/v2/#overview)

![ToDoist-Display](images/epd47Front.jpeg)

![ToDoist-Display](images/todoist.png)

![ToDoist.com Integrations](images/integ.png)

![ToDoist.com API Token](images/token.png)
