#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define BUZZER 5
#define LED_1 15
#define PB_CANCEL 34
#define PB_OK 32
#define PB_UP 33
#define PB_DOWN 35
#define DHTPIN 12
#define LDR_VP 36 
#define LDR_VN 39 
#define SERVMO 18
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET 0
#define UTC_OFFSET_DST 0

float GAMMA = 0.75;
int tOff = 30;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

char tempAr[10];
char lightArMax[10];
char lightArSide[1];

int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

bool isScheduledON = false;
unsigned long scheduledOnTime;

bool alarm_enable = true;
int n_alarms = 3; 
int alarm_hours[] = {0, 1, 2}; 
int alarm_minutes[] = {0, 10, 20}; 
int alarm_trigger[] = {false, false, false}; 

int B = 494;
int B_L = 247;
int E = 330;
int G = 392;
int F_ = 370;
int A = 440;
int D_ = 311;
int F = 349;
int C_ = 554;
int A_ = 466;
int D  = 587;
int C = 523;
int G_ = 415;
int A_L = 233;
int notes[] = {B_L,B_L,E,E,G,F_,F_,E,E,B,A,A,A,F_,F_,F_,E,E,G,F_,F_,D_,D_,F,B_L,B_L,0,0,B_L,B_L,E,E,G,F_,F_,E,E,B,D,D,C_,C,C,G_,C,C,B,A_,A_,A_L,A_L,G,E,E,E,0,0};
int n_notes = 57;


int current_mode = 0;
int max_modes = 5; 
String modes[] = {"1 - Set UTC Offset", "2 - Set Alarm_1", "3 - Set Alarm_2", "4 - Set Alarm_3", "5 - Disable Alarms"};

float utc_offset_hours = UTC_OFFSET;
int entered_hour = 0;

int pos = 0;
Servo servo;

float max_I;
   float D1 = 1.5;
   int maxSide;

void setup()
{
    pinMode(BUZZER, OUTPUT);
    pinMode(LED_1, OUTPUT);
    pinMode(PB_CANCEL, INPUT);
    pinMode(PB_OK, INPUT);
    pinMode(PB_UP, INPUT);
    pinMode(PB_DOWN, INPUT);
    pinMode(LDR_VP, INPUT);
    pinMode(LDR_VN, INPUT);

    dhtSensor.setup(DHTPIN, DHTesp::DHT22);

    servo.attach(SERVMO);

    Serial.begin(115200);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }

    display.display();
    delay(2000);

    setupWifi();
    setupMqtt();

    timeClient.begin();

    display.clearDisplay();
    print_line("Welcome to  Medibox", 5, 20, 2);
    delay(2000);
    display.clearDisplay();

    configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
}



void loop()
{
    if(!mqttClient.connected()){
    Serial.println("Reconnecting to MQTT Broker");
    connectToBroker();
    }
    mqttClient.loop();
    
    mqttClient.publish("CSE-SUR-TEMP", tempAr);
    check_schedule();
    delay(50);

    update_time_with_check_alarm();
    if (digitalRead(PB_OK) == LOW)
    {
        delay(40);
        go_to_menu();
    }
    check_temp();
    read_ldr() ;
    mqttClient.publish("CSE-SUR-LIGHT-MAX", lightArMax);
    delay(50);
    mqttClient.publish("CSE-SUR-LIGHT-Side", lightArSide);
    delay(50);   
}


void connectToBroker() {
  while(!mqttClient.connected()){
    Serial.println("Attempting MQTT connection");
    if(mqttClient.connect("ESP32-210626L8720")){
      Serial.println("connected");
      mqttClient.subscribe("CSE-SUR-ANGLE");
      mqttClient.subscribe("CSE-SUR-CONT-FACTOR");
      mqttClient.subscribe("CSE-SUR-ON-OFF");
      mqttClient.subscribe("CSE-SUR-SCH-ON");
    }else{
      Serial.println("failed");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}


void setupWifi() {
    WiFi.begin("Wokwi-GUEST", "", 6);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(250);
        print_line("Connecting to WiFi", 0, 0, 2);
    }
    display.clearDisplay();
    print_line("Connected to WiFi", 0, 0, 2);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
}

void setupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

void recieveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]); 
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.print("\n");
 
  //receive minimum angle 
  if (strcmp(topic, "CSE-SUR-ANGLE") == 0){
    tOff = String(payloadCharAr).toInt();
  }
  //receive control factor
  if (strcmp(topic, "CSE-SUR-CONT-FACTOR") == 0) {
    GAMMA = String(payloadCharAr).toFloat();
  }
  //receive main switch status
  if (strcmp(topic, "CSE-SUR-ON-OFF") == 0) {
    buzzerOn(payloadCharAr[0] == 't');
  }
  //receive scheduled time
  if(strcmp(topic, "CSE-SUR-SCH-ON") == 0){
    if(payloadCharAr[0] =='N'){
      isScheduledON = false;
    }else{
      isScheduledON = true;
      scheduledOnTime = atol(payloadCharAr);
    }
  }
}


