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
