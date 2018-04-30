/*
 * Pirmas etapas:
 * užklausinėkime BMS naujausių duomenų, bei tai atvaizduokime.
 */
//#define mock_data 1 //uzkomentuokim jeigu prijungtas BMS. Jeigu dirba be BMS - ims duomenis is Mock

#include <openGLCD.h>
#include <EEPROM.h>
#include <Wire.h>
#include "RTClib.h"
#include <ModbusMaster.h>
#include "variables.h"

RTC_DS3231 rtc;
#ifndef mock_data
  ModbusMaster bms;
  //ModbusMaster controller;
#endif

#define bms_retrieve_interval 100 //bus 100-200 intervale
#define controller_retrieve_interval 1000 //bus turbut 1000
#define screen_refresh_interval 2000 
#define button1Pin PA11
#define button2Pin PA12
#define button3Pin PA15
#define button4Pin PB3
#define backlight_pin PA8

void setup()
{
  //Serial.begin(9600);
  //Serial.println("Programa pradeda darba");//delay(5000);
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //isjunkime debug JTAG ant PA15, nenaudosime ir USB, nes per serial, tad lieka laisvi PA11, PA12
  GLCD.Init();
  rtc.begin();
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(button4Pin, INPUT_PULLUP);
  #ifndef mock_data
    bms.begin(170, STM32_USART0, 115200); //Serial
    bms_link_active = true;
  //  controller.begin(15, Serial3, 115200);
    bms.setResponseTimeout(200);
  //  controller.setResponseTimeout(200);
  #else
    bms_link_active = false;
  #endif

  //GLCD.SelectFont(lato_60);//lcdnums_60);
  LoadSettings();
  //Serial.println("Nustatymai uzkrauti");
  pinMode(backlight_pin, PWM);
  pwmWrite(backlight_pin, map(setting[0], 0, 100, 0, 65530));
  //Serial.println("Einame i loop");
}

void DrawScreen() {
  if(/*screen_refresh_time + screen_refresh_interval < millis() ||*/ go_next_screen || go_last_screen || go_home_screen) { //refreshiname ekrana 100% kai keiciasi aktyvus ekranas, arba kas nurodyta laika
    GLCD.ClearScreen();
//    screen_refresh_time = millis(); //nezinia ar gera mintis refreshint ekrana nei is sio nei is to, nes negrazu
    if(go_next_screen) {
      //Serial.println("Perjungiamas ekranas");
      active_screen++;
      go_next_screen = false;
      active_settings_block = 0;
    }
    else if(go_last_screen) {
      //Serial.println("Perjungiamas ankstesnis ekranas");
      if(active_screen>0)
        active_screen--;
      else
        active_screen = 5; //irasome paskutinio screen'o numeri, kad eitume i ji
      go_last_screen = false;
      active_settings_block = 0;
    }
    else if(go_home_screen) {
      //Serial.println("Perjungiamas pradinis ekranas");
      active_screen = 0;
      go_home_screen = false;
      active_settings_block = 0;
    }

    //papildomi veiksmai kurie turi buti atliekami tik 1 karta uzkraunant ta puslapi
    InitScreen();
  }
  //Serial.print("Active screen - ");
  //Serial.println(active_screen);
  switch(active_screen) {
    case 0:
      DrawIndexPage();
      break;
    case 1:
      DrawInfoPage(1);
      break;
    case 2:
      DrawInfoPage(2);
      break;
    case 3:
      DrawBatPage();
      break;
    case 4:
      DrawTemperatureGraph();
      break;
    case 5:
      DrawSettingsPage();
      break;
    default:
      active_screen = 0;
      break;
  }
}

void InitScreen() {
  switch(active_screen) {
    case 1:
      DrawInfoBlock(1,1, "Trip km");
      DrawInfoBlock(1,2, "Trip time");
      DrawInfoBlock(1,3, "AvgSped");
      DrawInfoBlock(2,1, "MaxSped");
      DrawInfoBlock(2,2, "Trip Wh");
      DrawInfoBlock(2,3, "Wh/km");
      DrawInfoBlock(3,1, "Max A");
      DrawInfoBlock(3,2, "Min V");
      DrawInfoBlock(3,3, "Max W");
      break;
    case 2:
      DrawInfoBlock(1,1, "Reg W");
      DrawInfoBlock(1,2, "Reg Ah");
      DrawInfoBlock(1,3, "RegMaxA");
      DrawInfoBlock(2,1, "Left Km");
      DrawInfoBlock(2,2, "Left tim");
      DrawInfoBlock(2,3, "Uptime");
      DrawInfoBlock(3,1, "BMS t0");
      DrawInfoBlock(3,2, "BMS t1");
      DrawInfoBlock(3,3, "BMS t2");
      break;
    case 3:
      cell_min = cell[16];
      cell_max = cell[17];
      break;
    case 5:
      GetHallImpusesPerDistanceBmsValue();
      setting[3] = hall_impuses_per_distance;
      
      DateTime dabar = rtc.now();
      setting[5] = dabar.hour();
      setting[6] = dabar.minute();
      break;
  }
  num_total = 5;
}

