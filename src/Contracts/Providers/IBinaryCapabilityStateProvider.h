#pragma once

#include <cstdint>
#include "Contracts/Common/StateResult.h"

namespace iotsmartsys::core::providers
{
    // BCS-029/BCS-DEC-004: observable terminal state of the single asynchronous
    // snapshot writer. It lets a test (or a controlled reboot) distinguish
    // pending work, an operation in flight, the last successful commit and a
    // failure, without ever converting quiescence into a presumed success.
    struct BinaryStateWriterStatus
    {
        // The writer was created and is usable. A creation failure leaves this
        // false and never authorises a synchronous fallback.
        bool available{false};
        // Identities holding a desired value not yet written (at most one entry
        // per identity, so never above the eight-capability limit).
        std::uint8_t pending{0};
        // A write/commit pair is executing right now.
        bool inProgress{false};
        std::uint32_t writes{0};
        std::uint32_t commits{0};
        std::uint32_t failures{0};
        common::StateResult lastError{common::StateResult::Ok};
    };

    // Persistence boundary for the binary (off/on) state of capabilities derived
    // from BinaryCommandCapability. Identity is the pair (capability_name, type);
    // the on/off vocabulary conversion stays in BinaryCommandCapability so the
    // same storage serves off/on and closed/open vocabularies alike.
    class IBinaryCapabilityStateProvider
    {
    public:
        virtual ~IBinaryCapabilityStateProvider() = default;

        // BCS-007: performs the single NVS data read for the boot and populates
        // the in-memory cache. Must be called once, before capabilities exist.
        virtual common::StateResult loadSnapshot() = 0;

        // BCS-029: activates the single asynchronous writer. Must run once, only
        // after the service graph is fully built and the snapshot has been read,
        // and before any capability can request persistence.
        virtual common::StateResult activateWriter() = 0;

        // BCS-007/BCS-008: cache-only lookup; must never touch storage. Returns
        // false when no valid record exists for (capability_name, type).
        virtual bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const = 0;

        // BCS-029/BCS-DEC-004: updates the desired in-memory state for the
        // identity and signals the asynchronous writer. Returns only whether the
        // identity, the value and the request were accepted; it never executes
        // nor awaits nvs_set_blob()/nvs_commit().
        virtual common::StateResult requestSave(const char *capability_name, const char *type, bool isOn) = 0;

        // BCS-029: terminal state of the writer, for diagnostics and for tests
        // that need to await quiescence.
        virtual BinaryStateWriterStatus writerStatus() const = 0;
    };
} // namespace iotsmartsys::core::providers
