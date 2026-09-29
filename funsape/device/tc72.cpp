//!
//! \file           funsapeLibTc72.cpp
//! \brief          TC72 Temperature sensor module interface for the FunSAPE
//!                     AVR8 Library
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2026-06-29
//! \version        26.07
//! \copyright      license
//! \details        TC72 Temperature sensor module interface for the FunSAPE
//!                     AVR8 Library
//! \todo           Todo list
//!

// =============================================================================
// Dependencies
// =============================================================================

#include "tc72.hpp"
#ifndef __TC72_HPP
#    error "Header file is corrupted!"
#elif __TC72_HPP != 2604
#    error "Version mismatch between source and header files!"
#endif

// =============================================================================
// File exclusive - Constants
// =============================================================================

// NONE

// =============================================================================
// File exclusive - New data types
// =============================================================================

// NONE

// =============================================================================
// File exclusive - Macro-functions
// =============================================================================

// NONE

// =============================================================================
// Global variables
// =============================================================================

// NONE

// =============================================================================
// Static functions declarations
// =============================================================================

// NONE

// =============================================================================
// Class constructors
// =============================================================================

Tc72::Tc72(void)
{
    // Reset data members
    this->_isInitialized                = false;
    this->_useGpioToSelectDevice        = false;
    this->_busHandler                   = nullptr;
    this->_csPin                        = nullptr;
    this->_activationFunction           = nullptr;
    this->_deactivationFunction         = nullptr;

    // Returns successfully
    this->_lastError = Error::NONE;
    return;
}

Tc72::~Tc72(void)
{
    // Returns successfully
    return;
}

// =============================================================================
// Class own methods - Public
// =============================================================================

