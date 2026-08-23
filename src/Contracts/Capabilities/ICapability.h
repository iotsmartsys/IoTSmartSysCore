#pragma once

#include <cstddef>
#include <string>
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

namespace iotsmartsys::app
{
    // forward-declare the builder: it is the only component allowed to fix a
    // capability's definitive identity, and it does so before registration.
    class CapabilitiesBuilder;
}

namespace iotsmartsys::core
{
    // forward-declare ICommandCapability so ICapability can expose a
    // safe runtime-query method without requiring RTTI (dynamic_cast).
    struct ICommandCapability;
    struct ICapability;

    // BCS-002/BCS-DEC-005: public identity limits, measured in bytes of the
    // UTF-8 representation before the first '\0' and excluding the terminator.
    static constexpr std::size_t kMaxCapabilityNameBytes = 63;
    static constexpr std::size_t kMaxCapabilityTypeBytes = 31;

    // BCS-DEC-006: identity field that stays publicly readable exactly like the
    // previous public std::string member, but offers no public assignment.
    // Only ICapability (constructors and the builder-only finalization) can set
    // it, so the identity used by the registry, the cache and the storage can
    // never diverge after registration.
    class CapabilityIdentityField
    {
    public:
        CapabilityIdentityField(const CapabilityIdentityField &) = default;

        const char *c_str() const { return _value.c_str(); }
        bool empty() const { return _value.empty(); }
        std::size_t size() const { return _value.size(); }
        std::size_t length() const { return _value.size(); }
        const std::string &str() const { return _value; }
        operator const std::string &() const { return _value; }

    private:
        friend struct ICapability;

        CapabilityIdentityField() = default;
        explicit CapabilityIdentityField(const char *value) : _value(value ? value : "") {}
        CapabilityIdentityField &operator=(const CapabilityIdentityField &) = default;

        std::string _value;
    };

    inline bool operator==(const CapabilityIdentityField &lhs, const std::string &rhs) { return lhs.str() == rhs; }
    inline bool operator==(const std::string &lhs, const CapabilityIdentityField &rhs) { return lhs == rhs.str(); }
    inline bool operator==(const CapabilityIdentityField &lhs, const char *rhs) { return lhs.str() == (rhs ? rhs : ""); }
    inline bool operator==(const char *lhs, const CapabilityIdentityField &rhs) { return (lhs ? lhs : "") == rhs.str(); }
    inline bool operator!=(const CapabilityIdentityField &lhs, const std::string &rhs) { return !(lhs == rhs); }
    inline bool operator!=(const std::string &lhs, const CapabilityIdentityField &rhs) { return !(lhs == rhs); }
    inline bool operator!=(const CapabilityIdentityField &lhs, const char *rhs) { return !(lhs == rhs); }
    inline bool operator!=(const char *lhs, const CapabilityIdentityField &rhs) { return !(lhs == rhs); }

    inline std::string operator+(const std::string &lhs, const CapabilityIdentityField &rhs) { return lhs + rhs.str(); }
    inline std::string operator+(const CapabilityIdentityField &lhs, const std::string &rhs) { return lhs.str() + rhs; }
    inline std::string operator+(const char *lhs, const CapabilityIdentityField &rhs) { return std::string(lhs ? lhs : "") + rhs.str(); }
    inline std::string operator+(const CapabilityIdentityField &lhs, const char *rhs) { return lhs.str() + (rhs ? rhs : ""); }

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

        // Safe runtime query: return a pointer to ICommandCapability when
        // the concrete instance implements it, otherwise nullptr. This
        // avoids using dynamic_cast and works without RTTI enabled.
        virtual ICommandCapability *asCommandCapability()
        {
            return nullptr;
        }

        // BCS-002/BCS-DEC-006: publicly readable, never publicly assignable.
        CapabilityIdentityField capability_name;
        CapabilityIdentityField type;
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

        // BCS-DEC-006/BCS-022: kept public and `void` for source compatibility,
        // but deprecated and inert. The definitive identity is resolved,
        // validated and fixed by the builder before registration, so a legacy
        // call silently preserves the current capability_name and type.
        [[deprecated("capability identity is immutable after registration (BCS-DEC-006)")]]
        void applyRenamedName(const char *) {}

        [[deprecated("capability identity is immutable after registration (BCS-DEC-006)")]]
        void rename(const char *) {}

        // Overloads accepting std::string
        [[deprecated("capability identity is immutable after registration (BCS-DEC-006)")]]
        void applyRenamedName(const std::string &) {}

        [[deprecated("capability identity is immutable after registration (BCS-DEC-006)")]]
        void rename(const std::string &) {}

    private:
        friend class iotsmartsys::app::CapabilitiesBuilder;

        // Builder-only seam used before registration to fix the definitive name
        // resolved from the configuration or generated automatically. There is
        // no counterpart available once the capability is registered.
        void finalizeIdentity(const char *definitive_name)
        {
            this->capability_name = CapabilityIdentityField(definitive_name);
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