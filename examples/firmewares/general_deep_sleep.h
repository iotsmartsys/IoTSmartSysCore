#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define LED_BUILTIN 2  // On most ESP32-WROOM dev boards, onboard LED is GPIO2

#define BUTTON_PIN 33 // ESP32-WROOM: use an RTC-capable GPIO (good choices: 33, 32, 25-27, 34-39 [inputs only])

RTC_DATA_ATTR int bootCount = 0;
unsigned long lastPressTime = 0;
unsigned long multiClickTimeout = 400;
int clickCount = 0;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason()
{
    esp_sleep_wakeup_cause_t wakeup_reason;

    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        Serial.println("Wakeup caused by external signal using RTC_IO");
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        Serial.println("Wakeup caused by external signal using RTC_CNTL");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        Serial.println("Wakeup caused by timer");
        break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        Serial.println("Wakeup caused by touchpad");
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        Serial.println("Wakeup caused by ULP program");
        break;
    default:
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting ESP32 (WROOM) GPIO Wakeup example");
    // Increment boot number and print it every reboot
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // Ensure RTC domain sees a stable high (so LOW press can wake via EXT0)
    rtc_gpio_pullup_en((gpio_num_t)BUTTON_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t)BUTTON_PIN);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    // delay(250);

    Serial.println("Turning off the LED");
    print_wakeup_reason();
    Serial.println("Enabling EXT0 wakeup on pin " + String(BUTTON_PIN) + " (LOW to wake)");
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0); // 0 = wake on LOW
    Serial.println("Going to sleep in 1 seconds");
    // delay(250);

    // esp_deep_sleep_start();

}

void loop()
{
    static bool lastState = HIGH;
    bool currentState = digitalRead(BUTTON_PIN);

    if (lastState == HIGH && currentState == LOW)
    {
        unsigned long now = millis();

        if (now - lastPressTime < multiClickTimeout)
        {
            clickCount++;
        }
        else
        {
            clickCount = 1;
        }

        lastPressTime = now;
    }

    if (clickCount > 0 && (millis() - lastPressTime > multiClickTimeout))
    {
        Serial.printf("👉 pressed_x%d\n", clickCount);
        
        digitalWrite(LED_BUILTIN, HIGH);
        delay(5000);
        Serial.printf("Going to sleep now com pressed_x%d\n", clickCount);
        clickCount = 0;
        esp_deep_sleep_start();
    }

    lastState = currentState;
}

/*
#define BUTTON_PIN 7 // D5

unsigned long lastPressTime = 0;
unsigned long multiClickTimeout = 400;
int clickCount = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop()
{
    static bool lastState = HIGH;
    bool currentState = digitalRead(BUTTON_PIN);

    if (lastState == HIGH && currentState == LOW)
    {
        unsigned long now = millis();

        if (now - lastPressTime < multiClickTimeout)
        {
            clickCount++;
        }
        else
        {
            clickCount = 1;
        }

        lastPressTime = now;
    }

    if (clickCount > 0 && (millis() - lastPressTime > multiClickTimeout))
    {

        Serial.printf("👉 pressed_x%d", clickCount);
        clickCount = 0;
    }

    lastState = currentState;
}
    */