//!
//! \file           spi.hpp
//! \brief          SPI peripheral control for the FunSAPE AVR8 Library
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2026-06-27
//! \version        24.05
//! \copyright      license
//! \details        SPI peripheral control for the FunSAPE AVR8 Library
//!

// =============================================================================
// Include guard (START)
// =============================================================================

#ifndef __SPI_HPP
#define __SPI_HPP                        2604

// =============================================================================
// Dependencies
// =============================================================================

//     /////////////////     GLOBAL DEFINITIONS FILE    /////////////////     //
#include "../globalDefines.hpp"
#if !defined(__GLOBAL_DEFINES_HPP)
#    error "Global definitions file is corrupted!"
#elif __GLOBAL_DEFINES_HPP != __SPI_HPP
#    error "Version mismatch between file header and global definitions file!"
#endif

//     //////////////////     LIBRARY DEPENDENCIES     //////////////////     //
#include "../util/debug.hpp"
#if !defined(__DEBUG_HPP)
#   error "Header file (debug.hpp) is corrupted!"
#elif __DEBUG_HPP != __SPI_HPP
#   error "Version mismatch between header file and library dependency (debug.hpp)!"
#endif

#include "../util/bus.hpp"
#if !defined(__BUS_HPP)
#   error "Header file (bus.hpp) is corrupted!"
#elif __BUS_HPP != __SPI_HPP
#   error "Version mismatch between header file and library dependency (bus.hpp)!"
#endif

#include "../util/systemStatus.hpp"
#if !defined(__SYSTEM_STATUS_HPP)
#   error "Header file (systemStatus.hpp) is corrupted!"
#elif __SYSTEM_STATUS_HPP != __SPI_HPP
#   error "Version mismatch between header file and library dependency (systemStatus.hpp)!"
#endif

// =============================================================================
// Undefining previous definitions
// =============================================================================

// NONE

// =============================================================================
// Constant definitions
// =============================================================================

// NONE

// =============================================================================
// New data types
// =============================================================================

// NONE

// =============================================================================
// Interrupt callback functions
// =============================================================================

void spiInterruptCallback();

// =============================================================================
// Spi Class
// =============================================================================

//!
//! \brief          Spi class
//! \details        Spi class
//!
class Spi : public Bus
{
    // -------------------------------------------------------------------------
    // New data types ----------------------------------------------------------
public:
    // NONE

    //     ///////////////////     SPI operation     ////////////////////     //
    enum class Mode : uint8_t {
        MODE_0                          = 0,
        MODE_1                          = 1,
        MODE_2                          = 2,
        MODE_3                          = 3
    };

    enum class ClockPrescaler : uint8_t {
        PRESCALER_2                     = 0,
        PRESCALER_4                     = 1,
        PRESCALER_8                     = 2,
        PRESCALER_16                    = 3,
        PRESCALER_32                    = 4,
        PRESCALER_64                    = 5,
        PRESCALER_128                   = 6
    };

    enum class DataOrder : bool_t {
        MSB_FIRST                       = false,
        LSB_FIRST                       = true
    };

    enum class ReceptionError : uint8_t {
        NONE                            = 0,
        DATA_OVERRUN_ERROR              = 1
    };

private:
    // NONE

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Constructors ------------------------------------------------------------
public:
    //!
    //! \brief      Spi class constructor
    //! \details    Creates an Spi object
    //!
    Spi(
            void
    );

    //!
    //! \brief      Spi class destructor
    //! \details    Destroys an Spi object
    //!
    ~Spi(
            void
    );

    // -------------------------------------------------------------------------
    // Methods - Inherited methods ---------------------------------------------
public:
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    Bus::BusType getBusType(
            void
    ) override;

    //     ////////////////////    DATA TRANSFER     ////////////////////     //
    bool_t readReg(
            cuint8_t reg_p,
            uint8_t *buffData_p,
            cuint16_t buffSize_p = 1
    ) override;

    bool_t writeReg(
            cuint8_t reg_p,
            cuint8_t *buffData_p,
            cuint16_t buffSize_p = 1
    ) override;

    bool_t read(
            uint8_t *buffData_p,
            cuint16_t buffSize_p = 1
    ) override;

    bool_t write(
            cuint8_t *buffData_p,
            cuint16_t buffSize_p = 1
    ) override;

    bool_t sendData(
            cuint8_t *txBuffData_p,
            uint8_t *rxBuffData_p,
            cuint16_t buffSize_p
    ) override;

