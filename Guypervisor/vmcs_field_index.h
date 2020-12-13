#pragma once

/*
 * VMCS Field encoding enumerator. 
 * 
 * Every component of the VMCS is encoded by a 32-bit field that can be used by VMREAD and VMWRITE
 * These field encodings can be found in volume 3d APPENDIX B part 1
 */
typedef enum vmcs_field_encoding {
	/**
	 * 16 bit Control Fields 
	 */

	// Index: 0x000000000
	// Exists if: 
	//   * 1-setting of the “enable VPID” VM-execution control. 
	kControlVPID = 0x00000000, 

	// Index: 0x000000001
	// Exists if: 
	//   * 1-setting of the “process posted interrupts” VM-execution control. 
	kControlPostedInterruptNotificationVector = 0x00000002,

	// Index: 0x000000002
	// Exists if: 
	//   * 1-setting of the “EPT-violation #VE” VM-execution control.
	kControlEPTPIndex = 0x00000004,

	/**
	 * 16 bit Guest State Fields
	 */

	// Index: 0x000000000
	kGuestEsSelector = 0x00000800,

	// Index: 0x000000001
	kGuestCsSelector = 0x00000802,

	// Index: 0x000000002
	kGuestSsSelector = 0x00000804,

	// Index: 0x000000003
	kGuestDsSelector = 0x00000806,

	// Index: 0x000000004
	kGuestFsSelector = 0x00000808,

	// Index: 0x000000005
	kGuestGsSelector = 0x0000080A,

	// Index: 0x000000006
	kGuestLdtrSelector = 0x0000080C,

	// Index: 0x000000007
	kGuestTrSelector = 0x0000080E,

	// Index: 0x000000008
	// Exists if: 
	//   * 1-setting of the “virtual-interrupt delivery” VM-execution control.
	kGuestInterruptStatus = 0x00000810,

	// Index: 0x000000009
	// Exists if: 
	//   * 1-setting of the “enable PML” VM-execution control.
	kGuestPmlIndex = 0x00000812,

	/**
	 * 16 bit Host State Fields
	 */

	// Index: 0x000000000
	kHostEsSelector = 0x00000C00,

	// Index: 0x000000001
	kHostCsSelector = 0x00000C02,

	// Index: 0x000000002
	kHostSsSelector = 0x00000C04,

	// Index: 0x000000003
	kHostDsSelector = 0x00000C06,

	// Index: 0x000000004
	kHostFsSelector = 0x00000C08,

	// Index: 0x000000005
	kHostGsSelector = 0x00000C0A,

	// Index: 0x000000006
	kHostTrSelector = 0x00000C0C,

	/**
	 * 64 bit Control Fields
	 */

	// Index: 0x000000000
	kControlAddressIOBitmapAFull = 0x00002000,
	kControlAddressIOBitmapAHigh = 0x00002001,

	// Index: 0x000000001
	kControlAddressIOBitmapBFull = 0x00002002,
	kControlAddressIOBitmapBHigh = 0x00002003,

	// Index: 0x000000002
	// Exists if: 
	//   * 1-setting of the “use MSR bitmaps” VM-execution control.
	kControlAddressMsrBitmapsFull = 0x00002004,
	kControlAddressMsrBitmapsHigh = 0x00002005,

	// Index: 0x000000003
	kControlVmExitMsrStoreAddrFull = 0x00002006,
	kControlVmExitMsrStoreAddrHigh = 0x00002007,

	// Index: 0x000000004
	kControlVmExitMsrLoadAddrFull = 0x00002008,
	kControlVmExitMsrLoadAddrHigh = 0x00002009,

	// Index: 0x000000005
	kControlVmEntryMsrLoadAddrFull = 0x0000200A,
	kControlVmEntryMsrLoadAddrHigh = 0x0000200B,

	// Index: 0x000000006
	kControlExecutiveVmcsPtrFull = 0x0000200C,
	kControlExecutiveVmcsPtrHigh = 0x0000200D,

	// Index: 0x000000007
	// Exists if: 
	//   * 1-setting of the “enable PML” VM-execution control.
	kControlPmlAddressFull = 0x0000200E,
	kControlPmlAddressHigh = 0x0000200F,

	// Index: 0x000000008
	kControlTscOffsetFull = 0x00002010,
	kControlTscOffsetHigh = 0x00002011,

	// Index: 0x000000009
	// Exists if: 
	//   * 1-setting of the “use TPR shadow” VM-execution control.
	kControlVirtualApicAddrFull = 0x00002012,
	kControlVirtualApicAddrHigh = 0x00002013,

	// Index: 0x00000000A
	// Exists if: 
	//   * 1-setting of the “virtualize APIC accesses” VM-execution control.
	kControlApicAccessAddrFull = 0x00002014,
	kControlApicAccessAddrHigh = 0x00002015,

	// Index: 0x00000000B
	// Exists if: 
	//   * 1-setting of the “process posted interrupts” VM-execution control.
	kControlPostedInterruptDescriptorAddrFull = 0x00002016,
	kControlPostedInterruptDescriptorAddrHigh = 0x00002017,

	// Index: 0x00000000C
	// Exists if: 
	//   * 1-setting of the “enable VM functions” VM-execution control.
	kControlVmFunctionControlsFull = 0x00002018,
	kControlVmFunctionControlsHigh = 0x00002019,

	// Index: 0x00000000D
	// Exists if: 
	//   * 1-setting of the “enable EPT” VM-execution control.
	kControlEPTPFull = 0x0000201A,
	kControlEPTPHigh = 0x0000201B,

	// Index: 0x00000000E
	// Exists if: 
	//   * 1-setting of the “virtual-interrupt delivery” VM-execution control.
	kControlEoiExitBitmap0Full = 0x0000201C,
	kControlEoiExitBitmap0High = 0x0000201D,

	// Index: 0x00000000F
	// Exists if: 
	//   * 1-setting of the “virtual-interrupt delivery” VM-execution control.
	kControlEoiExitBitmap1Full = 0x0000201E,
	kControlEoiExitBitmap1High = 0x0000201F,

	// Index: 0x000000010
	// Exists if: 
	//   * 1-setting of the “virtual-interrupt delivery” VM-execution control.
	kControlEoiExitBitmap2Full = 0x00002020,
	kControlEoiExitBitmap2High = 0x00002021,

	// Index: 0x000000011
	// Exists if: 
	//   * 1-setting of the “virtual-interrupt delivery” VM-execution control.
	kControlEoiExitBitmap3Full = 0x00002022,
	kControlEoiExitBitmap3High = 0x00002023,

	// Index: 0x000000012
	// Exists if: 
	//   * 1-setting of the “EPTP switching” VM-function control.
	kControlEptpListAddrFull = 0x00002024,
	kControlEptpListAddrHigh = 0x00002025,

	// Index: 0x000000013
	// Exists if: 
	//   * 1-setting of the “VMCS shadowing” VM-execution control.
	kControlVmreadBitmapAddrFull = 0x00002026,
	kControlVmreadBitmapAddrHigh = 0x00002027,

	// Index: 0x000000014
	// Exists if: 
	//   * 1-setting of the “VMCS shadowing” VM-execution control.
	kControlVmwriteBitmapAddrFull = 0x00002028,
	kControlVmwriteBitmapAddrHigh = 0x00002029,

	// Index: 0x000000015
	// Exists if: 
	//   * 1-setting of the “EPT-violation #VE” VM-execution control.
	kControlVirtualizationExceptionInformationAddrFull = 0x0000202A,
	kControlVirtualizationExceptionInformationAddrHigh = 0x0000202B,

	// Index: 0x000000016
	// Exists if: 
	//   * 1-setting of the “enable XSAVES/XRSTORS” VM-execution control.
	kControlXssExitingBitmapFull = 0x0000202C,
	kControlXssExitingBitmapHigh = 0x0000202D,

	// Index: 0x000000017
	// Exists if: 
	//   * 1-setting of the “enable ENCLS exiting” VM-execution control.
	kControlEnclsExitingBitmapFull = 0x0000202E,
	kControlEnclsExitingBitmapHigh = 0x0000202F,

	// Index: 0x000000018
	// Exists if: 
	//   * 1-setting of the “sub-page write permissions for EPT” VM-execution control.
	kControlSubpagePermissionTablePtrFull = 0x00002030,
	kControlSubpagePermissionTablePtrHigh = 0x00002031,

	// Index: 0x000000019
	// Exists if: 
	//   * 1-setting of the “use TSC scaling” VM-execution control.
	kControlTscMultiplierFull = 0x00002032,
	kControlTscMultiplierHigh = 0x00002033,

	// Index: 0x00000001A
	// Exists if: 
	//   * 1-setting of the “enable ENCLV exiting” VM-execution control.
	kControlEnclvExitingBitmapFull = 0x00002036,
	kControlEnclvExitingBitmapHigh = 0x00002037

	// TODO: Continue creating struct
	// TODO: Continue creating struct
	// TODO: Continue creating struct
} vmcs_field_encoding_e;