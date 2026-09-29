//!
//! \file           spi.cpp
//! \brief          SPI peripheral control for the FunSAPE AVR8 Library
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2026-06-27
//! \version        24.05
//! \copyright      license
//! \details        SPI peripheral control for the FunSAPE AVR8 Library
//!

// =============================================================================
// System file dependencies
// =============================================================================

#include "spi.hpp"
#if !defined(__SPI_HPP)
#    error "Header file is corrupted!"
#elif __SPI_HPP != 2604
#    error "Version mismatch between source and header files!"
#endif

#include <avr/interrupt.h>
#include <stdlib.h>

// =============================================================================
// File exclusive - Constants
// =============================================================================

cuint8_t    constSpiBufferSizeMax       = 100;
cuint8_t    constSpiBufferSizeMin       = 10;
cuint16_t   constSpiDefaultTimeout      = 20;

#define             SPI_SS_DIR          DDRB
#define             SPI_SS_OUT          PORTB
#define             SPI_SS_IN           PINB
#define             SPI_SS_BIT          PB2

#define             SPI_MOSI_DIR        DDRB
#define             SPI_MOSI_OUT        PORTB
#define             SPI_MOSI_IN         PINB
#define             SPI_MOSI_BIT        PB3

#define             SPI_MISO_DIR        DDRB
#define             SPI_MISO_OUT        PORTB
#define             SPI_MISO_IN         PINB
#define             SPI_MISO_BIT        PB4

#define             SPI_SCLK_DIR        DDRB
#define             SPI_SCLK_OUT        PORTB
#define             SPI_SCLK_IN         PINB
#define             SPI_SCLK_BIT        PB5

// =============================================================================
// File exclusive - New data types
// =============================================================================

// NONE

// =============================================================================
// File exclusive - Global variables
// =============================================================================

Spi spi;

// =============================================================================
// File exclusive - Macro-functions
// =============================================================================

// NONE

// =============================================================================
// Class constructors
// =============================================================================

Spi::Spi(void)
{
    // Reset data members
    this->_bufferData                   = nullptr;
    this->_bufferIndex                  = 0;
    this->_bufferLength                 = 0;
    this->_bufferMaxSize                = 0;
    this->_csPin                        = nullptr;
    this->_isDeviceSelected             = false;
    this->_isInitialized                = false;
    this->_useGpioToSelectDevice        = false;
    this->_timeout                      = constSpiDefaultTimeout;
    this->_spiError                     = ReceptionError::NONE;
    this->_csActivateFunc               = nullptr;
    this->_csDeactivateFunc             = nullptr;

    // Returns successfully
    this->_lastError = Error::NONE;
    return;
}

Spi::~Spi(void)
{
    // Returns successfully
    return;
}

// =============================================================================
// Class public methods - Inhirited methods
// =============================================================================

Bus::BusType Spi::getBusType(void)
{
    // Returns bus type
    return Bus::BusType::SPI;
}

// =============================================================================
// Class public methods - Own methods
// =============================================================================