void buzzerOn(bool on){
  if(on){
    tone(BUZZER, C);
  }
  else{
    noTone(BUZZER);
  }
}

unsigned long getTime(){
  timeClient.update();
  return timeClient.getEpochTime();
}

void read_ldr() {
float lsv = analogRead(LDR_VN) * 1.00;
float rsv = analogRead(LDR_VP) * 1.00;
float lsv_cha = 1 - lsv / 1023.0;
float rsv_cha = 1 - rsv / 1023.0;

Serial.println(String(lsv_cha) + " " + String(rsv_cha));
serv_mo(lsv_cha, rsv_cha);
}

void serv_mo(float lsv, float rsv){
  if (lsv > rsv) {
    max_I = lsv;
    maxSide = 0;
    D1 = 1.5;
  } else {
    max_I = rsv;
    maxSide = 1;
    D1 = 0.5;
  }
  String(max_I).toCharArray(lightArMax, 10);
  String(maxSide).toCharArray(lightArSide, 1);
  int theta = tOff * D1 + (180 - tOff) * max_I * GAMMA ;
  theta = min(theta, 180);
  servo.write(theta);
}

void check_schedule() {
  if (isScheduledON){
    unsigned long currentTime = getTime();
    if (currentTime > scheduledOnTime) {
      buzzerOn(true);
      isScheduledON = false;
      mqttClient.publish("CSE-SUR-ON-OFF-ESP", "1");
      mqttClient.publish("CSE-SUR-ON-OFF-SCH", "0");
      Serial.println("Scheduled ON");
      Serial.println(currentTime);
      Serial.println(scheduledOnTime);
    }
  }
}








