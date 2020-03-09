#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_ 1

#include <FileSystem.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>

AQUABOTICS_BEGIN_NAMESPACE

class Configuration {
 private:
  AQUABOTICS_NAMESPACE
  FileSystem fs;
 public:
  Configuration();
  Configuration(const Configuration &orig);
  virtual ~Configuration();
  void begin();
  void load(String doc);
};

AQUABOTICS_END_NAMESPACE

#endif