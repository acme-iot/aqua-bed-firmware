#include <Configuration.h>

AQUABOTICS_BEGIN_NAMESPACE

const String ConfigurationFile = "/config.json";

Configuration::Configuration() {}

Configuration::Configuration(const Configuration &orig) {}

Configuration::~Configuration() {}

void Configuration::begin() {
  fs.begin();
}

void Configuration::load(String doc) {

}

AQUABOTICS_END_NAMESPACE