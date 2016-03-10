#include <avr/interrupt.h>  
#include <avr/power.h>  
#include <avr/sleep.h>  
#include <avr/io.h> 
#include <SoftwareSerial.h>
#include <GSM_G510.h>

////////////////////////////////////////////
#define TX 10
#define RX 11
#define ON 8
#define SIG 12 //Signal
#define LED 13

////////////////////////////////////////////
int state=LOW;
int lastState=LOW;
int count=0;
volatile int f_timer=0;
char phone[]={"693378122"}; //
char msg[6];

////////////////////////////////////////////

GSM_G510 gsm=GSM_G510(RX,TX,ON);
SoftwareSerial *mySerial=gsm.GetSerial();

///////////////////////////////////////////

ISR(TIMER1_OVF_vect)
{
  /* set the flag. */
   if(f_timer == 0)
   {
     f_timer = 1;
   }
}

////////////////////////////////////////////


void setup() {
 Serial.println("No witam");

  
 msg[0]='S';
 msg[2]='B';
 gsm.init();
 Serial.begin(9600);
 pinMode(SIG, INPUT);
 state=digitalRead(SIG);
 pinMode(LED,OUTPUT);      // set pin 13 as an output so we can use LED to monitor  

 mySerial->flush();
 mySerial->write("\r\n");
 mySerial->write("AT\r\n");
 mySerial->flush(); 
 

 
 /*** Configure the timer.***/
  
  /* Normal timer operation.*/
  TCCR1A = 0x00; 
  
  /* Clear the timer counter register.
   * You can pre-load this register with a value in order to 
   * reduce the timeout period, say if you wanted to wake up
   * ever 4.0 seconds exactly.
   */
  TCNT1=0x0000; 
  
  /* Configure the prescaler for 1:1024, giving us a 
   * timeout of 4.09 seconds.
   */
  TCCR1B = 0x05;
  
  /* Enable the timer overlow interrupt. */
  TIMSK1=0x01;
  
}
/////////////////////////////////////

void Send();
void Signals();
void ReadSerial();
void SleepGSM();
void WakeUpGSM();
void EnterSleep();
void EnterSleep2();
void pinInterrupt();

////////////////////////////////////

void loop() {  
 
 Signals();
 ReadSerial();
 if(count == 7)
 {
  Send();
  count=0;
 }
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
 count = 0;
 SleepGSM();
  
}

void Signals()
{
  if (state==HIGH && lastState==LOW){
    count++;
    msg[1]=count+'0'; 
    msg[5]='0';
    Serial.println(count);
    delay(50);
  }
  
  lastState=state;
  state=digitalRead(SIG);
}

void ReadSerial()
{
  if (mySerial->available()) {
    Serial.write(mySerial->read());
  }
  if (Serial.available()) {
    mySerial->write(Serial.read());
  }
}

void SleepGSM(){
  Serial.println("GSM go sleep");
  mySerial->flush();
  mySerial->write("\r\n");
  delay(100);
  mySerial->write("ATS24=1\r\n");
}
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

void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  
  sleep_enable();
  /* Disable all of the unused peripherals. This will reduce power
   * consumption further and, more importantly, some of these
   * peripherals may generate interrupts that will wake our Arduino from
   * sleep!
   */
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();  

  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the timer timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

void EnterSleep2(void)
{
  
 // Choose our preferred sleep mode:  
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);  
    //  
    interrupts();  
    // Set pin 2 as interrupt and attach handler:  
    attachInterrupt(0, pinInterrupt, HIGH);  
    delay(100);  
    //  
    // Set sleep enable (SE) bit:  
    sleep_enable();  
    //  
    // Put the device to sleep:  
    digitalWrite(13,LOW);   // turn LED off to indicate sleep  
    sleep_mode();  
    //  
    // Upon waking up, sketch continues from this point.  
    sleep_disable();  
    digitalWrite(13,HIGH);   // turn LED on to indicate awake  
}
void pinInterrupt()  
{  
    detachInterrupt(0);  
    attachInterrupt(0, pinInterrupt, HIGH);  
}  


