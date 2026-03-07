#pragma once

typedef unsigned long long uint64;
typedef unsigned short uint16;

// defined by SensorBus.dbc
namespace Frame {
    constexpr int CanId{1797};
    constexpr int BitStart{32};
    constexpr int PacketLen{64};
    constexpr int DataLen{16};
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
 * Extracts bytes, converts to big endian and uses calibration constants to
 * scale and offset raw data into valid speed.
 * @param packet full data packet after # from can log entry
 * @return value converted to signed number and adjusted by calibration constants
 */
double getSpeed(uint64 packet);