void DrawBatPage() {
  //siame puslapyje bus celiu grafikas
  num_of_num = 3;
  DrawInnerPageTitle("Battery", num_of_num);

  for(int c=22;c<104;c+=5) {
    GLCD.SetDot(c, 11, PIXEL_ON);
    GLCD.SetDot(c, 48, PIXEL_ON);
  }

  if(cell_min > cell[16])
    cell_min = cell[16];

  if(cell_max < cell[17])
    cell_max = cell[17];
  
  GLCD.SelectFont(Wendy3x5);
  for(int c=0;c<16;c++) {
    GLCD.DrawVBarGraph( 24 + 5*c, 49, 3, -40, 0, cell_min-100, cell_max+100, cell[c]);
    if(c%2==0) {
      if(c<9)
        GLCD.CursorToXY(24 + 5*c, 51);
      else
        GLCD.CursorToXY(24 + 5*c - 2, 51);
      GLCD.print(c+1);
    }
  }
  GLCD.CursorToXY(0, 9);
  dtostrf(cell_max/10000.0, 5, 3, desimtainis);
  GLCD.print(desimtainis);
  GLCD.CursorToXY(0, 27);
  dtostrf(cell[18]/10000.0, 5, 3, desimtainis);
  GLCD.print(desimtainis);
  GLCD.CursorToXY(0, 46);
  dtostrf(cell_min/10000.0, 5, 3, desimtainis);
  GLCD.print(desimtainis);

  //baterijos grafinis simbolis desineje
  GLCD.DrawVBarGraph( 110, 52, 8, -41, 1, 0, 100, (int)soc/1000000.0);
  GLCD.DrawRect( 112, 10, 4, 3, PIXEL_ON);

  //procentai po baterijos simboliu
  GLCD.SelectFont(Wendy3x5);
  int soc_i = (int)soc/1000000.0;
  dtostrf(soc_i, 3, 0, desimtainis);
  c_w = (int)(GLCD.StringWidth(desimtainis)+GLCD.StringWidth(" %"))/2;
  GLCD.CursorToXY(116-c_w, 55);
  GLCD.print(soc_i);
  GLCD.print(" %");

  //baterijos tekstine info centre apacioj
  GLCD.SelectFont(System5x7);
  char* battery_info = GetBatteryStatus();
  c_w = (int)GLCD.StringWidth(battery_info)/2;
  GLCD.CursorToXY(64-c_w, 57);
  GLCD.print(battery_info);
}

void DrawInfoPage(int page_num) {
  char* title = "TRIP";
  active_settings_block_limit = 9;
  char temp[20];
  
  if(page_num == 1) {
    num_of_num = 1;

    dtostrf(trip_km, 8, 2, desimtainis);
    FillInfoBlock(1,1, desimtainis, active_settings_block==1);
    
    FillInfoBlock(1,2, getHumanTime(time_moving/1000), active_settings_block==2);
    
    dtostrf(avg_speed, 7, 1, desimtainis);
    FillInfoBlock(1,3, desimtainis, active_settings_block==3);

    dtostrf(max_speed, 7, 1, desimtainis);
    FillInfoBlock(2,1, desimtainis, active_settings_block==4);

    dtostrf(trip_w, 7, 0, desimtainis);
    FillInfoBlock(2,2, desimtainis, active_settings_block==5);

    dtostrf(trip_w_km, 8, 2, desimtainis);
    FillInfoBlock(2,3, desimtainis, active_settings_block==6);

    dtostrf(trip_max_a, 7, 1, desimtainis);
    FillInfoBlock(3,1, desimtainis, active_settings_block==7);

    dtostrf(trip_min_v, 7, 2, desimtainis);
    FillInfoBlock(3,2, desimtainis, active_settings_block==8);

    dtostrf(trip_max_w, 7, 0, desimtainis);
    FillInfoBlock(3,3, desimtainis, active_settings_block==9);
  }
  else if(page_num == 2) {
    num_of_num = 2;
//    sprintf(temp,"%d",millis()/1000);

    dtostrf(regen_w * -1.0, 7, 0, desimtainis);
    FillInfoBlock(1,1, desimtainis, active_settings_block==1);
    
    dtostrf(regen_ah * -1.0, 7, 2, desimtainis);
    FillInfoBlock(1,2, desimtainis, active_settings_block==2);
    
    dtostrf(regen_max_a * -1.0, 7, 1, desimtainis);
    FillInfoBlock(1,3, desimtainis, active_settings_block==3);

    sprintf(temp, "%d", distance_left);
    FillInfoBlock(2,1, temp, active_settings_block==4);

    FillInfoBlock(2,2, getHumanTime(time_left), active_settings_block==5);
    
    FillInfoBlock(2,3, getHumanTime(millis()/1000), active_settings_block==6);
    
    dtostrf(temp_onboard/10.0, 7, 1, desimtainis);
    FillInfoBlock(3,1, desimtainis, active_settings_block==7);

    dtostrf(temp_ext_1/10.0, 7, 1, desimtainis);
    FillInfoBlock(3,2, desimtainis, active_settings_block==8);

    dtostrf(temp_ext_2/10.0, 7, 1, desimtainis);
    FillInfoBlock(3,3, desimtainis, active_settings_block==9);
  }
  DrawInnerPageTitle(title, num_of_num);
}

