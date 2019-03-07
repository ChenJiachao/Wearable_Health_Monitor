#define USE_ARDUINO_INTERRUPTS true
#include <Wire.h>
#include <IR_Thermometer_Sensor_MLX90614.h>
#include <PulseSensorPlayground.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#define VBATPIN         A2  //battery pin
#define PIN_PULSE       A0  //heartrate pin 
#define PIN_MUSCLE      A1  //muscle pin

// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    0  //connecting the RST line to the Arduino Reset pin
#define TFT_DC     5

#define BACK_COLOR  ST7735_BLACK  //define back color
#define TEXT_COLOR	ST7735_WHITE  //define text color
#define TEXTSIZE    2 //define text size
#define TEXTWIDTH   6
#define TEXTHEIGHT  8


// Option 2: use any pins but a little slower!SOFT SPI
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


const int OUTPUT_TYPE = SERIAL_PLOTTER;
///const int PIN_INPUT = A0;
//const int PIN_BLINK = 13;    // Pin 13 is the on-board LED
//const int PIN_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle

PulseSensorPlayground pulseSensor;
IR_Thermometer_Sensor_MLX90614 MLX90614 = IR_Thermometer_Sensor_MLX90614();
int heartrate=0;
int muscle=0;
uint8_t battery=0;
float Ambient=22.2;
float Object=22.3;

uint32_t temperaturetime=0;
uint32_t heartratetime=0;
uint32_t batterytime=0;
uint32_t muscletime=0;
uint32_t tempheartratetime=0;
String titleAmbient="Ambient:";
String titleObject="Object:";
String titleHeartrate="Heartrate:";
String titleMuscle="Muscle:";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //Wire.begin();
  MLX90614.begin();
  pulseSensor.analogInput(PIN_PULSE);
  pinMode(PIN_MUSCLE, INPUT);
  pinMode(VBATPIN, INPUT);
  //pulseSensor.blinkOnPulse(PIN_BLINK);
  //pulseSensor.fadeOnPulse(PIN_FADE);
  
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  pulseSensor.setThreshold(THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    //for(;;) {
      // Flash the led to show things didn't work.
      //digitalWrite(PIN_BLINK, LOW);
      //delay(50);
      //digitalWrite(PIN_BLINK, HIGH);
      //delay(50);
      Serial.println("pulseSensor not working");
    //}
  }

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(BACK_COLOR); //back color
  tft.setRotation(1);//cross screen
  drawtitle();  //draw the sensor name
}

void loop() {
  //get temperature every 2s
  if((millis()-temperaturetime)>2000)
  {
      Ambient = MLX90614.GetAmbientTemp_Celsius();
      Object = MLX90614.GetObjectTemp_Celsius();
      Serial.print("Ambient = "); Serial.print(Ambient);    Serial.println(" *C");
      Serial.print("Object = "); Serial.print(Object);    Serial.println(" *C");
      if(Ambient>99)
        Ambient = 0;
      if(Object>99)
        Object = 0;
      temperaturetime = millis();
      drawAmbient();
      drawObject();
  }

  if((millis()-heartratetime)>=20)
  {
    heartratetime = millis();
    // write the latest sample to Serial.
    pulseSensor.outputSample();
  
    // write the latest analog value to the heart servo
    moveServo(pulseSensor.getLatestSample());
  
    /*
       If a beat has happened since we last checked,
       write the per-beat information to Serial.
     */
    if (pulseSensor.sawStartOfBeat()) {
      pulseSensor.outputBeat();
    }
  }
  
  //get heartrate every 1s
  // write the latest sample to Serial.
  //pulseSensor.outputSample();
  if((millis()-heartratetime)>1000)
  {
    heartrate = pulseSensor.getLatestSample();
    if(heartrate>999)
      heartrate=999;
    heartratetime = millis();
    drawHeartrate();
  }

  //get battery percent every 5s
  if((millis()-batterytime)>5000)
  {
    getbattery();
    batterytime = millis();
    drawBattery();
  }

  //get muscle every 0.5s
  if((millis()-muscletime)>500)
  {
    muscle = analogRead(PIN_MUSCLE);
    muscletime = millis();
    drawMuscle();
  }
  //delay(20);
}

