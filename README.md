# Snowplow2019

### Dependecies ###

- git
  - `sudo apt install git`
- sdl 1.2
  - `sudo apt install sdl1.2`
- ubuntu 16.04

## Lua is not currently needed or supported ##
- lua5.1
  - `sudo apt install lua5.1`
  - `sudo apt install liblua5.1-0`
  - `sudo apt install liblua5.1-0-dev`
  - `sudo apt install liblua5.1-0-dbg`
  - `sudo apt install lua5.1-doc`

### Building ###
# CPJL (From CPJL/lib/server/v3)
`bash build.sh`

# Snowplow code (from Snowplow2019)
`bash build.sh --lib`
`bash build.sh --bin`
`bash build.sh --nodes`

### Adding udev rules ###

- add udev files to:

    `/etc/udev/rules.d`

- reset the udev rules using:

    `sudo udevadm control --reload-rules`

### Computer login ###

username: thundar
password: password

### Router login ###

Address: 192.168.0.2

username: admin
password: admin

### Remote login ###

connect to `hide yo kids, hide you Wifi` network
password is `password` *Very secure*

ssh into either 192.168.0.100 or 192.168.0.101 with the credentials above

- use ssh to start VNC
    - `vnc server`
