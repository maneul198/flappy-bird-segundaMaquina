#include "enginesv.h"
#include <dpci_core_api.h>
#include <dpci_api_types.h>
#include <dpci_api_decl.h>

EngineSV::EngineSV(const string &i2cFileName, QObject *parent)
    : QObject(parent), i2cFileName(i2cFileName)
{
}

EngineSV::EngineSV(const int bus, QObject *parent)
    : QObject(parent), m_bus(bus)
{
    dpci_i2c_write8(0x0, 0x40, 0x0, 0x0);
    dpci_i2c_write8(0x0, 0x40, 0x1, 0x0);
}

EngineSV::~EngineSV()
{
}

int EngineSV::bus() const
{
    return m_bus;
}

void EngineSV::turnOnEngine(const array<uchar, 3> &eng1, int time_spin)
{
    int retVal = dpci_i2c_write8(m_bus, eng1.at(MCP_ADDR) , eng1.at(REG_ADDR), eng1.at(DATA));

    if (retVal < 0)
        throw std::runtime_error(errorMessage());

    // El timer se ejecutara luego 2.4 (por defecto) segundos
    // es el tiempo que tarda en dar una vuelta

    if(time_spin > 0){
        QTimer::singleShot(time_spin, [ &, eng1]() {
            qDebug() << "se apaga el hook" << endl;
            turnOffEngine(eng1);
            emit engineStoped();
    });
    }

}

void EngineSV::turnOnLight(const array<uchar, 3> &li)
{
    int retVal = dpci_i2c_write8(m_bus, li.at(MCP_ADDR) , li.at(REG_ADDR), li.at(DATA));

    if (retVal < 0)
        throw std::runtime_error(errorMessage());

}

//! SLOTS
void EngineSV::turnOffEngine(const Engine &eng)
{
    int retVal = dpci_i2c_write8(m_bus, eng.at(MCP_ADDR), eng.at(REG_ADDR), 0);

    if (retVal < 0)
        throw std::runtime_error(errorMessage());

    emit engineFinished();
}

void EngineSV::turnOffLight(const Engine &li)
{
    int retVal = dpci_i2c_write8(m_bus, li.at(MCP_ADDR), li.at(REG_ADDR), 0);

    if (retVal < 0)
        throw std::runtime_error(errorMessage());

    qDebug() <<  "Se emite engine finish" << endl;
    emit engineFinished();
}

void EngineSV::turnOffEngines()
{
    for (auto &e : eng) {
        int retVal = dpci_i2c_write8(m_bus, e.at(MCP_ADDR) , e.at(REG_ADDR), 0);

        if (retVal < 0)
            throw std::runtime_error(errorMessage());
    }
}

void EngineSV::turnOffLights()
{
    for (auto &l : li) {
        int retVal = dpci_i2c_write8(m_bus, l.at(MCP_ADDR), l.at(REG_ADDR), 0);

        if (retVal < 0)
            throw std::runtime_error(errorMessage());
    }
}

void EngineSV::init() const {
    int retval= dpci_i2c_write8(m_bus, mcp23017_1, IODIRA, 0);

    if(retval < 0)
        throw std::runtime_error(errorMessage());

    retval= dpci_i2c_write8(m_bus, mcp23017_1, IODIRB, 0);

    if(retval < 0)
        throw std::runtime_error(errorMessage());
}

string EngineSV::errorMessage() const
{
    return std::string("Write to I2C Device failed: ")
           + dpci_i2c_error_string(dpci_i2c_last_error());
}

const uchar EngineSV::OLATA;
const uchar EngineSV::OLATB;
const uchar EngineSV::IODIRA;
const uchar EngineSV::IODIRB;

const uchar EngineSV::mcp23017_1;
const uchar EngineSV::mcp23017_2;

const size_t EngineSV::MCP_ADDR;
const size_t EngineSV::REG_ADDR;
const size_t EngineSV::DATA;

const constexpr array<const Engine, 10> EngineSV::eng;
const constexpr array<const Engine, 10> EngineSV::li;

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
#include "moc_enginesv.cpp"
