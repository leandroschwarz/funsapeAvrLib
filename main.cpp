// =============================================================================
// Project:         Temperature LCD PWM (Recceiver Module)
// File:            main.cpp
// Author:          Leandro Schwarz
// Created:         2026-05-04
// Modified:        2024-05-04
// Version:         1.0
// Notes:           This project implements the data receiver module. The
//                      receiver reads the transmitted ADC by counting the time
//                      in HIGH and the time in LOW of the PWM on the Input
//                      Capture pin. The value is converted to temperature in
//                      Celsius degrees and is shown on the LCD.
// =============================================================================

// =============================================================================
// Precompiler constant defintions
// =============================================================================

#define F_CPU 16000000UL

// =============================================================================
// Dependencies
// =============================================================================

#include "funsape/globalDefines.hpp"
#include "funsape/peripheral/gpioBus.hpp"
#include "funsape/peripheral/gpioPin.hpp"
#include "funsape/device/hd44780.hpp"
// #include "funsape/device/keypad.hpp"
#include "funsape/peripheral/usart0.hpp"
#include "funsape/peripheral/int0.hpp"
#include "funsape/peripheral/twi.hpp"
#include "funsape/peripheral/spi.hpp"
#include "funsape/device/ds1307.hpp"
#include "funsape/device/tc72.hpp"

// =============================================================================
// Constant definitions
// =============================================================================

// NONE

// =============================================================================
// New data types
// =============================================================================

GpioPin gpioTempSensor;

// =============================================================================
// Static function declarations
// =============================================================================

// void processKey(cuint8_t key_p);
void csActivate(void);
void csDisable(void);

// =============================================================================
// Global variables
// =============================================================================

vbool_t readRtc = false;

// =============================================================================
// Main function
// =============================================================================

int main()
{
    // -------------------------------------------------------------------------
    // Local variables declaration and initialization
    // -------------------------------------------------------------------------

    Hd44780 lcd;
    // Keypad keypad;
    // GpioBus gpioKeypadRows;
    // GpioBus gpioKeypadCols;
    GpioBus gpioLcdData;
    GpioPin gpioLcdEn;
    GpioPin gpioLcdRs;
    // uint8_t auxKey = 0xFF;
    Ds1307 rtc;
    uint16_t rtcYear = 0;
    uint8_t rtcMonth = 0;
    uint8_t rtcMonthDay = 0;
    uint8_t rtcWeekDay = 0;
    uint8_t rtcHour = 0;
    uint8_t rtcMinute = 0;
    uint8_t rtcSecond = 0;
    Tc72 sensor;

    // -------------------------------------------------------------------------
    // USART configuration
    // -------------------------------------------------------------------------

    // usart0.setBaudRate(Usart0::BaudRate::BAUD_RATE_57600);
    // usart0.enableTransmitter();
    // usart0.init();
    // usart0.stdio();
    // printf("Foi\r");

    // -------------------------------------------------------------------------
    // GPIO configuration
    // -------------------------------------------------------------------------

    // gpioKeypadCols.init(&DDRD, PinIndex::P4, 4);
    // gpioKeypadRows.init(&DDRC, PinIndex::P0, 4);
    gpioLcdData.init(&DDRD, PinIndex::P4, 4);
    gpioLcdEn.init(&DDRC, PinIndex::P0);
    gpioLcdRs.init(&DDRC, PinIndex::P1);

    // -------------------------------------------------------------------------
    // LCD configuration
    // -------------------------------------------------------------------------

    lcd.setControlPort(&gpioLcdEn, &gpioLcdRs);
    lcd.setDataPort(&gpioLcdData);
    lcd.init(Hd44780::Size::LCD_16X2);


    // -------------------------------------------------------------------------
    // Keypad configuration
    // -------------------------------------------------------------------------

    // keypad.setPorts(&gpioKeypadRows, &gpioKeypadCols);
    // keypad.setKeyValues(
    //         Keypad::Type::KEYPAD_4X4,
    //         0x07, 0x08, 0x09, 0x0A,
    //         0x04, 0x05, 0x06, 0x0B,
    //         0x01, 0x02, 0x03, 0x0C,
    //         0x0E, 0x00, 0x0F, 0x0D
    // );
    // keypad.setCallbackFunction(processKey);
    // keypad.init(10);

    int0.init(Int0::SenseMode::FALLING_EDGE);
    int0.setPinMode(PinMode::INPUT_PULLED_UP);
    int0.clearInterruptRequest();
    int0.activateInterrupt();

    // -------------------------------------------------------------------------
    // Enables Global Interrupt
    // -------------------------------------------------------------------------

    sei();

    twi.init(100'000UL);
    rtc.init(&twi);
    readRtc = true;

    spi.setClockPrescaler(Spi::ClockPrescaler::PRESCALER_128);
    spi.setMasterMode();
    spi.enable();
    spi.setMode(Spi::Mode::MODE_1);
    spi.init();
    gpioTempSensor.init(&DDRB, PinIndex::P2);
    gpioTempSensor.setMode(PinMode::OUTPUT_PUSH_PULL);
    gpioTempSensor.low();

    sensor.init(&spi, csActivate, csDisable);
    sensor.startContinuousConversion();

    // -------------------------------------------------------------------------
    // Main loop
    // -------------------------------------------------------------------------

    while(1) {
        if(readRtc) {
            rtc.setSquareWaveGenerator(Ds1307::SquareWave::CLOCK_1_HZ);
            rtc.getDate(&rtcYear, &rtcMonth, &rtcMonthDay, &rtcWeekDay);
            rtc.getTime(&rtcHour, &rtcMinute, &rtcSecond);
            int16_t temp = 0;
            sensor.readTemperature(&temp);
            lcd.stdio();
            printf("%02u/%02u/%04u %u\n%02u:%02u:%02u %d\n",
                    rtcMonthDay, rtcMonth, rtcYear, rtcWeekDay,
                    rtcHour, rtcMinute, rtcSecond, temp
            );
            readRtc = false;
        }

    }

    return 0;
}

// =============================================================================
// Static function definitions
// =============================================================================

void csActivate(void)
{
    gpioTempSensor.high();
}

void csDisable(void)
{
    gpioTempSensor.low();
}


// =============================================================================
// Interrupt callback functions definitions
// =============================================================================

void int0InterruptCallback(void)
{
    readRtc = true;
}

// =============================================================================
// End of file (main.cpp)
// =============================================================================
