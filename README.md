![alt text](https://cftechsol.com/wp-content/uploads/2017/12/caiofrota-logo-300x171.png)

# Arduino CF Wi-Fi Thermostat

IoT - Configurable thermostat using ESP8266 (ESP01 or NodeMCU 1.0 ESP-12E), DHT (11 or 22) and ThingsBoard as server.
Version 1.0.0

## Getting Started

These instructions will get you a copy of the project up and running on your device for personal purposes. See [Running](#running) for notes on how to deploy the project on your device.

### Running

* Clone this repository
* Open /src/cf-thermostat/cf-thermostat.ino
* Upload your project

For more details our visit [Wiki](https://github.com/cftechsol/cf-iot-thermostat/wiki).

### Pre-requisites

* [Arduino IDE](https://arduino.cc/) - IDE to compile and upload program.
* [ESP8266 Configuration](https://dzone.com/articles/programming-the-esp8266-with-the-arduino-ide-in-3) - ESP8166 Configuration.
* [ThingsBoard](https://thingsboard.io/) - ThingsBoard to use as monitoring server.

### Libraries
* [Adafruit Unified Sensor by Adafruit version 1.1.4](https://github.com/adafruit/Adafruit_Sensor/releases/tag/1.1.4)
* [Adafruit ESP8266 by Adafruit version 1.1.0](https://github.com/adafruit/Adafruit_ESP8266/releases/tag/1.1.0)
* [DHT sensor library by Adafruit version 1.3.10](https://github.com/adafruit/DHT-sensor-library/releases/tag/1.3.10)
* [WiFiManager by tzapu version 0.15.0](https://github.com/tzapu/WiFiManager/releases/tag/0.15.0)
* [ArduinoJson by Benoit Blanchon version 6.16.1](https://github.com/bblanchon/ArduinoJson/releases/tag/v6.16.1)
* [ThingsBoard by ThingsBoard Team version 0.4](https://github.com/thingsboard/ThingsBoard-Arduino-MQTT-SDK/releases/tag/v0.4.0)
** [PubSubClient by Nick O'Leary version 2.6](https://github.com/knolleary/pubsubclient/releases/tag/v2.6)

### Hardware
* ESP8266 (NodeMCU 1.0 ESP-12E or ESP01)
* DHT (11 or 22) Module (Digital Humidity and Temperature Serson)

## Support or Contact

Contact us at contact@cftechsol.com for questions and we'll help you sort it out.

## Issues

Find a bug or want to request a new feature? Please let us know by [submitting an issue](https://github.com/cftechsol/cf-iot-thermostat/issues).

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/caiofrota/6e65a17fd3bf100d058cb48dcc780b21) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* **Caio Frota** - _Initial work_ - [caiofrota](https://github.com/caiofrota) | [CF Technology Solutions](https://cftechsol.com)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
