#include <Arduino.h>
#include <Wire.h>

#include <Tca6424.h>
#include <Tca6424KeyboardScanner.h>

namespace
{
constexpr uint8_t kI2cAddress = 0x22;
constexpr uint8_t kSdaPin = 16;
constexpr uint8_t kSclPin = 15;
constexpr uint32_t kI2cFreq = 400000;
constexpr uint8_t kDebounceSamples = 3;
constexpr uint16_t kScanPeriodMs = 10;
constexpr uint32_t kSerialWaitMs = 3000;
constexpr uint32_t kProbeRetryMs = 1000;

constexpr uint8_t kRowPins[] = {
    Tca6424::P00,
    Tca6424::P01,
    Tca6424::P02,
    Tca6424::P03,
    Tca6424::P04,
    Tca6424::P05,
    Tca6424::P06,
    Tca6424::P07,
    Tca6424::P10,
};

constexpr const char *kRowNames[] = {
    "ROW25",
    "ROW1",
    "ROW2",
    "ROW4",
    "ROW5",
    "ROW6",
    "ROW9",
    "ROW10",
    "ROW12",
};

constexpr uint8_t kColPins[] = {
    Tca6424::P11,
    Tca6424::P12,
    Tca6424::P13,
    Tca6424::P14,
    Tca6424::P15,
    Tca6424::P16,
    Tca6424::P17,
    Tca6424::P20,
    Tca6424::P21,
    Tca6424::P22,
    Tca6424::P23,
    Tca6424::P24,
    Tca6424::P25,
    Tca6424::P26,
};

constexpr const char *kColNames[] = {
    "COL3",
    "COL7",
    "COL8",
    "COL11",
    "COL13",
    "COL14",
    "COL15",
    "COL16",
    "COL17",
    "COL18",
    "COL22",
    "COL23",
    "COL24",
    "COL26",
};

constexpr uint16_t kKeyValueMap[][14] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 126},              // ROW25
    {0, 62, 0, 30, 46, 49, 50, 53, 61, 52, 44, 89, 0, 0},      // ROW1
    {0, 0, 0, 48, 34, 35, 36, 38, 39, 40, 0, 84, 58, 0},       // ROW2
    {0, 0, 0, 47, 19, 21, 23, 37, 26, 51, 0, 31, 0, 0},        // ROW4
    {0, 0, 0, 33, 18, 20, 22, 24, 25, 54, 0, 83, 0, 0},        // ROW5
    {0, 0, 0, 17, 3, 5, 7, 9, 11, 55, 0, 43, 0, 0},            // ROW6
    {0, 0, 0, 16, 4, 6, 8, 10, 0, 79, 0, 29, 0, 0},            // ROW9
    {0, 0, 59, 2, 112, 114, 116, 118, 120, 122, 0, 15, 0, 0},  // ROW10
    {127, 60, 0, 1, 110, 113, 115, 117, 119, 121, 57, 32, 0, 0} // ROW12
};

constexpr size_t kRowCount = sizeof(kRowPins) / sizeof(kRowPins[0]);
constexpr size_t kColCount = sizeof(kColPins) / sizeof(kColPins[0]);

static_assert(kRowCount == (sizeof(kRowNames) / sizeof(kRowNames[0])), "Row metadata must match");
static_assert(kColCount == (sizeof(kColNames) / sizeof(kColNames[0])), "Column metadata must match");
static_assert(kRowCount == (sizeof(kKeyValueMap) / sizeof(kKeyValueMap[0])), "Key map row count must match");
static_assert(kColCount == (sizeof(kKeyValueMap[0]) / sizeof(kKeyValueMap[0][0])), "Key map col count must match");

Tca6424 gExpander;
Tca6424KeyboardScanner gScanner(gExpander, kDebounceSamples);

bool gScannerReady = false;
uint32_t gLastScanMs = 0;
uint32_t gLastProbeMs = 0;
bool gLastProbeFailed = false;
} // namespace

void waitForSerialReady()
{
    const uint32_t start = millis();
    while (!Serial && (millis() - start < kSerialWaitMs))
    {
        delay(10);
    }
}

