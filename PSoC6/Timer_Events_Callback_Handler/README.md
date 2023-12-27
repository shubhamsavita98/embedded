# HAL: Utilizing Hardare Timer/Counter Interrupt functionality to control LED Toggling and polling of Enter key press check to pause and resume.

## This program implements below functionalities:

1) PSoC6 Hardware Timer
2) HAL APIs
3) UART Communication
4) Timer Interrupts
5) Keyboard Enter Key press check

## Summary

While CPU is busy checking Enter Key pressed, a periodic interrupt is triggered from the hardare timer on PSoC running at a freqency of 10KHz and a period of 20000. As soon as counter is elasped interrupt is triggered and LED is toggled. It uses up counter. Timer can be started and stopped using Enter Key from the keyboard. UART is enabled to read the key input and prints on the terminal.

