#include <avr/interrupt.h>  
#include <avr/power.h>  
#include <avr/sleep.h>  
#include <avr/io.h> 

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <GSM_G510.h>

////////////////////////////////////////////
#define TX 10
#define RX 8
#define SIG 2 //Signal
#define COM 3 //Communication
#define SWITCH 13 //Switch
#define VOLTAGE 5 //battery voltage
////////////////////////////////////////////
volatile int time1=0;
volatile int time2=0;

 int count=0;
 int block=0;
 bool zapadnia=0;
 bool a=1;

 int sensorValue;
 int lvl=1;
 float voltage;
 float option;

 char phone[]={"0048577824873"}; 
 char msg[14];


////////////////////////////////////////////
GSM_G510 gsm=GSM_G510(RX,TX,9);
SoftwareSerial *mySerial=gsm.GetSerial();
////////////////////////////////////////////
void Menu();
void Send(int flag);
void CountSignals();
void ReadSerial();
void SleepGSM();
void WakeUpGSM();
void EnterSleep();
void SignalInterrupt();
float ReadVoltage();
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);
////////////////////////////////////////////
void setup() {
 
 gsm.init();
 Serial.begin(9600);
 pinMode(SIG, INPUT);
 pinMode(13, OUTPUT);
 digitalWrite(13, LOW);
 lcd.begin(16, 2);
 lcd.print("hello, world!");
 

 mySerial->flush();
 mySerial->write("\r\n");
 mySerial->write("AT\r\n");
 mySerial->flush(); 
 
 MCUSR &= ~(1<<WDRF); //Clear the reset flag.
 WDTCSR |= (1<<WDCE) | (1<<WDE); // This will allow updates for 4 clock cycles
 WDTCSR = 1<<WDP0 | 1<<WDP3; //set new watchdog timeout prescaler value
 WDTCSR |= _BV(WDIE); //Enable the WD interrupt

 EnterSleep();
 SleepGSM();
}
/////////////////////////////////////
ISR(WDT_vect)
{
  time1 = time1+8;
  time2 = time2+8;
}
/////////////////////////////////////

void loop() {  
 
    if(a==1)
    {
      Send(3);
      ReadVoltage();
      a=0;
    }

     Serial.println(time1);
     Serial.print("Block: ");
     Serial.print(block);
     Serial.print(" Rats: ");
     Serial.print(count); 
 
    if(time1 >= 86400) //event time co ile sek?
    {
    ReadVoltage();
    //lcd.begin(16, 2);
    //lcd.setCursor(0, 0);
    //lcd.print(lvl);
    Serial.println("Time");
    Serial.println(count);
    Serial.println(lvl);
    time1=0;
    }
    if(time2 >= 10800) //event time co ile sek?
    {
    ReadVoltage();
    time2=0;
    }
      
   
     
    if(digitalRead(SIG)==LOW)
    {
        if(zapadnia==1)
     {
         CountSignals();
         zapadnia=0;
     }
      block=0;
      EnterSleep();
    }
    
    if(digitalRead(SIG)==HIGH)
    {
      if(block ==0)
      {
        zapadnia=1;
      }
      block++;
      if(block>=100)
      {
        lcd.begin(16, 2);
        lcd.setCursor(0, 0);
        lcd.print("!!BLOCKED!!");
        Serial.println("BLOCKED");
        Send(1);
        delay(5000);
        block=1;
      }
      
    }

    if(lvl == 0)
    {
      Send(2); //lowbattery
    }
 
  
  lcd.begin(16, 2);
  lcd.setCursor(0, 1);
  lcd.print(block);lcd.print(" ");lcd.print(count);lcd.print(" ");lcd.print(lvl);
  
  delay(100);
  
}
/////////////////////////////////////
void Send(int flag)
{
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("GSM WAKE UP!");
   WakeUpGSM();
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("Try to send SMS");
   
 
   if(flag==0)
   {
      ReadVoltage();
      msg[0]='P';msg[1]='C';msg[2]='P';msg[3]='L';msg[4]='M';msg[5]='T';msg[6]=' ';msg[7]='R';msg[8]=count+'0'; msg[9]='I';msg[10]='1';msg[11]='B';msg[12]=lvl+'0';msg[13]='\0';
      lcd.setCursor(0, 1);
      lcd.print(msg);
      Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
      delay(15000);
      Serial.println("Wysylanie sms:");
      Serial.println(msg);
 
      while(!gsm.sendSms(phone,msg)) {
        Serial.println("Nie wyslano sms");
        delay(1000);
        }   
      Serial.println("Wyslano sms");
      lcd.begin(16, 2);
      lcd.setCursor(0, 0);
      lcd.print("SMS SEND");
    
   }
   
   if(flag==1)
   {
    msg[0]='P';msg[1]='C';msg[2]='P';msg[3]='L';msg[4]='M';msg[5]='T';msg[6]=' ';msg[7]='B';msg[8]='L'; msg[9]='O';msg[10]='C';msg[11]='K';msg[12]='1';msg[13]='\0';
    lcd.setCursor(0, 1);
    lcd.print(msg);
    Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

    Serial.println("Wysylanie sms:");
    Serial.println(msg);
   
   while(!gsm.sendSms(phone,msg)) {
   Serial.println("Nie wyslano sms");
   delay(1000);
   }   
   Serial.println("Wyslano sms");
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("SMS SEND");
   }

   if(flag==2)
   {
     msg[0]='P';msg[1]='C';msg[2]='P';msg[3]='L';msg[4]='M';msg[5]='T';msg[6]=' ';msg[7]='L';msg[8]='O'; msg[9]='W';msg[10]=ReadVoltage()+'B';msg[11]='A';msg[12]='1';msg[13]='\0';
     lcd.setCursor(0, 1);
     lcd.print(msg);
     Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

     Serial.println("Wysylanie sms:");
     Serial.println(msg);
     
   
      while(!gsm.sendSms(phone,msg)) {
      Serial.println("Nie wyslano sms");
       delay(1000);
   }   
   Serial.println("Wyslano sms");
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("SMS SEND");
   }

     if(flag==3)
   {
     msg[0]='P';msg[1]='C';msg[2]='P';msg[3]='L';msg[4]='M';msg[5]='T';msg[6]=' ';msg[7]='R';msg[8]='D'; msg[9]='Y';msg[10]=ReadVoltage()+'I';msg[11]='D';msg[12]='1';msg[13]='\0';
     lcd.setCursor(0, 1);
     lcd.print(msg);
     Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

     Serial.println("Wysylanie sms:");
     Serial.println(msg);
   
      while(!gsm.sendSms(phone,msg)) {
      Serial.println("Nie wyslano sms");
       delay(1000);
   }   
   Serial.println("Wyslano sms");
   lcd.begin(16, 2);
   lcd.setCursor(0, 0);
   lcd.print("SMS SEND");
   }
   
 SleepGSM();
}
///////////////////////////////////

