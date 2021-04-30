 


//==========================FREQUENCY MEASUREMENT========================
//on pin 8
#include <FreqMeasure.h>
double BPM;
double sum=0;
int count=0;

//==========================LCD MODULE========================================

#include <LiquidCrystal.h> // LCD Display library header file
LiquidCrystal lcd(7,9 , 10, 11, 12, 13);//LCD PIN CONFIGURATION

#include <SoftwareSerial.h>

char frame[200];
char data[100];
int data_size;

char aux;
int x = 0;
char N_S,W_E;

char latitude[15];
char longitude[15];
char altitude[6];
char date[16];
char time[7];
//char satellites[3];
//char speedOTG[10];
//char course[10];

char msg[100];
//===========================================gprs
int8_t answer;

char aux_str[60];

char pin[]="";
char apn[]="tigo";
char user_name[]="";
char password[]="";
char Url[]="http://albinotracker.eu5.org/index.php";
char port[]="80";

//===================COMMON VARIABLES========================

#define PHONE "0714271301"//phone number to receive notification
volatile int IGNORE;     //  IGNORE 
volatile int  REPORT;      //   REPORT 
int SUN_HEAT;
//ML8511 UV Hardware pin definitions
int UVOUT = A0; //Output from the sensor
int REF_3V3 = A1; //3.3V power on the Arduino board
float uvIntensity;
unsigned long ignore_duration=600000;//ignore duration in ms
//---------------------interrupts services routines-------(ISR)------------
void report_ISR(){
REPORT=LOW;//set reporting flag
}
void ignore_ISR()
{
  IGNORE=LOW;//set IGNORE flag
}
//==========================initial setups=====================================
void setup() 
{
 
  Serial.begin(9600); //set the baud rate 
 FreqMeasure.begin(); //Starting frequency measure
 lcd.begin(2, 16); // set up the LCD's number of columns and rows:
 lcd.setCursor(0, 0); // set the cursor to column 0, line 1 
  delay(1000);
 
 lcd.setCursor(0, 0);
  lcd.print("INITIALIZING ");
   lcd.setCursor(0, 1);
     lcd.print("WAIT... ");


   //UV READ pin
   pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
   Serial.println("Starting...");
    power_on();  //power on gsm module
    //sets the PIN code
   // snprintf(aux_str, sizeof(aux_str), "AT+CPIN=%s", pin);
   // sendATcommand(aux_str, "OK", 2000);
     unsigned long previous;

    previous = millis();
    // starts the GPS and waits for signal
    while ( start_GPS() == 0);
    
        previous = millis();
    while (sendATcommand("AT+CREG?", "+CREG: 0,1", 2000) == 0)
    {
     
    if(!((millis()-previous)<10000))//wait for time out
    {
            Serial.println("Network error");

    break;
    }
      
    }
    // sets APN , user name and password
      sendATcommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 2000);
    snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"APN\",\"%s\"", apn);
    sendATcommand(aux_str, "OK", 2000);    
    snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"USER\",\"%s\"", user_name);
    sendATcommand(aux_str, "OK", 2000);    
    snprintf(aux_str, sizeof(aux_str), "AT+SAPBR=3,1,\"PWD\",\"%s\"", password);
    sendATcommand(aux_str, "OK", 2000);
    sendATcommand("AT+SAPBR=5,1", "OK", 2000);

 
 //attach interrupts on buttons on pin 2&3
pinMode(2,INPUT_PULLUP);
pinMode(3,INPUT_PULLUP);
IGNORE=HIGH;
REPORT=HIGH;
 attachInterrupt(0, report_ISR, LOW);
  attachInterrupt(1, ignore_ISR, LOW);
   }

