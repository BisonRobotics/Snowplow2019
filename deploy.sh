#!/bin/bash

# script to deploy the most up-to-date code to the robot

scp -r ./ thundar@192.168.0.100:~/Snowplow2019/
