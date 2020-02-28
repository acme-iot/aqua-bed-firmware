[![Build Status](https://travis-ci.org/acme-iot/aqua-bed-firmware.svg)](https://travis-ci.org/acme-iot/aqua-bed-firmware)

# Aquaponic firmware for planter

## Pre steps

#### Did you install PlatformIO's CLI?

##### on mac,
1. `pip install -U https://github.com/platformio/platformio-core/archive/develop.zip`
2. `vi ~/.bashrc` (or .profile or zshrc...) and add `export PATH="$PATH:~/.platformio/penv/bin"`
3. close your terminal and reopen or `source ~/.bashrc` to reload the PATH
4. (optional) activate the virtual environment, `. activate` which should now be in the PATH
5. can you run the cli? `pio`? if not go here, https://docs.platformio.org/en/latest/installation.html

#### PlatformIO extension to VSCode
1. Just search for and install the extension `PlatformIO`
2. Open `settings.json` in VSCode (from the Command Palette type `Preferences Open Settings (JSON)`)
3. Add the following (inside `{...}`),
   ```
    "platformio-ide.customPATH": "~/.platformio/penv/bin",
    "platformio-ide.useDevelopmentPIOCore": true
   ```
    **note**, customPATH probably isn't necessary since we added it to the `PATH` 
4. Restart VSCode

## Running/Uploading the program to a board
1. use the cli an run , `pio run` this will take a minute to fetch the board firmware and build it. Look in the newly created folder */.pio* to see what is pulled down
2. once everything is built then you need to upload the program (*/src/main.cpp*) to a board, this example uploads to the ESP8266 board, `pio run -e planter -t upload`. Note, if you have all your boards connected via a serial port then you don't need to specify `-e` or which environment to upload to. `upload_port` is pegged to a specific port, this should change as needed.

## Monitor Serial port
`pio device monitor -b 115200`

## Notes
This project doesn't need VSCode or the PlatformIO extension in VSCode. If you look closely at the commands executed, all that is needed is PlatformIO Core and the included CLI.

### Testing MQTT
Once the board is flashed it will run and connect to the MQTT broker. The board will subscribe to `test/lol` as well as publish to this topic. To test the board's subscritpion make sure **mosquitto** is installed `brew install mosquitto` then run from the terminal `mosquitto_pub -t 'test/topic' -m 'helloWorld' -h <IP OF BROKER>`. The board should receive the payload `Helloworld`