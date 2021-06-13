# Magnetic Field Controlled Robot

*This project is done 2016

## Goal:
Create the robot and the controller that uses magnetic field to communicate each other. 

## How it works:
At the front of the robot, there are two solenoid attached. One solenoid is attached for the controller as well. 
When the robot and the controller are close, the magnetic field from the robot's solenoids get interrupt by controller's solenoid. 
This interrupt solenoid signal is converted to ADC converter and move the wheels of the robot accordingly. 
As an example, if the left side of the robot's solenoid get interrupted more than the right side, then the left wheel of the robot will move backward. So the robot and the controller always can keep the constant distance.

# Two modes:
* Track mode: The robot adjust its direction and motion by itself to keep a fixed distance from the transmitter.
* Command mode: A WII Nunchuk Controller is connected to the robot by I2C serial connection, and the robot can move with the Nunchuk. 

## Component used:
* Two C8051 Microcontroller: one for the robot and one for the controller.
* Comparator: it simply compares the voltage from robot's left and right solenoid.
* P and N channel MOSFETs: They are used to amplify the signals. 