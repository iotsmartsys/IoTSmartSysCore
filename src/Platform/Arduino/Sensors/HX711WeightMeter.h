#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <stdint.h>
#include <cmath>

#include "Contracts/Sensors/IGlpMeter.h"

namespace iotsmartsys::platform::arduino
{
    class HX711WeightMeter final : public iotsmartsys::core::IGlpMeter
    {
    public:
        struct Config
        {
            int doutPin = -1;
            int sckPin = -1;
            float tare = 0.0f;
            float variationTolerance = 0.2f;

            // HX711 protocol
            uint8_t extraPulses = 1;     /// @brief 1 => channel A gain 128
            uint8_t pulseUs = 2;         /// @brief SCK pulse width (us)
            uint32_t readTimeoutMs = 300;

            // Sampling / filtering
            uint32_t sampleIntervalMs = 80;     /// @brief ~12.5Hz
            float stableDeltaKg = 0.08f;        /// @brief max span to be considered stable
            uint32_t stableHoldMs = 2500;       /// @brief stable time required

            float glitchRawJump = 400000.0f;    /// @brief drop absurd jumps (wiring/EMI glitches)

            // NVS
            const char *nvsNamespace = "hx711";
            uint32_t nvsMagic = 0x48583731;     /// @brief "HX71"
        };

        explicit HX711WeightMeter(const Config &cfg);
        ~HX711WeightMeter() override = default;

        void setup() override;
        void handle() override;

        float getKg() const override;

        long lastStateReadMillis() const override;

        // Extra API (not in interface)
        void requestTare();
        bool tareNow(uint32_t settleDelayMs = 500);

        void setCountsPerKg(float v, bool persist = true);
        float getCountsPerKg() const;

        void setOffset(int32_t rawOffset, bool persist = true);
        int32_t getOffset() const;

        int32_t getLastRaw() const;

    private:
        static constexpr uint8_t MEDIAN_N = 5;
        static constexpr uint8_t AVG_N = 20;

        struct AvgStats
        {
            float avg = 0;
            float mn = 0;
            float mx = 0;
            bool ok = false;
        };

        bool waitReady(uint32_t timeoutMs) const;
        bool readRaw(int32_t &outRaw);

        static int32_t median5(const int32_t *a);
        void pushAvg(float v);
        AvgStats computeAvg() const;

        float rawToKg(int32_t raw) const;

        bool nvsLoadCalibration();
        void nvsSaveCalibration();

        void resetFilters();

    private:
        Config _cfg;

        // calibration
        int32_t _offset = 0;
        float _countsPerKg = 23670.0f;

        // state
        uint32_t _lastSampleMs = 0;
        int32_t _lastRaw = 0;
        bool _hasLastRaw = false;

        int32_t _medBuf[MEDIAN_N]{};
        uint8_t _medCount = 0;
        uint8_t _medIdx = 0;

        float _avgBuf[AVG_N]{};
        uint8_t _avgCount = 0;
        uint8_t _avgIdx = 0;

        float _lastStableKg = 0.0f;
        uint32_t _stableSinceMs = 0;

        volatile bool _tareRequested = false;

        long _lastStateReadMs = 0;
    };
} // namespace iotsmartsys::core