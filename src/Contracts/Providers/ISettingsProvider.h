// Contracts/Providers/ISettingsProvider.h
#pragma once

#include "Contracts/Common/StateResult.h"
#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core::providers
{
    class ISettingsProvider
    {
    public:
        virtual ~ISettingsProvider() = default;

    // Carrega settings; retorna StateResult::NotFound se ainda não existe.
    virtual iotsmartsys::core::common::StateResult load(iotsmartsys::core::settings::Settings &out) = 0;

    // Salva settings (persistência).
    virtual iotsmartsys::core::common::StateResult save(const iotsmartsys::core::settings::Settings &settings) = 0;
        
    // Apaga settings (persistência).
    virtual iotsmartsys::core::common::StateResult erase() = 0;

        // Verifica se settings existem.
        virtual bool exists() = 0;
    };
} // namespace iotsmartsys::core::providers