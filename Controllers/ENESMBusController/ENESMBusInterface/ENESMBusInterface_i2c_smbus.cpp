/*---------------------------------------------------------*\
| ENESMBusInterface_i2c_smbus.cpp                           |
|                                                           |
|   ENE SMBus interface for I2C/SMBus                       |
|                                                           |
|   Adam Honse (CalcProgrammer1)                21 Nov 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <mutex>
#include "ENESMBusInterface_i2c_smbus.h"

/*---------------------------------------------------------*\
| ENE registers are reached in two SMBus transfers: set the |
| register pointer (command 0x00), then read or write at it |
| (0x81/0x01/0x03). The bus only locks single transfers, so |
| two threads talking to the same device could interleave   |
| and land one thread's data at the other's pointer (seen:  |
| effect colors written into 0x8020-0x8025, turning the     |
| LEDs off). Hold this lock across each pointer+data pair.  |
|                                                           |
|   SekaiOS (Oct 2026): added                               |
\*---------------------------------------------------------*/
static std::mutex ene_register_mutex;

ENESMBusInterface_i2c_smbus::ENESMBusInterface_i2c_smbus(i2c_smbus_interface* bus)
{
    this->bus = bus;
}

ENESMBusInterface_i2c_smbus::~ENESMBusInterface_i2c_smbus()
{

}

ene_interface_type ENESMBusInterface_i2c_smbus::GetInterfaceType()
{
    return(ENE_INTERFACE_TYPE_I2C_SMBUS);
}

std::string ENESMBusInterface_i2c_smbus::GetLocation()
{
    std::string return_string(bus->info.device_name);
    return("I2C: " + return_string);
}

int ENESMBusInterface_i2c_smbus::GetMaxBlock()
{
    return(3);
}

unsigned char ENESMBusInterface_i2c_smbus::ENERegisterRead(ene_dev_id dev, ene_register reg)
{
    std::lock_guard<std::mutex> lock(ene_register_mutex);

    //Write ENE register
    bus->i2c_smbus_write_word_data(dev, 0x00, ((reg << 8) & 0xFF00) | ((reg >> 8) & 0x00FF));

    //Read ENE value
    return(bus->i2c_smbus_read_byte_data(dev, 0x81));
}

void ENESMBusInterface_i2c_smbus::ENERegisterWrite(ene_dev_id dev, ene_register reg, unsigned char val)
{
    std::lock_guard<std::mutex> lock(ene_register_mutex);

    //Write ENE register
    bus->i2c_smbus_write_word_data(dev, 0x00, ((reg << 8) & 0xFF00) | ((reg >> 8) & 0x00FF));

    //Write ENE value
    bus->i2c_smbus_write_byte_data(dev, 0x01, val);
}

void ENESMBusInterface_i2c_smbus::ENERegisterWriteBlock(ene_dev_id dev, ene_register reg, unsigned char * data, unsigned char sz)
{
    std::lock_guard<std::mutex> lock(ene_register_mutex);

    //Write ENE register
    bus->i2c_smbus_write_word_data(dev, 0x00, ((reg << 8) & 0xFF00) | ((reg >> 8) & 0x00FF));

    //Write ENE block data
    if(bus->i2c_smbus_write_block_data(dev, 0x03, sz, data) == -1)
    {
        //Fall back to individual byte operations if the block operation fails
        for(unsigned int block_byte = 0; block_byte < sz; block_byte++)
        {
            bus->i2c_smbus_write_byte_data(dev, 0x01, data[block_byte]);
        }
    }
}