void loop() {
    get_GPS();//gets cordinates on each loop for real time tracking
   unsigned long runtime, interval=millis();//get current time

  do{
       unsigned long time=millis();//get current time

do{

   main_routine();
   //delay(1000);
    runtime=millis();


}while((runtime-time)<10000);

 lcd.clear();
 lcd.setCursor(0, 0);
  lcd.print("BPM: ");lcd.print(BPM); //prints BPM HERE
  lcd.setCursor(0, 1);
  lcd.print("UV: ");
  if(uvIntensity<5)//uv intensity hee
  {
lcd.print("NORMAL");//print uv here
}else
{
//notify sun heat 
lcd.setCursor(1,4);
lcd.print("TOOHOT");



}

  }
  while((runtime-interval)<30000);
    update();//update web server with latest gps infomation every interval

}



boolean report_2()
{   

  unsigned long cn=0,now=millis();
  do{
    //Serial.println("triggered 5");
  cn++;
  lcd.clear(); //Clears LCD Screen
    lcd.print("REPORTNG IN"); lcd.setCursor(0, 1);//notifying we're in ignore mode
  lcd.print(30-cn);
   lcd.print(" s");

  delay(1000); 

   if(IGNORE==LOW)
   {               
     delay(1000);//switch bounce
          IGNORE=HIGH;//Clear the ignore flag

 //  Serial.println("triggered 7");
    lcd.clear(); //Clears LCD Screen
    lcd.print("PAUSED "); lcd.setCursor(0, 1);//notifying we're in ignore mode
    unsigned long time=millis();
    cn=0;
    do{       
          cn++;
     lcd.setCursor(1, 1);
    unsigned int minutes =((ignore_duration-(1000*cn))/60000)+1;
    lcd.print(minutes);
    lcd.print(" Min");
    //delay here while listening to cancel button
      if(IGNORE==LOW){
          delay(1000);//switch bounce
          IGNORE=HIGH;
          // Serial.println("cancelled");

      return false;    //ignore cancelled
        } 
        //reporting here on button press
 if(REPORT==LOW)
 {
   REPORT=HIGH;//CLEar reporting flag
 report();

       return false;    //return to main loop

 }
        delay(1000);

   }
   while((millis()-time)<ignore_duration);
}
  }
  while((millis()-now)<30000);
  //
  
return report(); //report here;

}

boolean report()
{
  Serial.println("triggered 6");
lcd.clear(); //Clears LCD Screen
    lcd.print("SENDING"); 
    send_alert();//send alert message here
    delay(5000);
// update();//update web server here with current gps information
  REPORT=HIGH;//CLEar reporting flag
  return true;
}
void main_routine(){
get_bpm();
get_uv();
//reporting here on high heart rate

if((BPM>100))//check for heart rate here
  {
  
  report_2();
  }
//reporting here on button press
 if(REPORT==LOW)
 {
   REPORT=HIGH;//CLEar reporting flag
 report();
 
 }
  
}
//=========================functions to send message================================
void get_bpm(){
while(count < 15)
 {  
 if (FreqMeasure.available())
 { 
   sum = sum + FreqMeasure.read();// average several reading together
 count +=1;
 }
 } 
 float frequency =FreqMeasure.countToFrequency(sum / count);
 BPM=(frequency*60);//calculate BPM here
  count=0;//reset count

Serial.print("BPM: ");
 Serial.println(BPM); 


 
}

void get_uv(){
 //read sun heat
     int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);
   
  //Use the 3.3V power pin as a reference to get a very accurate output value from UV sensor
  float outputVoltage;
   
  if((uvLevel!=0)&&(refLevel!=0))//to avoid 0/x state
  {
  outputVoltage = 3.3 / refLevel * uvLevel;
  
  uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
  }else{
  uvIntensity=0;
  }
  Serial.print("UV Intensity (mW/cm^2): ");
  Serial.println(uvIntensity);
Serial.println(outputVoltage);

}

void send_alert()//this function sends an actual text msg to  the predefined number 
{
  delay(2000);
 if( sendATcommand("AT","OK",1000)){    
  //Send message
  sendATcommand("AT+CMGF=1","OK",1000);
  delay(1000); 
  snprintf(aux_str, sizeof(aux_str), "AT+CMGS=\"%s\"", PHONE);
  if(sendATcommand(aux_str,">",5000))
{
  //memset(msg, '\0', 60);    // Initialize the string
  /// snprintf(msg, sizeof(msg), "Lat=%s&Lon=%s&Time=%s&Alt=%s", latitude,longitude,date,altitude);
Serial.print(msg);//the message you want to send
  delay(1000);
  Serial.write(26); 
lcd.clear(); //Clears LCD Screen 
  lcd.print("MESSAGE SENT");
}
 }
    
  
return ;
}
//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//========================function to update web server with current information=========================
bool update()

