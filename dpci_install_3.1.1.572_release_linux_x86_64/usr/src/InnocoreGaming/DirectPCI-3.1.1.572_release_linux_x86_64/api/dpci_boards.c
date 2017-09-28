/******************************************************************************
 *
 * $Id: dpci_boards.c 12861 2016-10-17 16:28:50Z emil $
 *
 * Copyright 2011-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * Source file for DPCI installer plugin
 *
 *****************************************************************************/

#ifdef WIN32
# ifndef WINDDK
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <SetupAPI.h>
#include <devguid.h>
#  include "dpci_api_debug.h"
# else
#  define	dpci_debug_trace(level, fmt, ...) do {} while(0)
# endif
#else
# ifndef __KERNEL__
#  include <linux/types.h>
#  include "dpci_api_debug.h"
#  include <dpci_core_api.h>
# else
#  define	dpci_debug_trace(level, fmt, ...) do {} while(0)
# endif
#endif

#include <dpci_core_hw.h>
#include "dpci_boards.h"

/*
 * We need to define this here because of the complexity of including files
 * without accidentally breaking kernel and library builds on Windows and
 * Linux.
 */
#ifndef NULL
# define NULL ((void *)0)  
#endif

static const struct dpci_board_id board_ids[] = 
{
	/*
	 * Ordering is important in this list because cases where a vendor ID
	 * or device ID is 0xffff will be matched before cases where it is not;
	 * so catch-all cases need to come later.
	 */
	{DPX_112,
		"DPX-112",
		"dpx_112",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE112_ID,
		0xffff, 0xffff
	},
	{DPX_116, // we ignore DPX-116U
		"DPX-116",
		"dpx_116",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE116_ID,
		0xffff, 0xffff
	},
	{DPX_117,
		"DPX-117",
		"dpx_117",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE117_ID,
		0x8086, 0xffff
	},
	{DPX_E105S, // and DPX_E105F
		"DPX-E105",
		"dpx_e105",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_C705605_ID,
		0x8086, 0x27ac
	},
	{DPX_C705C605,
		"DPX-C705",
		"dpx_c705",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_C705605_ID,
		0x8086, 0xffff
	},
	{DPX_S410,
		"DPX-S410",
		"dpx_s410",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x8086, 0x2a10
	},
	{DPX_S425,
		"DPX-S425",
		"dpx_s425",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x8086, 0x006a
	},
	{DPX_S425,
		"DPX-S425",
		"dpx_s425",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x8086, 0x0044
	},
	{DPX_S305,
		"DPX-S305",
		"dpx_s305",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x8086, 0xffff
	},
	{DPX_E115,
		"DPX-E115",
		"dpx_e115",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x9600
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1200
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1201
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1202
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1203
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1204
	},
	{DPX_S415,
		"DPX-S415",
		"dpx_s415",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x9601
	},
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1400
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1401
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1402
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1403
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1404
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1405
        },
        {DPX_S430,
		"DPX-S430",
		"dpx_s430",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
                0x1022, 0x1410
        },
        {DPX_S435,
		"DPX-S435",
		"dpx_s435",
                DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_PCIE_ID,
                0x8086, 0xffff
        },
	{DPX_E120S, //and DPX_E120F
		"DPX-E120",
		"dpx_e120",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_C705605_ID,
		0x1022, 0x1510
	},
	{DPX_E130,
		"DPX-E130",
		"dpx_e130",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_S410305_ID,
		0x1022, 0x1510
	},
	{DPX_E135,
		"DPX-E135",
		"dpx_e135",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_PCIE_ID,
		0x1022, 0x1566
	},
	{DPX_S440,
		"DPX-S440",
		"dpx_s440",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_PCIE_ID,
		0x1022, 0x1576
	},
	{DPX_PCIE_DEVEL,
		"Unknown DirectPCI PCIe",
		"unknown_pcie",
		DENSITRON_VENDOR_ID, DPCI_DEVICE_CORE_PCIE_ID,
		0xffff, 0xffff
	},
	{DPX_NONE, 0, 0, 0, 0}
};


