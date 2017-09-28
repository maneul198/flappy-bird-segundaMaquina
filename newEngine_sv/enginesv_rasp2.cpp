#include "enginesv.h"

EngineSV::EngineSV(const string &i2cFileName)
    : i2cFileName(i2cFileName)
{
}

EngineSV::~EngineSV()
{
    closeI2C();
}

int EngineSV::openI2C()
{
    this->i2cDescriptor = open(i2cFileName.c_str(), O_RDWR);
    
    if (this->i2cDescriptor < 0)
        qWarning() << "Could not open file" << i2cFileName.data() << i2cDescriptor;
        
    return i2cDescriptor;
}

int EngineSV::openI2C(const string &i2cFileName)
{
    this->i2cFileName = i2cFileName;
    return openI2C();
}

int EngineSV::closeI2C()
{
    int retVal = close(i2cDescriptor);
    
    if (retVal < 0)
        qDebug() << "Could not close file" << i2cFileName.data() << i2cDescriptor;
        
    return retVal;
}

int EngineSV::turnOnEngine(const array<uchar, 3> &eng, int time_spin)
{
    if (time_spin == 0) return 0;
    
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    
    uchar buff[2] { eng.at(REG_ADDR), eng.at(DATA) };
    
    messages[0].addr = eng[MCP_ADDR];
    messages[0].flags = 0; // write
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;
    
    packets.msgs = messages;
    packets.nmsgs = 1;
    
    retVal = ioctl(this->i2cDescriptor, I2C_RDWR, &packets);
    
    copy(eng.cbegin(), eng.cend(), current_engine.begin());
    
    if (retVal < 0)
        qDebug() << "Write to I2C Device failed";
        
    // El timer se ejecutara luego 2.4 (por defecto) segundos
    // es el tiempo que tarda en dar una vuelta
    QTimer::singleShot(time_spin, this, SLOT(turnOffEngine()));
    return retVal;
}

int EngineSV::turnOnLight(const array<uchar, 3> &li)
{
    int retVal = -1;
    
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    
    uchar buff[2] { li.at(REG_ADDR), li.at(DATA) };
    
    messages[0].addr = li.at(MCP_ADDR);
    messages[0].flags = 0; // write
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;
    
    packets.msgs = messages;
    packets.nmsgs = 1;
    
    retVal = ioctl(this->i2cDescriptor, I2C_RDWR, &packets);
    
    if (retVal < 0)
        qDebug() << "Write to I2C Device failed";
        
    copy(li.cbegin(), li.cend(), current_light.begin());
    return retVal;
}

//! SLOTS
void EngineSV::turnOffEngine()
{
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    
    uchar buff[2] { current_engine.at(REG_ADDR), 0 } ;
    
    messages[0].addr = current_engine.at(MCP_ADDR);
    messages[0].flags = 0; // write
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;
    
    packets.msgs = messages;
    packets.nmsgs = 1;
    
    retVal = ioctl(this->i2cDescriptor, I2C_RDWR, &packets);
    
    if (retVal < 0)
        qDebug() << "Write to I2C Device failed";
        
    emit engineFinished();
}

void EngineSV::turnOffLight()
{
    int retVal = -1;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];
    
    uchar buff[2] { current_light.at(REG_ADDR), 0 } ;
    
    messages[0].addr = current_light.at(MCP_ADDR);
    messages[0].flags = 0; // write
    messages[0].len = sizeof(buff);
    messages[0].buf = buff;
    
    packets.msgs = messages;
    packets.nmsgs = 1;
    
    retVal = ioctl(this->i2cDescriptor, I2C_RDWR, &packets);
    
    if (retVal < 0)
        qDebug() << "Write to I2C Device failed";
}
//!

const uchar EngineSV::OLATA;
const uchar EngineSV::OLATB;
const uchar EngineSV::IODIRA;
const uchar EngineSV::IODIRB;

const uchar EngineSV::mcp23017_1;
const uchar EngineSV::mcp23017_2;

const size_t EngineSV::MCP_ADDR;
const size_t EngineSV::REG_ADDR;
const size_t EngineSV::DATA;

const constexpr array<const e, 10> EngineSV::eng;
const constexpr array<const e, 10> EngineSV::li;

// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
