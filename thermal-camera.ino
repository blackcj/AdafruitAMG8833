// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_ST7735.h>

// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_AMG88xx.h>


/***************************************************************************
  This is a library for the AMG88xx GridEYE 8x8 IR camera

  This sketch makes a 64 pixel thermal camera with the GridEYE sensor
  and a 128x128 tft screen https://www.adafruit.com/product/2088

  Designed specifically to work with the Adafruit AMG88 breakout
  ----> http://www.adafruit.com/products/3538

  These sensors use I2C to communicate. The device's I2C address is 0x69

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <SPI.h>

#include <Wire.h>
#include <math.h>

#define TFT_CS     A2 // chip select pin for the TFT screen
#define TFT_RST    A0 
#define TFT_DC     A1

// Allows the device to work without an internet connection
SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

//the colors we will be using
const uint16_t camColors[] = {0x480F,
0x400F,0x400F,0x400F,0x4010,0x3810,0x3810,0x3810,0x3810,0x3010,0x3010,
0x3010,0x2810,0x2810,0x2810,0x2810,0x2010,0x2010,0x2010,0x1810,0x1810,
0x1811,0x1811,0x1011,0x1011,0x1011,0x0811,0x0811,0x0811,0x0011,0x0011,
0x0011,0x0011,0x0011,0x0031,0x0031,0x0051,0x0072,0x0072,0x0092,0x00B2,
0x00B2,0x00D2,0x00F2,0x00F2,0x0112,0x0132,0x0152,0x0152,0x0172,0x0192,
0x0192,0x01B2,0x01D2,0x01F3,0x01F3,0x0213,0x0233,0x0253,0x0253,0x0273,
0x0293,0x02B3,0x02D3,0x02D3,0x02F3,0x0313,0x0333,0x0333,0x0353,0x0373,
0x0394,0x03B4,0x03D4,0x03D4,0x03F4,0x0414,0x0434,0x0454,0x0474,0x0474,
0x0494,0x04B4,0x04D4,0x04F4,0x0514,0x0534,0x0534,0x0554,0x0554,0x0574,
0x0574,0x0573,0x0573,0x0573,0x0572,0x0572,0x0572,0x0571,0x0591,0x0591,
0x0590,0x0590,0x058F,0x058F,0x058F,0x058E,0x05AE,0x05AE,0x05AD,0x05AD,
0x05AD,0x05AC,0x05AC,0x05AB,0x05CB,0x05CB,0x05CA,0x05CA,0x05CA,0x05C9,
0x05C9,0x05C8,0x05E8,0x05E8,0x05E7,0x05E7,0x05E6,0x05E6,0x05E6,0x05E5,
0x05E5,0x0604,0x0604,0x0604,0x0603,0x0603,0x0602,0x0602,0x0601,0x0621,
0x0621,0x0620,0x0620,0x0620,0x0620,0x0E20,0x0E20,0x0E40,0x1640,0x1640,
0x1E40,0x1E40,0x2640,0x2640,0x2E40,0x2E60,0x3660,0x3660,0x3E60,0x3E60,
0x3E60,0x4660,0x4660,0x4E60,0x4E80,0x5680,0x5680,0x5E80,0x5E80,0x6680,
0x6680,0x6E80,0x6EA0,0x76A0,0x76A0,0x7EA0,0x7EA0,0x86A0,0x86A0,0x8EA0,
0x8EC0,0x96C0,0x96C0,0x9EC0,0x9EC0,0xA6C0,0xAEC0,0xAEC0,0xB6E0,0xB6E0,
0xBEE0,0xBEE0,0xC6E0,0xC6E0,0xCEE0,0xCEE0,0xD6E0,0xD700,0xDF00,0xDEE0,
0xDEC0,0xDEA0,0xDE80,0xDE80,0xE660,0xE640,0xE620,0xE600,0xE5E0,0xE5C0,
0xE5A0,0xE580,0xE560,0xE540,0xE520,0xE500,0xE4E0,0xE4C0,0xE4A0,0xE480,
0xE460,0xEC40,0xEC20,0xEC00,0xEBE0,0xEBC0,0xEBA0,0xEB80,0xEB60,0xEB40,
0xEB20,0xEB00,0xEAE0,0xEAC0,0xEAA0,0xEA80,0xEA60,0xEA40,0xF220,0xF200,
0xF1E0,0xF1C0,0xF1A0,0xF180,0xF160,0xF140,0xF100,0xF0E0,0xF0C0,0xF0A0,
0xF080,0xF060,0xF040,0xF020,0xF800,};

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

Adafruit_AMG88xx amg;
unsigned long delayTime;

// Stores the 8x8 grid of values
float pixels[AMG88xx_PIXEL_ARRAY_SIZE];
// Stores the 16x16 grid of interpolated temperature values
double temp[256];

uint16_t displayPixelWidth, displayPixelHeight;

//low range of the sensor (this will be blue on the screen)
float mintemp = 16.0f;

//high range of the sensor (this will be red on the screen)
float maxtemp = 28.0f;

// Values used to store values printed to the tft
float maxDisplay;
float minDisplay;
float avgDisplay;

// Make data available to the cloud
String sensorData;

bool foundCamera = false;
int yOffset = 0;

// Length of time to look for WiFi before starting without it
const uint32_t msRetryTime  =   30000; // stop trying after 30sec
int frame = 8;

void setup() {
        Particle.connect();
    if (!waitFor(Particle.connected, msRetryTime)) { // Wait for 30 seconds
        WiFi.off();                // no luck, no need for WiFi
        RGB.control(true); 
        RGB.color(0, 0, 0);        // turn off blinking green light if no WiFi
    }
    if(Particle.connected()) {
        // If we're connected to the cloud, make the sensor data available
        Particle.variable("sensordata", sensorData);
    }
    setupTFT();
    delay(10000); // This could be reduced to 1 or 2 seconds
    
    // Clear the screen
    tft.fillScreen(ST7735_BLACK);
    displayPixelWidth = tft.width() / 16;
    displayPixelHeight = tft.height() / 16;

    setupCamera();
    
    delay(200); // let sensor boot up

}

void setupCamera() {
        
    bool status;
    
    // default settings
    status = amg.begin();
    if (!status) {
        if(Particle.connected()) {
            Particle.publish("no thermal camera found");
        }
    } else {
        foundCamera = true;
        if(Particle.connected()) {
            Particle.publish("thermal camera connected");
        }
    }
}

void setupTFT() {
    tft.initG();
	tft.fillScreen(ST7735_BLACK);
    tft.setRotation(1);
    tft.setCursor(5, 5);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextWrap(true);
    tft.print("Warming up..."); 
}

void updateLoc(int locToUpdate, float pixelValue) {
    locToUpdate = constrain(locToUpdate, 0, 255);
    temp[locToUpdate] = pixelValue;
}

void loop() {
    int loc = 0;
    yOffset = 0;
    sensorData = "";
    frame = frame + 1;
    if(foundCamera) {
        //read all the pixels
        amg.readPixels(pixels);
        // Set min and max to the first data point to start
        mintemp = pixels[0];
        maxtemp = pixels[0];
        avgDisplay = 0.0f;
        // Turn the 8x8 grid into a 16x16 grid
        for(int m=0; m<AMG88xx_PIXEL_ARRAY_SIZE; m++){
          //avgDisplay = avgDisplay + pixels[m];
          if(mintemp > pixels[m]) {
              mintemp = pixels[m];
          }
          if(maxtemp < pixels[m]) {
              maxtemp = pixels[m];
          }
          if(m > 0 && m % 8 == 0) {
              yOffset = yOffset + 1;
          }
          int actual = pixels[m];

          int nextX = actual;
          if(m < AMG88xx_PIXEL_ARRAY_SIZE - 1 && m % 15 != 0) {
              nextX = pixels[m+1];
          }

          int nextY = actual;
          if(m < AMG88xx_PIXEL_ARRAY_SIZE - 8) {
              nextY = pixels[m+8];
          }
          
          int nextZ = actual;
          if(m < AMG88xx_PIXEL_ARRAY_SIZE - 9) {
              nextZ = pixels[m+9];
          }
          
          // Interpolate the data. 'A' represents actual data. The remaining
          // data points are calculated based on averages between actual points.
          // A, B, A, B, A, B
          // C, D, C, D, C, D
          // A, B, A, B, A, B
          
          // A: 0, 2, 4, 6, 8, 10, 12, 14
          loc = m*2 + (16 * yOffset);
          updateLoc(loc, actual);
          
          // B: 1, 3, 5, 7, 9, 11, 13, 15
          // A, ?, X, -, -, -
          // -, -, -, -, -, -
          // -, -, -, -, -, -
          loc = m*2 + 1 + (16 * yOffset);
          updateLoc(loc, (actual + nextX) / 2);

          // C: 0, 2, 4, 6, 8, 10, 12, 14  
          // A, -, -, -, -, -
          // ?, -, -, -, -, -
          // Y, -, -, -, -, -
          loc = m*2 + (16 * (yOffset + 1));
          updateLoc(loc, (actual + nextY) / 2);//pixels[m]);

          // D: 1, 3, 5, 7, 9, 11, 13, 15
          // A, -, X, -, -, -
          // -, ?, -, -, -, -
          // Y, -, Z, -, -, -
          loc = m * 2 + (16 * (yOffset + 1)) + 1;
          updateLoc(loc, (actual + nextX + nextY + nextZ) / 4);//pixels[m]);

          //temp[loc] = (pixels[m] + nextY) / 2;
        }
    }
    // Only run this every second to prevent flickering on the screen
    if(frame > 7) {
        //  Make data available to a server
        if(Particle.connected()) {
            sensorData = "[";
            for(int i=0; i<AMG88xx_PIXEL_ARRAY_SIZE; i++){
                sensorData = sensorData + pixels[i];
                if( (i + 1) < AMG88xx_PIXEL_ARRAY_SIZE ) {
                    sensorData = sensorData + ",";
                }
            }
            sensorData = sensorData + "]";
        }
        frame = 0;
        avgDisplay = avgDisplay / 64;
        maxDisplay = maxtemp;
        minDisplay = mintemp;
        mintemp = mintemp - 0.2;
        maxtemp = maxtemp + 0.5;
        
        if(maxtemp - mintemp < 5.0) {
            maxtemp = maxtemp + 3.0;
        }
        
        tft.fillRect(0, 0,
                displayPixelHeight * 16, displayPixelWidth, ST7735_BLACK);
        tft.setCursor(5, 2);
        tft.setTextColor(ST7735_WHITE);
        tft.setTextWrap(true);
        char str[6];
        //float centerF = (((pixels[35] + pixels[34]) / 2) * 9/5) + 32;
        float minF = (minDisplay * 9/5) + 32;
        float maxF = (maxDisplay * 9/5) + 32;
        //float avgF = (maxDisplay * 9/5) + 32;
        sprintf(str, "Min: %.1f  Max: %.1f", minF, maxF);
        tft.print(str);
    }
    // Display data on the screen
    for(int x=0; x<256; x++) {
        uint8_t colorIndex = map(temp[x], mintemp, maxtemp, 0.0f, 255.0f);
        colorIndex = constrain(colorIndex, 0, 255);
        //draw the pixels!
        if(x % 16 != 15) { // Don't draw the top row
            tft.fillRect((displayPixelHeight * floor(x / 16)), (displayPixelWidth * 15) - (displayPixelWidth * (x % 16)),
            displayPixelHeight, displayPixelWidth, camColors[colorIndex]);
        }
    }
    // Refresh of the Adafruit AMG88xx is 10hz, no need to refresh faster than that
    delay(150);
}