/*******************************************************************************
 *
 * Function:    dpci_board_find_board_by_type()
 *
 * Parameters:  board_code - the board type to find (see DPX_* in dpci_boards.h)
 *
 * Returns:     NULL = couldn't identify board
 *              >0 = identified board.
 *
 * Description: Match the dpci and root bridge IDs with the list of dpx-boards
 *		and find one out what kind of dpx-series board we're running on.
 *
 ******************************************************************************/
const struct dpci_board_id * APIENTRY dpci_board_find_board_by_code(int board_code)
{
	const struct dpci_board_id *dbidp, *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_board_by_code(%d)\n",
				board_code);
	for (dbidp = board_ids; dbidp->board_code != DPX_NONE; dbidp++)
	{
		if (dbidp->board_code == board_code)
			ret = dbidp;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_board_by_code(%d)=%d\n",
				board_code,
				ret);
        return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_board_find_by_ids()
 *
 * Parameters:  dpci_vid - dpci vendor ID
 *		dpci_did - dpci device ID
 *		rb_vid - root-bridge vendor ID
 *		rb_did - root-bridge device ID
 *
 * Returns:     NULL = couldn't identify board
 *              >0 = identified board.
 *
 * Description: Match the dpci and root bridge IDs with the list of dpx-boards
 *				and find one out what kind of dpx-series board we're running on.
 *
 ******************************************************************************/
const struct dpci_board_id *APIENTRY dpci_board_find_board_by_ids(
				unsigned short dpci_vid,/* DPCI PCI vendor ID */
				unsigned short dpci_did,/* DPCI PCI device ID */
				unsigned short rb_vid,	/* Root-bridge PCI vendor ID */
				unsigned short rb_did)	/* Root-bridge PCI device ID */
{
	const struct dpci_board_id *dbidp = board_ids;
	const struct dpci_board_id *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_board_by_ids(0x%04x, 0x%04x, 0x%04x, 0x%04x)\n",
				dpci_vid,
				dpci_did,
				rb_vid,
				rb_did);
	for (dbidp = board_ids; dbidp->board_code != DPX_NONE; dbidp++)
	{
		if (dbidp->dpci_vid != dpci_vid)
			continue;
		if (dbidp->dpci_did != dpci_did)
			continue;
		if (dbidp->rb_vid != 0xffff && dbidp->rb_vid != rb_vid)
			continue;
		if (dbidp->rb_did != 0xffff && dbidp->rb_did != rb_did)
			continue;
		ret = dbidp;
		break;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_board_by_ids(0x%04x, 0x%04x, 0x%04x, 0x%04x)=0x%p\n",
				dpci_vid,
				dpci_did,
				rb_vid,
				rb_did,
				ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_board_find_type_by_ids()
 *
 * Parameters:  dpci_vid - dpci vendor ID
 *		dpci_did - dpci device ID
 *		rb_vid - root-bridge vendor ID
 *		rb_did - root-bridge device ID
 *
 * Returns:     0 = couldn't identify board
 *              >0 = identified board.
 *
 * Description: Match the dpci and root bridge IDs with the list of dpx-boards
 *				and find one out what kind of dpx-series board we're running on.
 *
 ******************************************************************************/
int APIENTRY dpci_board_find_code_by_ids(unsigned short dpci_vid,
				unsigned short dpci_did,
				unsigned short rb_vid,
				unsigned short rb_did)
{
	const struct dpci_board_id *dbidp;
	int ret = DPX_NONE;

	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_code_by_ids(0x%04x, 0x%04x, 0x%04x, 0x%04x)\n",
				dpci_vid,
				dpci_did,
				rb_vid,
				rb_did);
	dbidp = dpci_board_find_board_by_ids(dpci_vid, dpci_did, rb_vid, rb_did);
	if (dbidp)
	{
		ret = dbidp->board_code;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_find_code_by_ids(0x%04x, 0x%04x, 0x%04x, 0x%04x)=%d\n",
				dpci_vid,
				dpci_did,
				rb_vid,
				rb_did,
				ret);
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_board_get_name_from_code()
 *
 * Parameters:  board_code - type of the DPCI baord
 *
 * Returns:     string name of the board.
 *
 * Description: returns the board name for the requested dpx type ID .
 *
 ******************************************************************************/
const char* APIENTRY dpci_board_get_name_from_code(int board_code)
{
	const struct dpci_board_id *dbidp;
	const char *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_name_from_code(%d)\n",
				board_code);
	dbidp = dpci_board_find_board_by_code(board_code);
	if (dbidp)
	{
		ret = dbidp->board_name;
		goto done;
	}
        switch (board_code)
        {
        case DPX_USE_IOB_NONE:
                return "Standalone DPX board";
		break;
        case DPX_USE_IOB_801003:
                return "DPX Mainboard with 80-1003 IO-Board";
		break;
        case DPX_USE_IOB_800062:
                return "DPX Mainboard with 80-0062 IO-Board";
		break;
        case DPX_NONE:
        default:
                ret = "[Unknown Board]";
        }
done:
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_name_from_code(%d)=\"%s\"\n",
				board_code,
				STRING_OR_NULL(ret));
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_board_get_tag_from_code()
 *
 * Parameters:  board_code - type of the DPCI baord
 *
 * Returns:     string name of the board.
 *
 * Description: returns the board name for the requested dpx type ID .
 *
 ******************************************************************************/
