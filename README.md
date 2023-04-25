# Fan communication daemon for openFFUcontrol
This daemon used for interfacing various vendor specific fan bus systems into an ethernet network in the open source filter fan unit control system openFFUcontrol.
It provides a network based clear-text api on a TCP port for communication
with an openFFUcontrol server. The application needs libebmbus and
openffucontrol-qtmodbus to be installed.

## Building and installing
First make sure to have Qt5 installed on your system.
Create a directory for the build
```
mkdir bin
```

Create the Makefile
```
cd bin
qmake ../src
```

Compile the application wih a number of concurrent jobs of your choice

```
make -j 8
```

Install the application as root user
```
sudo make install
```

After the first installation copy the config example
```
sudo cp /etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini.example /etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini
```

Edit the configuration according to your needs
```
sudo vi /etc/openffucontrol/ebmbus-cmd/ebmbus-cmd.ini
```

The daemon can be managed with systemctl
```
sudo systemctl start ebmbus-cmd
sudo systemctl status ebmbus-cmd
sudo systemctl stop ebmbus-cmd
```

Automatic start at boot time is controlled with 

```
sudo systemctl enable ebmbus-cmd
sudo systemctl disable ebmbus-cmd
```

After startup test the communication with netcat
```
nc localhost 16001
```
You should get an interactive shell which says hello at the beginning. Type
help to get all available commands.
