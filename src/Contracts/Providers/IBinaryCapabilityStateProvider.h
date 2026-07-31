#pragma once

#include <cstdint>
#include "Contracts/Common/StateResult.h"

namespace iotsmartsys::core::providers
{
    // Persistence boundary for the binary (off/on) state of capabilities derived
    // from BinaryCommandCapability. Identity is the pair (capability_name, type);
    // the on/off vocabulary conversion stays in BinaryCommandCapability so the
    // same storage serves off/on and closed/open vocabularies alike.
    class IBinaryCapabilityStateProvider
    {
    public:
        virtual ~IBinaryCapabilityStateProvider() = default;

        // Performs the single NVS data read for the boot and populates the
        // in-memory cache. Must be called once, before capabilities are set up.
        virtual iotsmartsys::core::common::StateResult loadSnapshot() = 0;

        // Cache-only lookup; must never touch storage. Returns false when no
        // valid record exists for (capability_name, type).
        virtual bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const = 0;

        // Updates the in-memory cache, persists the snapshot and commits it.
        virtual iotsmartsys::core::common::StateResult save(const char *capability_name, const char *type, bool isOn) = 0;
    };
} // namespace iotsmartsys::core::providers