void print_line(String text, int column, int row, int text_size)
{
    display.setTextSize(text_size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(column, row);
    display.println(text);
    display.display();
}

void print_time_now()
{
    display.clearDisplay();
    print_line(String(days), 0, 0, 2);
    print_line(":", 20, 0, 2);
    print_line(String(hours), 30, 0, 2);
    print_line(":", 50, 0, 2);
    print_line(String(minutes), 60, 0, 2);
    print_line(":", 80, 0, 2);
    print_line(String(seconds), 90, 0, 2);
}

void update_time()
{
    timeClient.update(); 
    days = timeClient.getDay();
    hours = timeClient.getHours(); 
    minutes = timeClient.getMinutes(); 
    seconds = timeClient.getSeconds();
}


void update_time_with_check_alarm(void)
{
    update_time();
    print_time_now();

    if (alarm_enable == true)
    {
        for (int i = 0; i < n_alarms; i++)
        {
            if (alarm_trigger[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes)
            {
                ring_alarm();
                alarm_trigger[i] = true;
            }
        }
    }
}

void ring_alarm()
{
    display.clearDisplay();
    print_line("Medicine    Time", 10, 0, 2);
    digitalWrite(LED_1, HIGH);

    bool break_happened = false;

    while (digitalRead(PB_CANCEL) == HIGH && break_happened == false)
    {
        for (int i = 0; i < n_notes; i++)
        {
            if (digitalRead(PB_CANCEL) == LOW)
            {
                delay(200);
                break_happened = true;
                break;
            }
            tone(BUZZER, notes[i]);
            delay(245);
            noTone(BUZZER);
            delay(5);
        }
    }

    digitalWrite(LED_1, LOW);
    display.clearDisplay();
}

int wait_for_button_press()
{
    while (true)
    {
        if (digitalRead(PB_UP) == LOW)
        {
            delay(200);
            return PB_UP;
        }
        else if (digitalRead(PB_DOWN) == LOW)
        {
            delay(200);
            return PB_DOWN;
        }
        else if (digitalRead(PB_OK) == LOW)
        {
            delay(200);
            return PB_OK;
        }
        else if (digitalRead(PB_CANCEL) == LOW)
        {
            delay(200);
            return PB_CANCEL;
        }
        update_time();
    }
}

void go_to_menu()
{
    while (digitalRead(PB_CANCEL) == HIGH)
    {
        display.clearDisplay();
        print_line(modes[current_mode], 0, 0, 2);
        int pressed = wait_for_button_press();
        if (pressed == PB_UP)
        {
            delay(200);
            current_mode += 1;
            current_mode = current_mode % max_modes;
        }
        else if (pressed == PB_DOWN)
        {
            delay(200);
            current_mode -= 1;
            if (current_mode < 0)
            {
                current_mode = max_modes - 1;
            }
        }
        else if (pressed == PB_OK)
        {
            delay(200);
            run_mode(current_mode);
        }
        else if (pressed == PB_CANCEL)
        {
            delay(200);
            break;
        }
    }
}

void set_utc_offset()
{
    while (true)
    {
        display.clearDisplay();
        print_line("Enter UTC Hour: " + String(utc_offset_hours), 0, 0, 2);
        int pressed = wait_for_button_press();
        if (pressed == PB_UP)
        {
            delay(200);
            utc_offset_hours += 1;
            if (utc_offset_hours > 23)
            {
                utc_offset_hours = 0;
            }
        }
        else if (pressed == PB_DOWN)
        {
            delay(200);
            utc_offset_hours -= 1;
            if (utc_offset_hours < 0)
            {
                utc_offset_hours = 23;
            }
        }
        else if (pressed == PB_OK)
        {
            delay(200);
            entered_hour = int(utc_offset_hours);
            break;
        }
    }

    int temp_minute = 0;
    while (true)
    {
        display.clearDisplay();
        print_line("Enter minute: " + String(entered_hour) + "." + String(temp_minute < 10 ? "0" + String(temp_minute) : String(temp_minute)), 0, 0, 2);
        int pressed = wait_for_button_press();
        if (pressed == PB_UP)
        {
            delay(200);
            temp_minute += 15;
            if (temp_minute > 45)
            {
                temp_minute = 0;
            }
        }
        else if (pressed == PB_DOWN)
        {
            delay(200);
            temp_minute -= 15;
            if (temp_minute < 0)
            {
                temp_minute = 45;
            }
        }
        else if (pressed == PB_OK)
        {
            delay(200);
            break;
        }
    }

    int new_offset_seconds = utc_offset_hours * 3600 + temp_minute * 60;
    timeClient.setTimeOffset(new_offset_seconds);

    display.clearDisplay();
    print_line("UTC Offset set", 0, 0, 2);
    delay(1000);
}


void run_mode(int mode)
{
    if (mode == 0)
    {
        set_utc_offset();
    }
    else if (mode >= 1 && mode <= 3)
    {
        set_alarm(mode - 1);
    }
    else if (mode == 4)
    {
        alarm_enable = false;
        display.clearDisplay();
        print_line("Alarms Disabled", 0, 0, 2);
        delay(1000);
    }
}

void set_alarm(int alarm){
    int temp_hour = alarm_hours[alarm];
    while(true){
        display.clearDisplay();
        print_line("Enter hour: " + String(temp_hour), 0, 0, 2);
        int pressed = wait_for_button_press();
        if(pressed == PB_UP){
            delay(200);
            temp_hour += 1;
            temp_hour = temp_hour % 24;
        }
        else if(pressed == PB_DOWN){
            delay(200);
            temp_hour -= 1;
            if(temp_hour < 0){
                temp_hour = 23;
            }
        }
        else if(pressed == PB_OK){
            delay(200);
            alarm_hours[alarm] = temp_hour;
            break;
        }
        else if(pressed == PB_CANCEL){
            delay(200);
            break;
        }
        
    }

    int temp_minute = alarm_minutes[alarm];
    while(true){
        display.clearDisplay();
        print_line("Enter minute: " + String(temp_minute), 0, 0, 2);
        int pressed = wait_for_button_press();
        if(pressed == PB_UP){
            delay(200);
            temp_minute += 1;
            temp_minute = temp_minute % 60;
        }
        else if(pressed == PB_DOWN){
            delay(200);
            temp_minute -= 1;
            if(temp_minute < 0){
                temp_minute = 59;
            }
        }
        else if(pressed == PB_OK){
            delay(200);
            alarm_minutes[alarm] = temp_minute;
            break;
        }
        else if(pressed == PB_CANCEL){
            delay(200);
            break;
        }
        
    }
    display.clearDisplay();
    print_line("Alarm set", 0, 0, 2);
    delay(1000);
}

void check_temp()
{
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    if (data.temperature > 32)
    {
        display.clearDisplay();
        print_line("TEMP HIGH", 0, 40, 1);
    }
    else if (data.temperature < 26)
    {
        display.clearDisplay();
        print_line("TEMP LOW", 0, 40, 1);
    }
    if (data.humidity > 80)
    {
        display.clearDisplay();
        print_line("HUMIDITY HIGH", 0, 50, 1);
    }
    else if (data.humidity < 60)
    {
        display.clearDisplay();
        print_line("HUMIDITY LOW", 0, 50, 1);
    }
    String(data.temperature, 2).toCharArray(tempAr, 10);
}
