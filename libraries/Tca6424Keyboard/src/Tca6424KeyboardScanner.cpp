#include "Tca6424KeyboardScanner.h"

namespace
{
constexpr uint32_t kAllPortsMask = 0x00FFFFFFUL;
constexpr uint8_t kInputConfirmSamples = 3;
constexpr uint16_t kLineSettleDelayUs = 150;
constexpr uint16_t kInputSampleGapUs = 80;
} // namespace

Tca6424KeyboardScanner::Tca6424KeyboardScanner(Tca6424 &device, uint8_t debounceSamples)
    : device_(device),
      debounceSamples_(debounceSamples == 0 ? 1 : debounceSamples)
{
}

bool Tca6424KeyboardScanner::begin(const uint8_t *rows, uint8_t rowCount, const uint8_t *cols, uint8_t colCount)
{
    if (rows == nullptr || cols == nullptr || rowCount == 0 || colCount == 0)
    {
        return false;
    }

    if (rowCount > kMaxRows || colCount > kMaxCols)
    {
        return false;
    }

    for (uint8_t i = 0; i < rowCount; ++i)
    {
        if (!isPortValid(rows[i]))
        {
            return false;
        }
        rows_[i] = rows[i];

        for (uint8_t j = 0; j < i; ++j)
        {
            if (rows_[i] == rows_[j])
            {
                return false;
            }
        }
    }

    for (uint8_t i = 0; i < colCount; ++i)
    {
        if (!isPortValid(cols[i]))
        {
            return false;
        }
        cols_[i] = cols[i];

        for (uint8_t j = 0; j < i; ++j)
        {
            if (cols_[i] == cols_[j])
            {
                return false;
            }
        }

        for (uint8_t j = 0; j < rowCount; ++j)
        {
            if (cols_[i] == rows_[j])
            {
                return false;
            }
        }
    }

    rowCount_ = rowCount;
    colCount_ = colCount;

    clearState();
    return configureIdleState();
}

bool Tca6424KeyboardScanner::scan()
{
    if (rowCount_ == 0 || colCount_ == 0)
    {
        return false;
    }

    bool sample[kMaxRows][kMaxCols] = {};
    if (!sampleMatrix(sample))
    {
        return false;
    }

    processSample(sample);
    return true;
}

bool Tca6424KeyboardScanner::available() const
{
    return queueHead_ != queueTail_;
}

bool Tca6424KeyboardScanner::readEvent(Tca6424KeyEvent &event)
{
    if (!available())
    {
        return false;
    }

    event = queue_[queueHead_];
    queueHead_ = static_cast<uint8_t>((queueHead_ + 1) % kMaxQueuedEvents);
    return true;
}

uint8_t Tca6424KeyboardScanner::rowCount() const
{
    return rowCount_;
}

uint8_t Tca6424KeyboardScanner::colCount() const
{
    return colCount_;
}

bool Tca6424KeyboardScanner::configureIdleState()
{
    uint32_t configMask = kAllPortsMask;
    for (uint8_t i = 0; i < rowCount_; ++i)
    {
        configMask &= ~bitForPort(rows_[i]);
    }

    if (device_.configShadow() != configMask && !device_.writeConfig(configMask))
    {
        return false;
    }

    return device_.writeOutputs(kAllPortsMask);
}

