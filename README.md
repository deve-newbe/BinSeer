<img width="930" height="887" alt="image" src="https://github.com/user-attachments/assets/fe05c3a7-76c8-4c80-b3eb-569724ab8f2d" /># Fynx
ELF/DWARF Visualizer and off-line calibration tool

This application is able to process .elf files containing both ELF and DWARF information. It is aimed at cross-compiled binaries for microcontrollers.

The tool allows to offline calibrate the .hex binaries. In a nutshell, off-line calibration allows for changes in behaviour of the code by adjusting symbol values without the need to recompile.

![alt text](https://github.com/deve-newbe/Fynix/blob/main/Res/flow.png "Logo Title Text 1")

# Configure project (STM32CubeIDE)

Generate ELF with DWARF 4 debug information

![alt text](https://github.com/deve-newbe/Fynix/blob/main/Res/config_debug.png "Logo Title Text 1")

Generate HEX file

![alt text](https://github.com/deve-newbe/Fynix/blob/main/Res/config_hex.png "Logo Title Text 1")
