# RTC_Scheduler
This is a realtime scheduler component for ESPHome that allows the scheduling of switches independantly of Home Assistant. To acheive this it requires a couple of hardware components (DS3231 & 24LCxx E2). the easiest way to add these devices is via a ZS-042 (https://www.google.com/search?q=zs-04).
Note there is a modification needed to the module. Remove the 1N4148 and/or the 200R resistor and everything is fine.
