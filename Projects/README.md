# Project Descriptions

This repository is essentially a collection of some solo projects I've worked on over
the past year. This includes schematics, PCB layouts, and firmware. 

* **Emotion Sensor**

The firmware folder contains code for the on board microcontroller used in the designs in 
the GSR sensor folder. This folder contains a schematic and layout for a wearable,
wireless sensor I designed for a professor at OSU. The sensor measure galvanic skin 
response, which is essentially minute changes in skin conductance. This data is then 
transmitted to a computer for processing. The sensor is equipped with an accelerometer 
for measuring motion as well. The firmware consists of a simple API for a wireless 
transceiver as well as code for driving a battery management IC, a digital
potentiometer, and an accelerometer. Also included are libraries and code for 
communication protocols such as I2C SPI and USART. More information on this project
can be found at the following website, 
http://blogs.oregonstate.edu/industry/2014/03/12/art-meets-engineering/

* **Motor Controller PCB**

The Motor Controller folder consists of designs for a PCB used for Oregon State 
University's TekBot, which is a learning platform used to teach introductory level 
electrical engineering classes that teach principals in building circuits and programming
embedded systems. The boards purpose is to provide a socket for a microcontroller 
breakout board which provides connections to a motor controller. More information on the 
board and how it integrates with the TekBot can be seen at this website, 
http://eecs.oregonstate.edu/education/hardware/mtr_ctlr.4/

This collection is a sample of the projects I've worked on. Please inquire further about
other projects. 