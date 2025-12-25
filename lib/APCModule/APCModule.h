#ifndef APCModule_h
#define APCModule_h

#include <Arduino.h>
#include <Stream.h>
#include <HardwareSerial.h>
#include <SoftwareSerial.h>

/**
 * @brief APCModule class for managing ACP220 module
*/
class APCModule {
public:
    /**
     * @brief Construct a new APCModule object using a HardwareSerial object
     * 
     * @param serial Reference to a HardwareSerial object
     * @param pinSet Pin number for setting the ACP220 module
     */
    APCModule(HardwareSerial &serial, int pinSet);

    /**
     * @brief Construct a new APCModule object using a HardwareSerial object specifying RX/TX pins (ESP32)
     * 
     * @param serial Reference to a HardwareSerial object (e.g., Serial2)
     * @param pinSet Pin SET del módulo (LOW = config, HIGH = operación)
     * @param rxPin  Pin RX usado por el puerto serie (por ejemplo 16 en ESP32)
     * @param txPin  Pin TX usado por el puerto serie (por ejemplo 17 en ESP32)
     */
    APCModule(HardwareSerial &serial, int pinSet, int rxPin, int txPin);

    /**
     * @brief Construct a new APCModule object using a SoftwareSerial object
     * 
     * @param serial Reference to a SoftwareSerial object
     * @param pinSet Pin number for setting the ACP220 module
     */
    APCModule(SoftwareSerial &serial, int pinSet);

    /**
     * @brief Initialize the ACP220 module
     * 
     * @param baudarate Baudarate for the serial communication
     * @param maxSetTimeOut Maximum time for the serial communication
    */
    void init(int baudarate, int maxSetTimeOut);

    /**
     * @brief Set the settings of the ACP220 module
     * 
     * @param ACPConfig Configuration string for the ACP220 module
     * 
     * @note "WR 434000 3 9 3 0"
     * @note Possible values for all these settings:
     * @note Frequency: Unit is KHz,for example 434MHz is 434000
     * @note RF Data Rate: 1,2,3 and 4 refer to 2400,4800,9600,19200bps
     * @note Output Power: 0 to 9, 9 means 13dBm(20mW)
     * @note UART Rate: 0,1,2,3,4,5 and 6 refers to 1200,2400,4800,9600, 19200,38400, 57600 bps
     * @note Series Checkout: Series checkout:0 means no check,1 means even parity,2 means odd parity
    */
    void setSettings(String ACPConfig);
    
    /**
     * @brief Get the current settings of the ACP220 module
     * 
     * @return String with the current settings of the ACP220 module
    */
    String getSettings();
    private:
        Stream &serial;
        HardwareSerial *hwSerial;
        SoftwareSerial *swSerial;
        int setPin;
        int rxPin;
        int txPin;

        bool tryReadConfig(String &out, unsigned long waitMs);
        void beginSerial(int baudrate);
};

#endif