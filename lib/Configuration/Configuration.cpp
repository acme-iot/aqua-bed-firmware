#include <Configuration.h>

AQUABOTICS_BEGIN_NAMESPACE

const String ConfigurationFile = "/fs/config.json";

Configuration::Configuration() {
    fs.begin();
}

Configuration::Configuration(const Configuration &orig) {}

Configuration::~Configuration() {}

AQUABOTICS_END_NAMESPACE