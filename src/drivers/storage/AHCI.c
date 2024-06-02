#include "AHCI.h"
#include "core/mm/mem.h"
#include "utils/log.h"
#include <drivers/meta/PCI.h>
#include <libk/endian.h>

HBA_mem *hba_mem;

void start_cmd(HBA_port *port)
{
	while (port->cmd & AHCI_HBA_CMD_CR);

	port->cmd |= AHCI_HBA_CMD_FRE;
	port->cmd |= AHCI_HBA_CMD_ST; 
}

void stop_cmd(HBA_port *port)
{
	port->cmd &= ~AHCI_HBA_CMD_ST;
	port->cmd &= ~AHCI_HBA_CMD_FRE;

	while(1)
	{
		if (port->cmd & AHCI_HBA_CMD_FR)
			continue;
		if (port->cmd & AHCI_HBA_CMD_CR)
			continue;
		break;
	}
 
}

static int check_type(HBA_port *port) {
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != AHCI_HBA_DET_PRESENT)
        return AHCI_DEV_TYPE_NULL;
    if (ipm != AHCI_HBA_IPM_ACTIVE)
        return AHCI_DEV_TYPE_NULL;

    switch (port->sig) {
    case AHCI_DEV_SIG_SATAPI:
        return AHCI_DEV_TYPE_SATAPI;
    case AHCI_DEV_SIG_SEMB:
        return AHCI_DEV_TYPE_SEMB;
    case AHCI_DEV_SIG_PM:
        return AHCI_DEV_TYPE_PM;
    default:
        return AHCI_DEV_TYPE_SATA;
    }
}

void probe_port(HBA_mem *abar) {
    uint32_t pi = abar->pi;
    int i = 0;
    while (i < AHCI_MAX_PORTS) {
        if (pi & (1 << i)) {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_TYPE_SATA) {
                log_trace("SATA drive found at port %d!\n", i);
            } else if (dt == AHCI_DEV_TYPE_SATAPI) {
                log_trace("SATAPI is unimplemented...\n", i);
            } else if (dt == AHCI_DEV_TYPE_SEMB) {
                log_trace("SMBus is unimplemented...\n", i);
            } else if (dt == AHCI_DEV_TYPE_PM) {
                log_trace("PM is unimplemented...\n", i);
            } else {
                // print "WTF is that device at port %d????" but in Russian
                log_trace("Что за устройство находится на порту %d?\n", i);
            }
        }
        i++;
    }
}

_bool ahci_init() {
    pci_multi_dev_t devs = pci_find_devices_by_class(0x1, 0x6);

    if (devs.count == 0) {
        log_error("AHCI: No devices found\n");
        return false;
    }
    // TODO: check for multiple AHCI controllers
    hba_mem = (HBA_mem *)PHYS_TO_VIRT(pci_get_bar5(devs.addrs[0]).addr);

    probe_port(hba_mem);

    return true;
}
