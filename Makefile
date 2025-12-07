ESP32_IP=esp32-4DA738.local            # IP do seu ESP32 na rede local
OTA_PORT=3232                   # Porta OTA configurada no firmware do ESP32
BINARIO=.pio/build/esp32dev/firmware.bin      # Caminho para o arquivo binário compilado
ESPOTA_PY=~/espota.py

ota:
	@echo "Enviando firmware para $(ESP32_IP) na porta $(OTA_PORT)..."
	python3 $(ESPOTA_PY) -i $(ESP32_IP) -p $(OTA_PORT) -f $(BINARIO) -r -d