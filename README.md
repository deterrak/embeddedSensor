# embeddedSensor
This project is for an embedded IR sensor that interfaces to Extreme's workflow Composer via a webhook.

The code complies on QT version 5.9.2 and runs on the beaglebone black ARM processor. 

I've developed a custom 'cape' that matches the GPIO to the IR sensors and LEDs.

See the code for the appropriate GPIO pins.


# Workflow Composer rule
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


# Workflow Composer Workflow
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