void DrawTemperatureGraph() {
  char* title = "MotorT";
  num_of_num = 4;
  DrawInnerPageTitle(title, num_of_num);

  GLCD.SelectFont(Wendy3x5);
  GLCD.CursorToXY(0, 8);
  dtostrf(temperatur_graph_max, 3, 0, desimtainis);
  GLCD.print(desimtainis);
  GLCD.CursorToXY(0, 53);
  GLCD.print("0");//desimtainis);
  int t;
  for(t=0;t<117;t++)
    if(temperatur_graph[t] > 0)
      GLCD.DrawVBarGraph(11+t, 56, 1, -48, 0, 0, temperatur_graph_max, temperatur_graph[t]);
  GLCD.CursorToXY(11, 58);
  GLCD.print("0");
  dtostrf(motor_graph_interval*58.5/60.0/1000.0, 1, 0, desimtainis);
  c_w = GLCD.StringWidth(desimtainis);
  GLCD.CursorToXY(67-c_w/2, 58);
  GLCD.print(desimtainis);
  dtostrf(motor_graph_interval*117.0/60.0/1000.0, 1, 0, desimtainis);
  c_w = GLCD.StringWidth(desimtainis);
  GLCD.CursorToXY(127-c_w, 58);
  GLCD.print(desimtainis);
  
}

void DrawSettingsPage() {
  char* title = "Settings";
  num_of_num = 5;
  active_settings_block_limit = 7;
  DrawInnerPageTitle(title, num_of_num);

  GLCD.SelectFont(Iain5x7, PIXEL_ON);
  GLCD.CursorToXY(0, 11);
  GLCD.print("Brightness");
  DrawSettingEntry(100, 11, 14, active_settings_block==1, 0, 5, 100);
    
  GLCD.CursorToXY(0, 20);
  GLCD.print("Speeds");
  DrawSettingEntry(70,  20, 14, active_settings_block==2, 1, 20, 100);
  DrawSettingEntry(100, 20, 14, active_settings_block==3, 2, 20, 100);
  
  GLCD.CursorToXY(0, 29);
  GLCD.print("Clock");
  DrawSettingEntry(75,  29, 12, active_settings_block==4, 5, 0, 23);
  DrawSettingEntry(100, 29, 12, active_settings_block==5, 6, 0, 59);
  
  
  GLCD.CursorToXY(0, 38);
  GLCD.print("Hall impulses dist");
  DrawSettingEntry(92, 38, 26, active_settings_block==6, 3, 0, 4000);
//  GLCD.print("Temp for home");
//  GLCD.print("Temp for graph");
  
  GLCD.CursorToXY(0, 47);
  GLCD.print("Pump ON temp");
  DrawSettingEntry(100, 47, 14, active_settings_block==7, 4, 0, 120);

//  GLCD.CursorToXY(0, 54);
}

void DrawSettingEntry(byte x, byte y, byte wid, bool selected, byte setting_index, byte val_min, unsigned long val_max) {
//  
  uint32_t min_val, max_val, current;

  GLCD.SelectFont(Iain5x7, PIXEL_ON);
  c_w = GLCD.StringWidth("100");
  GLCD.CursorToXY(x+5+wid/2-c_w/2-2, y);
  if(selected) {
    if(decrease_value && setting[setting_index]>val_min) {
      decrease_value = false;
      setting[setting_index]--;
      SetSingleSetting(setting_index, setting[setting_index]);
    }
    if(increase_value && setting[setting_index]<val_max) {
      increase_value = false;
      setting[setting_index]++;
      SetSingleSetting(setting_index, setting[setting_index]);
    }
  }
  GLCD.print(" ");
  GLCD.print(setting[setting_index]);
  GLCD.print(" ");
  if(selected) {
    //spausdiname rodykles
    GLCD.SelectFont(webdings);
    GLCD.CursorToXY(x, y);
    GLCD.print(3);
    GLCD.CursorToXY(x+5+wid, y);
    GLCD.print(4);
    GLCD.SelectFont(Iain5x7, PIXEL_ON);
  }
  else {
    GLCD.CursorToXY(x, y);
    GLCD.print("  ");
    GLCD.CursorToXY(x+5+wid, y);
    GLCD.print("  ");
  }
}

