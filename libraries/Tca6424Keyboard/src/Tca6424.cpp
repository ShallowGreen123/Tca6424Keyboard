#include "Tca6424.h"

namespace
{
constexpr uint8_t kRegisterCount = 3;
constexpr uint32_t kPortMask24 = 0x00FFFFFFUL;
} // namespace

bool Tca6424::begin(TwoWire &wire, uint8_t address)
{
    wire_ = &wire;
    address_ = address;
    outputShadow_ = kPortMask24;
    configShadow_ = kPortMask24;
    return probe();
}

bool Tca6424::probe()
{
    if (wire_ == nullptr)
    {
        return false;
    }

    wire_->beginTransmission(address_);
    return wire_->endTransmission() == 0;
}

bool Tca6424::readInputs(uint32_t &value24)
{
    return readRegisters(kInputPort0Command, value24);
}

bool Tca6424::writeOutputs(uint32_t value24)
{
    return writeRegisters(kOutputPort0Command, value24, outputShadow_);
}

bool Tca6424::writeConfig(uint32_t value24)
{
    return writeRegisters(kConfigurationPort0Command, value24, configShadow_);
}

uint8_t Tca6424::address() const
{
    return address_;
}

uint32_t Tca6424::outputShadow() const
{
    return outputShadow_;
}

uint32_t Tca6424::configShadow() const
{
    return configShadow_;
}

const char *Tca6424::portLabel(uint8_t port)
{
    static constexpr const char *kLabels[] = {
        "P00", "P01", "P02", "P03", "P04", "P05", "P06", "P07",
        "P10", "P11", "P12", "P13", "P14", "P15", "P16", "P17",
        "P20", "P21", "P22", "P23", "P24", "P25", "P26", "P27",
    };

    if (port >= (sizeof(kLabels) / sizeof(kLabels[0])))
    {
        return "UNK";
    }
    return kLabels[port];
}

bool Tca6424::readRegisters(uint8_t command, uint32_t &value24)
{
    if (wire_ == nullptr)
    {
        return false;
    }

    wire_->beginTransmission(address_);
    wire_->write(command);
    if (wire_->endTransmission(false) != 0)
    {
        return false;
    }

    if (wire_->requestFrom(static_cast<int>(address_), static_cast<int>(kRegisterCount)) != kRegisterCount)
    {
        return false;
    }

    uint8_t bytes[kRegisterCount] = {};
    for (uint8_t i = 0; i < kRegisterCount; ++i)
    {
        if (!wire_->available())
        {
            return false;
        }
        bytes[i] = static_cast<uint8_t>(wire_->read());
    }

    value24 = join24(bytes);
    return true;
}

bool Tca6424::writeRegisters(uint8_t command, uint32_t value24, uint32_t &shadow)
{
    if (wire_ == nullptr)
    {
        return false;
    }

    uint8_t bytes[kRegisterCount] = {};
    split24(value24, bytes);

    wire_->beginTransmission(address_);
    wire_->write(command);
    wire_->write(bytes, kRegisterCount);
    if (wire_->endTransmission() != 0)
    {
        return false;
    }

    shadow = value24 & kPortMask24;
    return true;
}

void Tca6424::split24(uint32_t value24, uint8_t bytes[3])
{
    const uint32_t masked = value24 & kPortMask24;
    bytes[0] = static_cast<uint8_t>(masked & 0xFFU);
    bytes[1] = static_cast<uint8_t>((masked >> 8) & 0xFFU);
    bytes[2] = static_cast<uint8_t>((masked >> 16) & 0xFFU);
}

uint32_t Tca6424::join24(const uint8_t bytes[3])
{
    return static_cast<uint32_t>(bytes[0]) |
           (static_cast<uint32_t>(bytes[1]) << 8) |
           (static_cast<uint32_t>(bytes[2]) << 16);
}
