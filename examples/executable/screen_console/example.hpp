#pragma once

#include <Arduino.h>

#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/ServiceProvider.h"
#include "Platform/Arduino/Display/ST7789ScreenConsole.h"
#include "Platform/Arduino/Logging/ScreenMirrorLogger.h"
#include "SmartSysApp.h"

#ifndef IOTSMARTSYS_SCREEN_CONSOLE_ENABLED
#error "screen_console requires IOTSMARTSYS_SCREEN_CONSOLE_ENABLED"
#elif !IOTSMARTSYS_SCREEN_CONSOLE_ENABLED
#error "screen_console requires IOTSMARTSYS_SCREEN_CONSOLE_ENABLED=1"
#endif

#ifndef EXAMPLE_BOARD_ID
#error "screen_console requires EXAMPLE_BOARD_ID"
#endif

#if !defined(EXAMPLE_SCREEN_CS_PIN) || !defined(EXAMPLE_SCREEN_DC_PIN) || \
    !defined(EXAMPLE_SCREEN_RESET_PIN) || !defined(EXAMPLE_SCREEN_CLOCK_PIN) || \
    !defined(EXAMPLE_SCREEN_MOSI_PIN) || !defined(EXAMPLE_SCREEN_BACKLIGHT_PIN)
#error "screen_console requires all EXAMPLE_SCREEN_*_PIN macros"
#endif

#if !defined(EXAMPLE_SCREEN_NATIVE_WIDTH) || !defined(EXAMPLE_SCREEN_NATIVE_HEIGHT)
#error "screen_console requires EXAMPLE_SCREEN_NATIVE_WIDTH and EXAMPLE_SCREEN_NATIVE_HEIGHT"
#endif

namespace executable_example
{
    static iotsmartsys::SmartSysApp app;
}

void setup()
{
    Serial.begin(115200);
    executable_example::app.setup();

    const iotsmartsys::platform::arduino::ST7789ScreenConsoleConfig screenConfig{
        EXAMPLE_SCREEN_CS_PIN,
        EXAMPLE_SCREEN_DC_PIN,
        EXAMPLE_SCREEN_RESET_PIN,
        EXAMPLE_SCREEN_CLOCK_PIN,
        EXAMPLE_SCREEN_MOSI_PIN,
        EXAMPLE_SCREEN_BACKLIGHT_PIN,
        true,
        EXAMPLE_SCREEN_NATIVE_WIDTH,
        EXAMPLE_SCREEN_NATIVE_HEIGHT,
        0,
        1,
        2,
        1,
        ST77XX_BLACK,
        ST77XX_WHITE};

    static iotsmartsys::platform::arduino::ST7789ScreenConsole console(screenConfig);
    iotsmartsys::core::ServiceProvider &services = iotsmartsys::core::ServiceProvider::instance();
    services.setScreenConsole(&console);
    console.begin();

    static iotsmartsys::platform::arduino::ScreenMirrorLogger mirror(
        iotsmartsys::core::Log::get(), console, iotsmartsys::core::LogLevel::Info);
    services.setLogger(&mirror);
    iotsmartsys::core::Log::setLogger(&mirror);

    iotsmartsys::core::Log::get().info(
        "screen_console",
        "id=screen_console board=%s display=ST7789 dimensions=%dx%d "
        "cs=%d dc=%d rst=%d sclk=%d mosi=%d backlight=%d",
        EXAMPLE_BOARD_ID,
        EXAMPLE_SCREEN_NATIVE_WIDTH,
        EXAMPLE_SCREEN_NATIVE_HEIGHT,
        EXAMPLE_SCREEN_CS_PIN,
        EXAMPLE_SCREEN_DC_PIN,
        EXAMPLE_SCREEN_RESET_PIN,
        EXAMPLE_SCREEN_CLOCK_PIN,
        EXAMPLE_SCREEN_MOSI_PIN,
        EXAMPLE_SCREEN_BACKLIGHT_PIN);
}

void loop()
{
    executable_example::app.handle();
}
