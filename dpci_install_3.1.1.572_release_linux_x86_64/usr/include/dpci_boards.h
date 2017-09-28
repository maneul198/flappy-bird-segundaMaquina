/******************************************************************************
 *
 * $Id: dpci_boards.h 12081 2015-11-09 12:53:15Z aidan $
 *
 * Copyright 2010-2015 Advantech Co. Ltd
 * All rights reserved.
 *
 * Description:
 * Header file for DPCI board types
 *
 *****************************************************************************/
#ifndef _DPCI_BOARDS_H_
#define _DPCI_BOARDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WINDDK) || defined(__KERNEL__)
# define DPCI_API
# define APIENTRY
#else
# include "dpci_api_decl.h"
#endif

#define DPX_NONE                0
#define DPX_112                 1
#define DPX_116                 2
#define DPX_116U                3
#define DPX_117                 4
#define DPX_S410                5
#define DPX_S305                6
#define DPX_C705C605            7
#define DPX_E105S               8
#define DPX_E105F               9
#define DPX_E115                10
#define DPX_S415                11
#define DPX_S420                12
#define DPX_S425                13
#define DPX_E120S               14
#define DPX_E120F               15
#define	DPX_S430		16
#define	DPX_C710		17
#define	DPX_PCIE_DEVEL		18
#define	DPX_S435		19
#define DPX_E130		20
#define DPX_E135		21
#define DPX_S440		22

#define DPX_USE_IOB_NONE        100
#define DPX_USE_IOB_801003      101
#define DPX_USE_IOB_800062      102

struct dpci_board_id
{
	int		board_code;
	const char 	*board_name;
	const char 	*board_tag;
	unsigned short	dpci_vid;	/* PCI vendor ID of CPLD */
	unsigned short	dpci_did;	/* PCI device ID of CPLD */
	unsigned short	rb_vid;		/* PCI vendor ID of root bridge */
	unsigned short	rb_did;		/* PCI device ID of root bridge */
};


DPCI_API const struct dpci_board_id * APIENTRY dpci_board_find_board_by_code(int board_code);

DPCI_API const struct dpci_board_id * APIENTRY dpci_board_find_board_by_ids(
				unsigned short dpci_vid,/* DPCI PCI vendor ID */
				unsigned short dpci_did,/* DPCI PCI device ID */
				unsigned short rb_vid,	/* Root-bridge PCI vendor ID */
				unsigned short rb_did);	/* Root-bridge PCI device ID */

DPCI_API int APIENTRY dpci_board_find_code_by_ids(unsigned short dpci_vid,	/* DPCI PCI vendor ID */
				unsigned short dpci_did,/* DPCI PCI device ID */
				unsigned short rb_vid,	/* Root-bridge PCI vendor ID */
				unsigned short rb_did);	/* Root-bridge PCI device ID */

DPCI_API const char* APIENTRY dpci_board_get_name_from_code(int board_code);

DPCI_API const char* APIENTRY dpci_board_get_tag_from_code(int board_code);

DPCI_API const struct dpci_board_id *APIENTRY dpci_board_get_host_board(void);
DPCI_API int APIENTRY dpci_board_get_host_board_code(void);
DPCI_API const char* APIENTRY dpci_board_get_host_board_name(void);
DPCI_API const char* APIENTRY dpci_board_get_host_board_tag(void);

#ifdef __cplusplus
}
#endif

#endif /*_DPCI_BOARDS_H_*/
