#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_ 1

#include <FileSystem.h>
#include <ArduinoJson.h>
#include <ArduinoLog.h>

#define config_default_json "{\"unit\":{\"name\":\"SHTech-R\"},\"web\":{\"user\":\"admin\",\"pass\":\"admin\"},\"wifi\":{},\"plugins\":[]}"

AQUABOTICS_BEGIN_NAMESPACE

class Configuration
{
    private:
        AQUABOTICS_NAMESPACE
        FileSystem fs;
    public:
        Configuration();
        Configuration(const Configuration& orig);
        virtual ~Configuration();

};

AQUABOTICS_END_NAMESPACE

#endif