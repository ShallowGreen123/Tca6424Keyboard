#include <Arduino.h>
#include <Wire.h>

namespace
{
constexpr uint8_t I2C_SDA_PIN = 16;
constexpr uint8_t I2C_SCL_PIN = 15;
constexpr uint32_t I2C_FREQUENCY_HZ = 400000;
constexpr uint32_t SCAN_INTERVAL_MS = 3000;
constexpr uint32_t SERIAL_WAIT_MS = 3000;
} // namespace

void waitForSerialReady()
{
    const uint32_t start = millis();
    while (!Serial && (millis() - start < SERIAL_WAIT_MS))
    {
        delay(10);
    }
}

void scanI2CDevices(TwoWire &bus)
{
    uint8_t foundDevices = 0;

    Serial.println();
    Serial.println("Scanning I2C bus...");
    Serial.println("Address range: 0x01 - 0x7E");

    for (uint8_t address = 0x01; address < 0x7F; ++address)
    {
        bus.beginTransmission(address);
        const uint8_t error = bus.endTransmission();

        if (error == 0)
        {
            Serial.printf("Found I2C device at 0x%02X\r\n", address);
            ++foundDevices;
        }
        else if (error == 4)
        {
            Serial.printf("Unknown error at 0x%02X\r\n", address);
        }

        delay(1);
    }

    if (foundDevices == 0)
    {
        Serial.printf("No I2C devices found.\r\n");
    }
    else
    {
        Serial.printf("Scan complete, %u device(s) found.\r\n", foundDevices);
    }
}

void setup()
{
    Serial.begin(115200);
    waitForSerialReady();
    delay(200);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY_HZ);

    Serial.println();
    Serial.println("ESP32-S3 I2C scanner");
    Serial.printf("SDA: GPIO %u, SCL: GPIO %u, Freq: %lu Hz\r\n",
                  I2C_SDA_PIN,
                  I2C_SCL_PIN,
                  I2C_FREQUENCY_HZ);
    Serial.flush();

    scanI2CDevices(Wire);
}

void loop()
{
    delay(SCAN_INTERVAL_MS);
    scanI2CDevices(Wire);
}
