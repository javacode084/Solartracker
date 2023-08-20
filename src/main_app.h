#include <Arduino.h>
#include <Wire.h>              // Memanggil library wire.h
#include <LiquidCrystal_I2C.h> // Memanggil library LCD dengan I2C
#include "RTClib.h"
#include <Adafruit_INA219.h>
#include <SPI.h>

Adafruit_INA219 ina219a;
Adafruit_INA219 ina219b (0x41);

LiquidCrystal_I2C lcd(0x27, 20, 4); // Alamat I2C(0x27) LCD dan Jenis LCD (20x4)

RTC_DS3231 rtc;
int pompa = A9; //ke pompa

#define LDRT A0                 // Deklarasi LDR pada pin A0
#define LDRB A1
#define LDRU A2                 // Deklarasi LDR pada pin A0
#define LDRS A3

// pembacaan LDR
int nilaildrt;
int nilaildrb;
int nilaildru;
int nilaildrs;

// nilai LDR
int Ti=400;
int Ba=400;
int Ut=400;
int Se=400;

int PWMR1 = 2;//barat
int PWML1 = 3;//timur
int PWMR2 = 4;//utara
int PWML2 = 5;//selatan

const DateTime m_start = DateTime(2000, 1, 1, 2, 55, 0); ///< time when realy turn on
const DateTime m_stop = DateTime(2000, 1, 1, 2, 56, 0); ///< time when realy turn off
const int r_pompa = A8; ///< pin connected to relay
const uint8_t m_min_in_h = 60; ///< minutes in an hour
const unsigned long m_refresh_time_ms = 1000; ///< time of repeting check time is in range and sending message
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

uint16_t min_calculate(const DateTime& time)
{
  uint16_t time_minute = time.hour() * m_min_in_h;
  time_minute += time.minute();
  return time_minute;
}
bool is_in_range(const DateTime& current, const DateTime& start, const DateTime& end)
{
  auto current_minute = min_calculate(current);
  auto start_minute = min_calculate(start);
  auto end_minute = min_calculate(end);
  return ((current_minute >= start_minute) && (current_minute < end_minute));
}
void readjam()
{
  int soil=analogRead(A6); // nilai moisture sensor di pin A6
  Serial.println(soil);
  static unsigned long last_loop_time = 0;
  unsigned long loop_time = millis();
  if (loop_time - last_loop_time > m_refresh_time_ms)
  {
    
    DateTime now = rtc.now();
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.println(now.second(), DEC);
    last_loop_time = millis();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.setCursor(0,1);
    lcd.print("Jam:");
    lcd.setCursor(5,1);
    lcd.print(now.hour(), DEC); 
    lcd.setCursor(7,1);
    lcd.print(":"); 
    lcd.setCursor(8,1);
    lcd.print(now.minute(), DEC);
    lcd.setCursor(10,1);
    lcd.print(":");
    lcd.setCursor(11,1);
    lcd.print(now.second(), DEC);
    lcd.setCursor(14,1);
  
    // calculate a date which is 7 days, 12 hours, 30 minutes, 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
    

    static bool is_turn_on = true;
    if (is_in_range(now, m_start, m_stop) && !is_turn_on)
    {
      Serial.println("Start");
      lcd.clear();lcd.setCursor(7,0);lcd.print("Pump On");
      digitalWrite(r_pompa, LOW);
      is_turn_on = true;
    }
    else if (!is_in_range(now, m_start, m_stop) && is_turn_on)
    {
      Serial.println("Stop");
      lcd.setCursor(7,0);lcd.print("Pump Off");
      digitalWrite(r_pompa, HIGH);
      is_turn_on = false;
  
    }
  }  
  
  
}

void read_219a()
{
  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;
  float power_mW1 = 0;

  shuntvoltage1 = ina219a.getShuntVoltage_mV();
  busvoltage1 = ina219a.getBusVoltage_V();
  current_mA1 = ina219a.getCurrent_mA();
  power_mW1 = ina219a.getPower_mW();
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);
  
  Serial.print("Bus Voltage 1:   "); Serial.print(busvoltage1); Serial.println(" V");
  Serial.print("Shunt Voltage 1: "); Serial.print(shuntvoltage1); Serial.println(" mV");
  Serial.print("Load Voltage 1:  "); Serial.print(loadvoltage1); Serial.println(" V");
  Serial.print("Current 1:       "); Serial.print(current_mA1); Serial.println(" mA");
  Serial.print("Power 1:         "); Serial.print(power_mW1); Serial.println(" mW");
  Serial.println("");

    lcd.setCursor(0,3);
    lcd.print("V1:");
    lcd.setCursor(4,3);
    lcd.print(busvoltage1);
    lcd.setCursor(10,3);
    lcd.print("I1:");
    lcd.setCursor(13,3);
    lcd.print(current_mA1);
    // delay(2000);
    
  
  
}

void read_219b()
{
  float shuntvoltage2 = 0;
  float busvoltage2 = 0;
  float current_mA2 = 0;
  float loadvoltage2 = 0;
  float power_mW2 = 0;

  shuntvoltage2 = ina219b.getShuntVoltage_mV();
  busvoltage2 = ina219b.getBusVoltage_V();
  current_mA2 = ina219b.getCurrent_mA();
  power_mW2 = ina219b.getPower_mW();
  loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000);
  
  Serial.print("Bus Voltage 2:   "); Serial.print(busvoltage2); Serial.println(" V");
  Serial.print("Shunt Voltage 2: "); Serial.print(shuntvoltage2); Serial.println(" mV");
  Serial.print("Load Voltage 2:  "); Serial.print(loadvoltage2); Serial.println(" V");
  Serial.print("Current 2:       "); Serial.print(current_mA2); Serial.println(" mA");
  Serial.print("Power 2:         "); Serial.print(power_mW2); Serial.println(" mW");
  Serial.println("");

    lcd.setCursor(0,3);
    lcd.print("V2:");
    lcd.setCursor(4,3);
    lcd.print(busvoltage2);
    lcd.setCursor(10,3);
    lcd.print("I2:");
    lcd.setCursor(13,3);
    lcd.print(current_mA2);
    // delay(2000);
}