const char* APIENTRY dpci_board_get_tag_from_code(int board_code)
{
	const struct dpci_board_id *dbidp;
	const char *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_tag_from_code(%d)\n",
				board_code);
	dbidp = dpci_board_find_board_by_code(board_code);
	if (dbidp)
	{
		ret = dbidp->board_tag;
		goto done;
	}
        switch (board_code)
        {
        case DPX_USE_IOB_801003:
                ret = "80-1003";
		break;
        case DPX_USE_IOB_800062:
                ret = "80-0062";
		break;
        case DPX_NONE:
        default:
                ret = "[unknown]";
        }
done:
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_tag_from_code(%d)=\"%s\"\n",
				board_code,
				STRING_OR_NULL(ret));
	return ret;
}


// Define this to show debugging of the set up APIs
#undef DEVDEBUG

#ifdef WIN32
# ifndef WINDDK
/*******************************************************************************
 *
 * Function:    extract_vendevcc()
 *
 * Parameters:  szHwId - the string to parse.
 *		vidp - where to return the vendor ID extracted
 *		didp - where to return the device ID extracted
 *		classp - where to return the class code extracted
 *
 * Returns:     0 = didn't find root bridge device
 *              >0 = did find root bridge device
 *
 * Description: Go through the list of system PCI devices and find one that
 *              matches the header profile of the root bridge device.
 *
 ******************************************************************************/
static int extract_vendevcc(const char *szHwId,
			unsigned short *vidp,
			unsigned short *didp,
			unsigned int *classp)
{
	int ret = 0;
	char tstr[7];
	const char *str_ptr;

#ifdef DEVDEBUG
	printf("extract_vendevcc: str=%s\n", szHwId);
#endif
	str_ptr = strstr(szHwId, "VEN_");
	if (str_ptr)
	{
		strncpy(tstr, &str_ptr[4], 4);
		tstr[4] = 0;
		*vidp = (unsigned short)strtol(tstr, NULL, 16);
	}
	else
	{
		goto done;
	}
	str_ptr = strstr(szHwId, "DEV_");
	if (str_ptr)
	{
		strncpy(tstr, &str_ptr[4], 4);
		tstr[4] = 0;
		*didp = (unsigned short)strtol(tstr, NULL, 16);
	}
	else
	{
		goto done;
	}
	str_ptr = strstr(szHwId, "CC_");
	if (str_ptr)
	{
		strncpy(tstr, &str_ptr[3], 4);
		tstr[4] = 0;
		*classp = strtol(tstr, NULL, 16);
	}
	else
	{
		goto done;
	}
	ret = 1;
#ifdef DEVDEBUG
	printf("extract_vendevcc: vid=%04x did=%04x cc=%06x\n", *vidp, *didp, *classp);
#endif
done:
	return ret;
}


