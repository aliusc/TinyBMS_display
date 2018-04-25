// is BMS gaunami duomenys
unsigned int cell[19]; //celes 1-16, minV, maxV, deltaV
float battery_voltage, battery_current, speed;
unsigned long uptime, time_left, soc, hall_impuses_per_distance;
unsigned int distance_left;
int temp_onboard, temp_ext_1, temp_ext_2, motor_temp, battery_status, motor_temp_max;
byte speed_mode; //is greicio kontrolerio o ne is BMS

// apskaiciuojami kintamieji
float trip_km, trip_w, trip_w_km, trip_max_w, trip_max_a, trip_min_v, avg_speed, max_speed, regen_w, regen_ah, regen_max_a;
unsigned long time_moving;

// ekrano kintamieji
byte active_screen, active_settings_block, active_settings_block_limit;
int c_w, temperatur_graph_max, num_of_num, num_total; //char_width
char desimtainis[15];
unsigned int cell_min, cell_max;
#define graph_height  41
int current_graph_x_begin[graph_height] = {97,97,   98,98,98,98,     99,99,99,99,99,99,       100,100,100,100, 101,101,101,101,101, 102,102,102,102,102, 103,103,103,103,103, 104,104,104,104, 105,105,105,105,105, 106};
int current_graph_x_end[graph_height] =   {124,124, 123,123,123,123, 122,122,122,122,122,122, 121,121,121,121, 120,120,120,120,120, 119,119,119,119,119, 118,118,118,118,118, 117,117,117,117, 116,116,116,116,116, 115};
int temperatur_graph[117];// = {60.2,50,40.8,30.1,25.4,24.7,25,25.3,25.6,25.9,26.2,26.5,26.8,27.1,27.4,27.7,28,28.3,28.6,28.9,29.2,29.5,29.8,30.1,32.5,34.9,37.3,39.7,42.1,44.5,46.9,49.3,51.7,54.1,56.5,58.9,61.3,63.7,66.1,68.5,70.9,73.3,75.7,78.1,80.5,82.9,85.3,87.7,90.1,110,118,120,112.5,92.5,92.5,88.3,84.1,79.9,75.7,71.5,67.3,63.1,58.9,54.7,50.5,46.3,42.1,37.9,33.7,29.5,25.3,21.1,22,22.9,23.8,24.7,25.6,26.5,27.4,28.3,29.2,30.1,31,31.9,32.8,33.7,34.6,35.5,36.4,37.3,28.6,28.9,29.2,29.5,29.8,30.1,32.5,34.9,37.3,39.7,42.1,44.5,46.9,49.3,51.7,54.1};
//#define menu_blocks_1 9
//char* menu_blocks_1[menu_blocks_1] = {"Sec", "Minutes", "Hour", "Na12", "Na22", "Na23", "Na31", "Na32", "Na33"};
//char
#define settings_count 10
unsigned long setting[settings_count];

//touch kintamieji
byte touchedKeyVal;
byte touchDelay = 200;
unsigned long button1PressBegin, button2PressBegin, button3PressBegin, button4PressBegin;
boolean button1Pressed, button2Pressed, button3Pressed, button4Pressed;


// pagalbiniai kintamieji
unsigned long bms_retrieve_time, controller_retrieve_time, screen_refresh_time, motor_graph_time;
volatile unsigned long button_down_begin;
bool go_next_screen, go_last_screen, go_home_screen, increase_value, decrease_value;
int button_short_click = 20;
int button_long_click = 1000;
int motor_graph_interval = 5130;//kas kiek laiko talpiname temperatura i grafika (116 iteraciju, 12sek, turesime 20min intevala)

//laiko kintamieji
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
//#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) ( _time_ / SECS_PER_HOUR) //(( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
//#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  