void DrawInnerPageTitle(char* title, int num_of_num) {
  //pavadinimas
  GLCD.SelectFont(System5x7);
  GLCD.CursorToXY(4, 0);
  GLCD.print(title);

  //laikrodis arba pranesimas
  char* clock_text = GetMessageBlock();
  c_w = (int)(GLCD.StringWidth(clock_text)/2);
  GLCD.CursorToXY(73-c_w, 0);
  GLCD.print(clock_text);

  //puslapiavimo informacija
  sprintf(desimtainis, "%d/%d", num_of_num, num_total);
  c_w = GLCD.StringWidth(desimtainis);
  GLCD.CursorToXY(126-c_w, 0);
  GLCD.print(desimtainis);
}

void FillInfoBlock(int row, int col, char* val, bool selected) {
  sprintf(desimtainis, " %s", val);
  int left = 43*(col-1);
  int top = 8 + 19*(row-1);
  //kol to nenaudojame - tol nepieskime
//  if(selected)
//    GLCD.FillCircle( left + 37, top + 3, 2, PIXEL_OFF); 
//  else
//    GLCD.FillCircle( left + 37, top + 3, 2, PIXEL_ON); 
  GLCD.SelectFont(Iain5x7);
  c_w = GLCD.StringWidth(desimtainis);
  GLCD.CursorToXY(left+38-c_w, top+9);
  GLCD.print(desimtainis);
}

void DrawInfoBlock(int row, int col, char* title) {
  int left = 43*(col-1);
  int top = 8 + 19*(row-1);
  GLCD.DrawRect(left, top, 41, 18, PIXEL_ON);
  GLCD.FillRect(left, top, 41, 8, PIXEL_ON);
  GLCD.SelectFont(Iain5x7, PIXEL_OFF);
  GLCD.CursorToXY(left+3, top);
  GLCD.print(title);
}

