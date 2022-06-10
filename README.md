# joystick

Simple library to get joystick and buttons states from a any USB videogame controller. The lib also gives access to the haptic feedback presents in most of the controller (vibration motor).

# Building source code

To build the project run:
```bash
cd joystick
mkdir build && cd build
cmake .. && make
```

# Demonstration app

When the project have been built, you can run:
```bash
./joystick -h
```
to get the demonstration app usage.

# Example
Open the ![main.cpp](cpp:src/main.cpp) file to get an example how to use the lib.