bool Tca6424KeyboardScanner::sampleMatrix(bool sample[kMaxRows][kMaxCols])
{
    uint32_t columnMask = 0;
    for (uint8_t colIndex = 0; colIndex < colCount_; ++colIndex)
    {
        columnMask |= bitForPort(cols_[colIndex]);
    }

    if (!configureIdleState())
    {
        return false;
    }

    delayMicroseconds(kLineSettleDelayUs);

    uint32_t idleHighMask = 0;
    uint32_t idleLowMask = 0;
    if (!readStableInputMasks(idleHighMask, idleLowMask))
    {
        return false;
    }
    idleHighMask &= columnMask;

    for (uint8_t rowIndex = 0; rowIndex < rowCount_; ++rowIndex)
    {
        uint32_t outputMask = kAllPortsMask;
        outputMask &= ~bitForPort(rows_[rowIndex]);

        if (!device_.writeOutputs(outputMask))
        {
            return false;
        }

        delayMicroseconds(kLineSettleDelayUs);

        uint32_t activeHighMask = 0;
        uint32_t activeLowMask = 0;
        if (!readStableInputMasks(activeHighMask, activeLowMask))
        {
            return false;
        }
        activeLowMask &= columnMask;

        for (uint8_t colIndex = 0; colIndex < colCount_; ++colIndex)
        {
            const uint32_t bit = bitForPort(cols_[colIndex]);

            // A valid press must read high in the idle state and stable low
            // only when the active row is driven low.
            sample[rowIndex][colIndex] = ((idleHighMask & bit) != 0) &&
                                         ((activeLowMask & bit) != 0);
        }
    }

    return configureIdleState();
}

bool Tca6424KeyboardScanner::readStableInputMasks(uint32_t &stableHighMask, uint32_t &stableLowMask)
{
    stableHighMask = kAllPortsMask;
    stableLowMask = kAllPortsMask;

    for (uint8_t sampleIndex = 0; sampleIndex < kInputConfirmSamples; ++sampleIndex)
    {
        uint32_t inputs = 0;
        if (!device_.readInputs(inputs))
        {
            return false;
        }

        stableHighMask &= inputs;
        stableLowMask &= (~inputs & kAllPortsMask);

        if (sampleIndex + 1 < kInputConfirmSamples)
        {
            delayMicroseconds(kInputSampleGapUs);
        }
    }

    return true;
}

void Tca6424KeyboardScanner::processSample(const bool sample[kMaxRows][kMaxCols])
{
    for (uint8_t rowIndex = 0; rowIndex < rowCount_; ++rowIndex)
    {
        for (uint8_t colIndex = 0; colIndex < colCount_; ++colIndex)
        {
            const bool current = sample[rowIndex][colIndex];
            if (current == lastRaw_[rowIndex][colIndex])
            {
                if (debounceCount_[rowIndex][colIndex] < debounceSamples_)
                {
                    ++debounceCount_[rowIndex][colIndex];
                }
            }
            else
            {
                lastRaw_[rowIndex][colIndex] = current;
                debounceCount_[rowIndex][colIndex] = 1;
            }

            if (debounceCount_[rowIndex][colIndex] >= debounceSamples_ &&
                stableState_[rowIndex][colIndex] != lastRaw_[rowIndex][colIndex])
            {
                stableState_[rowIndex][colIndex] = lastRaw_[rowIndex][colIndex];

                Tca6424KeyEvent event = {};
                event.rowIndex = rowIndex;
                event.colIndex = colIndex;
                event.rowPort = rows_[rowIndex];
                event.colPort = cols_[colIndex];
                event.pressed = stableState_[rowIndex][colIndex];
                enqueueEvent(event);
            }
        }
    }
}

void Tca6424KeyboardScanner::clearState()
{
    for (uint8_t rowIndex = 0; rowIndex < kMaxRows; ++rowIndex)
    {
        for (uint8_t colIndex = 0; colIndex < kMaxCols; ++colIndex)
        {
            lastRaw_[rowIndex][colIndex] = false;
            stableState_[rowIndex][colIndex] = false;
            debounceCount_[rowIndex][colIndex] = 0;
        }
    }

    queueHead_ = 0;
    queueTail_ = 0;
}

bool Tca6424KeyboardScanner::enqueueEvent(const Tca6424KeyEvent &event)
{
    const uint8_t nextTail = static_cast<uint8_t>((queueTail_ + 1) % kMaxQueuedEvents);
    if (nextTail == queueHead_)
    {
        queueHead_ = static_cast<uint8_t>((queueHead_ + 1) % kMaxQueuedEvents);
    }

    queue_[queueTail_] = event;
    queueTail_ = nextTail;
    return true;
}

bool Tca6424KeyboardScanner::isPortValid(uint8_t port)
{
    return port <= Tca6424::P27;
}

uint32_t Tca6424KeyboardScanner::bitForPort(uint8_t port)
{
    return 1UL << port;
}
