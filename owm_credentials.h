const bool DebugDisplayUpdate = false;

// Change to your WiFi credentials
const char* ssid     = "your_SSID";
const char* password = "your_WIFI_password";

// Use your own API key by signing up for a free account at https://ToDoist.com/ 
String apikey       = "Bearer b269282c44f23c58d5076eaba1290c3aa4e35c14";            // Under Setting > Ingegrations get API key,   prefix with "Bearer "       

String projectsUrl = "https://api.todoist.com/rest/v2/projects";// See:https://developer.todoist.com/rest/v2#overview
String sectionsUrl = "https://api.todoist.com/rest/v2/sections?project_id=";
String tasksUrl = "https://api.todoist.com/rest/v2/tasks";

//left side panel shows the tasks under one Project  and one Section. all others  tasks show on the right side.
//Specify the left panel Project
String project = "Inbox";
RTC_DATA_ATTR uint32_t project_id=1;
//Specify the left panel Section
String section = "Work";
RTC_DATA_ATTR  uint32_t section_id=1;

//Day of the week
const char* weekday_D[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

//Month
const char* month_M[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


String Units            = "I";                             // Use 'M' for Metric or I for Imperial 
const char* Timezone    = "EST5EDT,M3.2.0,M11.1.0";  // Choose your time zone from: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv 
                                                           // See below for examples
const char* ntpServer   = "0.north-america.pool.ntp.org";             // Or, choose a time server close to you, but in most cases it's best to use pool.ntp.org to find an NTP server
                                                           // then the NTP system decides e.g. 0.pool.ntp.org, 1.pool.ntp.org as the NTP syem tries to find  the closest available servers
                                                           // EU "0.europe.pool.ntp.org"
                                                           // US "0.north-america.pool.ntp.org"
                                                           // See: https://www.ntppool.org/en/                                                           
int   gmtOffset_sec     = -18000;    // UK normal time is GMT, so GMT Offset is 0, for US (-5Hrs) is typically -18000, AU is typically (+8hrs) 28800
int  daylightOffset_sec = 3600; // In the UK DST is +1hr or 3600-secs, other countries may use 2hrs 7200 or 30-mins 1800 or 5.5hrs 19800 Ahead of GMT use + offset behind - offset

// Example time zones
//const char* Timezone = "MET-1METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";       // Central Europe
//const char* Timezone = "EST-2METDST,M3.5.0/01,M10.5.0/02"; // Most of Europe
//const char* Timezone = "EST5EDT,M3.2.0,M11.1.0";           // EST USA  
//const char* Timezone = "CST6CDT,M3.2.0,M11.1.0";           // CST USA
//const char* Timezone = "MST7MDT,M4.1.0,M10.5.0";           // MST USA
//const char* Timezone = "NZST-12NZDT,M9.5.0,M4.1.0/3";      // Auckland
//const char* Timezone = "EET-2EEST,M3.5.5/0,M10.5.5/0";     // Asia
//const char* Timezone = "ACST-9:30ACDT,M10.1.0,M4.1.0/3":   // Australia
