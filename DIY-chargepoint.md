# DIY charging station
Look, we've done all the prep work for you!
We'll now help you put everything together step by step.

Follow the steps in the overview below:
1. [The basic configuration](installation/module-kl.png) – the “dumb charging station”, i.e. without load balancing
2. With power control (load balancing). Then choose one of the options below:
- Use an ESP device or a Pi pico connected to a DSMR 4 or 5 smart meter and connect it to the charging station via an RS485 cable (Modbus)
    - Use a Raspberry Pi mini-computer with a USB cable connected to a DSMR 4 or 5 smart meter and an RS485 interface. Use the EVCC package to control the charging station
    - Use the Edgetech WiFi module:
1. Do you have a DSMR5 smart meter? Then you can connect the EVSE-SC-02 module directly to the smart meter
2. Do you not have a smart meter or do you have an older type? Then the current must be measured with current clamps (CT) and you will need type EVSE-SC-01.

> [!IMPORTANT]
> This document is written for experienced hobbyists. In other words, people who know how to safely connect devices to a 3-phase power cable.
> People who know what earth leakage circuit breakers are and understand the basic principles of connecting large power consumers via the meter box.
> We always recommend that changes to the meter box be carried out by a certified electrician.

# 1. The basic configuration
With the basic configuration, you can create a charging point without a current transformer.

Translated with DeepL.com (free version)
