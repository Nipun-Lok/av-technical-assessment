#include <iostream>
#include <fstream>
#include <string>
#include "canBus.h"
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

int main(void) {
    std::ifstream canLog{"../Q1/candump.log"};
    if (!canLog) throw std::runtime_error("File not found\n");
    std::ofstream output{"../Q1/output.txt"};
    if (!output) throw std::runtime_error("Cannot open file\n");

    // initialise fstream formats
    output.setf(std::ios::fixed, std::ios::floatfield);
    output.precision(1);
    canLog >> std::hex;

    std::string time{};
    // dummy string to skip "vcan0"
    std::string buffer{};
    int packetId{};
    while (canLog >> time >> buffer >> packetId) {
        if (packetId == Frame::CanId) {
            // skip '#'
            canLog.ignore();

            uint64 packet{};
            canLog >> packet;
            double speed{convSpeed(convBigEndian(getBytes(packet)))};
            output << time << ": " << speed << '\n';
        } else {
            // skip rest of packet
            canLog >> buffer;
        }
        // skip to next line
        canLog.ignore();
    }
    canLog.close();
    output.close();
    return 0;
}

//=====================   Helper functions    ===========================

uint16 getBytes(uint64 packet) {
    return static_cast<uint16>((packet >> Frame::BitStart) & 0xFFFF);
}

uint16 convBigEndian(uint16 rawData) {
    return rawData >> 8 | rawData << 8;
}

double convSpeed(uint16 rawData) {
    // reverse byte order and apply 2's complement sign check 
    short rawValue{static_cast<short>(convBigEndian(rawData))};
    // apply scaling and offset
    return Frame::Offset + Frame::Scale * rawValue;
}
