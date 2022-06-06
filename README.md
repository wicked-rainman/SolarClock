# SolarClock

Not to be confused with a clock that's powered by solar energy. That's too useful. Instead, this project represents an attempt at calculating what the time is based on the position of the sun. Yes, you could look at your watch, get the time from the Internet, listen to the radio or even look at a classic garden sun dial. That's not the point. I won't pretend this project has any value - It's more of a nonsense machine, but I think it will be interesting to see how accurate a time can be calculated by using a bunch of electronic hardware and 3D printed parts.
 
There will probably be two components in all of this:
- A "Stepper motor server" being driven via an M5Atom-matrix, although any esp32/Arduino type platform would work. I chose the Matrix because I had one spare and could make use of the LED array for informative visual effects.
- A "Stepper client" that will read light values and instruct the server how far to rotate after each reading. This component is based round an M5Stick because it's got a built in battery which removes the need for trailing wires.

This is a work in progress
