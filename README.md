# battery-analyzer
Open Source battery analyzer

# Resistor Matching
When building your channel boards, make sure to match the resistors. See `firmware/src/resistorMatcher.cpp` for details.

# Calibration

1. Calibrate Voltage: connect a PSU to the plus pole of the battery, apply about 4.2V. Measure the voltage and enter it.
1. Zero PWM: 