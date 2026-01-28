/*
 * LD2420.h - Library for HLK LD2420 Radar Sensor
 * 
 * This library provides an easy-to-use interface for the HLK LD2420 radar sensor.
 * It supports both hardware and software serial communication and provides
 * callbacks for detection events.
 * 
 * Author: cyrixninja
 * Date: July 22, 2025
 * Version: 1.0.0
 */

#ifndef LD2420_H
#define LD2420_H

#include <Arduino.h>
#include <Stream.h>

// Default configuration values
#define LD2420_DEFAULT_BAUD_RATE 115200
#define LD2420_BUFFER_SIZE 128
#define LD2420_MAX_DISTANCE 600  // Maximum detection range in cm
#define LD2420_MIN_DISTANCE 0    // Minimum detection range in cm

// Command types
#define LD2420_CMD_INIT "FDFCFBFA0800120000006400000004030201"
#define LD2420_CMD_RESTART "FDFCFBFA0400A100000004030201"
#define LD2420_CMD_FACTORY_RESET "FDFCFBFA0400A200000004030201"

// Detection states
enum LD2420_DetectionState {
  LD2420_NO_DETECTION = 0,
  LD2420_DETECTION_ACTIVE = 1,
  LD2420_DETECTION_LOST = 2
};

// Sensor data structure
struct LD2420_Data {
  int distance;                    // Distance in cm
  LD2420_DetectionState state;     // Current detection state
  unsigned long timestamp;         // When the reading was taken
  bool isValid;                    // Whether the reading is valid
};

// Callback function types
typedef void (*LD2420_DetectionCallback)(int distance);
typedef void (*LD2420_StateChangeCallback)(LD2420_DetectionState oldState, LD2420_DetectionState newState);
typedef void (*LD2420_DataCallback)(LD2420_Data data);

class LD2420 {
private:
  Stream* _serial;                
  bool _initialized;               // Initialization status
  
  // Current sensor state
  LD2420_Data _currentData;
  LD2420_DetectionState _lastState;
  unsigned long _lastUpdate;
  
  // Configuration
  int _maxDistance;
  int _minDistance;
  unsigned long _updateInterval;
  
  // Callbacks
  LD2420_DetectionCallback _onDetection;
  LD2420_StateChangeCallback _onStateChange;
  LD2420_DataCallback _onDataUpdate;
  
  // Internal methods
  void sendHexCommand(const String& hexString);
  bool parseResponse(const String& response);
  void updateState(LD2420_DetectionState newState);
  void processIncomingData();
  
public:
  // Constructor
  LD2420();
  
  // Initialization methods
  bool begin(Stream& serial);
  bool begin(Stream& serial, unsigned long baudRate);
  void end();
  
  // Configuration methods
  void setDistanceRange(int minDistance, int maxDistance);
  void setUpdateInterval(unsigned long interval);
  bool sendInitCommand();
  bool restart();
  bool factoryReset();
  
  // Data reading methods
  void update();                   
  LD2420_Data getCurrentData();
  int getDistance();
  LD2420_DetectionState getState();
  bool isDetecting();
  bool isDataValid();
  unsigned long getLastUpdateTime();
  
  // Callback methods
  void onDetection(LD2420_DetectionCallback callback);
  void onStateChange(LD2420_StateChangeCallback callback);
  void onDataUpdate(LD2420_DataCallback callback);
  
  // Utility methods
  bool isInitialized();
  void enableDebug(bool enable);
  String getVersionInfo();
  
  // Static utility methods
  static String formatHexString(const String& hex);
  static bool validateHexString(const String& hex);
};

#endif // LD2420_H