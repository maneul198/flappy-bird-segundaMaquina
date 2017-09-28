#ifndef DIGITAL_PORTS_H
#define DIGITAL_PORTS_H

#include <cstddef>
#include <array>

namespace DPX {
// DIGITAL I/O PORTS
static const unsigned int PORT0 = 0x0;
static const unsigned int PORT1 = 0x1;
static const unsigned int PORT2 = 0x2;
static const unsigned int PORT3 = 0x3;

// BIT MASK
static const unsigned int DO0 = 1 << 0;
static const unsigned int DO1 = 1 << 1;
static const unsigned int DO2 = 1 << 2;
static const unsigned int DO3 = 1 << 3;
static const unsigned int DO4 = 1 << 4;
static const unsigned int DO5 = 1 << 5;
static const unsigned int DO6 = 1 << 6;
static const unsigned int DO7 = 1 << 7;

static const unsigned int DI0 = 1 << 0;
static const unsigned int DI1 = 1 << 1;
static const unsigned int DI2 = 1 << 2;
static const unsigned int DI3 = 1 << 3;
static const unsigned int DI4 = 1 << 4;
static const unsigned int DI5 = 1 << 5;
static const unsigned int DI6 = 1 << 6;
static const unsigned int DI7 = 1 << 7;

// Alias for Line or port
using Line = unsigned int;

inline Line toLine(unsigned int port, unsigned int pin)
{
    return (1 << (port * 8)) << pin;
}

constexpr unsigned int DIGITAL_IO_PORTS{ 4 };
constexpr unsigned int DIGITAL_INPUT_LINES{ 8 * DIGITAL_IO_PORTS };
constexpr unsigned int DIGITAL_OUTPUT_LINES{ 8 * DIGITAL_IO_PORTS };

constexpr std::size_t GET_PORT{ 0 };
constexpr std::size_t GET_PIN{ 1 };

static const std::array<std::array<unsigned int, 2>, DIGITAL_INPUT_LINES> DI {
    {
        {PORT0, DI0}, {PORT0, DI1},
        {PORT0, DI2}, {PORT0, DI3},
        {PORT0, DI4}, {PORT0, DI5},
        {PORT0, DI6}, {PORT0, DI7},
        
        {PORT1, DI0}, {PORT1, DI1},
        {PORT1, DI2}, {PORT1, DI3},
        {PORT1, DI4}, {PORT1, DI5},
        {PORT1, DI6}, {PORT1, DI7},
        
        {PORT2, DI0}, {PORT2, DI1},
        {PORT2, DI2}, {PORT2, DI3},
        {PORT2, DI4}, {PORT2, DI5},
        {PORT2, DI6}, {PORT2, DI7},
        
        {PORT3, DI0}, {PORT3, DI1},
        {PORT3, DI2}, {PORT3, DI3},
        {PORT3, DI4}, {PORT3, DI5},
        {PORT3, DI6}, {PORT3, DI7}
    }};
    
static const std::array<std::array<unsigned int, 2>, DIGITAL_OUTPUT_LINES> DO {
    {
        {PORT0, DO0}, {PORT0, DO1},
        {PORT0, DO2}, {PORT0, DO3},
        {PORT0, DO4}, {PORT0, DO5},
        {PORT0, DO6}, {PORT0, DO7},
        
        {PORT1, DO0}, {PORT1, DO1},
        {PORT1, DO2}, {PORT1, DO3},
        {PORT1, DO4}, {PORT1, DO5},
        {PORT1, DO6}, {PORT1, DO7},
        
        {PORT2, DO0}, {PORT2, DO1},
        {PORT2, DO2}, {PORT2, DO3},
        {PORT2, DO4}, {PORT2, DO5},
        {PORT2, DO6}, {PORT2, DO7},
        
        {PORT3, DO0}, {PORT3, DO1},
        {PORT3, DO2}, {PORT3, DO3},
        {PORT3, DO4}, {PORT3, DO5},
        {PORT3, DO6}, {PORT3, DO7}
    }};
}
#endif // DIGITAL_PORTS_H

