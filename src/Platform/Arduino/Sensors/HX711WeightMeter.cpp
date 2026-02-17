#include <math.h>
#include "Platform/Arduino/Sensors/HX711WeightMeter.h"

namespace iotsmartsys::platform::arduino
{
    static constexpr long READ_OF_TIMES_INTERVAL_MS_MIN = 60000L;

    HX711WeightMeter::HX711WeightMeter(const Config &cfg)
        : _cfg(cfg)
    {
        if (_cfg.readOfTimesIntervalMs < READ_OF_TIMES_INTERVAL_MS_MIN)
            _cfg.readOfTimesIntervalMs = READ_OF_TIMES_INTERVAL_MS_MIN;
    }

    void HX711WeightMeter::setup()
    {
        if (_cfg.doutPin < 0 || _cfg.sckPin < 0)
            return;

        pinMode(_cfg.doutPin, INPUT);
        pinMode(_cfg.sckPin, OUTPUT);
        digitalWrite(_cfg.sckPin, LOW);

        // Load calibration; only do initial tare if nothing exists in NVS
        if (!nvsLoadCalibration())
        {
            delay(200);
            tareNow(500);
        }

        resetFilters();
    }

    void HX711WeightMeter::handle()
    {
        const uint32_t now = millis();

        // Tare only when requested (unless first boot with no calibration -> handled in setup)
        if (_tareRequested)
        {
            _tareRequested = false;
            tareNow(500);
        }

        if (_cfg.readType == ReadType::INTERVAL && !_intervalWindowActive)
        {
            if (_nextIntervalWindowMs != 0 && (int32_t)(now - _nextIntervalWindowMs) < 0)
                return;

            _intervalWindowActive = true;
            _stableSinceMs = 0;
            _medCount = 0;
            _medIdx = 0;
            _avgCount = 0;
            _avgIdx = 0;
            _lastSampleMs = 0;
        }

        if (_lastSampleMs != 0 && (uint32_t)(now - _lastSampleMs) < _cfg.sampleIntervalMs)
            return;
        _lastSampleMs = now;

        int32_t raw = 0;
        if (!readRaw(raw))
            return;
        // Serial.printf("Raw read: %d\n", raw);
        // Drop invalid sentinel, if it ever appears from upstream wiring glitches
        if (raw == -8388608)
            return;

        if (_hasLastRaw)
        {
            const float jump = fabsf((float)(raw - _lastRaw));
            if (jump > _cfg.glitchRawJump)
            {
                // Re-sync baseline to the new level and restart filters.
                _lastRaw = raw;
                _hasLastRaw = true;
                _lastStateReadMs = (long)now;
                resetFilters();
                return;
            }
        }

        _lastRaw = raw;
        _hasLastRaw = true;
        _lastStateReadMs = (long)now;

        // Median(5)
        _medBuf[_medIdx] = raw;
        _medIdx = (uint8_t)((_medIdx + 1) % MEDIAN_N);
        if (_medCount < MEDIAN_N)
            _medCount++;

        if (_medCount < MEDIAN_N)
            return;

        const int32_t rawMed = median5(_medBuf);

        // Convert + moving average window
        const float kg = rawToKg(rawMed);
        pushAvg(kg);

        const AvgStats st = computeAvg();
        if (!st.ok)
            return;

        const float span = st.mx - st.mn;
        const bool stable = (span >= 0.0f) && (span < _cfg.stableDeltaKg);

        if (stable)
        {
            if (_stableSinceMs == 0)
                _stableSinceMs = now;

            if ((uint32_t)(now - _stableSinceMs) >= _cfg.stableHoldMs)
            {
                const float finalValue = st.avg - _cfg.tare;

                // First stable value must always be accepted. After that, apply tolerance.
                if (isnan(_lastStableKg) || fabsf(finalValue - _lastStableKg) > _cfg.variationTolerance)
                {
                    _lastStableKg = finalValue;
                }

                if (_cfg.readType == ReadType::INTERVAL)
                {
                    _intervalWindowActive = false;
                    _nextIntervalWindowMs = now + static_cast<uint32_t>(_cfg.readOfTimesIntervalMs);
                }
            }
        }
        else
        {
            _stableSinceMs = 0;
        }
    }

    float HX711WeightMeter::getKg() const
    {
        return isnan(_lastStableKg) ? 0.0f : _lastStableKg;
    }

    long HX711WeightMeter::lastStateReadMillis() const
    {
        return _lastStateReadMs;
    }

    void HX711WeightMeter::requestTare()
    {
        _tareRequested = true;
    }

    bool HX711WeightMeter::tareNow(uint32_t settleDelayMs)
    {
        if (_cfg.doutPin < 0 || _cfg.sckPin < 0)
            return false;

        if (settleDelayMs > 0)
            delay(settleDelayMs);

        const uint8_t N = 30;
        int got = 0;
        int64_t sum = 0;

        for (uint8_t i = 0; i < N; i++)
        {
            int32_t raw = 0;
            if (readRaw(raw) && raw != -8388608)
            {
                sum += raw;
                got++;
            }
            delay(20);
        }

        if (got <= 0)
            return false;

        _offset = (int32_t)(sum / got);

        resetFilters();
        nvsSaveCalibration();

        return true;
    }

