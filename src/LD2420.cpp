/*
 * LD2420.cpp - Implementation for HLK LD2420 Radar Sensor Library
 * 
 * This file contains the implementation of the LD2420 class methods.
 * 
 * Author: cyrixninja
 * Date: July 22, 2025
 * Version: 1.0.0
 */

#include "LD2420.h"

// Constructor
LD2420::LD2420() {
  _serial = nullptr;
  _initialized = false;
  _maxDistance = LD2420_MAX_DISTANCE;
  _minDistance = LD2420_MIN_DISTANCE;
  _updateInterval = 10; // 10ms default update interval
  _lastUpdate = 0;
  _lastState = LD2420_NO_DETECTION;
  
  // Initialize callbacks to nullptr
  _onDetection = nullptr;
  _onStateChange = nullptr;
  _onDataUpdate = nullptr;
  
  // Initialize current data
  _currentData.distance = 0;
  _currentData.state = LD2420_NO_DETECTION;
  _currentData.timestamp = 0;
  _currentData.isValid = false;
}

// Initialization methods
bool LD2420::begin(Stream& serial) {
  return begin(serial, LD2420_DEFAULT_BAUD_RATE);
}

bool LD2420::begin(Stream& serial, unsigned long baudRate) {
  _serial = &serial;
  
  // Wait a moment for serial to stabilize
  delay(100);
  
  // Send initial configuration command
  bool initSuccess = sendInitCommand();
  
  if (initSuccess) {
    _initialized = true;
    _lastUpdate = millis();
    
    // Clear any existing data
    while (_serial->available()) {
      _serial->read();
    }
    
    return true;
  }
  
  return false;
}

void LD2420::end() {
  _serial = nullptr;
  _initialized = false;
}

// Configuration methods
void LD2420::setDistanceRange(int minDistance, int maxDistance) {
  _minDistance = minDistance;
  _maxDistance = maxDistance;
}

void LD2420::setUpdateInterval(unsigned long interval) {
  _updateInterval = interval;
}

bool LD2420::sendInitCommand() {
  if (!_serial) return false;
  
  sendHexCommand(LD2420_CMD_INIT);
  delay(100); // Wait for command to be processed
  return true;
}

bool LD2420::restart() {
  if (!_serial) return false;
  
  sendHexCommand(LD2420_CMD_RESTART);
  delay(500); // Wait for restart to complete
  return sendInitCommand(); // Re-initialize after restart
}

bool LD2420::factoryReset() {
  if (!_serial) return false;
  
  sendHexCommand(LD2420_CMD_FACTORY_RESET);
  delay(1000); // Wait for reset to complete
  return sendInitCommand(); // Re-initialize after reset
}

// Data reading methods
void LD2420::update() {
  if (!_initialized || !_serial) return;
  
  unsigned long currentTime = millis();
  
  // Check if it's time to update
  if (currentTime - _lastUpdate < _updateInterval) {
    return;
  }
  
  _lastUpdate = currentTime;
  processIncomingData();
}

LD2420_Data LD2420::getCurrentData() {
  return _currentData;
}

int LD2420::getDistance() {
  return _currentData.distance;
}

LD2420_DetectionState LD2420::getState() {
  return _currentData.state;
}

bool LD2420::isDetecting() {
  return _currentData.state == LD2420_DETECTION_ACTIVE;
}

bool LD2420::isDataValid() {
  return _currentData.isValid;
}

unsigned long LD2420::getLastUpdateTime() {
  return _currentData.timestamp;
}

// Callback methods
void LD2420::onDetection(LD2420_DetectionCallback callback) {
  _onDetection = callback;
}

void LD2420::onStateChange(LD2420_StateChangeCallback callback) {
  _onStateChange = callback;
}

void LD2420::onDataUpdate(LD2420_DataCallback callback) {
  _onDataUpdate = callback;
}

// Utility methods
bool LD2420::isInitialized() {
  return _initialized;
}

String LD2420::getVersionInfo() {
  return "LD2420 Library v1.0.0";
}

// Private methods
void LD2420::sendHexCommand(const String& hexString) {
  if (!_serial) return;
  
  String cleanHex = formatHexString(hexString);
  int len = cleanHex.length();
  
  if (len % 2 != 0 || !validateHexString(cleanHex)) {
    return; // Invalid hex string
  }
  
  // Convert hex string to bytes and send
  for (int i = 0; i < len; i += 2) {
    String byteStr = cleanHex.substring(i, i + 2);
    byte hexByte = (byte)strtoul(byteStr.c_str(), NULL, 16);
    _serial->write(hexByte);
  }
}

bool LD2420::parseResponse(const String& response) {
  String trimmedResponse = response;
  trimmedResponse.trim();
  
  // Check if this is a range/distance reading
  if (trimmedResponse.startsWith("Range ")) {
    String distStr = trimmedResponse.substring(6);
    int distance = distStr.toInt();
    
    // Validate distance is within configured range
    if (distance >= _minDistance && distance <= _maxDistance) {
      LD2420_DetectionState newState = (distance > 0) ? LD2420_DETECTION_ACTIVE : LD2420_NO_DETECTION;
      
      // Update current data
      _currentData.distance = distance;
      _currentData.timestamp = millis();
      _currentData.isValid = true;
      
      // Check for state change
      if (newState != _currentData.state) {
        updateState(newState);
      }
      
      _currentData.state = newState;
      
      // Call callbacks
      if (_onDetection && newState == LD2420_DETECTION_ACTIVE) {
        _onDetection(distance);
      }
      
      if (_onDataUpdate) {
        _onDataUpdate(_currentData);
      }
      
      return true;
    }
  }
  
  return false;
}

void LD2420::updateState(LD2420_DetectionState newState) {
  LD2420_DetectionState oldState = _lastState;
  _lastState = newState;
  
  if (_onStateChange && oldState != newState) {
    _onStateChange(oldState, newState);
  }
}

void LD2420::processIncomingData() {
  if (!_serial || !_serial->available()) return;
  
  // Read available data line by line
  while (_serial->available()) {
    String line = _serial->readStringUntil('\n');
    parseResponse(line);
  }
}

// Static utility methods
String LD2420::formatHexString(const String& hex) {
  String formatted = hex;
  formatted.toUpperCase();
  formatted.replace(" ", "");
  formatted.replace("0x", "");
  formatted.replace("0X", "");
  return formatted;
}

bool LD2420::validateHexString(const String& hex) {
  for (unsigned int i = 0; i < hex.length(); i++) {
    char c = hex.charAt(i);
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F'))) {
      return false;
    }
  }
  return hex.length() % 2 == 0;
}