/*******************************************************************************
 *
 * Function:    find_pci_device()
 *
 * Parameters:  vidp - where to return the vendor ID found
 *		didp - where to return the device ID found
 *		classp - where to return the class code found
 *
 * Returns:     0 = didn't find matching device
 *              >0 = did find matching device.
 *
 * Description: Go through the list of system PCI devices and find one that
 *              matches the profile vidp:didp:classp.  If any of the pointers
 *		is NULL or points to a zero then that parameter is not matched;
 *		if the parameter pointer is valid and points to a non-NULL then
 *		the parameter of the device must match the pointer's value.
 *		If the pointer is non-NULL put points to a zero then that
 *		parameter found of the matching device is returned on success
 *		via the pointer.
 *
 ******************************************************************************/
static int find_pci_device(unsigned short *vidp,
				unsigned short *didp,
				unsigned int *classp)
{
	int ret = 0;
	HDEVINFO dev_info;
	SP_DEVINFO_DATA dev_info_data;
	DWORD dwIndex = 0;
	DWORD dataType;
	char data[MAX_PATH];
	DWORD reqSize;
	unsigned int i, j, k;
	char *bufptrs[10];
	unsigned short vid, did;
	unsigned int classcode;

	dev_info = SetupDiGetClassDevs(&GUID_DEVCLASS_SYSTEM, "PCI", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (INVALID_HANDLE_VALUE == dev_info)
	{
		goto done;
	}

#ifdef DEVDEBUG
	printf("find_pci_device: want ");
	if (vidp && *vidp)
		printf("vid=%04x ", *vidp);
	else
		printf("vid=xxxx ");
	if (didp && *didp)
		printf("did=%04x ", *didp);
	else
		printf("did=xxxx ");
	if (classp && *classp)
		printf("classcode=%04x ", *classp);
	else
		printf("classcode=xxxx ");
	printf("\n");
#endif
	// Enumerate over each device
	//
	while (dev_info_data.cbSize = sizeof(SP_DEVINFO_DATA),
		SetupDiEnumDeviceInfo(dev_info,
					dwIndex, &dev_info_data))
	{
		dwIndex++;
		// get the device instance ID
		if (!SetupDiGetDeviceRegistryProperty(dev_info, 
					&dev_info_data, 
					SPDRP_HARDWAREID, 
					&dataType,
					(LPBYTE)((char*)data),
					MAX_PATH,
					&reqSize)) 
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
			{
				goto done;
			}
			if (dataType != REG_MULTI_SZ) 
			{
				goto done;
			}
		}

		for (i = 0, j = 0, k = 1; i < reqSize; i++)
		{
			if (k)
			{
				bufptrs[j] = &data[i];
#ifdef DEVDEBUG
				printf("bufptrs[%d]=%s\n", j, bufptrs[j]);
#endif
				k = 0;
				j++;
			}
			else if (!data[i])
			{
				k++;
			}
		}

		// Parse to get the dev ID and vendor ID
		//
		if (!extract_vendevcc(bufptrs[j-2],
					&vid,
					&did,
					&classcode))
		{
			continue;
		}
#ifdef DEVDEBUG
		printf("find_pci_device: found vid=%04x did=%04x cc=%06x\n", vid, did, classcode);
#endif
		if (vidp)	// pointer valid
		{
			if (*vidp) // match required
			{
			 	if (*vidp == vid)
					ret++;
			}
			else // no match required but fill pointer in
			{
				ret++;
			}
		}
		else // pointer invalid, user doesn't care at all
		{
			ret++;
		}
		if (didp)	// pointer valid
		{
			if (*didp) // match required
			{
			 	if (*didp == did)
					ret++;
			}
			else // no match required but fill pointer in
			{
				ret++;
			}
		}
		else // pointer invalid, user doesn't care at all
		{
			ret++;
		}
		if (classp)	// pointer valid
		{
			if (*classp) // match required
			{
			 	if (*classp == classcode)
					ret++;
			}
			else // no match required but fill pointer in
			{
				ret++;
			}
		}
		else // pointer invalid, user doesn't care at all
		{
			ret++;
		}
		if (ret == 3)
		{
#ifdef DEVDEBUG
			printf("find_pci_device: match vid=%04x did=%04x cc=%06x\n", vid, did, classcode);
#endif
			if (vidp)
				*vidp = vid;
			if (didp)
				*didp = did;
			if (classp)
				*classp = classcode;
			break;
		}
#ifdef DEVDEBUG
		else
		{
			printf("find_pci_device: ignore vid=%04x did=%04x cc=%06x ret=%d\n", vid, did, classcode, ret);
		}
#endif
		ret = 0;
	}
	// release the device info list
	SetupDiDestroyDeviceInfoList(dev_info);
done:
	return ret;
}


