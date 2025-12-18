#pragma once

#include <vector>
#include "Contracts/Events/CapabilityStateChanged.h"
#include "Contracts/Events/ICapabilityEventSink.h"
#include "Contracts/Events/CapabilityCommand.h"
#include "ICapabilityType.h"
#include "Contracts/Adapters/IHardwareAdapter.h"
#include "Contracts/Providers/ITimeProvider.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Logging/Log.h"
#include "Contracts/Providers/Time.h"

namespace iotsmartsys::core
{
    struct ICapability
    {
    public:
        ICapability(std::string type, std::string value, ICapabilityEventSink *event_sink)
            : capability_name(""), type(type), value(value), event_sink(event_sink) {}
        ICapability(std::string capability_name, std::string type, std::string value)
            : capability_name(capability_name), type(type), value(value) {}

        ICapability(ICapabilityEventSink *event_sink,
                    std::string capability_name,
                    std::string type,
                    std::string value)
            : capability_name(capability_name), type(type), value(value), event_sink(event_sink) {}

        ICapability(
            ICapabilityEventSink *event_sink,
            std::string type,
            std::string value)
            : capability_name(""), type(type), value(value), event_sink(event_sink) {}

        virtual ~ICapability() {}

        std::string capability_name;
        std::string type;
        std::string value;

        void updateState(std::string value)
        {
            this->value = value;
            this->changed = true;
            if (this->event_sink)
            {
                CapabilityStateChanged ev = readState();
                this->event_sink->onStateChanged(ev);
                this->changed = false;
            }
        }

        CapabilityStateChanged readState()
        {
            this->changed = false;
            return CapabilityStateChanged(capability_name, value, type);
        }

        bool hasChanged()
        {
            return changed;
        }

        virtual void handle()
        {
        }

        virtual void setup()
        {
        }

        void applyRenamedName(std::string device_id)
        {
            this->capability_name = device_id + "_" + this->type;
        }

        void rename(std::string new_capability_name)
        {
            this->capability_name = new_capability_name;
        }

    protected:
        // Forwarding logger: always forwards at call-time to core::Log::get(),
        // so instances constructed before Log::setLogger(...) still see the
        // real logger once it's registered.
        struct ForwardingLogger : public ILogger
        {
            void logf(LogLevel level, const char *tag, const char *fmt, va_list args) override
            {
                // Forward to the current global logger implementation
                core::Log::get().logf(level, tag, fmt, args);
            }
        };

    // single forwarding instance shared by all capabilities
    static ForwardingLogger _forwardingLogger;

        core::ILogger &logger = _forwardingLogger;
        core::ITimeProvider &timeProvider = core::Time::get();
        ICapabilityEventSink *event_sink;

    private:
        bool changed = false;
    };
}