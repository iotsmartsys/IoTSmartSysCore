#pragma once

#include <vector>
#include <string>
#include <cstring>
#include "Contracts/Providers/IBinaryCapabilityStateProvider.h"

namespace iotsmartsys::test::mocks
{
    // In-memory stand-in for the NVS-backed provider, so BinaryCommandCapability
    // protocol tests (restore/persist ordering, identity isolation, failure
    // handling) run without touching real storage.
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

        bool tryGet(const char *capability_name, const char *type, bool &outIsOn) const override
        {
            for (const auto &rec : records)
            {
                if (rec.capability_name == capability_name && rec.type == type)
                {
                    outIsOn = rec.isOn;
                    return true;
                }
            }
            return false;
        }

        iotsmartsys::core::common::StateResult save(const char *capability_name, const char *type, bool isOn) override
        {
            saveCalls++;
            lastSavedName = capability_name;
            lastSavedType = type;
            lastSavedIsOn = isOn;

            if (failNextSave)
            {
                failNextSave = false;
                return iotsmartsys::core::common::StateResult::StorageWriteFail;
            }

            for (auto &rec : records)
            {
                if (rec.capability_name == capability_name && rec.type == type)
                {
                    rec.isOn = isOn;
                    return iotsmartsys::core::common::StateResult::Ok;
                }
            }
            records.push_back(Record{capability_name, type, isOn});
            return iotsmartsys::core::common::StateResult::Ok;
        }

        void seed(const char *capability_name, const char *type, bool isOn)
        {
            records.push_back(Record{capability_name, type, isOn});
        }

        std::vector<Record> records;
        int loadSnapshotCalls{0};
        int saveCalls{0};
        bool failNextSave{false};
        std::string lastSavedName;
        std::string lastSavedType;
        bool lastSavedIsOn{false};
    };

} // namespace iotsmartsys::test::mocks
