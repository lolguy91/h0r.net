#include <drivers/pci.h>
#include <io/io.h>
#include <util/logger.h>
#include <lib/printf.h>
namespace PCI{

    PCIDevice devices[2048];
    bool initialized;

uint_32 Read(uint_16 bus,uint_16 device,uint_16 function,uint_32 offset)
{
    uint_32 id =
        0x1 << 31 // first bit is 1
        | ((bus & 0xFF) << 16)// bits 2-15 are the bus
        | ((device & 0x1F) << 11) // bits 16-21 are the device
        | ((function & 0x07) << 8)// bits 22-24 are the function
        | (offset & 0xFC); // the remaining bits are the offset, so every function gets 255 bytes of registers minus the header
    outb32(0xCF8,id);
    register uint_32 result = inb32(0xCFC);
    return result >> (8* (offset % 4));
}


void Write(uint_16 bus, uint_16 device, uint_16 function, uint_32 offset,uint_32 value)
{
    
    uint_32 id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (offset & 0xFC);
    outb32(0xCF8,id);
    outb32(0xCFC,value);
}

bool HasFunction(uint_16 bus, uint_16 device)
{
    return Read(bus,device,0,0x0E) & (1<<7);
}

void SelectDrivers()
{
    //https://wiki.osdev.org/PCI
    LogINFO("PCI Devices: \n");
    for(int bus = 0;bus < 8;bus++)
    {
        for(int device = 0;device < 32;device++)
        {
            int numfuncs = HasFunction(bus,device) ? 8 : 1;
            for(int function = 0;function < numfuncs;function++)
            {
                PCIDevice dev  =  GetDevice(bus,device,function);
                devices[(bus * 8) + (device * 32) + function] = dev;

                if(dev.vendor_id == 0x0000 ||dev.vendor_id == 0xFFFF){
                    //printf("nothing\n");
                    continue;
                }
                
                LogINFO("VendorID: 0x%x ,DeviceID: 0x%x \n",dev.vendor_id,dev.device_id);
            }
            
        }   
    }
    initialized = true;
}
void list_devices(){
    printf("PCI Devices: \n");
    for(int bus = 0;bus < 8;bus++)
    {
        for(int device = 0;device < 32;device++)
        {
            int numfuncs = HasFunction(bus,device)? 8 : 1;
            for(int function = 0;function < numfuncs;function++)
            {
                PCIDevice dev  =  GetDevice(bus,device,function);
                if(dev.vendor_id == 0x0000 ||dev.vendor_id == 0xFFFF){
                    continue;
                }
                printf("VendorID: 0x%x,DeviceID: 0x%x \n",dev.vendor_id,dev.device_id);
            }
        }
    }
}

PCIDevice GetDevice(uint_16 bus, uint_16 device, uint_16 function)
{
    if(initialized){
        return devices[(bus * 8) + (device * 32) + function]; 
    }

    PCIDevice result;
    result.bus = bus;
    result.device = device;
    result.function = function;

    result.vendor_id = Read(bus,device,function,0x00);
    result.device_id = Read(bus,device,function,0x02);

    result.class_id = Read(bus,device,function,0x0B) & 0x000000FF;
    result.subclass_id = Read(bus,device,function,0x0A) & 0x000000FF;
    result.interface_id = Read(bus,device,function,0x09) & 0x000000FF;

    result.revision = Read(bus,device,function,0x08) & 0x000000FF;
    result.interrupt = Read(bus,device,function,0x3c) & 0x000000FF;
    return result;
}



}

PCIDevice::PCIDevice()
{
    
}

void PCIDevice::Write(uint_32 offset,uint_32 value)
{
    PCI::Write(bus,device,function,offset,value);
}

uint_32 PCIDevice::Read(uint_32 offset)
{
    return PCI::Read(bus,device,function,offset);
}