void DrawIndexPage() {
  //piesime pagrindini langa

//  //Serial.println("DrawIndexPage. Round #1");
  //baterija begin
//    //Serial.println("DrawIndexPage. Round #1.1");  
    //baterijos grafinis simbolis
    GLCD.DrawVBarGraph( 9, 52, 7, -41, 1, 0, 100, (int)soc/1000000);
    GLCD.DrawHLine( 11, 11, 3, PIXEL_ON);

//    //Serial.println("DrawIndexPage. Round #1.2");
    //itampa
    GLCD.SelectFont(System5x7);
    dtostrf(battery_voltage, 2, 0, desimtainis);
    c_w = (int)GLCD.StringWidth(desimtainis)/2;
    GLCD.CursorToXY(10-c_w, 2);
    GLCD.print(desimtainis);
    GLCD.SelectFont(Wendy3x5);
    dtostrf((battery_voltage-(int)battery_voltage)*10, 1, 0, desimtainis);
    GLCD.CursorToXY(11+c_w-1, 4);
    GLCD.print(desimtainis);
    GLCD.CursorToXY(20, 4);
    GLCD.print("v");

//    //Serial.println("DrawIndexPage. Round #1.3");
    //galimas kilometrazas
    dtostrf(battery_voltage, 3, 0, desimtainis);
    GLCD.SelectFont(Iain5x7);
    c_w = (int)GLCD.StringWidth(desimtainis)/2;
    GLCD.CursorToXY(10-c_w, 55);
    GLCD.print(distance_left);
    GLCD.SelectFont(Wendy3x5);
    GLCD.CursorToXY(19, 57);
    GLCD.print("km");
  //baterija end

  //vidurine dalis begin - laikrodis, greitis, trip_km
    //zinuciu-laikrodzio atvaizdavimas
//    //Serial.println("DrawIndexPage. Round #1.4");
    char* clock_text = GetMessageBlock();
    GLCD.SelectFont(System5x7);
    c_w = (int)(GLCD.StringWidth(clock_text)/2);
    GLCD.CursorToXY(64-c_w, 2);
    GLCD.print(clock_text);

//    //Serial.println("DrawIndexPage. Round #1.5");
    //greicio atvaizdavimas
    dtostrf(speed, 2, 0, desimtainis);
    GLCD.SelectFont(lato_50);
    c_w = (int)GLCD.StringWidth(desimtainis)/2;
    GLCD.CursorToXY(64-c_w, 16);
    GLCD.print(desimtainis);
//    GLCD.SelectFont(Wendy3x5);
//    GLCD.CursorToXY(19, 57);
//    GLCD.print("km");

//    //Serial.println("DrawIndexPage. Round #1.6");
    //TRIP atstumo atvaizdavimas
    dtostrf((int)(trip_km*10)/10.0, 5, 1, desimtainis); //palikime tik viena skaiciu po kablelio, nes kitaip apvalina ir buna netikslu
    GLCD.SelectFont(Iain5x7);
    c_w = (int)(GLCD.StringWidth(desimtainis) + GLCD.StringWidth(" km"))/2;
    GLCD.CursorToXY(64-c_w, 57);
    GLCD.print(desimtainis);
    GLCD.print(" km");
  //vidurine dalis end

////Serial.println("DrawIndexPage. Round #2");
  //desine dalis begin - srove ir temperatura
    //piesiame trapecija sroves grafiniam atvaizdavimui
      int graph_top = 11;
      GLCD.DrawHLine(current_graph_x_begin[0], graph_top, current_graph_x_end[0]-current_graph_x_begin[0]+1, PIXEL_ON);
      int graph_current = graph_height-((int)(graph_height/150.0*abs(battery_current)));
      for(int temp_i=0; temp_i<graph_height; temp_i++) {
        if(temp_i >= graph_current)
          GLCD.DrawHLine(current_graph_x_begin[temp_i]+1, graph_top+temp_i+1, current_graph_x_end[temp_i]-current_graph_x_begin[temp_i]-1, PIXEL_ON);
        else
          GLCD.DrawHLine(current_graph_x_begin[temp_i]+1, graph_top+temp_i+1, current_graph_x_end[temp_i]-current_graph_x_begin[temp_i]-1, PIXEL_OFF);
        GLCD.SetDot(current_graph_x_begin[temp_i], graph_top+temp_i+1, PIXEL_ON); 
        
        GLCD.SetDot(current_graph_x_end[temp_i], graph_top+temp_i+1, PIXEL_ON); 
      }

      GLCD.DrawHLine(current_graph_x_begin[graph_height-1], graph_top+graph_height+1, current_graph_x_end[graph_height-1]-current_graph_x_begin[graph_height-1]+1, PIXEL_ON);
      //turime 41 eil. Piesime daug eiluciu tam tikro ilgio. Nusipiesime pries tai remelius. Pagal reiksme apskaiciuosime, ar reikia eilute daryti ON ar OFF.

    //Variklio temperatura virsuje
    dtostrf(motor_temp, 3, 0, desimtainis);
    GLCD.SelectFont(Iain5x7);
    c_w = (int)GLCD.StringWidth(desimtainis)/2;
    GLCD.CursorToXY(109-c_w, 3);
    GLCD.print(desimtainis);
    GLCD.SelectFont(Wendy3x5);
    GLCD.CursorToXY(120, 5);
    GLCD.print("c");

    //naudojama srove begin apacioje po grafiku
    if(battery_current > 100 || battery_current <= -100)
      dtostrf(battery_current, 3, 0, desimtainis); //kada nereikia tokio detalaus tikslumo
    else
      dtostrf(battery_current, 5, 1, desimtainis);
    GLCD.SelectFont(Iain5x7);
    c_w = (int)GLCD.StringWidth(desimtainis)/2;
    GLCD.CursorToXY(109-c_w, 55);
    GLCD.print(desimtainis);
    GLCD.SelectFont(Wendy3x5);
    GLCD.CursorToXY(124, 57);
    GLCD.print("A");
    
  //desine dalis end
}

char* GetMessageBlock() {
  DateTime dabar = rtc.now();
  // naudojame egzistuojanti tinkamo tipo kintamaji (desimtainis), nes char* yra rodykle, ir isejus is funkcijos tai, kur ji rodo isnyksta
  if(dabar.second()%2)
    sprintf(desimtainis,"%d:%02d",dabar.hour(), dabar.minute());
  else {
    if(!bms_link_active) {
      return "NoLink";
    }
    else {
      sprintf(desimtainis," %d %02d ",dabar.hour(), dabar.minute());
    }
  }
  return desimtainis;
}

char* GetBatteryStatus() {
  switch(battery_status) {
    case 145:
      return "Charging";
      break;
    case 146:
      return "Fully charged";
      break;
    case 147:
      return "Discharging";
      break;
    case 149:
      return "   Sleep   ";
      break;
    case 150:
      return "Regeneration";
      break;
    case 151:
      return "    Idle    ";
      break;
    case 155:
      return "Fault";
      break;
  }
  return "Unknown";
}

