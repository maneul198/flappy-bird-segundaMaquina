#ifndef ENGINESV_H
#define ENGINESV_H

#include <string>
#include <algorithm>
#include <array>
#include <map>

#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <QtCore/QObject>
#include <QDebug>
#include <QTimer>

//#include "enginesv_global.h"

using namespace std;

typedef std::array<uchar, 3> Engine;

class EngineSV : public QObject {
    Q_OBJECT

public:
    EngineSV(const string &i2cFileName = "/dev/i2c-1", QObject *parent = nullptr);
    
    /**
     * @brief set the number of the I2C bus on which slave resides.
     * @param bus, the default bus is 0x0
     */
    EngineSV(const int bus = 0x0, QObject *parent = nullptr);
    ~EngineSV();
    
    int bus() const;

    void init() const;
    
    // Enciende el motor
    void turnOnEngine(const Engine &eng, int time_spin = 2400);
    void turnOnLight(const Engine &li);
    
    void turnOffEngine(const Engine &eng);
    void turnOffLight(const Engine &li);
    
    void turnOffEngines();
    void turnOffLights();
    
signals:
    // Este evento ocurre cuando el motor ha
    // terminado de dar un giro
    void engineFinished();
    void engineStoped();
    
public:
    static const uchar OLATA {0x14};
    static const uchar OLATB {0x15};
    static const uchar IODIRA {0x0};
    static const uchar IODIRB {0x1};
    
    // Direccion del registro del mcp23017(Extensor de pines)
    static const uchar mcp23017_1 {0x40};
    static const uchar mcp23017_2 {0x41};
    
    // uchar = { mcp23017_addr, reg_addr, data }
    // Definicion de motores
    static constexpr array<const Engine, 10> eng {{
            {{ mcp23017_1, OLATA, 1   }},
            {{ mcp23017_1, OLATA, 2   }},
            {{ mcp23017_1, OLATA, 4   }},
            {{ mcp23017_1, OLATA, 8   }},
            {{ mcp23017_1, OLATA, 16  }},
            {{ mcp23017_1, OLATA, 32  }},
            {{ mcp23017_1, OLATA, 64  }},
            {{ mcp23017_1, OLATA, 128 }},
            {{ mcp23017_1, OLATB, 2   }},
            {{ mcp23017_1, OLATB, 4   }}
        }};
        
    // Definicion de  luces
    static constexpr array<const Engine, 10> li {{
            {{ mcp23017_1, OLATB, 8   }},
            {{ mcp23017_1, OLATB, 16  }},
            {{ mcp23017_1, OLATB, 32  }},
            {{ mcp23017_1, OLATB, 64  }},
            {{ mcp23017_1, OLATB, 128 }},

            //Estos no controan nada talvesmicrocontrolador quemado
            {{ mcp23017_1, OLATB, 1   }},
            {{ mcp23017_2, OLATA, 128 }},
            {{ mcp23017_2, OLATA, 16  }},
            {{ mcp23017_2, OLATA, 64  }},
            {{ mcp23017_2, OLATA, 32  }}
        }};
        
    static const size_t MCP_ADDR {0};
    static const size_t REG_ADDR {1};
    static const size_t DATA {2};
    
private:
    std::string errorMessage() const;
    
    int m_bus{0x0};
    int i2cDescriptor { -1};
    string i2cFileName;
    uchar mcp23017Address; // i2c device address
};

#endif // ENGINESV_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on;
