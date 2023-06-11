#ifndef _SATURNINTERFACE_H_
#define _SATURNINTERFACE_H_

#include "gpaddon.h"

#include "GamepadEnums.h"

#include "BoardConfig.h"

#define SIName "SaturnInterface"

class SaturnInterfaceInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void process();
	virtual void preprocess();
    virtual std::string name() { return SIName; }
private:
uint32_t testwork;
};

#endif  // SATURNINTERFACE_H_