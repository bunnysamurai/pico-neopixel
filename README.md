# pico-neopixel
Baseline project for interacting with NeoPixels, built on the RP2040

# Serial Interface
Provide a shell upon which commands can be issued

Command List
- MODE
  - SET : Individual PIXEL control
  - PATTERN : choose from a list of preset patterns
  - CLOCK : run as a clock!
- START
- STOP
- SET
    - SET W R G B
    - SET INDX W R G B
- PATTERN [SINE, TEST, etc]
- CONF