//!
//! \file           funsapeLibTc72.hpp
//! \brief          TC72 Temperature sensor module interface for the FunSAPE
//!                     AVR8 Library
//! \author         Leandro Schwarz (bladabuska+funsapeavr8lib@gmail.com)
//! \date           2026-06-29
//! \version        24.07
//! \copyright      license
//! \details        TC72 Temperature sensor module interface for the FunSAPE
//!                     AVR8 Library
//! \todo           Todo list
//!

// =============================================================================
// Include guard
// =============================================================================

#ifndef __TC72_HPP
#define __TC72_HPP                          2604

// =============================================================================
// Dependencies
// =============================================================================

// -----------------------------------------------------------------------------
// Global definitions file -----------------------------------------------------

#include "../globalDefines.hpp"
#ifndef __GLOBAL_DEFINES_HPP
#   error "Global definitions file is corrupted!"
#elif __GLOBAL_DEFINES_HPP != __TC72_HPP
#   error "Version mismatch between file header and global definitions file!"
#endif

// -----------------------------------------------------------------------------
// Header files - FunSAPE Library header files ---------------------------------

#include "../util/debug.hpp"
#ifndef __DEBUG_HPP
#   error "Header file (debug.hpp) is corrupted!"
#elif __DEBUG_HPP != __TC72_HPP
#   error "Version mismatch between header file and library dependency (debug.hpp)!"
#endif

#include "../util/bus.hpp"
#ifndef __BUS_HPP
#   error "Header file (bus.hpp) is corrupted!"
#elif __BUS_HPP != __TC72_HPP
#   error "Version mismatch between header file and library dependency (bus.hpp)!"
#endif

#include "../peripheral/gpioPin.hpp"
#ifndef __GPIO_PIN_HPP
#   error "Header file (gpioPin.hpp) is corrupted!"
#elif __GPIO_PIN_HPP != __TC72_HPP
#   error "Version mismatch between header file and library dependency (gpioPin.hpp)!"
#endif

// =============================================================================
// Platform verification
// =============================================================================

// NONE

// =============================================================================
// Undefining previous definitions
// =============================================================================

// NONE

// =============================================================================
// Constant definitions
// =============================================================================

// NONE

// =============================================================================
// Macro-function definitions
// *INDENT-OFF*
// =============================================================================

// NONE

// *INDENT-ON*

// =============================================================================
// New data types
// =============================================================================

// NONE

// =============================================================================
// Extern global variables
// =============================================================================

// NONE

// =============================================================================
// Tc72 - Class declaration
// =============================================================================

class Tc72
{
    // -------------------------------------------------------------------------
    // New data types ----------------------------------------------------------

public:
    // NONE

private:
    enum class Mode : uint8_t {
        SHUTDOWN            = 0x01,
        CONTINUOS           = 0x00,
        ONE_SHOT            = 0x11,
    };

    enum class Register : uint8_t {
        CONTROL             = 0x00,
        TEMP_LSB            = 0x01,
        TEMP_MSB            = 0x02,
        MANUFACTURER_ID     = 0x03
    };

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Constant values ---------------------------------------------------------

public:
    // NONE

private:
    static constexpr uint8_t manufacturerIdValue        = 0x54;
    static constexpr uint8_t readMask                   = 0x00;
    static constexpr uint8_t writeMask                  = 0x80;
    static constexpr uint8_t dummyByte                  = 0xFF;

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Operators overloading ---------------------------------------------------

public:
    // NONE

private:
    // NONE

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Constructors ------------------------------------------------------------

public:
    Tc72(
            void
    );
    ~Tc72(
            void
    );

    // -------------------------------------------------------------------------
    // Methods - Class own methods ---------------------------------------------

public:
    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    Error inlined getLastError(
            void
    );
    bool_t init(
            Bus *spiBus_p,
            GpioPin *csPin_p
    );
    bool_t init(
            Bus *spiBus_p,
            void (*activateCs_p)(void),
            void (*deactivateCs_p)(void)
    );
    bool_t shutdown(
            void
    );
    bool_t startContinuousConversion(
            void
    );
    bool_t startSingleConversion(
            void
    );

    //     ////////////////////    DATA HANDLING     ////////////////////     //
    bool_t readTemperature(
            int16_t *temperature_p
    );

private:
    // NONE

protected:
    // NONE

    // -------------------------------------------------------------------------
    // Properties --------------------------------------------------------------

public:
    // NONE

private:
    //     ////////////////    PERIPHERAL BUS HANDLER     ////////////////     //
    Bus             *_busHandler;
    GpioPin         *_csPin;
    void (*_activationFunction)(void);
    void (*_deactivationFunction)(void);

    //     /////////////////     CONTROL AND STATUS     /////////////////     //
    bool_t          _isInitialized              : 1;
    bool_t          _useGpioToSelectDevice      : 1;
    Error           _lastError;
}; // class Tc72

// =============================================================================
// Tc72 - Class overloading operators
// =============================================================================

// NONE

// =============================================================================
// Global variables
// =============================================================================

// -----------------------------------------------------------------------------
// Externally defined global variables -----------------------------------------

// NONE

// -----------------------------------------------------------------------------
// Internally defined global variables -----------------------------------------

// NONE

// =============================================================================
// Tc72 - Class inline function definitions
// =============================================================================

Error inlined Tc72::getLastError(void)
{
    // Returns last error
    return this->_lastError;
}

// =============================================================================
// General public functions declarations
// =============================================================================

// NONE

// =============================================================================
// General inline functions definitions
// =============================================================================

// NONE

// =============================================================================
// External default objects
// =============================================================================

// NONE

#endif // __TC72_HPP

// =============================================================================
// END OF FILE
// =============================================================================
