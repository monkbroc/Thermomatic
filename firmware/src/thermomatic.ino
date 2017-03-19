/*
 * Internet connected fleet of thermometers
 * 
 * Connect the MCP9808 temperature sensor
 * Vdd to 3V3
 * Gnd to GND
 * SCL to D1
 * SDA to D0
 */
#include "MCP9808.h"

PRODUCT_ID(3341);
PRODUCT_VERSION(1);

SYSTEM_THREAD(ENABLED);

MCP9808 sensor = MCP9808();
int publishDelay = 60; // seconds

String deviceName;

void setup() {
  Serial.begin(9600);
  delay(200);
  // What's my name?
  Particle.function("name", saveDeviceName);
  loadDeviceName();
  // Doesn't this work for products??
  //Particle.subscribe("spark/device/name", setDeviceName);
  //Particle.publish("spark/device/name", PRIVATE);

  // Don't have a bright light in our bedrooms
  RGB.control(true);
  RGB.brightness(0);
  while (!sensor.begin()) {
    delay(500);
    Serial.println("MCP9808 not found");
  }
  sensor.setResolution(MCP9808_SLOWEST);
}

void loop(void) {
  // Wait for device name
  if (deviceName.length() == 0) {
    return;
  }

  // Read the temperature
  float celsius = sensor.getTemperature();

  Serial.printlnf("Sensor T=%f C", celsius);
  if (celsius >= 5 && celsius < 50) {
    String eventName = "HouseTemp/" + deviceName;
    String eventData = String::format("%.2f", celsius);
    Particle.publish(eventName, eventData, PRIVATE);
  }

  // Publish temperature once per minute
  delay(publishDelay * 1000);
}

void setDeviceName(const char *topic, const char *data) {
  deviceName = data;
}

int saveDeviceName(String newName) {
  deviceName = newName;
  char buffer[32];
  newName.toCharArray(buffer, sizeof(buffer));
  buffer[sizeof(buffer)] = '\0';
  EEPROM.put(0, buffer);
  return 0;
}

void loadDeviceName() {
  char buffer[32];
  EEPROM.get(0, buffer);
  buffer[sizeof(buffer)] = '\0';
  if (buffer[0] == 0xFF) {
    deviceName = System.deviceID();
  } else {
    deviceName = buffer;
  }
}
