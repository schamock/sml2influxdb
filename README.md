# SML2InfluxDB
## What is it all about?
The purpose of this programm is to read out the metrics of a smart meter, interpret the values and to store them in an InfluxDB v2.

This is helpful for tracking the energy consumption, energy generation (e.g. PV) and collection the meter readings.

## Caution!!!
I'm by no means an expert in developing C code (which I use in this project) and also no expert for software development in general! If you would like to use this code, please make sure, that you understand the code and confirm, that the program won't harm your environment.

## My setup a.k.a. what seams to work
I use this project in the following environment with success:
* The program runs on an Raspberry Pi 4 (should work on any RPi with an USB port...)
* The RPi runs RapsberryPi OS 64bit on Buster basis (a bit old at this point, newer installation should work well too)
* I use this IR/USB-adpater (german shop): https://shop.weidmann-elektronik.de/index.php?page=product&info=24  
I'm happy with this one, I assume any non-trash adapter should work
* My smart meter is an Easymeter Q3A. Any other smart meter, that "talks" via SML could work, but i can't confirm this.
* The InfluxDB v2 is installed on another machine.

## How to use

### Prerequisites
The following tools should be installed:
```
apt install build-essential libcurl4-openssl-dev
```
Download the code however you wish (git, download from github, ...) and extract it.

### Compiling and Installation
You need to tweak atleast two files to match your needs:
1. **include/config.h**  
Copy `include/config.default.h` to `include/config.h` and adjust the parameter as described in the file.
2. **accessory/sml2influxdb.default.service**  
Copy `accessory/sml2influxdb.default.service` to a`ccessory/sml2influxdb.service` and adjust the user name to the user, that should be used to run the programm as service (see later)
3. **adjust anything else**  
Check, if you want to adjust anything else. This could be useful, if you use another smart meter or you want to save other values to the InfluxDB.

After this, you can compile and install the program
* Go to the "main" directory where you find this Readme
* run `make` to compile
* run `sudo make install` to put the binary to /usr/local/bin/

### Run as systemd service
If you don't want to start the program yourself and you have a systemd-based systemd (like Debian/RaspberryPi OS), you can use the custom service file:
* run `sudo make systemd-install`
* After that, you need to tell systemd to activate and start the program:
```
sudo systemctl enable sml2influxdb.service
sudo systemctl start sml2influxdb.service
```

The script should now start automatically after it crashes or you reboot the host.  
It is a good idea to check the logs from time to time, to see if there are any errors and/or if systemd needs to restart the program for whatever reason:
```
# Check the latest logs...
journalctl -r -u sml2influxdb.service
# ... or check the "tail -f"-like "liveview"
journalctl -f -u sml2influxdb.service
```

## Which data is collected?
Currently the programm collects the following data:
* **Value 1.8.0**: This is the meter reading, showing how much energy you bought from the grid
* **Value 2.8.0**: This is the meter reading, showing how much energy you sold to the grid
* **Voltage L1**: Current voltage on phase 1
* **Voltage L2**: Current voltage on phase 2
* **Voltage L3**: Current voltage on phase 3
* **Current power L1**: Current power on phase 1 (negativ: selling, positiv: buying)
* **Current power L2**: Current power on phase 2
* **Current power L3**: Current power on phase 3
* **Current total power**: sum of all three phases (which is typically the most interessting power value...)

## Enjoy
If everything goes well, your InfluxDB should now receive lots of data and you have to decide, what to do with it :)