void barat () {
// barat
analogWrite(PWMR1, 255);
analogWrite(PWML1, 0);
}

void timur () {
// timur
analogWrite(PWMR1, 0);
analogWrite(PWML1, 255);
}

void utara () {
// utara
analogWrite(PWMR2, 0);
analogWrite(PWML2, 255);
}

void selatan () {
// selatan
analogWrite(PWMR2, 255);
analogWrite(PWML2, 0);
}

void stop2 () {
  //stop utara selatan
  analogWrite(PWMR2, 0);
  analogWrite(PWMR2, 0);
}

void stop1 () {
  //stop motor timur barat
  analogWrite(PWMR1, 0);
  analogWrite(PWMR1, 0);
}

void bacaLDR()
{
nilaildrt=analogRead(LDRT);
nilaildrb=analogRead(LDRB);
nilaildru=analogRead(LDRU);
nilaildrs=analogRead(LDRS);

// lcd.clear();
    lcd.setCursor(0,2);
    lcd.print("T:");
    lcd.setCursor(4,2);
    lcd.print(nilaildrt);
    lcd.setCursor(10,2);
    lcd.print("B:");
    lcd.setCursor(13,2);
    lcd.print(nilaildrb);
    delay(3000);
    lcd.setCursor(0,2);  
    lcd.print("U:");
    lcd.setCursor(4,2);
    lcd.print(nilaildru);
    lcd.setCursor(10,2);
    lcd.print("S:");
    lcd.setCursor(13,2);
    lcd.print(nilaildrs);
    delay(3000);
    
    Serial.print("nilai LDR Timur: ");Serial.print(nilaildrt);
    Serial.print("nilai LDR Barat: ");Serial.println(nilaildrb); 
    Serial.print("nilai LDR Utara: ");Serial.print(nilaildru);
    Serial.print("nilai LDR Selatan: ");Serial.println(nilaildrs); 
    
}

void readsoil()
{
int soil =analogRead(A7);
delay(2000);
Serial.println(soil);

if(soil<=250)
{
  digitalWrite(pompa,LOW);
// onsiram
}
else{
  digitalWrite(pompa,HIGH);
  // offsiram
}
}

void setup() {
//   Serial.begin(9600);     // Baud komunikasi serial pada 9600
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
  Serial.println("Hello!");
  lcd.init();             // instruksi untuk memulai LCD
  lcd.begin(20,4);        // instruksi untuk menggunakan lcd 20x4
  lcd.backlight();        // instruksi untuk mengaktifkan lampu latar LCD
  lcd.setCursor(3,0);
  lcd.print("Hello, world!");
  lcd.setCursor(4,1);
  lcd.print("TEST PROJECT");
  lcd.setCursor(3,2);
  lcd.print("Project Juan");
  lcd.setCursor(3,3);
  lcd.print("SOLAR TRACKER");
  delay(500);

  pinMode(LDRT,INPUT);
  pinMode(LDRB,INPUT);
  pinMode(LDRU,INPUT);
  pinMode(LDRS,INPUT);

 if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 pinMode(r_pompa, OUTPUT);
 pinMode(pompa, OUTPUT);

  ina219a.begin();
  ina219b.begin();
  Serial.println("Measuring voltage and current with INA219 ...");
}

void loop() {
    unsigned long waktusekarang1=millis(); //task RTC & soil
    unsigned long waktusekarang2=millis(); //task LDR
    unsigned long waktusekarang3=millis(); //task SensorINA219A
    unsigned long waktusekarang4=millis(); //task SensorINA219B

    unsigned long waktusebelumnya1=0;
    unsigned long waktusebelumnya2=0;
    unsigned long waktusebelumnya3=0;
    unsigned long waktusebelumnya4=0;

if(waktusekarang1-waktusebelumnya1>0)
{
    readjam();
    readsoil();
    waktusebelumnya1=waktusekarang1;
}

if(waktusekarang2-waktusebelumnya2>1000)
{
    bacaLDR();
    waktusebelumnya2=waktusekarang2;
    if (nilaildrb>=Ba)
    {
        timur();
    }
    else if (nilaildrt>=Ti)
    {
        barat();
    }
    // else if (nilaildrb<Ba && nilaildrt<Ti)
    // {
    //     stop1();
    // }
    else    {
        stop1();
    }

    // if (nilaildru>=Ut)
    // {
    //     selatan();
    // }
    // else if (nilaildrs>=Se)
    // {
    //     utara();
    // }
    // else if (nilaildru>Ut && nilaildrs>Se)
    // {
    //     stop2();
    // }
    // else 
    // {
    //     stop2();
    // }

}

if(waktusekarang3-waktusebelumnya3>1500)
{
    read_219a();
    waktusebelumnya3=waktusekarang3;
    delay(2000);
}

if(waktusekarang4-waktusebelumnya4>2000)
{
    
    read_219b();
    waktusebelumnya4=waktusekarang4;
    delay(2000);
}

}