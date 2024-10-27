#include <FastLED.h>

#define LED_PIN     6       
#define NUM_LEDS    25      
#define BRIGHTNESS  100     
#define LED_TYPE    WS2812B 
#define COLOR_ORDER GRB     
#define EFFECT_DURATION 10000 

CRGB leds[NUM_LEDS];
enum Effects { RAINBOW, COLOR_WIPE, BPM, SINELON, NUM_EFFECTS };
int currentEffect = 0;       
unsigned long lastChangeTime = 0;  

void setup() {
  Serial.begin(115200);  // Start serial communication
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  if (millis() - lastChangeTime > EFFECT_DURATION) {
    currentEffect = (currentEffect + 1) % NUM_EFFECTS;  
    lastChangeTime = millis();
  }

  switch (currentEffect) {
    case RAINBOW:
      rainbowEffect();
      break;
    case COLOR_WIPE:
      colorWipeEffect(CRGB::Blue);  
      break;
    case BPM:
      bpmEffect();
      break;
    case SINELON:
      sinelonEffect();
      break;
  }

  // Send LED data to serial
  for (int i = 0; i < NUM_LEDS; i++) {
    Serial.print(leds[i].r); Serial.print(",");
    Serial.print(leds[i].g); Serial.print(",");
    Serial.print(leds[i].b);
    if (i < NUM_LEDS - 1) Serial.print("|");  // Separator for each LED
  }
  Serial.println();  // End the line to mark end of LED data
  delay(20);       
}

void rainbowEffect() {
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 7);
  hue++;
}

void colorWipeEffect(CRGB color) {
  static int ledIndex = 0;  
  leds[ledIndex] = color;
  ledIndex = (ledIndex + 1) % NUM_LEDS;
  if (ledIndex == 0) {
    FastLED.clear();
  }
}

void bpmEffect() {
  uint8_t beat = beatsin8(62, 64, 255);  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(beat - (i * 2), 255, 255);  
  }
}

void sinelonEffect() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(200, 255, 192);  
}