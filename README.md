# Semaphore buffer
## Description
Code demonstrating usage of semaphores and mutexes in esp32 freeRTOS.
## Hardware
For this mini project the only piece of hardware that is needed is an esp32 microcontroller.
## Working principle
We have 3 tasks. The first one is emitting values from 1 to 10 every 150 miliseconds. Those values
are sent to the buffer controlled by the second task. This buffer works as queue. Finally the last task is collecting
those values. When the last data from the last cell is collected, this task calculates the average value of them all. This
value is then sent to idf monitor.
Because the size of the buffer is the same as the range of numbers, the mean value is the same every time.


##
Author: Mateusz Szpot \
Date: 20.02.2025