void CalculateIterationData() {
  unsigned int iteration_delay_ms = millis() - bms_retrieve_time;
  trip_km += speed / 3600.0 / 1000.0 * iteration_delay_ms; //gauname kiek nuvaziavo kilometru tarp sekundziu
  if(battery_current > 0) {
    trip_w += battery_voltage * battery_current / 3600.0 / 1000.0 * iteration_delay_ms;
  }
  else {
    regen_w += battery_voltage * battery_current / 3600.0 / 1000.0 * iteration_delay_ms;
    regen_ah += battery_current / 3600.0 / 1000.0 * iteration_delay_ms;
    regen_max_a = battery_current < regen_max_a ? battery_current : regen_max_a;
  }
  if(abs(battery_current) > 1) {
    current_0_begin_time = millis();
  }
  trip_w_km = trip_w / trip_km;
//  //Serial.print("trip_km-");//Serial.print(trip_km,3);
//  //Serial.print(", trip_w-");//Serial.print(trip_w,3);
//  //Serial.print(", trip_w_km-");//Serial.println(trip_w_km,3);
  trip_max_w = battery_voltage * battery_current > trip_max_w ? battery_voltage * battery_current : trip_max_w;
  trip_max_a = battery_current > trip_max_a ? battery_current : trip_max_a;
  trip_min_v = battery_voltage < trip_min_v || trip_min_v == 0 ? battery_voltage : trip_min_v;
  if(speed > 1)
    time_moving += iteration_delay_ms;
  avg_speed = trip_km / (time_moving / 3600.0 / 1000.0);
  max_speed = max_speed < speed ? speed : max_speed;
  if(motor_graph_time + motor_graph_interval < millis()) {
    motor_graph_time = millis();
    byte t;
    temperatur_graph_max = motor_temp;
    for(t=116;t>0;t--) {
      temperatur_graph[t] = temperatur_graph[t-1];
      temperatur_graph_max = temperatur_graph[t] > temperatur_graph_max ? temperatur_graph[t] : temperatur_graph_max;
    }
    if(temperatur_graph_max < 60)
      temperatur_graph_max = 60;
    temperatur_graph[0]=motor_temp;
  }
  motor_temp_max = motor_temp > motor_temp_max ? motor_temp : motor_temp_max;
}

void GetControllerData() {
  if(controller_retrieve_time + controller_retrieve_interval < millis()) {

    uint8_t result;
    
    //paimame motor temp, aktyvu rezima is greicio kontrolerio
    /*
    result = controller.readHoldingRegisters(0, 2);
    if (result == controller.ku8MBSuccess) {
      motor_temp = controller.getResponseBuffer(0);
      speed_mode = controller.getResponseBuffer(1);
    }
    */
    controller_retrieve_time = millis();
  }
}

void GetBmsData() {
  if(bms_retrieve_time + bms_retrieve_interval < millis()) {
    CalculateIterationData();
    #ifdef mock_data
    /*fake duomenys begin*/
    //Serial.println("read fake BMS data");
    cell[0] = 38120; //celes 1-16, minV, maxV, deltaV
    cell[1] = 38250;
    cell[2] = 39250;
    cell[3] = 40000;
    cell[4] = 39000;
    cell[5] = 36555;
    cell[6] = 38990;
    cell[7] = 38150;
    cell[8] = 38650;
    cell[9] = 39250;
    cell[10] = 40020;
    cell[11] = 38250;
    cell[12] = 38250;
    cell[13] = 38250;
    cell[14] = 38250;
    cell[15] = 38250;
    cell[16] = 36555;
    cell[17] = 40020;
    cell[18] = cell[17]-cell[16];

    battery_voltage = 61.25658585;
    battery_current = 35.25;
    speed = 30;
    
    uptime = millis()/1000;
    time_left = 15760;
    soc = 7000000;
    distance_left = 42;
    temp_onboard = 233;
    temp_ext_1 = 250;
    temp_ext_2 = 350;
    battery_status = 151;
    motor_temp = 29;
    //Serial.println("read fake BMS data END");
    /*fake duomenys end*/

    #else
//tikri duomenys imami is BMS
    if(bms_link_active) {
      uint8_t result; //rezultato kintamasis
      unsigned long temp_bms; //laikinas kintamasis reikalingas vercians i float
      
      //celiu itampa
      //Serial.println("Skaitome celiu itampa - pirmas rodmuo");
      result = bms.readHoldingRegisters(0, 16);
      if (result == bms.ku8MBSuccess) {
        for (int j = 0; j < 16; j++) {
          cell[j] = bms.getResponseBuffer(j);
          //Serial.print(j);
          //Serial.print("-");
          //Serial.print(cell[j]);
          //Serial.print(", ");
        }
      }
      //Serial.println("Celes perskaitytos - judame toliau");
  
      //BMS uptime; estimated time left; battery voltage; battery current; min, max celes, skirtumas; t1, t2; dist left; 
      result = bms.readHoldingRegisters(32, 13);
      if (result == bms.ku8MBSuccess) {
        uptime = (unsigned long)bms.getResponseBuffer(1) << 16 | bms.getResponseBuffer(0);
        time_left = (unsigned long)bms.getResponseBuffer(3) << 16 | bms.getResponseBuffer(2);
        temp_bms = (unsigned long)bms.getResponseBuffer(5) << 16 | bms.getResponseBuffer(4);
        battery_voltage = *(float*)&temp_bms; 
        temp_bms = (unsigned long)bms.getResponseBuffer(7) << 16 | bms.getResponseBuffer(6);
        battery_current = *(float*)&temp_bms * -1.0;
        cell[16] = bms.getResponseBuffer(8)*10;
        cell[17] = bms.getResponseBuffer(9)*10;
        cell[18] = cell[17]-cell[16];
        temp_ext_1 = bms.getResponseBuffer(10) < 150 ? bms.getResponseBuffer(10) : 0;
        temp_ext_2 = bms.getResponseBuffer(11) < 150 ? bms.getResponseBuffer(11) : 0;
        distance_left = bms.getResponseBuffer(12);
      }
  
      //SOC, onboard temperature (t0)
      result = bms.readHoldingRegisters(46, 3);
      if (result == bms.ku8MBSuccess) {
        soc = (unsigned long)bms.getResponseBuffer(1) << 16 | bms.getResponseBuffer(0);
        temp_onboard = bms.getResponseBuffer(2);
      }
  
      //battery status
      result = bms.readHoldingRegisters(50, 1);
      if (result == bms.ku8MBSuccess) {
        battery_status = bms.getResponseBuffer(0);
      }
  
      //speed
      result = bms.readHoldingRegisters(54, 2);
      if (result == bms.ku8MBSuccess) {
        temp_bms = (unsigned long)bms.getResponseBuffer(1) << 16 | bms.getResponseBuffer(0);
        speed = *(float*)&temp_bms; 
      }
    }
    #endif

//    //Serial.println("GetBmsData fired");
    bms_retrieve_time = millis();
  }
  //Serial.println("BMS data calculations done");
}