void CountSignals()
{
    count++;
    
    Serial.println(count);
    //if(count > 600000)
    //{
      Send(0);
    //}
    delay(50);
}

//////////////////////////////////

void ReadSerial()
{
  if (mySerial->available()) {
    Serial.write(mySerial->read());
  }
  if (Serial.available()) {
    mySerial->write(Serial.read());
  }
}
//////////////////////////////////
float ReadVoltage()
{
  digitalWrite(13, HIGH);
  delay(300);
  sensorValue = analogRead(A0);  
  voltage = (sensorValue * 5) / 1023.0;
  digitalWrite(13, LOW);
  if(voltage>0.70 && voltage <= 0.72)
  {
    lvl = 3;
  }
  if(voltage>63 && voltage <= 0.70)
  {
    lvl =2;
  }
  if(voltage>0.61 && voltage<= 0.63)
  {
    lvl=1;
  }
  if(voltage<0.61)
  {
    lvl=0; 
  }
  return voltage;
}

//////////////////////////////////

void SleepGSM(){
  Serial.println("GSM go sleep");
  mySerial->flush();
  mySerial->write("\r\n");
  delay(100);
  mySerial->write("ATS24=1\r\n");
}

//////////////////////////////////

void WakeUpGSM()
{
    Serial.println("GSM wake up!");
    mySerial->flush();
    mySerial->write("\r\n");
    mySerial->write("AT\r\n");
    mySerial->flush();
    delay(4000);
    Serial.println("GSM woke wp");
}

/////////////////////////////////////

void EnterSleep() 
{

    // Choose our preferred sleep mode:  
    set_sleep_mode(SLEEP_MODE_PWR_SAVE); 
   
    interrupts();  
    // Set pin 2 as interrupt and attach handler:  
    attachInterrupt(0, SignalInterrupt, HIGH);  
    // Set pin 3 as interrupt and attach handler:
    //attachInterrupt(1, OptionInterrupt, HIGH);
    delay(100);  
    // Set sleep enable (SE) bit:  
    sleep_enable();  
    // Put the device to sleep: 
    sleep_mode();  
    // Upon waking up, sketch continues from this point.  
    sleep_disable();  
    
  
}

//////////////////////////////////
void SignalInterrupt()  
{  
}  
//////////////////////////////////
/*void OptionInterrupt()  
{  
}  */
//////////////////////////////////

void Menu() //todo
{
}
//////////////////////////////////
