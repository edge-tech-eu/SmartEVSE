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
# 1. The Basic configuration

With the basic configuration, you create a charging point without power control. This means that the charging point does not take other power users in the house into account. This can potentially cause problems if the power demand is too high. Therefore, ensure that you only charge when there is sufficient capacity available. This is typically the case from 10 p.m. to 4 a.m. Take into account the possible presence of heat pumps, home batteries and electric boilers. The basic configuration consists of the following components:

- charging module

- DC earth leakage module with connection cable

- charging cable

- housing and cable glands

- mounting brackets


You can easily assemble this housing yourself. You also have the option of using the mounting brackets if the standard mounting of the charging module is not suitable for your situation.


## Connecting the LED

You can connect a 5VDC or 12VDC LED to the charging module.

[Click here](installation/adding%20LED%20to%20charger.pdf) for the connection diagram.


## Assembly steps

- Below is an overview of all connections

![Connections](installation/connecting-charger-module.png)

- Decide where to place the charging point. We recommend always installing a Modbus cable alongside the power cable. Make sure you use the **correct Modbus cable**. This cable, for example, is very suitable: https://www.elektroproducten.nl/product/stuurstroom-flex-liyy-3x05mm%C2%B2-per-meter/

- Choose a convenient location for mounting the glands for the incoming power cable and outgoing charging cable. Drill holes for the glands using a step drill. Also drill holes in the rear wall for secure mounting on a wall or pole. Make sure you seal the screw holes with, for example, a rubber ring or drip sealant.

- Choose the orientation of the charging module – screw it in place or glue the mounting brackets securely to the rear wall of the cabinet.

- Mount the charging module in the cabinet.

- Connect the DC earth leakage module to the charging module using the supplied wire.

- Connect the supplied earth wire (green-yellow) to the grey block with 5 inputs. Place this on input PE. Connect the other side to the supplied WAGO connector.

- Feed the charging cable through the cable gland. Then:

- Insert the wires into the correct outputs on the grey block with 4 outputs. Make sure you use the correct colours for the connection. Connect the earth wire (PE in the diagram / green-yellow) to the supplied separate WAGO connector. You can unlock the output by inserting a flat screwdriver into the rectangular hole – at that point, the wire can be inserted into the output.

  Insert the thin wire into the grey block with two orange levers, select the “PP” output, indicated in the diagram above by the yellow wire with “CP”. Make sure you click the lever all the way up and insert the wire deep enough into the output. Then click the lever back down and check that the wire is securely fastened.


- Now screw the housing onto the wall or pole and insert the power cable through the cable gland. Next:

- Insert the wires of the power cable into the correct inputs, paying close attention to the colours on the grey connector with 5 inputs.

  - Connect the earth wire (PE) to the WAGO connector.

  - Check that the wires in the grey connector do NOT touch each other. This can happen if wires with braided strands are used. Loose copper strands can remain just outside the input and cause a short circuit. So be careful!

- Does everything look good? Then you can activate the earth leakage circuit breaker in the fuse box. Check that the green LED on the charging module flashes every 3 seconds. See here for the location of the green LED:

![LEDs](installation/LEDs.png)

> [!IMPORTANT]
> If the red LED flashes, check that the earth is properly connected, both to the charging module and to the earth of the house.


## Test by charging a car

If you have followed the above steps and the LED on the charging module is flashing green, you can check whether your car can be charged. Connect the charging cable to the car. Is the car charging? Congratulations! If not, check the charging module and see if the red LED is flashing occasionally. Depending on the flashing pattern, you can determine what the fault is:



| Condition                                         | Red LED display                                                  |
|---------------------------------------------------|---------------------------------- -------------------------------|
| Start-up error (00000001)                         | Very long off, very short on                                     |
| High temperature (10010000)                       | Short on, short off, short on, long off                          |
| Temperature too high, charging stopped (10011000) | Short on, short off, 2x short on, long off                       |
| No earth connected (10011100)                     | Short on, short off, 3x short on, briefly off                    |
| Relay remains switched on (10011110)              | Short on, short off, 4x short on, short off                      |
| DC earth fault (10010110)                         | Short on, short off, short on, short off, 2x short on, short off |
| OK                                                | Continuous off                                                   |
