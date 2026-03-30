#pragma once

#include <Arduino.h>
#include <Wire.h>

class Tca6424
{
public:
    enum Port : uint8_t
    {
        P00 = 0,
        P01,
        P02,
        P03,
        P04,
        P05,
        P06,
        P07,
        P10,
        P11,
        P12,
        P13,
        P14,
        P15,
        P16,
        P17,
        P20,
        P21,
        P22,
        P23,
        P24,
        P25,
        P26,
        P27,
    };

    static constexpr uint8_t kDefaultAddress = 0x22;
    static constexpr uint8_t kInputPort0Command = 0x80;
    static constexpr uint8_t kOutputPort0Command = 0x84;
    static constexpr uint8_t kConfigurationPort0Command = 0x8C;

    bool begin(TwoWire &wire, uint8_t address = kDefaultAddress);
    bool probe();

    bool readInputs(uint32_t &value24);
    bool writeOutputs(uint32_t value24);
    bool writeConfig(uint32_t value24);

    uint8_t address() const;
    uint32_t outputShadow() const;
    uint32_t configShadow() const;

    static const char *portLabel(uint8_t port);

private:
    bool readRegisters(uint8_t command, uint32_t &value24);
    bool writeRegisters(uint8_t command, uint32_t value24, uint32_t &shadow);

    static void split24(uint32_t value24, uint8_t bytes[3]);
    static uint32_t join24(const uint8_t bytes[3]);

    TwoWire *wire_ = nullptr;
    uint8_t address_ = kDefaultAddress;
    uint32_t outputShadow_ = 0x00FFFFFFUL;
    uint32_t configShadow_ = 0x00FFFFFFUL;
};