void printMatrixInfo()
{
    Serial.printf("Matrix lines: %u rows x %u cols\r\n",
                  static_cast<unsigned>(kRowCount),
                  static_cast<unsigned>(kColCount));

    Serial.println("Rows:");
    for (size_t i = 0; i < kRowCount; ++i)
    {
        Serial.printf("  R%u -> %s [%s]\r\n",
                      static_cast<unsigned>(i + 1),
                      kRowNames[i],
                      Tca6424::portLabel(kRowPins[i]));
    }

    Serial.println("Cols:");
    for (size_t i = 0; i < kColCount; ++i)
    {
        Serial.printf("  C%u -> %s [%s]\r\n",
                      static_cast<unsigned>(i + 1),
                      kColNames[i],
                      Tca6424::portLabel(kColPins[i]));
    }

    Serial.println("P27 is not used by the keyboard matrix.");
}

void printEvent(const Tca6424KeyEvent &event)
{
    const uint16_t keyValue = kKeyValueMap[event.rowIndex][event.colIndex];

    Serial.printf("%-7s key=%u R%uC%u [row=%s/%s col=%s/%s]\r\n",
                  event.pressed ? "PRESS" : "RELEASE",
                  static_cast<unsigned>(keyValue),
                  static_cast<unsigned>(event.rowIndex + 1),
                  static_cast<unsigned>(event.colIndex + 1),
                  Tca6424::portLabel(event.rowPort),
                  kRowNames[event.rowIndex],
                  Tca6424::portLabel(event.colPort),
                  kColNames[event.colIndex]);
}

bool startScanner()
{
    if (!gExpander.begin(Wire, kI2cAddress))
    {
        return false;
    }

    if (!gScanner.begin(kRowPins,
                        static_cast<uint8_t>(kRowCount),
                        kColPins,
                        static_cast<uint8_t>(kColCount)))
    {
        Serial.println("Scanner init failed.");
        return false;
    }

    Serial.printf("TCA6424 detected at 0x%02X\r\n", gExpander.address());
    printMatrixInfo();
    Serial.println("Scanning for key events...");

    gScannerReady = true;
    gLastProbeFailed = false;
    gLastScanMs = millis();
    return true;
}

void setup()
{
    Serial.begin(115200);
    waitForSerialReady();

    Serial.println();
    Serial.println("TCA6424 keyboard verification");
    Serial.printf("I2C: SDA=%u SCL=%u Freq=%lu Hz Addr=0x%02X\r\n",
                  kSdaPin,
                  kSclPin,
                  kI2cFreq,
                  kI2cAddress);

    Wire.begin(kSdaPin, kSclPin, kI2cFreq);

    if (!startScanner())
    {
        Serial.printf("TCA6424 not found at 0x%02X, retrying every %lu ms\r\n",
                      kI2cAddress,
                      kProbeRetryMs);
        gLastProbeFailed = true;
        gLastProbeMs = millis();
    }
}

void loop()
{
    const uint32_t now = millis();

    if (!gScannerReady)
    {
        if ((now - gLastProbeMs) < kProbeRetryMs)
        {
            delay(10);
            return;
        }

        gLastProbeMs = now;
        if (!startScanner() && !gLastProbeFailed)
        {
            Serial.printf("TCA6424 not found at 0x%02X, retrying every %lu ms\r\n",
                          kI2cAddress,
                          kProbeRetryMs);
            gLastProbeFailed = true;
        }

        delay(10);
        return;
    }

    if ((now - gLastScanMs) < kScanPeriodMs)
    {
        delay(1);
        return;
    }
    gLastScanMs = now;

    if (!gScanner.scan())
    {
        Serial.println("Lost communication with TCA6424, restarting probe loop.");
        gScannerReady = false;
        gLastProbeMs = now;
        gLastProbeFailed = true;
        return;
    }

    Tca6424KeyEvent event = {};
    while (gScanner.readEvent(event))
    {
        printEvent(event);
    }
}
