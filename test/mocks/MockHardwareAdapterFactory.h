#pragma once

#include <cstddef>
#include <cstdint>

#include "Contracts/Adapters/IHardwareAdapterFactory.h"
#include "Platform/Arduino/Adapters/OutputHardwareAdapter.h"

namespace iotsmartsys::test::mocks
{

    // Mock factory that returns pre-set instances for sensors (useful in unit tests).
    // You can pass the instances (ownership remains with the test) via constructor or setter.
    class MockHardwareAdapterFactory final : public iotsmartsys::core::IHardwareAdapterFactory
    {
    public:
        MockHardwareAdapterFactory(iotsmartsys::core::IWaterLevelSensor *waterSensor = nullptr)
            : _waterSensor(waterSensor)
        {
        }

        ~MockHardwareAdapterFactory() override = default;

        // Allow tests to change the instance at runtime
        void setWaterLevelSensor(iotsmartsys::core::IWaterLevelSensor *sensor)
        {
            _waterSensor = sensor;
        }

        /* Relay Adapter - not used in this mock */
        std::size_t relayAdapterSize() const override { return 0; }
        std::size_t relayAdapterAlign() const override { return 1; }
        iotsmartsys::core::ICommandHardwareAdapter *createRelay(void * /*mem*/, std::uint8_t /*pin*/, bool /*highIsOn*/) override
        {
            return new iotsmartsys::platform::arduino::OutputHardwareAdapter(43, iotsmartsys::platform::arduino::HardwareDigitalLogic::HIGH_IS_ON);
        }
        AdapterDestructor relayAdapterDestructor() const override { return &MockHardwareAdapterFactory::noopDestructor; }

        /* Output Adapter - not used in this mock */
        std::size_t outputAdapterSize() const override { return 0; }
        std::size_t outputAdapterAlign() const override { return 1; }
        iotsmartsys::core::ICommandHardwareAdapter *createOutput(void * /*mem*/, std::uint8_t /*pin*/, bool /*highIsOn*/) override { return nullptr; }
        AdapterDestructor outputAdapterDestructor() const override { return &MockHardwareAdapterFactory::noopDestructor; }

        /* Input Adapter - not used in this mock */
        std::size_t inputAdapterSize() const override { return 0; }
        std::size_t inputAdapterAlign() const override { return 1; }
        iotsmartsys::core::IInputHardwareAdapter *createInput(void * /*mem*/, std::uint8_t /*pin*/) override { return nullptr; }
        AdapterDestructor inputAdapterDestructor() const override { return &MockHardwareAdapterFactory::noopDestructor; }

        /* IWaterLevelSensor - returns the pre-set instance (no ownership taken)
           The factory does not construct or destroy the instance; destructor is a noop.
        */
        std::size_t waterLevelSensorAdapterSize() const override { return 0; }
        std::size_t waterLevelSensorAdapterAlign() const override { return 1; }
        iotsmartsys::core::IWaterLevelSensor *createWaterLevelSensor(void * /*mem*/, std::uint8_t /*trigPin*/, std::uint8_t /*echoPin*/, float /*minLevelCm*/, float /*maxLevelCm*/, iotsmartsys::core::WaterLevelRecipentType /*recipentType*/) override
        {
            return _waterSensor;
        }
        AdapterDestructor waterLevelSensorAdapterDestructor() const override { return &MockHardwareAdapterFactory::noopDestructor; }

    private:
        static void noopDestructor(void *) {}

        // Pointer to the test-provided water level sensor. The factory does not own it.
        iotsmartsys::core::IWaterLevelSensor *_waterSensor = nullptr;
    };

} // namespace iotsmartsys::test::mocks
