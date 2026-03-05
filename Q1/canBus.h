#pragma once

typedef unsigned long long uint64;
typedef unsigned short uint16;

// defined by SensorBus.dbc
namespace Frame {
    constexpr int CanId{1797};
    constexpr int BitStart{32};
    constexpr double Scale{0.1};
    constexpr int Offset{0};
};

/**
 * Returns bit masked packet in little endian
 * @param packet full data packet after #
 * @return 2 byte data at bitStart and bitStart + 8(bits)
 */
uint16 getBytes(uint64 packet);

/**
 * Converts 2 byte (16 bit) unsigned short from little to big endian
 * @param rawData 2 Byte int in little endian form
 * @return number converted to big endian
 */
uint16 convBigEndian(uint16 rawData);

/**
 * uses calibration constants to scale and offset raw data into valid speed
 * @param rawData data extraxted from can bus packet
 * @return value converted to signed number and adjusted by calibration constants
 */
double convSpeed(uint16 rawData);