void drawtitle()
{
  tft.setCursor(0, 16);
  tft.setTextSize(TEXTSIZE);
  tft.setTextColor(TEXT_COLOR);
  tft.println(titleAmbient);
  tft.println(titleObject);
  tft.println(titleHeartrate);
  tft.println(titleMuscle);
}


void drawAmbient()
{
  char temp[10]="";
  String tempstr="";
  
  dtostrf(Ambient, 3, 1, temp);
  tempstr = temp;
  tempstr+="C";
  tft.fillRect(titleAmbient.length()*TEXTSIZE*TEXTWIDTH, 16, 4*TEXTWIDTH*TEXTSIZE, TEXTHEIGHT*TEXTSIZE, BACK_COLOR),
  tft.setCursor(titleAmbient.length()*TEXTSIZE*TEXTWIDTH, 16);
  tft.print(tempstr);
}

void drawObject()
{
  char temp[10]="";
  String tempstr="";
  
  dtostrf(Object, 3, 1, temp);
  tempstr = temp;
  tempstr+="C";
  tft.fillRect(titleObject.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE, 4*TEXTWIDTH*TEXTSIZE, TEXTHEIGHT*TEXTSIZE, BACK_COLOR),
  tft.setCursor(titleObject.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE);
  tft.print(tempstr);
}

void drawHeartrate()
{
  char temp[10]="";
  String tempstr="";
  
  //dtostrf(Heartrate, 3, 1, temp);
  tempstr = String(heartrate,DEC);
  //tempstr+="C";
  tft.fillRect(titleHeartrate.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE*2, 3*TEXTWIDTH*TEXTSIZE, TEXTHEIGHT*TEXTSIZE, BACK_COLOR),
  tft.setCursor(titleHeartrate.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE*2);
  tft.print(tempstr);
}

void drawMuscle()
{
  String tempstr="";
  
  tempstr = String(muscle,DEC);
  tft.fillRect(titleMuscle.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE*3, 4*TEXTWIDTH*TEXTSIZE, TEXTHEIGHT*TEXTSIZE, BACK_COLOR),
  tft.setCursor(titleMuscle.length()*TEXTSIZE*TEXTWIDTH, 16+TEXTHEIGHT*TEXTSIZE*3);
  tft.print(tempstr);
}

void drawBattery()
{
  String tempstr="";
  tft.setTextSize(1);
  tempstr = String(battery,DEC);
  tempstr+="%";
  tft.fillRect(160-4*TEXTWIDTH-5, 0, TEXTWIDTH*4, TEXTHEIGHT, BACK_COLOR),
  
  tft.setCursor(160-tempstr.length()*TEXTWIDTH-5, 0);
  tft.print(tempstr);
  tft.setTextSize(TEXTSIZE);
}

void getbattery()
{
  float measuredvbat = analogRead(VBATPIN);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
  Serial.print("VBat: " ); Serial.println(measuredvbat);
  //battery from 3.2 to 4.2V,0~100%
  if(measuredvbat<=3.2)
  {
    battery = 0;
  }
  else if(measuredvbat >= 4.2)
  {
    battery = 100;
  }
  else
  {
    battery=(measuredvbat-3.2)/0.01;
  }
}

//tft test
void rotateText() {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(0, 30);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(1);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(2);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_GREEN);
    tft.setTextSize(3);
    tft.println("Hello World!");
    tft.setTextColor(ST7735_BLUE);
    tft.setTextSize(4);
    tft.print(1234.567);
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();
  
    tft.setRotation(tft.getRotation()+1);
  }
}
//tft test
void rotateString(void) {
  for (uint8_t i=0; i<4; i++) {
    tft.fillScreen(ST7735_BLACK);
    Serial.println(tft.getRotation(), DEC);

    tft.setCursor(8, 25);
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE);
    tft.print("Adafruit Industries");
    while (!Serial.available());
    Serial.read();  Serial.read();  Serial.read();

    tft.setRotation(tft.getRotation()+1);
  }
}
