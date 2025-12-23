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
        ICapability(const char *type, const char *value, ICapabilityEventSink *event_sink)
            : capability_name(""), type(type), value(value), event_sink(event_sink) {}
        ICapability(const char *capability_name, const char *type, const char *value)
            : capability_name(capability_name), type(type), value(value) {}

        ICapability(ICapabilityEventSink *event_sink,
                    const char *capability_name,
                    const char *type,
                    const char *value)
            : capability_name(capability_name), type(type), value(value), event_sink(event_sink) {}

        ICapability(
            ICapabilityEventSink *event_sink,
            const char *type,
            const char *value)
            : capability_name(""), type(type), value(value), event_sink(event_sink) {}

        // Backwards-compatible overloads to accept std::string
        ICapability(const std::string &type, const std::string &value, ICapabilityEventSink *event_sink)
            : ICapability(type.c_str(), value.c_str(), event_sink) {}

        ICapability(const std::string &capability_name, const std::string &type, const std::string &value)
            : ICapability(capability_name.c_str(), type.c_str(), value.c_str()) {}

        ICapability(ICapabilityEventSink *event_sink,
                    const std::string &capability_name,
                    const std::string &type,
                    const std::string &value)
            : ICapability(event_sink, capability_name.c_str(), type.c_str(), value.c_str()) {}

        ICapability(
            ICapabilityEventSink *event_sink,
            const std::string &type,
            const std::string &value)
            : ICapability(event_sink, type.c_str(), value.c_str()) {}

        virtual ~ICapability() {}

    std::string capability_name;
    std::string type;
    std::string value;

        void updateState(const char *value)
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

        // Convenience overload to accept std::string
        void updateState(const std::string &value)
        {
            updateState(value.c_str());
        }

        CapabilityStateChanged readState()
        {
            this->changed = false;
            return CapabilityStateChanged(capability_name.c_str(), value.c_str(), type.c_str());
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

        void applyRenamedName(const char *device_id)
        {
            this->capability_name = std::string(device_id) + "_" + this->type;
        }

        void rename(const char *new_capability_name)
        {
            this->capability_name = new_capability_name;
        }

        // Overloads accepting std::string
        void applyRenamedName(const std::string &device_id)
        {
            applyRenamedName(device_id.c_str());
        }

        void rename(const std::string &new_capability_name)
        {
            rename(new_capability_name.c_str());
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