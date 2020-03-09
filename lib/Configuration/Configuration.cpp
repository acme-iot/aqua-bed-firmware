#include <Configuration.h>

#include <stdexcept>
#include <ArduinoJson.h>

AQUABOTICS_BEGIN_NAMESPACE

const String ConfigurationFile = "/config.json";

const String KeyId = "id";
const String KeyMqtt = "mqtt";
const String KeyMqttIp = "ip";
const String KeyMqttPort = "port";
const String KeyMqttUsername = "username";
const String KeyMqttPassword = "password";

Configuration::Configuration() {}

Configuration::Configuration(const Configuration &orig) {}

Configuration::~Configuration() {}

void Configuration::begin() {
  fs.begin();
}

/**
 * takes the configuration response string of valid JSON and writes/updates the configuration file
 *
 * @param json String that contains valid JSON
 * @throws std::runtime_error if the JSON string could not be deserialized
 */
void Configuration::load(String json) {
  DynamicJsonDocument doc(1024);
  auto error = deserializeJson(doc, json.c_str());

  if (error) {
    throw std::runtime_error(error.c_str());
  }

  auto jsonObject = doc.as<JsonObject>();

  // refresh settings
  this->refresh(jsonObject);

  // save response in config.json
  String cleanJson;
  serializeJson(doc, cleanJson);
  fs.writeFile(ConfigurationFile.c_str(), cleanJson.c_str());
}

void Configuration::load() {
  if (!fs.existsFile(ConfigurationFile.c_str())) {
    return;
  }

  auto json = fs.readFile(ConfigurationFile.c_str());
  if (json) {
    load(*json);
  }
}

/**
 *
 * @param object JsonObject with MQTT properties
 * @throws std::runtime_error if any expected keys are missing
 */
void Configuration::refresh(JsonObject object) {

  if (!object.containsKey(KeyId)) {
    throw std::runtime_error(("missing key " + KeyId).c_str());
  }

  if (!object.containsKey(KeyMqtt)) {
    throw std::runtime_error(("missing key " + KeyMqtt).c_str());
  }

  auto properties = object[KeyMqtt];

  if (!properties.containsKey(KeyMqttIp)) {
    throw std::runtime_error(("missing key " + KeyMqttIp).c_str());
  }

  if (!properties.containsKey(KeyMqttPort)) {
    throw std::runtime_error(("missing key " + KeyMqttPort).c_str());
  }

  if (!properties.containsKey(KeyMqttUsername)) {
    throw std::runtime_error(("missing key " + KeyMqttUsername).c_str());
  }

  if (!properties.containsKey(KeyMqttPassword)) {
    throw std::runtime_error(("missing key " + KeyMqttPassword).c_str());
  }

  auto ip = properties[KeyMqttIp];
  auto port = properties[KeyMqttPort];
  auto username = properties[KeyMqttUsername];
  auto password = properties[KeyMqttPassword];
  auto id = object[KeyId];

  setMqttIp(ip);
  setMqttPort(port);
  setMqttUsername(username);
  setMqttPassword(password);
  setId(id);

  initialized = true;
}

bool Configuration::hasInitialized() {
  return initialized;
}

bool Configuration::hasChanged(bool reset) {
  auto temp = changed;

  if (reset) {
    changed = false;
  }

  return temp;
}

void Configuration::setId(String value) {
  if (value.isEmpty()) {
    throw std::runtime_error("ID can't be empty");
  }

  if (id==value) {
    return;
  }

  id = value;
  changed = true;
}

void Configuration::setMqttIp(String value) {
  if (value.isEmpty()) {
    throw std::runtime_error("MQTT IP can't be empty");
  }

  if (mqttIp==value) {
    return;
  }

  mqttIp = value;
  changed = true;
}

void Configuration::setMqttPort(uint16_t value) {
  if (value==0) {
    throw std::runtime_error("MQTT port can't be empty");
  }

  if (mqttPort==value) {
    return;
  }

  mqttPort = value;
  changed = true;
}

void Configuration::setMqttUsername(String value) {
  if (mqttUsername==value) {
    return;
  }

  mqttUsername = value;
  changed = true;
}

void Configuration::setMqttPassword(String value) {
  if (mqttPassword==value) {
    return;
  }

  mqttPassword = value;
  changed = true;
}

String Configuration::getId() {
  return id;
}

String Configuration::getMqttIp() {
  return mqttIp;
}

uint16_t Configuration::getMqttPort() {
  return mqttPort;
}

String Configuration::getMqttUsername() {
  return mqttUsername;
}

String Configuration::getMqttPassword() {
  return mqttPassword;
}

AQUABOTICS_END_NAMESPACE