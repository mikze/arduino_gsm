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
////////////////////////////////////////////
volatile int toggle=0;
volatile int time=0;

int state=LOW;
int lastState=LOW;
int count=0;


int sensorValue;
float voltage;
float option;

char phone[]={"693378122"}; 
char msg[6];

////////////////////////////////////////////
GSM_G510 gsm=GSM_G510(RX,TX,8);
SoftwareSerial *mySerial=gsm.GetSerial();
////////////////////////////////////////////

////////////////////////////////////////////

void setup() {
 Serial.println("No witam");

 msg[0]='S';
 msg[2]='B';
 gsm.init();
 Serial.begin(9600);
 pinMode(SIG, INPUT);
 state=digitalRead(SIG);

 mySerial->flush();
 mySerial->write("\r\n");
 mySerial->write("AT\r\n");
 mySerial->flush(); 
 
 MCUSR &= ~(1<<WDRF); //Clear the reset flag.
 WDTCSR |= (1<<WDCE) | (1<<WDE); // This will allow updates for 4 clock cycles
 WDTCSR = 1<<WDP0 | 1<<WDP3; //set new watchdog timeout prescaler value
 WDTCSR |= _BV(WDIE); //Enable the WD interrupt
  
}
/////////////////////////////////////

void Send();
void CountSignals();
void ReadSerial();
void SleepGSM();
void WakeUpGSM();
void EnterSleep();
void SignalInterrupt();
float ReadVoltage();

////////////////////////////////////

ISR(WDT_vect)
{
  time = time+8;
}
/////////////////////////////////////

void loop() {  
 
  EnterSleep();
  if(time == 24)
  {
    Serial.println("Time");
    Serial.println(count);
    Serial.println(ReadVoltage());
    //Send();
    time=0;
  }
  else
  {
    
  }
  if(toggle == 1)
  {
    CountSignals();
    Serial.println("Signal");
    Serial.println(count);
    Serial.println(ReadVoltage());
  }
  else{
    
  }
  toggle=0;
}

/////////////////////////////////////

void Send()
{
 
 WakeUpGSM();
   
   Serial.println("Start - oczekiwanie na polaczenie z siecia (15 sekund)");
  delay(15000);

  Serial.println("Wysylanie sms:");
   Serial.println(msg);
 
 while(!gsm.sendSms(phone,msg)) {
   Serial.println("Nie wyslano sms");
   delay(1000);
 }   
 Serial.println("Wyslano sms");
 SleepGSM();
  
}

///////////////////////////////////

void CountSignals()
{
    count++;
    msg[1]=count+'0'; 
    msg[5]='0';
    Serial.println(count);
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
  sensorValue = analogRead(A0);  
  voltage = sensorValue * (5.0 / 1023.0);
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
    toggle = 1;
}  
//////////////////////////////////
//////////////////////////////////
void OptionInterrupt()  
{  
    toggle = 2;
}  
//////////////////////////////////