    void HX711WeightMeter::setCountsPerKg(float v, bool persist)
    {
        if (v == 0.0f)
            return;

        _countsPerKg = v;
        resetFilters();

        if (persist)
            nvsSaveCalibration();
    }

    float HX711WeightMeter::getCountsPerKg() const
    {
        return _countsPerKg;
    }

    void HX711WeightMeter::setOffset(int32_t rawOffset, bool persist)
    {
        _offset = rawOffset;
        resetFilters();

        if (persist)
            nvsSaveCalibration();
    }

    int32_t HX711WeightMeter::getOffset() const
    {
        return _offset;
    }

    int32_t HX711WeightMeter::getLastRaw() const
    {
        return _lastRaw;
    }

    bool HX711WeightMeter::waitReady(uint32_t timeoutMs) const
    {
        const uint32_t start = millis();
        while (digitalRead(_cfg.doutPin) != LOW)
        {
            if ((uint32_t)(millis() - start) >= timeoutMs)
                return false;
            delayMicroseconds(50);
        }
        return true;
    }

    bool HX711WeightMeter::readRaw(int32_t &outRaw)
    {
        if (!waitReady(_cfg.readTimeoutMs))
            return false;

        noInterrupts();

        uint32_t value = 0;
        for (uint8_t i = 0; i < 24; i++)
        {
            digitalWrite(_cfg.sckPin, HIGH);
            delayMicroseconds(_cfg.pulseUs);
            value = (value << 1) | (uint32_t)(digitalRead(_cfg.doutPin) & 1);
            digitalWrite(_cfg.sckPin, LOW);
            delayMicroseconds(_cfg.pulseUs);
        }

        for (uint8_t i = 0; i < _cfg.extraPulses; i++)
        {
            digitalWrite(_cfg.sckPin, HIGH);
            delayMicroseconds(_cfg.pulseUs);
            digitalWrite(_cfg.sckPin, LOW);
            delayMicroseconds(_cfg.pulseUs);
        }

        interrupts();

        if (value & 0x800000UL)
            value |= 0xFF000000UL;

        outRaw = (int32_t)value;
        return true;
    }

    int32_t HX711WeightMeter::median5(const int32_t *a)
    {
        int32_t x[5] = {a[0], a[1], a[2], a[3], a[4]};
        for (int i = 0; i < 5; i++)
            for (int j = i + 1; j < 5; j++)
                if (x[j] < x[i])
                {
                    const int32_t t = x[i];
                    x[i] = x[j];
                    x[j] = t;
                }
        return x[2];
    }

    void HX711WeightMeter::pushAvg(float v)
    {
        _avgBuf[_avgIdx] = v;
        _avgIdx = (uint8_t)((_avgIdx + 1) % AVG_N);
        if (_avgCount < AVG_N)
            _avgCount++;
    }

    HX711WeightMeter::AvgStats HX711WeightMeter::computeAvg() const
    {
        AvgStats st;
        if (_avgCount == 0)
            return st;

        float sum = 0.0f;
        float mn = _avgBuf[0];
        float mx = _avgBuf[0];

        for (uint8_t i = 0; i < _avgCount; i++)
        {
            const float v = _avgBuf[i];
            sum += v;
            if (v < mn)
                mn = v;
            if (v > mx)
                mx = v;
        }

        st.avg = sum / (float)_avgCount;
        st.mn = mn;
        st.mx = mx;
        st.ok = true;
        return st;
    }

    float HX711WeightMeter::rawToKg(int32_t raw) const
    {
        if (_countsPerKg == 0.0f)
            return 0.0f;
        return (float)(raw - _offset) / _countsPerKg;
    }

    bool HX711WeightMeter::nvsLoadCalibration()
    {
        Preferences prefs;
        if (!prefs.begin(_cfg.nvsNamespace, true))
            return false;

        const uint32_t magic = prefs.getUInt("magic", 0);
        if (magic != _cfg.nvsMagic)
        {
            prefs.end();
            return false;
        }

        _offset = (int32_t)prefs.getInt("offset", 0);
        _countsPerKg = prefs.getFloat("cperkg", _countsPerKg);

        prefs.end();
        return true;
    }

    void HX711WeightMeter::nvsSaveCalibration()
    {
        Preferences prefs;
        if (!prefs.begin(_cfg.nvsNamespace, false))
            return;

        prefs.putUInt("magic", _cfg.nvsMagic);
        prefs.putInt("offset", (int)_offset);
        prefs.putFloat("cperkg", _countsPerKg);

        prefs.end();
    }

    void HX711WeightMeter::resetFilters()
    {
        _lastSampleMs = 0;
        _hasLastRaw = false;

        _medCount = 0;
        _medIdx = 0;

        _avgCount = 0;
        _avgIdx = 0;

        _stableSinceMs = 0;
        _nextIntervalWindowMs = 0;
        _intervalWindowActive = (_cfg.readType == ReadType::CONTINUOUS);
        // Don't force the stable value to 0 on every filter reset; keep it as "unknown" until
        // we actually converge. This avoids wiping the stable reading after large load jumps.
        _lastStableKg = NAN;
    }

} // namespace iotsmartsys::platform::arduino