bool_t Tc72::init(Bus *spiBus_p, GpioPin *csPin_p)
{
    // Local variables
    uint8_t auxBuffer[2] = {0};

    // Reset data members
    this->_isInitialized                = false;
    this->_activationFunction           = nullptr;
    this->_deactivationFunction         = nullptr;
    this->_csPin                        = nullptr;

    // Checks for error - Handler pointer invalid
    if(!isPointerValid(spiBus_p)) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_POINTER_NULL;
        return false;
    }
    // Checks for error - Handler type unsupported
    if(spiBus_p->getBusType() != Bus::BusType::SPI) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_NOT_SUPPORTED;
        return false;
    }
    // Checks for error - GpioPin pointer invalid
    if(!isPointerValid(csPin_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }
    // Checks for error - GpioPin not initialized
    if(!csPin_p->isInitialized()) {
        // Returns error
        this->_lastError = Error::GPIO_NOT_INITIALIZED;
        return false;
    }

    // Configures GpioPin
    csPin_p->low();
    csPin_p->setMode(PinMode::OUTPUT_PUSH_PULL);

    // Checks manufacturer's id
    if(!spiBus_p->setDevice(csPin_p)) {
        // Returns error
        this->_lastError = spiBus_p->getLastError();
        return false;
    }
    auxBuffer[0] = (uint8_t)Register::MANUFACTURER_ID | readMask;
    auxBuffer[1] = dummyByte;
    if(!spiBus_p->sendData(auxBuffer, 2)) {
        // Returns error
        this->_lastError = spiBus_p->getLastError();
        return false;
    }
    if(auxBuffer[1] != manufacturerIdValue) {
        // Returns error
        this->_lastError = Error::DEVICE_ID_MATCH_FAILED;
        return false;
    }

// Updates data members
    this->_busHandler                   = spiBus_p;
    this->_csPin                        = csPin_p;
    this->_isInitialized                = true;
    this->_useGpioToSelectDevice        = true;

// Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Tc72::init(Bus *spiBus_p, void (*activateCs_p)(void), void (*deactivateCs_p)(void))
{
    // Local variables
    uint8_t auxBuffer[2] = {0};

    // Reset data members
    this->_isInitialized                = false;
    this->_activationFunction           = nullptr;
    this->_deactivationFunction         = nullptr;
    this->_csPin                        = nullptr;

    // Checks for error - Handler pointer invalid
    if(!isPointerValid(spiBus_p)) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_POINTER_NULL;
        return false;
    }
    // Checks for error - Handler type unsupported
    if(spiBus_p->getBusType() != Bus::BusType::SPI) {
        // Returns error
        this->_lastError = Error::BUS_HANDLER_NOT_SUPPORTED;
        return false;
    }
    // Checks for error - Function pointer invalid
    if((!isPointerValid(activateCs_p)) || (!isPointerValid(deactivateCs_p))) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Checks manufacturer's id
    if(!spiBus_p->setDevice(activateCs_p, deactivateCs_p)) {
        // Returns error
        this->_lastError = spiBus_p->getLastError();
        return false;
    }
    auxBuffer[0] = (uint8_t)Register::MANUFACTURER_ID | readMask;
    auxBuffer[1] = dummyByte;
    if(!spiBus_p->sendData(auxBuffer, 2)) {
        // Returns error
        this->_lastError = spiBus_p->getLastError();
        return false;
    }
    if(auxBuffer[1] != manufacturerIdValue) {
        // Returns error
        this->_lastError = Error::DEVICE_ID_MATCH_FAILED;
        return false;
    }

    // Updates data members
    this->_busHandler                   = spiBus_p;
    this->_activationFunction           = activateCs_p;
    this->_deactivationFunction         = deactivateCs_p;
    this->_isInitialized                = true;
    this->_useGpioToSelectDevice        = false;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Tc72::readTemperature(int16_t *temperature_p)
{
    // Local variables
    uint8_t auxBuffer[3]    = {0};
    bool_t  auxBool         = false;

    // Checks for error - Not initialized
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Checks for error - Handler pointer invalid
    if(!isPointerValid(temperature_p)) {
        // Returns error
        this->_lastError = Error::ARGUMENT_POINTER_NULL;
        return false;
    }

    // Selects device
    if(this->_useGpioToSelectDevice) {
        auxBool = this->_busHandler->setDevice(this->_csPin);
    } else {
        auxBool = this->_busHandler->setDevice(this->_activationFunction, this->_deactivationFunction);
    }
    if(!auxBool) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Sends data to device
    auxBuffer[0] = (uint8_t)Register::TEMP_MSB | readMask;
    auxBuffer[1] = dummyByte;
    auxBuffer[2] = dummyByte;
    if(!this->_busHandler->sendData(auxBuffer, 3)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Updates argument value
    *temperature_p = (((int16_t)((int8_t)auxBuffer[1]) << 2) | (auxBuffer[2] >> 6)) * 25;

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Tc72::shutdown(void)
{
    // Local variables
    uint8_t auxBuffer[2]    = {0};
    bool_t  auxBool         = false;

    // Checks for error - Not initialized
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Selects device
    if(this->_useGpioToSelectDevice) {
        auxBool = this->_busHandler->setDevice(this->_csPin);
    } else {
        auxBool = this->_busHandler->setDevice(this->_activationFunction, this->_deactivationFunction);
    }
    if(!auxBool) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Prepares data to transmission
    auxBuffer[0] = (uint8_t)Register::CONTROL | writeMask;
    auxBuffer[1] = (uint8_t)Tc72::Mode::SHUTDOWN;
    if(!this->_busHandler->sendData(auxBuffer, 2)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Tc72::startSingleConversion(void)
{
    // Local variables
    uint8_t auxBuffer[2]    = {0};
    bool_t  auxBool         = false;

    // Checks for error - Not initialized
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Selects device
    if(this->_useGpioToSelectDevice) {
        auxBool = this->_busHandler->setDevice(this->_csPin);
    } else {
        auxBool = this->_busHandler->setDevice(this->_activationFunction, this->_deactivationFunction);
    }
    if(!auxBool) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Sends data to device
    auxBuffer[0] = (uint8_t)Register::CONTROL | writeMask;
    auxBuffer[1] = (uint8_t)Tc72::Mode::ONE_SHOT;
    if(!this->_busHandler->sendData(auxBuffer, 2)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

bool_t Tc72::startContinuousConversion(void)
{
    // Local variables
    uint8_t auxBuffer[2]    = {0};
    bool_t  auxBool         = false;

    // Checks for error - Not initialized
    if(!this->_isInitialized) {
        // Returns error
        this->_lastError = Error::NOT_INITIALIZED;
        return false;
    }

    // Selects device
    if(this->_useGpioToSelectDevice) {
        auxBool = this->_busHandler->setDevice(this->_csPin);
    } else {
        auxBool = this->_busHandler->setDevice(this->_activationFunction, this->_deactivationFunction);
    }
    if(!auxBool) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Sends data to device
    auxBuffer[0] = (uint8_t)Register::CONTROL | writeMask;
    auxBuffer[1] = (uint8_t)Tc72::Mode::CONTINUOS;
    if(!this->_busHandler->sendData(auxBuffer, 2)) {
        // Returns error
        this->_lastError = this->_busHandler->getLastError();
        return false;
    }

    // Returns successfully
    this->_lastError = Error::NONE;
    return true;
}

// =============================================================================
// Class own methods - Private
// =============================================================================

// NONE

// =============================================================================
// Class own methods - Protected
// =============================================================================

// NONE

// =============================================================================
// END OF FILE
// =============================================================================
