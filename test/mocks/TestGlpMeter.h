#pragma once

#include "Contracts/Sensors/IGlpMeter.h"

namespace iotsmartsys
{
    class TestGlpMeter : public iotsmartsys::core::IGlpMeter
    {
    public:
        TestGlpMeter() = default;
        virtual ~TestGlpMeter() = default;

        void setup() override {}
        void handle() override {}

        float getKg() const override { return _kg; }
        float getPercent() const override { return _percent; }
        std::string getLevelString() const override { return _level; }

        void setKg(float k)
        {
            _kg = k;
            // keep level string in sync with kg when kg changes
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", _kg);
            _level = std::string(buf);
        }

        void setPercent(float p)
        {
            _percent = p;
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", _percent);
            _level = std::string(buf);
        }

        void setLevelString(const std::string &s)
        {
            _level = s;
        }

    private:
        float _kg{0.0f};
        float _percent{0.0f};
        std::string _level{"0"};
    };

} // namespace iotsmartsys
