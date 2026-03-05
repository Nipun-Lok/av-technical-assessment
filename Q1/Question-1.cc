#include <iostream>
#include <fstream>
#include <string>

// Question 1: This is an extension task that requires you to decode sensor data from a CAN log file.
// CAN (Controller Area Network) is a communication standard used in automotive applications (including Redback cars)
// to allow communication between sensors and controllers.
//
// Your Task: Using the definition in the Sensors.dbc file, extract the "WheelSpeedRR" values
// from the candump.log file. Parse these values correctly and store them in an output.txt file with the following format:
// (<UNIX_TIME>): <DECODED_VALUE>
// eg:
// (1705638753.913408): 1234.5
// (1705638754.915609): 6789.0
// ...
// The above values are not real numbers; they are only there to show the expected data output format.
// You do not need to use any external libraries. Use the resources below to understand how to extract sensor data.
// Hint: Think about manual bit masking and shifting, data types required,
// what formats are used to represent values, etc.
// Resources:
// https://www.csselectronics.com/pages/can-bus-simple-intro-tutorial
// https://www.csselectronics.com/pages/can-dbc-file-database-intro

typedef unsigned long long uint64;
typedef unsigned short uint16;

// defined by SensorBus.dbc
namespace Frame {
    constexpr int CanId{1797};
    constexpr int BitStart{32};
    constexpr double Scale{0.1};
    constexpr int Offset{0};
};

// remove format specifiers (vcan0) and replace '#' delimiter with space
// modifies in-place
void cleanLine(std::string& line) {
    line.erase(line.find("vcan0 "),6);
    line.replace(line.find('#'), 1, " ");
}

// returns bit masked packet in little endian
uint16 getBytes(uint64 packet) {
    return static_cast<uint16>((packet >> Frame::BitStart) & 0xFFFF);
}
