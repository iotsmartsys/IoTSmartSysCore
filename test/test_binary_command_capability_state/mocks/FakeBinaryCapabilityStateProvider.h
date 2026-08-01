#pragma once

#include <vector>
#include <string>
#include <cstring>
#include "Contracts/Providers/IBinaryCapabilityStateProvider.h"

namespace iotsmartsys::test::mocks
{
    // In-memory stand-in for the NVS-backed provider, so BinaryCommandCapability
    // protocol tests (restore/persist ordering, identity isolation, blink
    // exclusion, failure handling) run without touching real storage.
    //
    // Fidelity (spec 8.2/BCS-AC-028): requestSave() only updates the desired
    // state and counts the request — it never writes nor commits, so any NVS
    // work observed in the requesting context would be a defect. The test drives
    // the writer explicitly through releaseWriter(), which lets it hold the
    // writer blocked, inspect the consolidated pending entries and only then
    // reach a terminal state. Write and commit fail independently and only a
    // successful commit updates what survives a reboot.
    class FakeBinaryCapabilityStateProvider : public iotsmartsys::core::providers::IBinaryCapabilityStateProvider
    {
    public:
        struct Record
        {
            std::string capability_name;
            std::string type;
            bool isOn;
        };

        iotsmartsys::core::common::StateResult loadSnapshot() override
        {
            loadSnapshotCalls++;
            return iotsmartsys::core::common::StateResult::Ok;
        }

        iotsmartsys::core::common::StateResult activateWriter() override
        {
            activateWriterCalls++;
            if (failWriterCreation)
            {
                writerAvailable = false;
                return iotsmartsys::core::common::StateResult::NoMem;
            }
            writerAvailable = true;
            return iotsmartsys::core::common::StateResult::Ok;
        }

        // Answers from the last successfully committed snapshot: exactly what a
        // reboot would restore.
        bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const override
        {
            for (const auto &rec : committed)
            {
                if (rec.capability_name == capability_name && rec.type == type)
                {
                    outIsOn = rec.isOn;
                    return true;
                }
            }
            return false;
        }

        iotsmartsys::core::common::StateResult requestSave(const char *capability_name, const char *type, bool isOn) override
        {
            requestCalls++;
            lastRequestedName = capability_name;
            lastRequestedType = type;
            lastRequestedIsOn = isOn;

            if (!writerAvailable)
            {
                // No synchronous fallback: the refusal is observable and the
                // caller keeps its confirmed hardware/logical state.
                return iotsmartsys::core::common::StateResult::InvalidState;
            }

            for (auto &rec : desired)
            {
                if (rec.capability_name == capability_name && rec.type == type)
                {
                    // Consolidation: one entry per identity, most recent value.
                    rec.isOn = isOn;
                    return iotsmartsys::core::common::StateResult::Ok;
                }
            }

            if (desired.size() >= kMaxIdentities)
            {
                return iotsmartsys::core::common::StateResult::Overflow;
            }

            desired.push_back(Record{capability_name, type, isOn});
            return iotsmartsys::core::common::StateResult::Ok;
        }

        iotsmartsys::core::providers::BinaryStateWriterStatus writerStatus() const override
        {
            iotsmartsys::core::providers::BinaryStateWriterStatus status;
            status.available = writerAvailable;
            status.pending = static_cast<std::uint8_t>(pendingCount());
            status.inProgress = false;
            status.writes = writes;
            status.commits = commits;
            status.failures = failures;
            status.lastError = lastError;
            return status;
        }

        // Runs the writer once for every pending identity, as the real worker
        // would after being unblocked. Write and commit can be failed
        // independently; only a successful commit updates the committed view.
        void releaseWriter()
        {
            for (const auto &want : desired)
            {
                if (isCommitted(want))
                {
                    continue;
                }

                writes++;
                if (failNextWrite)
                {
                    failNextWrite = false;
                    failures++;
                    lastError = iotsmartsys::core::common::StateResult::StorageWriteFail;
                    continue;
                }

                if (failNextCommit)
                {
                    failNextCommit = false;
                    failures++;
                    lastError = iotsmartsys::core::common::StateResult::StorageWriteFail;
                    continue;
                }

                commit(want);
                commits++;
                lastError = iotsmartsys::core::common::StateResult::Ok;
            }
        }

        // Seeds a record as already committed, i.e. present in NVS at boot.
        void seed(const char *capability_name, const char *type, bool isOn)
        {
            committed.push_back(Record{capability_name, type, isOn});
            desired.push_back(Record{capability_name, type, isOn});
        }

        size_t pendingCount() const
        {
            size_t pending = 0;
            for (const auto &want : desired)
            {
                if (!isCommitted(want))
                {
                    pending++;
                }
            }
            return pending;
        }

        void clear()
        {
            committed.clear();
            desired.clear();
            requestCalls = 0;
            loadSnapshotCalls = 0;
            activateWriterCalls = 0;
            writes = 0;
            commits = 0;
            failures = 0;
            failNextWrite = false;
            failNextCommit = false;
            failWriterCreation = false;
            writerAvailable = true;
            lastError = iotsmartsys::core::common::StateResult::Ok;
            lastRequestedName.clear();
            lastRequestedType.clear();
            lastRequestedIsOn = false;
        }

        static constexpr size_t kMaxIdentities = 8;

        std::vector<Record> committed;
        std::vector<Record> desired;
        int loadSnapshotCalls{0};
        int activateWriterCalls{0};
        int requestCalls{0};
        std::uint32_t writes{0};
        std::uint32_t commits{0};
        std::uint32_t failures{0};
        bool writerAvailable{true};
        bool failWriterCreation{false};
        bool failNextWrite{false};
        bool failNextCommit{false};
        iotsmartsys::core::common::StateResult lastError{iotsmartsys::core::common::StateResult::Ok};
        std::string lastRequestedName;
        std::string lastRequestedType;
        bool lastRequestedIsOn{false};

    private:
        bool isCommitted(const Record &want) const
        {
            for (const auto &rec : committed)
            {
                if (rec.capability_name == want.capability_name && rec.type == want.type)
                {
                    return rec.isOn == want.isOn;
                }
            }
            return false;
        }

        void commit(const Record &want)
        {
            for (auto &rec : committed)
            {
                if (rec.capability_name == want.capability_name && rec.type == want.type)
                {
                    rec.isOn = want.isOn;
                    return;
                }
            }
            committed.push_back(want);
        }
    };

} // namespace iotsmartsys::test::mocks
