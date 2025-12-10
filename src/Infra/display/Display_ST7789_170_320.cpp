#include "Display_ST7789_170_320.h"
#include "Infra/Utils/Logger.h"
#if defined(ST7789_170x320_ENABLED)

Adafruit_ST7789 lcd = Adafruit_ST7789(LCD_CS, LCD_DC, LCD_RST);
unsigned int totalLines = 0;
unsigned int actualLine = 0;
int totalLinesPossibleInDisplay = 9; // (LCD_HEIGHT / (6 * LCD_TEXTSIZE)) - 1;
void setup_ST7789_170x320()
{
  delay(1000);

  lcd.init(LCD_WIDTH, LCD_HEIGHT);
  lcd.fillScreen(ST77XX_BLACK);
  lcd.setRotation(LCD_ROTATION);
  lcd.setTextSize(LCD_TEXTSIZE);
  lcd.setTextColor(ST77XX_WHITE);

  lcd.setCursor(0, 0);
}

void moveDisplayUp()
{
  if (actualLine >= totalLinesPossibleInDisplay)
  {
    lcd.fillScreen(ST77XX_BLACK);
    actualLine = 0;
  }
  else
  {
    actualLine++;
  }
  totalLines++;
  lcd.setCursor(0, actualLine * (6 * 3));
}

void displayMessage(const String &message)
{
  lcd.print(message);
  LOG_INFO(message.c_str());
  moveDisplayUp();
}

void displayError(const String &message)
{
  lcd.setTextColor(ST77XX_RED);
  lcd.print("ERROR: " + message);
  LOG_ERROR(message.c_str());
  lcd.setTextColor(ST77XX_WHITE);
  moveDisplayUp();
}
void displayInfo(const String &message)
{
  lcd.setTextColor(ST77XX_GREEN);
  lcd.print("INFO: " + message);
  LOG_INFO(message.c_str());
  lcd.setTextColor(ST77XX_WHITE);
  moveDisplayUp();
}
void displayWarning(const String &message)
{
  lcd.setTextColor(ST77XX_YELLOW);
  lcd.print("WARN: " + message);
  LOG_WARN(message.c_str());
  lcd.setTextColor(ST77XX_WHITE);
  moveDisplayUp();
}

#endif

/**
 Adafruit_ST7789 lcd = Adafruit_ST7789(LCD_CS, LCD_DC, LCD_RST);

void setup()
{
  lcd.init(LCD_WIDTH, LCD_HEIGHT);
  lcd.fillScreen(ST77XX_BLACK);
  lcd.setRotation(LCD_ROTATION);
  lcd.setTextSize(LCD_TEXTSIZE);
  lcd.setTextColor(ST77XX_RED);
  // lcd.println("Iteração: " + String(count++));
  lcd.fillCircle(50, 50, 20, ST77XX_BLUE);
  // lcd.fillCircle(150, 50, 20, ST77XX_GREEN);
  // lcd.fillCircle(100, 100, 20, ST77XX_YELLOW);
  // lcd.fillEllipse(100, 150, 60, 15, ST77XX_RED);
  // lcd.drawRect(10, 10, 220, 220, ST77XX_WHITE);
  lcd.drawLine(10, 100, 10, 24, ST77XX_WHITE);
  // lcd.drawLine(240, 0, 0, 240, ST77XX_WHITE);
  // lcd.drawTriangle(60, 200, 120, 170, 180, 200, ST77XX_MAGENTA);
  // lcd.drawRoundRect(70, 70, 100, 50, 10, ST77XX_ORANGE);
  lcd.setCursor(0, 0);
}

 */