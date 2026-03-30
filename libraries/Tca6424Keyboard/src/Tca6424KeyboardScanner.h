#pragma once

#include <Arduino.h>

#include "Tca6424.h"

struct Tca6424KeyEvent
{
    uint8_t rowIndex;
    uint8_t colIndex;
    uint8_t rowPort;
    uint8_t colPort;
    bool pressed;
};

class Tca6424KeyboardScanner
{
public:
    static constexpr uint8_t kMaxRows = 12;
    static constexpr uint8_t kMaxCols = 16;
    static constexpr uint8_t kMaxQueuedEvents = 32;

    explicit Tca6424KeyboardScanner(Tca6424 &device, uint8_t debounceSamples = 3);

    bool begin(const uint8_t *rows, uint8_t rowCount, const uint8_t *cols, uint8_t colCount);
    bool scan();

    bool available() const;
    bool readEvent(Tca6424KeyEvent &event);

    uint8_t rowCount() const;
    uint8_t colCount() const;

private:
    bool configureIdleState();
    bool sampleMatrix(bool sample[kMaxRows][kMaxCols]);
    void processSample(const bool sample[kMaxRows][kMaxCols]);
    void clearState();
    bool enqueueEvent(const Tca6424KeyEvent &event);

    static bool isPortValid(uint8_t port);
    static uint32_t bitForPort(uint8_t port);

    Tca6424 &device_;
    uint8_t rows_[kMaxRows] = {};
    uint8_t cols_[kMaxCols] = {};
    uint8_t rowCount_ = 0;
    uint8_t colCount_ = 0;
    uint8_t debounceSamples_ = 3;

    bool lastRaw_[kMaxRows][kMaxCols] = {};
    bool stableState_[kMaxRows][kMaxCols] = {};
    uint8_t debounceCount_[kMaxRows][kMaxCols] = {};

    Tca6424KeyEvent queue_[kMaxQueuedEvents] = {};
    uint8_t queueHead_ = 0;
    uint8_t queueTail_ = 0;
};
