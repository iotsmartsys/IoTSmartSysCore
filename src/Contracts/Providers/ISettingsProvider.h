// Contracts/Providers/ISettingsProvider.h
#pragma once

#include "Contracts/Common/Error.h"
#include "Contracts/Settings/Settings.h"

namespace iotsmartsys::core::providers
{
    class ISettingsProvider
    {
    public:
        virtual ~ISettingsProvider() = default;

    // Carrega settings; retorna Error::NotFound se ainda não existe.
    virtual iotsmartsys::core::common::Error load(iotsmartsys::core::settings::Settings &out) = 0;

    // Salva settings (persistência).
    virtual iotsmartsys::core::common::Error save(const iotsmartsys::core::settings::Settings &settings) = 0;
        
    // Apaga settings (persistência).
    virtual iotsmartsys::core::common::Error erase() = 0;

        // Verifica se settings existem.
        virtual bool exists() = 0;
    };
} // namespace iotsmartsys::core::providers