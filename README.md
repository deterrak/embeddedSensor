# embeddedSensor
This project is for an embedded IR sensor that interfaces to Extreme's workflow Composer via a webhook. The goal of this project was to use a lazer tag gun system as an example of a legacy system and a target composed of an infrared sensor and a Beaglebone black acting as an Internet of Things (IoT) device. The goal of this project was to provide a demo system that shows how to interface a legacy system with the world of automation and orchestration. 

The demo is as follows a user aims a lazertag gun at the sensor and shoots the target. After 10 sucessfull shots are executed a webhook is launched that contains a JSON payload similar to this. 

## JSON Payload

````
{
    "username": "user",
    "description": "iot-1",
    "hostname": "beaglebone",
    "value": "111111111",
    "trigger": "IR_Sensor#1",
    "gpio": "gpio66",
    "password": "********",
    "ipaddress": "192.168.1.115"
  }
````

The workflow will configure a switch to allow multicast video to flow from sender to receiver to a period of 60 seconds. Each step of the workflow will update a SLACK channel with the current status. Once 60 seconds has passed the switch will be reconfigured to block the video traffic so the next user can take a turn. 

## Multiple Components
In this project there are multiple components:
* Embedded BeagleBone Processor running QT/C++ code
* Extreme Network's Workflow Composer instance running on either bare-metal or a virtual machine
* Video sender connected to a video receiver via a switch (our example uses a Linux based switch, but any switch with an API would do)


## How to Compile

The code complies on QT version 5.9.2 and runs on the beaglebone black ARM processor. 

I've developed a custom 'cape' circuit board that matches the GPIO to the IR sensors and LEDs.

See the code for the appropriate GPIO pins.

# Interfacing to Workflow Composer
The Beaglebone black IoT uses a webhook to interface to Workflow Composer. A webhook is a http POST method carrying JSON formatted informtation over SSL. Essnetially it is a REST API.

## Workflow Composer Rule
The Workflow Composer uses Rules as a way to link up incomming sensor informaiton to the right workflow. 
````
---  
description: 'Start the BeagleTag Workflow'  
tags: []  
type:  
  ref: standard  
  parameters:  
enabled: true  
name: beagleTag  
trigger:  
  type: core.st2.webhook  
  parameters:  
    url: beagleTag  
criteria:  
action:  
  ref: default.beagleTagEnableNetworking  
  parameters:  
    username: user  
    description: '{{trigger.body.description}}'  
    hostname: '{{trigger.body.hostname}}'  
    value: '{{trigger.body.value}}'  
    trigger: '{{trigger.body.trigger}}'  
    gpio: '{{trigger.body.gpio}}'  
    password: $password$  
    ipaddress: '{{trigger.body.ipaddress}}'  
pack: core  
````


## Workflow Composer Workflow
Here is the actual workflow. We cna see the inputs that we expect the rule to provide and each task within the workflow are defined here.
````
---  
version: '2.0'  
  
default.beagleTagEnableNetworking:  
  input:  
    - username  
    - description  
    - hostname  
    - value  
    - trigger  
    - gpio  
    - password  
    - ipaddress  
    - switch_ip  
  tasks:  
    WorkflowStarted:  
      # [365, 26]  
      action: chatops.post_message  
      input:  
        message: '<% $.hostname %> started workflow with trigger <% $.trigger %>'  
        user: "test-bot"  
        channel: "#demo"  
      on-success:  
        - EnableNetwork  
    EnableNetwork:  
      # [365, 128]  
      action: core.remote_sudo  
      input:  
        hosts: '<% $.switch_ip %>'  
        username: '<% $.username %>'  
        password: '<% $.password %>'  
        cmd: "ifconfig tap0 up"  
      on-success:  
        - videoPlays60Sec  
        - VideoPlaying  
    videoPlays60Sec:  
      # [235, 230]  
      action: campus_ztp.delay  
      input:  
        seconds: 60  
      on-success:  
        - DisableNetwork  
    VideoPlaying:  
      # [495, 230]  
      action: chatops.post_message  
      input:  
        message: 'Network Established on switch <% $.switch_ip %> / Video Playing'  
        user: "test-bot"  
        channel: "#demo"  
    DisableNetwork:  
      # [235, 332]  
      action: core.remote_sudo  
      input:  
        hosts: '<% $.switch_ip %>'  
        username: '<% $.username %>'  
        password: '<% $.password %>'  
        cmd: "ifconfig tap0 down"  
      on-success:  
        - WorkflowEnded  
        - ResetDisplay  
    WorkflowEnded:  
      # [105, 434]  
      action: chatops.post_message  
      input:  
        message: 'Disabled Video on switch <% $.switch_ip %> after 60 seconds. Workflow ended.'  
        user: "test-bot"  
        channel: "#demo"  
    ResetDisplay:  
      # [365, 434]  
      action: core.remote  
      input:  
        hosts: '<% $.ipaddress %>'  
        username: '<% $.username %>'  
        password: '<% $.password %>'  
        cmd: "python ~/BeagleTag/display_off.py"          
````
