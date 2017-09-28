/******************************************************************************
 *
 * $Id: dpci_core_types.h 12197 2015-12-04 09:50:47Z aidan $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * DirectPCI common data structures and types for core module 
 *
 *****************************************************************************/
#ifndef _DPCICORETYPES_H
#define _DPCICORETYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Used to set battery error levels.
 */
struct battlvl {
	int battno;
	int millivolts;
};

#define MIN_ERROR_VOLTAGE_LEVEL		18	/* minimum error voltage level = 18mv */

/* 
 * Used to get/set Quiet mode states
 */
struct quiet_mode {
	unsigned int passthrough;
	unsigned int level;
};

/*
 * Used for obtaining version info.
 */
struct dpci_version {
	unsigned int version_code;
	int hw_device_id;
	int hw_device_rev;
	char version_string[256];
};

/*
 * Constants used for reading system BIOS memory
 */
#define BIOS_SIZE_512KB		0x80000		// 512KB DPX-112, 116
#define BIOS_SIZE_1MB		0x100000	// 1MB DPX-117, S410, C705, E105
#define BIOS_SIZE_2MB		0x200000	// 2MB on DPX-S425 ? really
#define BIOS_SIZE_4MB		0x400000	// 4MB on DPX-S430/C710/S435/BJ08
#define BIOS_SIZE_8MB		0x800000	// 8MB on DPX-S440 and E135 
#define SYSTEM_MEMORY_END_ADDR	0x100000000	


/******************************************************************************
 *
 * Battery stuff
 */
struct dpci_battery
{
        char            *dbat_desc;
#define DPCI_BAT_REG_IDLP       -1
        int             dbat_reg;
        int             dbat_idx; // status bit mask or IDLP battery no.
        int             dbat_cal; // 100s-microvolt per gradation
};

#ifdef __cplusplus
}
#endif /*end __cplusplus*/
#endif /*end _DPCICORETYPES_H*/