    bool_t sendData(
            uint8_t *buffData_p,
            cuint16_t buffSize_p
    ) override;

    //     //////////////////    PROTOCOL SPECIFIC     //////////////////     //
    bool_t setDevice(
            void (* actFunc_p)(void),
            void (* deactFunc_p)(void)
    ) override;

    bool_t setDevice(
            GpioPin *csPin_p
    ) override;

protected:

    // -------------------------------------------------------------------------
    // Methods - Class own methods ---------------------------------------------
public:
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    bool_t init(
            cuint8_t bufferSize_p       = 20
    );

    bool_t setClockPrescaler(
            const ClockPrescaler clockPrescaler_p
    );

    bool_t setMode(
            const Mode mode_p
    );

    bool_t setDataOrder(
            const DataOrder dataOrder_p
    );

    bool_t setTimeout(
            cuint16_t timeout_p
    );

    void inlined disable(
            void
    );

    void inlined enable(
            void
    );

    void inlined setMasterMode(
            void
    );

    void inlined setSlaveMode(       // Future implementation
            void
    );

    bool_t inlined isTransferComplete(
            void
    );

    bool_t getReceptionErrors(
            ReceptionError *errorCode_p
    );

    void inlined activateInterrupt(
            void
    );

    void inlined deactivateInterrupt(
            void
    );

    Error inlined getLastError(
            void
    );

    void interruptHandler(
            void
    );

private:
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    bool_t _waitWhileIsBusy(
            void
    );

    bool_t _transferByte(
            uint8_t *data_p
    );

    void inlined _selectDevice(
            void
    );

    void inlined _deselectDevice(
            void
    );

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Properties --------------------------------------------------------------
public:
    // NONE

private:
    //     /////////////////    CONTROL AND STATUS     //////////////////     //
    Error           _lastError;
    bool_t          _isDeviceSelected           : 1;
    bool_t          _useGpioToSelectDevice      : 1;
    uint16_t        _timeout;
    bool_t          _isInitialized              : 1;

    //     /////////////////     DEVICE ADDRESSING      /////////////////     //
    GpioPin   *_csPin;
    void (*_csActivateFunc)(void);
    void (*_csDeactivateFunc)(void);
    ReceptionError  _spiError;

    //     ////////////////////    DATA BUFFERS      ////////////////////     //
    uint8_t         _bufferLength               : 7;
    uint8_t         *_bufferData;
    uint8_t         _bufferIndex                : 7;
    uint8_t         _bufferMaxSize              : 7;
}; // class Spi

// =============================================================================
// Spi - Class inline function definitions
// =============================================================================

void inlined Spi::disable(void)
{
    clrBit(SPCR, SPE);

    return;
}

void inlined Spi::enable(void)
{
    setBit(SPCR, SPE);

    return;
}

void inlined Spi::setMasterMode(void)
{
    setBit(SPCR, MSTR);

    return;
}

void inlined Spi::setSlaveMode(void)
{
    clrBit(SPCR, MSTR);

    return;
}

bool_t inlined Spi::isTransferComplete(void)
{
    return isBitSet(SPSR, SPIF);
}

void inlined Spi::activateInterrupt(void)
{
    setBit(SPCR, SPIE);

    return;
}

void inlined Spi::deactivateInterrupt(void)
{
    clrBit(SPCR, SPIE);

    return;
}

Error inlined Spi::getLastError(void)
{
    return this->_lastError;
}

void inlined Spi::_selectDevice(void)
{
    if(this->_isDeviceSelected) {
        if(this->_useGpioToSelectDevice) {
            // Evaluates GPIO logic to assert CS (Active Low)
            this->_csPin->low();
        } else {
            // Executes custom callback for CS assertion
            this->_csActivateFunc();
        }
    }

    return;
}

void inlined Spi::_deselectDevice(void)
{
    if(this->_isDeviceSelected) {
        if(this->_useGpioToSelectDevice) {
            // Evaluates GPIO logic to deassert CS (Active High)
            this->_csPin->high();
        } else {
            // Executes custom callback for CS deassertion
            this->_csDeactivateFunc();
        }
    }

    return;
}

// =============================================================================
// Extern global variables
// =============================================================================

extern Spi spi;

// =============================================================================
// Include guard (END)
// =============================================================================

#endif  // __SPI_HPP

// =============================================================================
// END OF FILE
// =============================================================================