/*******************************************************************************
 *
 * Function:    find_dpcicore_id()
 *
 * Parameters:  vidp - where to return the vendor ID found
 *		didp - where to return the device ID found
 *
 * Returns:     0 = didn't find dpci core device
 *              >0 = did find dpci core device
 *
 * Description: Go through the list of system PCI devices and find one that
 *              matches the header profile of the DPCI Core I/O device.
 *
 ******************************************************************************/
static int find_dpcicore_id(unsigned short *vidp, unsigned short *didp)
{
	unsigned short vid = DENSITRON_VENDOR_ID, did = 0;
	unsigned int classcode = 0x880;
	int ret;

	ret = find_pci_device(&vid, &did, &classcode);
	if (ret)
	{
		*vidp = vid;
		*didp = did;
#ifdef DEVDEBUG
		printf("find_dpcicore_id: got %04x:%04x\n", vid, did);
#endif
	}
	return ret;
}


/*******************************************************************************
 *
 * Function:    dpci_board_get_host_board_code()
 *
 * Parameters:  none
 *
 * Returns:     0 = couldn't identify board
 *              >0 = identified board.
 *
 * Description: Go through the list of system PCI devices and find one out
 *              what kind of dpx-series board we're running on.
 *
 ******************************************************************************/
DPCI_API const struct dpci_board_id *APIENTRY dpci_board_get_host_board(void)
{
// Note if you want parameters from the stack, pop them off in order.
// i.e. if you are called via exdll::myFunction file.dat poop.dat
// calling popstring() the first time would give you file.dat,
// and the second time would give you poop.dat.
// you should empty the stack of your parameters, and ONLY your
// parameters.

	unsigned short dpci_vid, dpci_did;
	const struct dpci_board_id *board_id = NULL;

	HDEVINFO dev_info;
	SP_DEVINFO_DATA dev_info_data;
	DWORD dwIndex = 0;
	DWORD dataType;
	char data[MAX_PATH];
	DWORD reqSize;
	unsigned int i, j, k;
	char *bufptrs[10];
	unsigned short vid, did;
	unsigned int classcode;

	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board()\n");
	if (!find_dpcicore_id(&dpci_vid, &dpci_did))
	{
		goto done;
	}

	dev_info = SetupDiGetClassDevs(&GUID_DEVCLASS_SYSTEM, "PCI", NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (INVALID_HANDLE_VALUE == dev_info)
	{
		goto done;
	}

	// Enumerate over each device
	//
	while (dev_info_data.cbSize = sizeof(SP_DEVINFO_DATA),
		SetupDiEnumDeviceInfo(dev_info,
					dwIndex, &dev_info_data))
	{
		dwIndex++;
		// get the device instance ID
		if (!SetupDiGetDeviceRegistryProperty(dev_info, 
					&dev_info_data, 
					SPDRP_HARDWAREID, 
					&dataType,
					(LPBYTE)((char*)data),
					MAX_PATH,
					&reqSize)) 
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
			{
				goto done;
			}
			if (dataType != REG_MULTI_SZ) 
			{
				goto done;
			}
		}

		for (i = 0, j = 0, k = 1; i < reqSize; i++)
		{
			if (k)
			{
				bufptrs[j] = &data[i];
#ifdef DEVDEBUG
				printf("bufptrs[%d]=%s\n", j, bufptrs[j]);
#endif
				k = 0;
				j++;
			}
			else if (!data[i])
			{
				k++;
			}
		}

		// Parse to get the dev ID and vendor ID
		//
		if (!extract_vendevcc(bufptrs[j-2],
					&vid,
					&did,
					&classcode))
		{
			continue;
		}

#ifdef DEVDEBUG
		printf("dpci_vid=%04x dpci_did=%04x classcode=%06x cs_vid=%04x cs_did=%04x\n", dpci_vid, dpci_did, classcode, vid, did);
#endif
		if (classcode != 0x600) // PCI/PCIe Root bridge class code.
		{
			continue;
		}
		board_id = dpci_board_find_board_by_ids(dpci_vid, dpci_did, vid, did);
		if (board_id != NULL)
		{
			goto done_destroyInfoList;
		}
	}

	/*
	 * Fallback position: if we cannot identify the board.
	 */
	board_id = NULL;

done_destroyInfoList:
	// release the device info list
	SetupDiDestroyDeviceInfoList(dev_info);
done:
	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board_code()=0x%p\n", board_id);
	return board_id;
}
# endif /* WINDDK */
#else
#if !defined(__KERNEL__)
const struct dpci_board_id *dpci_board_get_host_board(void)
{
	struct dpci_hardware_profile dhw;
	const struct dpci_board_id *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board()\n");
	if (dpci_drv_hw_profile(&dhw) != -1)
	{
		ret = dpci_board_find_board_by_code(dhw.board_code);
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board()=0x%p\n", ret);
	return ret;
}
#endif /* __KERNEL__ */
#endif /* WIN32 */


#if !defined(WINDDK) && !defined(__KERNEL__)
DPCI_API int APIENTRY dpci_board_get_host_board_code(void)
{
	int ret = DPX_NONE;
	const struct dpci_board_id *board_id;

	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board_code()\n");
	board_id = dpci_board_get_host_board();
	if (board_id)
	{
		ret = board_id->board_code;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board_code()=%d\n", ret);
	return ret;
}


DPCI_API const char* APIENTRY dpci_board_get_host_board_name(void)
{
	const struct dpci_board_id *board_id;
	const char *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board_name()\n");
	board_id = dpci_board_get_host_board();
	if (board_id)
	{
		ret = board_id->board_name;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_host_board_name()=\"%s\"\n",
				STRING_OR_NULL(ret));
	return ret;
}


DPCI_API const char* APIENTRY dpci_board_get_host_board_tag(void)
{
	const struct dpci_board_id *board_id;
	const char *ret = NULL;

	dpci_debug_trace(DPCI_DEBUG_MISC_API, "dpci_board_get_host_board_tag()\n");
	board_id = dpci_board_get_host_board();
	if (board_id)
	{
		ret = board_id->board_tag;
	}
	dpci_debug_trace(DPCI_DEBUG_MISC_API,
				"dpci_board_get_host_board_tag()=\"%s\"\n",
				STRING_OR_NULL(ret));
	return ret;
}

#endif /*  !defined(WINDDK) && !defined(__KERNEL__) */
