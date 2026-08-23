#pragma once

#include <new>
#include <string>
#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Contracts/Providers/IDeviceIdentityProvider.h"

namespace iotsmartsys::test::mocks
{
    // Minimal command adapter: the builder only needs it to exist and to be
    // constructible in place, so the identity contract can be exercised without
    // touching real GPIOs.
    class NoopCommandAdapter : public iotsmartsys::core::ICommandHardwareAdapter
    {
    public:
        NoopCommandAdapter(std::uint8_t pin, bool highIsOn) : _pin(pin), _highIsOn(highIsOn) {}

        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }
        bool applyCommand(const iotsmartsys::core::IHardwareCommand &command) override
        {
            return applyCommand(command.getCommand().c_str());
        }
        bool applyCommand(const char *value) override
        {
            _state = value;
            if (_state == "toggle")
                _state = (_state == "on") ? "off" : "on";
            return true;
        }
        std::string getStateValue() override { return _state; }
        iotsmartsys::core::IHardwareState getState() override
        {
            iotsmartsys::core::IHardwareState state;
            state.value = _state;
            return state;
        }

    private:
        std::uint8_t _pin;
        bool _highIsOn;
        std::string _state{"off"};
    };

    class NoopInputAdapter : public iotsmartsys::core::IInputHardwareAdapter
    {
    public:
        explicit NoopInputAdapter(std::uint8_t pin) : _pin(pin) {}
        void setup() override {}
        void handle() override {}
        long lastStateReadMillis() const override { return 0; }
        int32_t readInput() override { return 0; }
        bool digitalActive() override { return false; }
        int32_t readDigitalState() override { return 0; }

    private:
        std::uint8_t _pin;
    };

    // Counts how many adapters were actually constructed, so a rejected
    // identity can be proven to leave no partial artefact behind.
    class FakeAdapterFactory : public iotsmartsys::core::IHardwareAdapterFactory
    {
    public:
        std::size_t outputAdapterSize() const override { return sizeof(NoopCommandAdapter); }
        std::size_t outputAdapterAlign() const override { return alignof(NoopCommandAdapter); }
        iotsmartsys::core::ICommandHardwareAdapter *createOutput(void *mem, std::uint8_t pin, bool highIsOn) override
        {
            outputsCreated++;
            return new (mem) NoopCommandAdapter(pin, highIsOn);
        }
        AdapterDestructor outputAdapterDestructor() const override
        {
            return [](void *p) { static_cast<NoopCommandAdapter *>(p)->~NoopCommandAdapter(); };
        }

        std::size_t inputAdapterSize() const override { return sizeof(NoopInputAdapter); }
        std::size_t inputAdapterAlign() const override { return alignof(NoopInputAdapter); }
        iotsmartsys::core::IInputHardwareAdapter *createInput(void *mem, std::uint8_t pin) override
        {
            inputsCreated++;
            return new (mem) NoopInputAdapter(pin);
        }
        iotsmartsys::core::IInputHardwareAdapter *createInput(void *mem, std::uint8_t pin,
                                                             iotsmartsys::core::HardwareDigitalLogic,
                                                             iotsmartsys::core::InputPullMode) override
        {
            inputsCreated++;
            return new (mem) NoopInputAdapter(pin);
        }
        AdapterDestructor inputAdapterDestructor() const override
        {
            return [](void *p) { static_cast<NoopInputAdapter *>(p)->~NoopInputAdapter(); };
        }

        std::size_t waterLevelSensorAdapterSize() const override { return 0; }
        std::size_t waterLevelSensorAdapterAlign() const override { return 1; }
        iotsmartsys::core::IWaterLevelSensor *createWaterLevelSensor(void *, std::uint8_t, std::uint8_t, float, float,
                                                                    iotsmartsys::core::WaterLevelRecipentType) override
        {
            return nullptr;
        }
        AdapterDestructor waterLevelSensorAdapterDestructor() const override
        {
            return [](void *) {};
        }

        int outputsCreated{0};
        int inputsCreated{0};
    };

    class FakeDeviceIdentityProvider : public iotsmartsys::core::IDeviceIdentityProvider
    {
    public:
        std::string getDeviceID() const override { return deviceId; }
        std::string getDeviceUniqueId() const override { return deviceId; }
        std::string getDeviceModel() const override { return "test-board"; }

        std::string deviceId{"dev1"};
    };

    class NoopEventSink : public iotsmartsys::core::ICapabilityEventSink
    {
    public:
        void onStateChanged(const iotsmartsys::core::CapabilityStateChanged &) override {}
    };

} // namespace iotsmartsys::test::mocks