void GetHallImpusesPerDistanceBmsValue() {
  uint8_t result;

  #ifndef mock_data
  if(bms_link_active) {
    result = bms.readHoldingRegisters(312, 2);
    if (result == bms.ku8MBSuccess) {
      hall_impuses_per_distance = (unsigned long)bms.getResponseBuffer(1) << 16 | bms.getResponseBuffer(0); 
    }
  }
  #else
    hall_impuses_per_distance = 0;
  #endif
}

void LoadSettings() {
  active_screen = 0;
  trip_min_v = 150;
  byte settings_exists = EEPROM.read(100);
  if(settings_exists == 1) {
    //radome uzsaugotus nustatymus, uzkraukime juos
    setting[0] = EEPROM.read(101 + 0);; //brightness
    setting[1] = EEPROM.read(101 + 1);  //greicio limitas #2
    setting[2] = EEPROM.read(101 + 2); //greicio limitas #3
    setting[4] = EEPROM.read(101 + 4);  //pump ON temperature
    
    //taip pat dali uzsakymu turime "persiusti" i 'greicio' kontroleri naudojantis MODBUS protokolu
  }
  else {
    //nustatymu nera, bus "default" reiksmes
    setting[0] = 100; //brightness
    setting[1] = 50;  //greicio limitas #2
    setting[2] = 100; //greicio limitas #3
//    setting[3] skirta hall impulses per distance
    setting[4] = 60;  //pump ON temperature
//    setting[5] skirta laikrodzio valandai,
//    setting[6] skirta laikrodzio minutem, ju nesaugome, bet nustatymus darant to reikes
  }
}

void SaveSettings() {
  EEPROM.write(100, 1);
  EEPROM.write(101+0, setting[0]); //saugome nustatyma
  EEPROM.write(101+1, setting[1]); //saugome nustatyma
  EEPROM.write(101+2, setting[2]); //saugome nustatyma
  EEPROM.write(101+4, setting[4]); //saugome nustatyma
  //Serial.println("Settings saved");

#ifndef mock_data
  if(bms_link_active) {
//upload hall setting to BMS
      // set word 0 of TX buffer to least-significant word of counter (bits 15..0)
      int result;
//      result=bms.writeSingleRegister(312, 1);//((val) & 0xFFFF));
  //    //Serial.println(result, DEC);
      // set word 1 of TX buffer to most-significant word of counter (bits 31..16)
  //    result=bms.writeSingleRegister(313, 1);//(((val) >> 16) & 0xFFFF));
  //    //Serial.println(result, DEC);
  //    //Serial.println("hall saugojimas baigtas");
      bms.clearTransmitBuffer();
      result = bms.setTransmitBuffer(0, lowWord(setting[3]));
      //Serial.print("buferis 0 - ");
      //Serial.println(result, DEC);
      bms.setTransmitBuffer(1, highWord(setting[3])); 
      //Serial.print("buferis 1 - ");
      //Serial.println(result, DEC);
      // slave: write TX buffer to (2) 16-bit registers starting at register 0
//      result = bms.writeMultipleRegisters(312, 2);
//      //Serial.println(setting[3]);
  //    result = bms.writeMultipleRegisters(312, val);
//    result = bms.maskWriteRegister(312, 0, 150);
      //Serial.println(result, DEC);
  //    result = bms.maskWriteRegister(313, 1, 0);
  //    //Serial.println(result, DEC);
      //Serial.println("hall saugojimas baigtas");
    
  //nustatome controleriui uzstatyta greiti
//  controller.writeSingleRegister(1, setting[1]);
//  controller.writeSingleRegister(2, setting[2]);
  }
#endif
}

