#include <FastLED.h>
#include <RTClib.h>

// RTC
RTC_DS3231 RTC;

// LEDs
#define LED_PIN     6        
#define NUM_LEDS    216      
#define LED_TYPE    WS2812B  
#define COLOR_ORDER GRB      
CRGB leds[NUM_LEDS];

int buttonPin = 7;
int lastSecond = -1;

// Global mode times
int growModeStartHour = 0;
int growModeEndHour = 18;
int sunsetModeStartHour = 19;
int sunsetPeakMinute = 45;
int nightModeStartHour = 20;
int nightPeakMinute = 15;

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(100);

  // Check LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(1000);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  pinMode(buttonPin, INPUT);

  // Start serial communication
  Serial.begin(9600);
  if (!RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("Enter time as HH:MM:SS to set the clock.");
}

void loop() {
  DateTime now = RTC.now();

  int currentHour = now.hour();
  int currentMinute = now.minute();
  int currentSecond = now.second();

  // // Print time once every second
  // if (currentSecond != lastSecond) {
  //   lastSecond = currentSecond;
  //   Serial.print("Time: ");
  //   Serial.print(currentHour);
  //   Serial.print(":");
  //   Serial.print(currentMinute);
  //   Serial.print(":");
  //   Serial.println(currentSecond);
  // }

  // Check for serial input to set the time
  if (Serial.available()) {
    setTimeFromSerial();
  }

  // Determine the current mode based on time
  setMode(currentHour, currentMinute, currentSecond);

  FastLED.show();

  // Send LED data to serial (send the RGB values of each LED)
  for (int i = 0; i < NUM_LEDS; i++) {
    Serial.print(leds[i].r); Serial.print(",");
    Serial.print(leds[i].g); Serial.print(",");
    Serial.print(leds[i].b);
    if (i < NUM_LEDS - 1) Serial.print("|");  // Separator for each LED
  }
  Serial.println();  // End the line to mark end of LED data

  delay(50);
}

// Function to set time based on serial input
void setTimeFromSerial() {
  if (Serial.available()) {
    String timeInput = Serial.readStringUntil('\n');  // Read input until newline
    int hour = timeInput.substring(0, 2).toInt();
    int minute = timeInput.substring(3, 5).toInt();
    int second = timeInput.substring(6, 8).toInt();

    RTC.adjust(DateTime(2024, 1, 1, hour, minute, second));  // Set with a placeholder date
    Serial.print("Time set to: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.println(second);
  }
}

// Determines which mode to set based on the current time
void setMode(int hour, int minute, int second) {
  if (hour >= growModeStartHour && hour < growModeEndHour) {
    growMode();
  } else if (hour == sunsetModeStartHour) {
    int totalSeconds = minute * 60 + second;
    int transitionPoint = sunsetPeakMinute * 60;
    float orangeIntensity = min(1.0, (float)totalSeconds / transitionPoint);
    sunsetMode(orangeIntensity);
  } else if (hour >= nightModeStartHour) {
    int totalSeconds = (minute + 60) * 60 + second;
    int transitionPoint = nightPeakMinute * 60;
    float redIntensity = min(1.0, (float)totalSeconds / transitionPoint);
    nightMode(redIntensity);
  }
}

// Grow mode: Flickers between blue and red with randomized movement
void growMode() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int redValue = random(100, 255);  
    int blueValue = random(100, 255);  
    leds[i] = CRGB(redValue, 0, blueValue);
  }
}

// Sunset mode: Ramps up to a given level of orange
void sunsetMode(float intensity) {
  CRGB color = CRGB(intensity * 255, intensity * 128, 0);
  fill_solid(leds, NUM_LEDS, color);
}

// Night mode: Ramps up to a given level of red
void nightMode(float intensity) {
  CRGB color = CRGB(intensity * 255, 0, 0);
  fill_solid(leds, NUM_LEDS, color);
}
