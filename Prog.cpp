#include <avr/interrupt.h>  
#include <avr/power.h>  
#include <avr/sleep.h>  
#include <avr/io.h> 

#include <SoftwareSerial.h>
#include <GSM_G510.h>

////////////////////////////////////////////
#define TX 10
#define RX 11
#define SIG 2 //Signal
#define COM 3 //Communication
#define VOLTAGE 5 //battery voltage
////////////////////////////////////////////
volatile int time=0;

 int count=0;
 int block=0;
 bool zapadnia=0;

  int sensorValue;
  float voltage;
  float option;

char phone[]={"693378122"}; 
char msg[14];


////////////////////////////////////////////
GSM_G510 gsm=GSM_G510(RX,TX,8);
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
int ReadVoltage();
////////////////////////////////////////////
void setup() {
 Serial.println("No witam");
 
 gsm.init();
 Serial.begin(9600);
 pinMode(SIG, INPUT);

 mySerial->flush();
 mySerial->write("\r\n");
 mySerial->write("AT\r\n");
 mySerial->flush(); 
 
 MCUSR &= ~(1<<WDRF); //Clear the reset flag.
 WDTCSR |= (1<<WDCE) | (1<<WDE); // This will allow updates for 4 clock cycles
 WDTCSR = 1<<WDP0 | 1<<WDP3; //set new watchdog timeout prescaler value
 WDTCSR |= _BV(WDIE); //Enable the WD interrupt

 EnterSleep();
}
/////////////////////////////////////
ISR(WDT_vect)
{
  time = time+8;
}
/////////////////////////////////////

void loop() {  
    if(time >= 24)
    {
    Serial.println("Time");
    Serial.println(count);
    Serial.println(ReadVoltage());
    time=0;
    }
      
    if(digitalRead(SIG)==LOW)
    {
      if(zapadnia==1)
      {
         CountSignals();
         block=0;
         zapadnia=0;
         EnterSleep();
      }
    }
    
    if(digitalRead(SIG)==HIGH)
    {
      block++;
      if(block==100)
      {
        Serial.println("BLOCKED");
        //Send(1);
        delay(5000);
        block=1;
      }
      zapadnia=1;
    }

    if(ReadVoltage() <= 3)
    {
      Send(2); //lowbattery
    }
  Serial.println(time);
  Serial.print("Block: ");
  Serial.print(block);
  Serial.print(" Rats: ");
  Serial.println(count);
  delay(100);  
  
}
/////////////////////////////////////
void Send(int flag)
{
   WakeUpGSM();
 
   if(flag==0)
   {
      Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

      Serial.println("Wysylanie sms:");
      Serial.println(msg);
 
      while(!gsm.sendSms(phone,msg)) {
        Serial.println("Nie wyslano sms");
        delay(1000);
        }   
      Serial.println("Wyslano sms");
    
   }
   
   if(flag==1)
   {
    Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

    Serial.println("Wysylanie sms:");
    Serial.println("BLOCKED ID 1");
   
   while(!gsm.sendSms(phone,"BLOCKED ID 1")) {
   Serial.println("Nie wyslano sms");
   delay(1000);
   }   
   Serial.println("Wyslano sms");
  
   }

   if(flag==3)
   {
     Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
     delay(15000);

     Serial.println("Wysylanie sms:");
     Serial.println("LOW BATTERY");
   
      while(!gsm.sendSms(phone,"LOW BATTER")) {
      Serial.println("Nie wyslano sms");
       delay(1000);
   }   
   Serial.println("Wyslano sms");
   }
   
 SleepGSM();
}
///////////////////////////////////

void CountSignals()
{
    count++;
    msg[0]='P';
    msg[1]='C';
    msg[2]='P';
    msg[3]='L';
    msg[4]='M';
    msg[5]='T';
    msg[6]=' ';
    msg[7]='R';
    msg[8]=count+'0'; 
    msg[9]='B';
    msg[10]=ReadVoltage()+'0';
    msg[11]='I';
    msg[12]='1';
    msg[13]='\0';
    
    Serial.println(count);
    if(count > 6)
    {
      //Send(0);
    }
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
int ReadVoltage()
{
  sensorValue = analogRead(A0);  
  voltage = sensorValue * (VOLTAGE / 1023.0);
  return (int)voltage;
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

    SleepGSM();
    // Choose our preferred sleep mode:  
    set_sleep_mode(SLEEP_MODE_PWR_SAVE); 
   
    interrupts();  
    // Set pin 2 as interrupt and attach handler:  
    attachInterrupt(0, SignalInterrupt, HIGH);  
    // Set pin 3 as interrupt and attach handler:
    attachInterrupt(1, OptionInterrupt, HIGH);
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
void OptionInterrupt()  
{  
}  
//////////////////////////////////

void Menu()
{
}
//////////////////////////////////

