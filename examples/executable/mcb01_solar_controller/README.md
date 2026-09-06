# MCB01 solar controller

Aplicação executável versionada selecionada pelo environment `ESP32_MCB01`.
Ela registra, antes de `SmartSysApp::setup()`, as nove capabilities do controlador
solar e exige capacidade estática igual a 12.

As capabilities `pv-voltage-1` e `pv-power-1` compartilham o mesmo adaptador de
tensão; `pv-current-1` e `pv-power-1` compartilham o mesmo adaptador de corrente.
As três medições da bateria compartilham uma única instância do dispositivo
INA3221, em canais explicitamente separados.

O build não acessa hardware. Upload e validação elétrica/operacional devem ser
ordenados separadamente pelo Arquiteto.

A tensão do painel usa `VoltageSensorConfig::voltageCalibrationFactor = 0.9788f`,
configurado antes da construção do adaptador: 45,26 V passam a aproximadamente
44,3005 V. Esse ganho foi obtido para o conjunto medido e deve ser recalibrado
para outro hardware. O padrão geral é `1.0f`; limites do ADC, sentinela e
saturação permanecem inalterados. A potência compartilhada recebe a tensão
corrigida automaticamente. O fator deve ser finito e estritamente positivo.