void SetSingleSetting(int index, int val) {
  if(index == 5 || index == 6) {
    //pasiimame dabartine data ir laika
    DateTime dabar = rtc.now();
    //nustatome laika
    rtc.adjust(DateTime(dabar.year(), dabar.month(), dabar.day(), setting[5], setting[6], 0));
  }
  else if(index == 3) {
    
  }
  else {
    if(index == 0)
      pwmWrite(backlight_pin, map(val, 0, 100, 0, 65530));
  }
}


void ReadTouchButtons() {
  touchedKeyVal = 0;
  
  if(digitalRead(button1Pin) == LOW) {
    button1Pressed = true;
    button1PressBegin = button1Pressed ? button1PressBegin : millis();
  }
  else {
    button1Pressed = false;
  }
  
  if(digitalRead(button2Pin) == LOW) {
    button2Pressed = true;
    button2PressBegin = button2Pressed ? button2PressBegin : millis();
  }
  else {
    button2Pressed = false;
  }
  
  if(digitalRead(button3Pin) == LOW) {
    button3Pressed = true;
    button3PressBegin = button3Pressed ? button3PressBegin : millis();
  }
  else {
    button3Pressed = false;
  }
    //Serial.print("Button4 level is ");
    //if(digitalRead(button4Pin) == HIGH)
      //Serial.println("HIGHT");
    //else
      //Serial.println("LOW");
      
  if(digitalRead(button4Pin) == LOW) {
    button4PressBegin = button4Pressed ? button4PressBegin : millis();
    button4Pressed = true;
  }
  else {
    button4Pressed = false;
  }
  
  if(button1Pressed && button1PressBegin - millis() > touchDelay) { //click BACK
    touchedKeyVal = 1;
  }
  if(button2Pressed && button2PressBegin - millis() > touchDelay) { //click LEFT
    touchedKeyVal = 2;
  }
  if(button3Pressed && button3PressBegin - millis() > touchDelay) { //click RIGHT
    touchedKeyVal = 3;
  }
  if(button4Pressed && button4PressBegin - millis() > touchDelay) { //click ENTER
    touchedKeyVal = 4;
  }

  if(touchedKeyVal > 0) {
    buttonAnyPressBegin = millis();
      //Serial.print("button #");
      //Serial.print(touchedKeyVal);  
      //Serial.println(" Down");
      switch(touchedKeyVal) {
        case 1:
          if(active_settings_block == 0) {
            go_home_screen = true;
          }
          else {
            if(active_screen == 5)
              SaveSettings();
          }
          active_settings_block = 0;
          break;
        case 2:
          if(active_screen == 5 && active_settings_block > 0) //nustatymu lange iskart nepabega i pagrindini
            decrease_value = true;
          else
            go_last_screen = true;
          break;
        case 3:
          if(active_screen == 5 && active_settings_block > 0) //nustatymu lange iskart nepabega i pagrindini
            increase_value = true;
          else
            go_next_screen = true;
          break;
        case 4:
          active_settings_block++;
          if(active_settings_block > active_settings_block_limit)
            active_settings_block = 1;
          break;
      }
  }   
  
}

void CheckShouldDisconnectUart() {
  //testuojame UART atsijungima
  /*
   * Atsijungia kai:
   * 1 - mygtukai nespaudomi bent 1 minute ir
   * 2 - baterijos busena yra Sleep/Idle/Fault ir srove mazesne uz 1 A bent 1 minute
   */
  if(millis() - buttonAnyPressBegin > 60000 && ((battery_status == 149 || battery_status == 151 || battery_status == 155) && millis() - current_0_begin_time > 60000)) {
    bms_link_active = false;
    bms.end(STM32_USART0);
  }
}

char* getHumanTime(unsigned long val){  
//  int days = elapsedDays(val);
  int hours = numberOfHours(val);
  int minutes = numberOfMinutes(val);
  int seconds = numberOfSeconds(val);

  sprintf(desimtainis, "%3d:%02d:%02d", hours, minutes, seconds); 
  return desimtainis;
}

void loop()
{ 
  GetBmsData();
//  GetControllerData();
  DrawScreen();
  
  ReadTouchButtons();
  CheckShouldDisconnectUart();
  
//  GLCD.CursorToXY(0, 0);//GLCD.CenterX-width, GLCD.CenterY-height);


//  GLCD.print(millis()/1000);
////Serial.println(millis()/1000);
//delay(20);
}