{
   Serial.println("updating");
    delay(3000);  
    if( (latitude[0]!=0)||(longitude[0]!=0)){
    power_on();
    send_HTTP(); }
 }

void send_HTTP(){
unsigned long time=millis();
     while ((sendATcommand("AT+SAPBR=1,1", "OK", 20000) == 0)&&(millis()-time<60000))
    {
        delay(5000);
    }
    // Initializes HTTP service
    answer = sendATcommand("AT+HTTPINIT", "OK", 10000);
    if (answer == 1)
    {
        // Sets CID parameter
        answer = sendATcommand("AT+HTTPPARA=\"CID\",1", "OK", 5000);
        if (answer == 1)
        {
            // Sets url 
            sprintf(aux_str, "AT+HTTPPARA=\"URL\",\"%s", Url);
            Serial.print(aux_str);
            sprintf(frame, "?Lat=%s&Lon=%s&Alt=%s\"",
            latitude, longitude, altitude, date);
           // Serial.print(frame);
            answer = sendATcommand(frame, "OK", 5000);
            if (answer == 1)
            {
                // Starts GET action
                answer = sendATcommand("AT+HTTPACTION=0", "+HTTPACTION:0,200", 30000);
                if (answer == 1)
                {

                    Serial.println(("Done!"));
                }
                else
                {
                    Serial.println(("Error getting url"));
                    
                }

            }
            else
            {
                Serial.println(("Error setting the url"));
            }
        }
        else
        {
            Serial.println(("Error setting the CID"));
        }    
    }
    else
    {
        Serial.println(("Error initializating"));
    }

    sendATcommand("AT+HTTPTERM", "OK", 5000);
    sendATcommand("AT+SAPBR=0,1", "OK", 5000);

    
}


void power_on(){

    uint8_t answer=0;
    
    // checks if the module is started
    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        
    
        // waits for an answer from the module
        while(answer == 0){     // Send AT every two seconds and wait for the answer
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }
    
}
int8_t start_GPS(){

    unsigned long previous;

    previous = millis();
    // starts the GPS
    while(sendATcommand("AT+CGPSPWR=1", "OK", 500)  == 0 );
  delay(1000);
  //reset GPS in autonomy mode
 
 while(sendATcommand("AT+CGPSRST=1", "OK", 500)  == 0 );
  delay(1000);

 while(sendATcommand("AT+CGPSIPR=9600", "OK", 500)  == 0 );
 delay(1000);

    // waits for fix GPS
    while((sendATcommand2("AT+CGPSSTATUS?", "+CGPSSTATUS: Location 2D Fix","+CGPSSTATUS: Location 3D Fix", 1000)  == 0 ))
    {
    if(((millis() - previous) > 10000))//wait for timeout
    {
      Serial.println("GPS fix error");
    break;
    }
    }

   

    if ((millis() - previous) < 90000)
    {
        return 1;
    }
    else
    {
        return 0;    
    }
  
}

