#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ── TFT pins ──────────────────────────────────────────────
#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ── Soil Moisture (A1, powered by D7) ─────────────────────
int soilPin   = A1;
int soilPower = 7;

int thresholdUp   = 400;
int thresholdDown = 250;

// ── Pressure Sensor (A0, always on) ───────────────────────
int pressurePin = A0;

// ── Plot settings ─────────────────────────────────────────
int plotWidth  = 320;
int plotHeight = 200;
int yOffset    = 28;   // slightly taller top bar to fit two labels
int xPos       = 0;

// ── Colors ────────────────────────────────────────────────
#define COLOR_WET      ILI9341_GREEN
#define COLOR_MID      ILI9341_GREEN
#define COLOR_DRY      ILI9341_RED
#define COLOR_PRESSURE ILI9341_YELLOW

// ── Helpers ───────────────────────────────────────────────
int readSoil() {
  digitalWrite(soilPower, HIGH);
  delay(10);
  int val = analogRead(soilPin);
  digitalWrite(soilPower, LOW);
  return val;
}

uint16_t moistureColor(int val) {
  if (val <= thresholdDown) return COLOR_DRY;
  if (val >= thresholdUp)   return COLOR_WET;
  return COLOR_MID;
}

void drawStatus(int soilValue, int pressureValue) {
  tft.fillRect(0, 0, plotWidth, yOffset - 2, ILI9341_BLACK);

  // ── Line 1: Soil moisture ──
  tft.setTextSize(1);
  tft.setCursor(4, 2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.print("Soil: ");
  tft.print(soilValue);
  tft.print("  ");

  if (soilValue <= thresholdDown) {
    tft.setTextColor(COLOR_DRY, ILI9341_BLACK);
    tft.print("Dry - Water it!");
  } else if (soilValue >= thresholdUp) {
    tft.setTextColor(COLOR_WET, ILI9341_BLACK);
    tft.print("Wet - Leave it!");
  } else {
    tft.setTextColor(COLOR_MID, ILI9341_BLACK);
    tft.print("OK");
  }

  // ── Line 2: Pressure ──
  tft.setCursor(4, 12);
  tft.setTextColor(COLOR_PRESSURE, ILI9341_BLACK);
  tft.print("Pressure: ");
  tft.print(pressureValue);
}

void drawGuideLines() {
  int yUp   = map(thresholdUp,   0, 1023, yOffset + plotHeight - 1, yOffset);
  int yDown = map(thresholdDown, 0, 1023, yOffset + plotHeight - 1, yOffset);
  tft.drawFastHLine(1, yUp,   plotWidth - 2, ILI9341_DARKGREY);
  tft.drawFastHLine(1, yDown, plotWidth - 2, ILI9341_DARKGREY);
}

// ── Setup ─────────────────────────────────────────────────
void setup() {
  pinMode(soilPower, OUTPUT);
  digitalWrite(soilPower, LOW);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.drawRect(0, yOffset - 1, plotWidth, plotHeight + 2, ILI9341_WHITE);
  drawGuideLines();
}

// ── Loop ──────────────────────────────────────────────────
void loop() {
  int soilValue     = readSoil();
  int pressureValue = analogRead(pressurePin);

  // Update status bar with both readings
  drawStatus(soilValue, pressureValue);

  // Plot soil moisture (A1) — colour-coded
  int y1 = map(soilValue,     0, 1023, yOffset + plotHeight - 1, yOffset);
  tft.drawPixel(xPos, y1, moistureColor(soilValue));

  // Plot pressure (A0) — yellow
  int y2 = map(pressureValue, 0, 1023, yOffset + plotHeight - 1, yOffset);
  tft.drawPixel(xPos, y2, COLOR_PRESSURE);

  // Advance scroll position
  xPos++;
  if (xPos >= plotWidth) {
    xPos = 0;
    tft.fillRect(1, yOffset, plotWidth - 2, plotHeight, ILI9341_BLACK);
    drawGuideLines();
  }

  delay(50);  // ~20 Hz
}