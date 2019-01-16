# Snowplow2019

### Dependecies

- ubuntu 16.04 or greater
- sdl 1.2
  - `sudo apt install sdl1.2`

### Building

`bash build.sh --lib`
`bash build.sh --bin`

### Adding udev rules

- add udev files to:

    `/etc/udev/rules.d`

- reset the udev rules using:

    `sudo udevadm control --reload-rules`

### Computer login

username: thundar

password: password

### Remote login

connect to hide yo kids, hide you Wifi network

ssh into either 192.168.1.100 or 192.168.1.101 with the credentials above

- use ssh to start VNC
    - `vnc server`