bool_t Spi::init(cuint8_t bufferSize_p)
{
    // Local variables
    volatile ignored uint8_t dummyRead = 0;

    // Allocates memory (Free first if already allocated)
    if(isPointerValid(this->_bufferData)) {
        free(this->_bufferData);
        this->_bufferData = nullptr;
    }

    // Allocates dynamic memory for SPI communication buffer
    this->_bufferData = (uint8_t *)calloc(bufferSize_p, sizeof(uint8_t));
    if(!isPointerValid(this->_bufferData)) {
        // Returns error
        this->_lastError = Error::MEMORY_ALLOCATION_FAILED;
        return false;
    }

    // Initialize buffer control variables
    this->_bufferMaxSize        = bufferSize_p;
    this->_bufferIndex          = 0;
    this->_bufferLength         = 0;

    // Configure hardware GPIO pins using preprocessor definitions
    setBit(SPI_SS_DIR, SPI_SS_BIT);                     // Force SS as output to guarantee Master mode
    setBit(SPI_MOSI_DIR, SPI_MOSI_BIT);                 // Set MOSI as output
    setBit(SPI_SCLK_DIR, SPI_SCLK_BIT);                 // Set SCLK as output
    clrBit(SPI_MISO_DIR, SPI_MISO_BIT);                 // Set MISO as input

    // Optional: Enable internal pull-up on MISO line to prevent floating states
    setBit(SPI_MISO_OUT, SPI_MISO_BIT);

    // Configure SPI Control Register (SPCR) to default Master state
    // Enable SPI, Set Master, Mode 0, MSB First, Clock fck/4
    this->enable();
    this->setMasterMode();

    // Clear SPIF and WCOL flags by reading SPSR and SPDR
    dummyRead = SPSR;
    dummyRead = SPDR;

    // Update internal status properties
    this->_spiError = ReceptionError::NONE;
    this->_isInitialized = true;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::setMode(const Mode mode_p)
{
    // Set new mode using the enum values (Mode 0 to 3 directly map to bit logic)
    clrMaskOffset(SPCR, 0x03, CPHA);
    setMaskOffset(SPCR, (uint8_t)mode_p, CPHA);

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::setClockPrescaler(const ClockPrescaler clockPrescaler_p)
{
    // Clear SPR1 and SPR0 in SPCR, and SPI2X in SPSR
    clrMaskOffset(SPCR, 0x03, SPR0);
    clrBit(SPSR, SPI2X);

    // Evaluate new clock prescaler mask
    switch(clockPrescaler_p) {
    case ClockPrescaler::PRESCALER_2:
        setBit(SPSR, SPI2X);
        break;
    case ClockPrescaler::PRESCALER_4:
        break;
    case ClockPrescaler::PRESCALER_8:
        setBit(SPSR, SPI2X);
        setBit(SPCR, SPR0);
        break;
    case ClockPrescaler::PRESCALER_16:
        setBit(SPCR, SPR0);
        break;
    case ClockPrescaler::PRESCALER_32:
        setBit(SPSR, SPI2X);
        setBit(SPCR, SPR1);
        break;
    case ClockPrescaler::PRESCALER_64:
        setBit(SPCR, SPR1);
        break;
    case ClockPrescaler::PRESCALER_128:
        setBit(SPCR, SPR1);
        setBit(SPCR, SPR0);
        break;
    default:
        // Returns error
        this->_lastError = Error::ARGUMENT_VALUE_INVALID;
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::setDataOrder(const DataOrder dataOrder_p)
{
    if(dataOrder_p == DataOrder::LSB_FIRST) {
        setBit(SPCR, DORD);
    } else {
        clrBit(SPCR, DORD);
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::getReceptionErrors(ReceptionError *errorCode_p)
{
    // Check for pointer validity
    if(!isPointerValid(errorCode_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Assign internal error status to pointer
    *errorCode_p = this->_spiError;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::sendData(uint8_t *buffData_p, cuint16_t buffSize_p)
{
    // Check for initialization
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for errors - Device not selected
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for pointer validity
    if(!isPointerValid(buffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for valid size
    if(buffSize_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        return false;
    }

    // Check if requested size exceeds allocated internal buffer
    if(buffSize_p > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Copy external data to internal buffer
    memcpy(this->_bufferData, buffData_p, buffSize_p);
    this->_bufferLength = (uint8_t)buffSize_p;

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Loop through internal buffer and transfer each byte in-place
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            // Returns error
            this->_deselectDevice();
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Copy received data back to the external buffer
    memcpy(buffData_p, this->_bufferData, buffSize_p);

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::sendData(cuint8_t *txBuffData_p, uint8_t *rxBuffData_p, cuint16_t buffSize_p)
{
    // Check for initialization
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for errors - Device not selected
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for pointers validity
    if(!isPointerValid(txBuffData_p) || !isPointerValid(rxBuffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for valid size
    if(buffSize_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        return false;
    }

    // Check if requested size exceeds allocated internal buffer
    if(buffSize_p > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Copy external TX data to internal buffer
    memcpy(this->_bufferData, txBuffData_p, buffSize_p);
    this->_bufferLength = (uint8_t)buffSize_p;

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Perform transfers using the isolated internal buffer
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            // Returns error
            this->_deselectDevice();
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Copy received data from internal buffer to external RX buffer
    memcpy(rxBuffData_p, this->_bufferData, buffSize_p);

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::write(cuint8_t *buffData_p, cuint16_t buffSize_p)
{
    // Check for initialization
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for errors - Device not selected
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for pointer validity
    if(!isPointerValid(buffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for valid size
    if(buffSize_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        return false;
    }

    // Check if requested size exceeds allocated internal buffer
    if(buffSize_p > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Copy external constant data to internal buffer
    memcpy(this->_bufferData, buffData_p, buffSize_p);
    this->_bufferLength = (uint8_t)buffSize_p;

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Loop through internal buffer and transfer each byte
    // The received data stored back into _bufferData is intentionally ignored
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            // Returns error
            this->_deselectDevice();
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::read(uint8_t *buffData_p, cuint16_t buffSize_p)
{
    // Check for initialization
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for errors - Device not selected
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for pointer validity
    if(!isPointerValid(buffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for valid size
    if(buffSize_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        return false;
    }

    // Check if requested size exceeds allocated internal buffer
    if(buffSize_p > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Update internal state variable
    this->_bufferLength = (uint8_t)buffSize_p;

    // Fill internal buffer with dummy bytes (0xFF) to generate SCLK
    memset(this->_bufferData, 0xFF, this->_bufferLength);

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Loop through internal buffer and transfer each byte
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            // Returns error
            this->_deselectDevice();
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Copy received data from internal buffer to external buffer
    memcpy(buffData_p, this->_bufferData, this->_bufferLength);

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::setDevice(GpioPin *csPin_p)
{
    // Check for pointer validity
    if(!isPointerValid(csPin_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    if(!csPin_p->isInitialized()) {
        // Returns error
        this->_lastError = Error::GPIO_NOT_INITIALIZED;
        return false;
    }

    // Update internal properties for GpioPin usage
    this->_csPin = csPin_p;
    this->_useGpioToSelectDevice = true;
    this->_isDeviceSelected = true;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::setDevice(void (* actFunc_p)(void), void (* deactFunc_p)(void))
{
    // Check for function pointers validity
    if(!isPointerValid(actFunc_p) || !isPointerValid(deactFunc_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Update internal properties for custom callback usage
    this->_csActivateFunc = actFunc_p;
    this->_csDeactivateFunc = deactFunc_p;
    this->_useGpioToSelectDevice = false;
    this->_isDeviceSelected = true;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::writeReg(cuint8_t reg_p, cuint8_t *buffData_p, cuint16_t buffSize_p)
{
    // Check for initialization
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for device selection
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for pointer validity
    if(!isPointerValid(buffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for valid size
    if(buffSize_p == 0) {
        // Returns error
        this->_lastError = Error::ARGUMENT_CANNOT_BE_ZERO;
        return false;
    }

    // Check if requested total size exceeds allocated internal buffer
    if((buffSize_p + 1) > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Set register address with pre-configured write mask
    this->_bufferData[0] = (reg_p & 0x7F) | 0x00;

    // Copy external data to internal buffer starting at index 1 for memory safety
    memcpy(&this->_bufferData[1], buffData_p, buffSize_p);
    this->_bufferLength = (uint8_t)(buffSize_p + 1);

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Loop through internal buffer and transmit data payload
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            // Returns error
            this->_deselectDevice();
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::readReg(cuint8_t reg_p, uint8_t *buffData_p, cuint16_t buffSize_p)
{
    // Check for errors - Not initialized
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Check for errors - Device not selected
    if(!this->_isDeviceSelected) {
        // Returns error
        this->_lastError = Error::DEVICE_NOT_SELECTED;
        return false;
    }

    // Check for errors - Pointer null
    if(!isPointerValid(buffData_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Check for errors - Size exceeds buffer
    if((buffSize_p + 1) > this->_bufferMaxSize) {
        // Returns error
        this->_lastError = Error::BUFFER_SIZE_TOO_LARGE;
        return false;
    }

    // Set register address
    this->_bufferData[0] = (reg_p & 0x7F) | 0x80;
    memset(&this->_bufferData[1], 0xFF, buffSize_p);
    this->_bufferLength = (uint8_t)(buffSize_p + 1);

    // Assert Chip Select to initiate communication frame
    this->_selectDevice();

    // Loop through internal buffer to generate clock and sample incoming data
    this->_bufferIndex = 0;
    for(; this->_bufferIndex < this->_bufferLength; this->_bufferIndex++) {
        if(!this->_transferByte(&(this->_bufferData[this->_bufferIndex]))) {
            this->_deselectDevice();
            // Returns error
            return false;
        }
    }

    // Deassert Chip Select to terminate communication frame
    this->_deselectDevice();

    // Copy sampled data from internal buffer to external buffer
    memcpy(buffData_p, &this->_bufferData[1], buffSize_p);

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

// =============================================================================
// Class private methods
// =============================================================================

bool_t Spi::_waitWhileIsBusy(void)
{
// Local variables
    uint32_t stopwatchMark              = 0;
    uint32_t stopwatchDeadline          = 0;

    // Evaluates stopwatch deadline
    if(this->_timeout == 0) {
        stopwatchDeadline = 0xFFFFFFFF;
    } else {
        systemStatus.getStopwatchValue(&stopwatchDeadline);
        stopwatchDeadline += this->_timeout;
    }

    // Wait until SPI interrupt flag (SPIF) is set
    while(!this->isTransferComplete()) {
        stopwatchMark = 0;
        systemStatus.getStopwatchValue(&stopwatchMark);
        if(stopwatchMark > stopwatchDeadline) {
            // Returns error
            this->_lastError = Error::TIMED_OUT;
            return false;
        }
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Spi::_transferByte(uint8_t *data_p)
{
    // Check for errors - Pointer null
    if(!isPointerValid(data_p)) {
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Loads data into the SPI Data Register to start transmission
    SPDR = *data_p;

    // Waits for the transmission to complete
    if(!this->_waitWhileIsBusy()) {
        // Returns error (timeout already set in _waitWhileIsBusy)
        return false;
    }

    // Check for Write Collision Flag (WCOL)
    if(isBitSet(SPSR, WCOL)) {
        this->_spiError = ReceptionError::DATA_OVERRUN_ERROR;
    }

    // Reads the received data from the buffer
    *data_p = SPDR;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

// =============================================================================
// Class protected methods
// =============================================================================

// NONE

// =============================================================================
// General public functions definitions
// =============================================================================

// NONE

// =============================================================================
// Interrupt callback functions
// =============================================================================

void Spi::interruptHandler(void)
{

}

// =============================================================================
// Interrupt handlers
// =============================================================================

ISR(SPI_STC_vect)
{
    spi.interruptHandler();
}

// =============================================================================
// END OF FILE
// =============================================================================