int8_t get_GPS(){

    int8_t counter, answer;
    long previous;

    // First get the NMEA string
    // Clean the input buffer
    while( Serial.available() > 0) Serial.read(); 
    // request Basic string
    sendATcommand("AT+CGPSINF=0", "AT+CGPSINF=0\r\n\r\n", 2000);

    counter = 0;
    answer = 0;
    memset(frame, '\0', 100);    // Initialize the string
    previous = millis();
    // this loop waits for the NMEA string
    do{

        if(Serial.available() != 0){    
            frame[counter] = Serial.read();
            counter++;
            // check if the desired answer is in the response of the module
            if (strstr(frame, "OK") != NULL)    
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    }
    while((answer == 0) && ((millis() - previous) < 2000));  

    frame[counter-3] = '\0'; 
    
    // Parses the string 
    strtok(frame, ",");
    strcpy(longitude,strtok(NULL, ",")); // Gets longitude
    strcpy(latitude,strtok(NULL, ",")); // Gets latitude
    strcpy(altitude,strtok(NULL, ".")); // Gets altitude 
    strtok(NULL, ",");    
    strcpy(date,strtok(NULL, ".")); // Gets date
    //strtok(NULL, ",");
    //strtok(NULL, ",");  
    //strcpy(satellites,strtok(NULL, ",")); // Gets satellites
    //strcpy(speedOTG,strtok(NULL, ",")); // Gets speed over ground. Unit is knots.
    //strcpy(course,strtok(NULL, "\r")); // Gets course

    convert2Degrees(latitude);
    convert2Degrees(longitude);
   
    
    memset(msg, '\0', 100);    // Initialize the string
   snprintf(msg, sizeof(msg), "Latitude: %s\nLongitude: %s\nDate: %s\nAltitude:%sM\n", latitude,longitude,date,altitude);
    Serial.println(msg);
    return answer;
}

/* convert2Degrees ( input ) - performs the conversion from input 
 * parameters in  DD°MM.mmm’ notation to DD.dddddd° notation. 
 * 
 * Sign '+' is set for positive latitudes/longitudes (North, East)
 * Sign '-' is set for negative latitudes/longitudes (South, West)
 *  
 */
int8_t convert2Degrees(char* input){

    float deg;
    float minutes;
    boolean neg = false;    

    //auxiliar variable
    char aux[10];

    if (input[0] == '-')
    {
        neg = true;
        strcpy(aux, strtok(input+1, "."));

    }
    else
    {
        strcpy(aux, strtok(input, "."));
    }

    // convert string to integer and add it to final float variable
    deg = atof(aux);

    strcpy(aux, strtok(NULL, '\0'));
    minutes=atof(aux);
    minutes/=1000000;
    if (deg < 100)
    {
        minutes += deg;
        deg = 0;
    }
    else
    {
        minutes += int(deg) % 100;
        deg = int(deg) / 100;    
    }

    // add minutes to degrees 
    deg=deg+minutes/60;


    if (neg == true)
    {
        deg*=-1.0;
    }

    neg = false;

    if( deg < 0 ){
        neg = true;
        deg*=-1;
    }
    
    float numeroFloat=deg; 
    int parteEntera[10];
    int cifra; 
    long numero=(long)numeroFloat;  
    int size=0;
    
    while(1){
        size=size+1;
        cifra=numero%10;
        numero=numero/10;
        parteEntera[size-1]=cifra; 
        if (numero==0){
            break;
        }
    }
   
    int indice=0;
    if( neg ){
        indice++;
        input[0]='-';
    }
    for (int i=size-1; i >= 0; i--)
    {
        input[indice]=parteEntera[i]+'0'; 
        indice++;
    }

    input[indice]='.';
    indice++;

    numeroFloat=(numeroFloat-(int)numeroFloat);
    for (int i=1; i<=6 ; i++)
    {
        numeroFloat=numeroFloat*10;
        cifra= (long)numeroFloat;          
        numeroFloat=numeroFloat-cifra;
        input[indice]=char(cifra)+48;
        indice++;
    }
    input[indice]='\0';


}


int8_t sendATcommand(char* ATcommand, char* expected_answer, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(Serial.available() != 0){    
            // if there are data in the UART input buffer, reads it and checks for the asnwer
            response[x] = Serial.read();
            //Serial.print(response[x]);
            x++;
            // check if the desired answer  is in the response of the module
            if (strstr(response, expected_answer) != NULL)    
            {
                answer = 1;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    

        return answer;
}

int8_t sendATcommand2(char* ATcommand, char* expected_answer1, 
        char* expected_answer2, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial.available() > 0) Serial.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial.available() != 0){    
            response[x] = Serial.read();
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
}


