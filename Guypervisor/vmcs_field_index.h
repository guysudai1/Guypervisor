#pragma once

/*
 * VMCS Field encoding enumerator. 
 * 
 * Every component of the VMCS is encoded by a 32-bit field that can be used by VMREAD and VMWRITE
 * These field encodings can be found in volume 3d APPENDIX B part 1 (page 4517)
 */
typedef enum vmcs_field_encoding {
	/**
	 * 16 bit Control Fields 
	 */

	// Index: 0x000000000
	// Exists if: 
	//   * 1-setting of the "enable VPID" VM-execution control. 
	kControlVPID = 0x00000000, 

	// Index: 0x000000001
	// Exists if: 
	//   * 1-setting of the "process posted interrupts" VM-execution control. 
	kControlPostedInterruptNotificationVector = 0x00000002,

	// Index: 0x000000002
	// Exists if: 
	//   * 1-setting of the "EPT-violation #VE" VM-execution control.
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
	//   * 1-setting of the "virtual-interrupt delivery" VM-execution control.
	kGuestInterruptStatus = 0x00000810,

	// Index: 0x000000009
	// Exists if: 
	//   * 1-setting of the "enable PML" VM-execution control.
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
	//   * 1-setting of the "use MSR bitmaps" VM-execution control.
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
	//   * 1-setting of the "enable PML" VM-execution control.
	kControlPmlAddressFull = 0x0000200E,
	kControlPmlAddressHigh = 0x0000200F,

	// Index: 0x000000008
	kControlTscOffsetFull = 0x00002010,
	kControlTscOffsetHigh = 0x00002011,

	// Index: 0x000000009
	// Exists if: 
	//   * 1-setting of the "use TPR shadow" VM-execution control.
	kControlVirtualApicAddrFull = 0x00002012,
	kControlVirtualApicAddrHigh = 0x00002013,

	// Index: 0x00000000A
	// Exists if: 
	//   * 1-setting of the "virtualize APIC accesses" VM-execution control.
	kControlApicAccessAddrFull = 0x00002014,
	kControlApicAccessAddrHigh = 0x00002015,

	// Index: 0x00000000B
	// Exists if: 
	//   * 1-setting of the "process posted interrupts" VM-execution control.
	kControlPostedInterruptDescriptorAddrFull = 0x00002016,
	kControlPostedInterruptDescriptorAddrHigh = 0x00002017,

	// Index: 0x00000000C
	// Exists if: 
	//   * 1-setting of the "enable VM functions" VM-execution control.
	kControlVmFunctionControlsFull = 0x00002018,
	kControlVmFunctionControlsHigh = 0x00002019,

	// Index: 0x00000000D
	// Exists if: 
	//   * 1-setting of the "enable EPT" VM-execution control.
	kControlEPTPFull = 0x0000201A,
	kControlEPTPHigh = 0x0000201B,

	// Index: 0x00000000E
	// Exists if: 
	//   * 1-setting of the "virtual-interrupt delivery" VM-execution control.
	kControlEoiExitBitmap0Full = 0x0000201C,
	kControlEoiExitBitmap0High = 0x0000201D,

	// Index: 0x00000000F
	// Exists if: 
	//   * 1-setting of the "virtual-interrupt delivery" VM-execution control.
	kControlEoiExitBitmap1Full = 0x0000201E,
	kControlEoiExitBitmap1High = 0x0000201F,

	// Index: 0x000000010
	// Exists if: 
	//   * 1-setting of the "virtual-interrupt delivery" VM-execution control.
	kControlEoiExitBitmap2Full = 0x00002020,
	kControlEoiExitBitmap2High = 0x00002021,

	// Index: 0x000000011
	// Exists if: 
	//   * 1-setting of the "virtual-interrupt delivery" VM-execution control.
	kControlEoiExitBitmap3Full = 0x00002022,
	kControlEoiExitBitmap3High = 0x00002023,

	// Index: 0x000000012
	// Exists if: 
	//   * 1-setting of the "EPTP switching" VM-function control.
	kControlEptpListAddrFull = 0x00002024,
	kControlEptpListAddrHigh = 0x00002025,

	// Index: 0x000000013
	// Exists if: 
	//   * 1-setting of the "VMCS shadowing" VM-execution control.
	kControlVmreadBitmapAddrFull = 0x00002026,
	kControlVmreadBitmapAddrHigh = 0x00002027,

	// Index: 0x000000014
	// Exists if: 
	//   * 1-setting of the "VMCS shadowing" VM-execution control.
	kControlVmwriteBitmapAddrFull = 0x00002028,
	kControlVmwriteBitmapAddrHigh = 0x00002029,

	// Index: 0x000000015
	// Exists if: 
	//   * 1-setting of the "EPT-violation #VE" VM-execution control.
	kControlVirtualizationExceptionInformationAddrFull = 0x0000202A,
	kControlVirtualizationExceptionInformationAddrHigh = 0x0000202B,

	// Index: 0x000000016
	// Exists if: 
	//   * 1-setting of the "enable XSAVES/XRSTORS" VM-execution control.
	kControlXssExitingBitmapFull = 0x0000202C,
	kControlXssExitingBitmapHigh = 0x0000202D,

	// Index: 0x000000017
	// Exists if: 
	//   * 1-setting of the "enable ENCLS exiting" VM-execution control.
	kControlEnclsExitingBitmapFull = 0x0000202E,
	kControlEnclsExitingBitmapHigh = 0x0000202F,

	// Index: 0x000000018
	// Exists if: 
	//   * 1-setting of the "sub-page write permissions for EPT" VM-execution control.
	kControlSubpagePermissionTablePtrFull = 0x00002030,
	kControlSubpagePermissionTablePtrHigh = 0x00002031,

	// Index: 0x000000019
	// Exists if: 
	//   * 1-setting of the "use TSC scaling" VM-execution control.
	kControlTscMultiplierFull = 0x00002032,
	kControlTscMultiplierHigh = 0x00002033,

	// Index: 0x00000001A
	// Exists if: 
	//   * 1-setting of the "enable ENCLV exiting" VM-execution control.
	kControlEnclvExitingBitmapFull = 0x00002036,
	kControlEnclvExitingBitmapHigh = 0x00002037,

	/**
	 * 64 bit VM Exit Information (Read Only)
	 */

	 // Index: 0x000000000
	 // Exists if: 
	 //   * 1-setting of the "enable EPT" VM-execution control.
	kGuestPhysicalAddrFull = 0x00002400,
	kGuestPhysicalAddrHigh = 0x00002401,

	/**
	* 64 bit Guest Fields
	*/

	// Index: 0x000000000
	kGuestVmcsLinkPtrFull = 0x00002800,
	kGuestVmcsLinkPtrHigh = 0x00002801,

    // Index: 0x000000001
    kGuestIa32DebugCtlFull = 0x00002802,
    kGuestIa32DebugCtlHigh = 0x00002803,

    // Index: 0x000000002
    // Exists if:
    //   * 1-setting of the "load IA32_PAT" VM-entry control or that of the "save IA32_PAT" VM-exit control.
    kGuestIa32PatFull = 0x00002804,
    kGuestIa32PatHigh = 0x00002805,

    // Index: 0x000000003
    // Exists if:
    //   * 1-setting of the "load IA32_EFER" VM-entry control or that of the "save IA32_EFER" VM-exit control.
    kGuestIa32EferFull = 0x00002806,
    kGuestIa32EferHigh = 0x00002807,

    // Index: 0x000000004
    // Exists if:
    //   * 1-setting of the "load IA32_PERF_GLOBAL_CTRL" VM-entry control.
    kGuestIa32PerfGlobalCtrlFull = 0x00002808,
    kGuestIa32PerfGlobalCtrlHigh = 0x00002809,

    // Index: 0x000000005
    // Exists if:
    //   * 1-setting of the "enable EPT" VM-execution control.
    kGuestPdpte0Full = 0x0000280A,
    kGuestPdpte0High = 0x0000280B,

    // Index: 0x000000006
    // Exists if:
    //   * 1-setting of the "enable EPT" VM-execution control.
    kGuestPdpte1Full = 0x0000280C,
    kGuestPdpte1High = 0x0000280D,

    // Index: 0x000000007
    // Exists if:
    //   * 1-setting of the "enable EPT" VM-execution control.
    kGuestPdpte2Full = 0x0000280E,
    kGuestPdpte2High = 0x0000280F,

    // Index: 0x000000008
    // Exists if:
    //   * 1-setting of the "enable EPT" VM-execution control.
    kGuestPdpte3Full = 0x00002810,
    kGuestPdpte3High = 0x00002811,

    // Index: 0x000000009
    // Exists if:
    //   * 1-setting of the "load IA32_BNDCFGS" VM-entry control or that of the "clear IA32_BNDCFGS" VM-exit control.
    kGuestIa32BndcfgsFull = 0x00002812,
    kGuestIa32BndcfgsHigh = 0x00002813,

    // Index: 0x00000000A
    // Exists if:
    //   * 1-setting of the "load IA32_RTIT_CTL" VM-entry control or that of the "clear IA32_RTIT_CTL" VM-exit control.
    kGuestIa32RtitCtlFull = 0x00002814,
    kGuestIa32RtitCtlHigh = 0x00002815,

    // Index: 0x00000000B
    // Exists if:
    //   * 1-setting of the "load PKRS" VM-entry control.
    kGuestIa32PkrsFull = 0x00002816,
    kGuestIa32PkrsHigh = 0x00002817,

    /**
     * 64 bit Host State Fields
     */

    // Index: 0x000000000
    // Exists if:
    //   * 1-setting of the "load IA32_PAT" VM-exit control.
    kHostIa32PatFull = 0x00002C00,
    kHostIa32PatHigh = 0x00002C01,

    // Index: 0x000000001
    // Exists if:
    //   * 1-setting of the "load IA32_EFER" VM-exit control.
    kHostIa32EferFull = 0x00002C02,
    kHostIa32EferHigh = 0x00002C03,

    // Index: 0x000000002
    // Exists if:
    //   * 1-setting of the "load IA32_PERF_GLOBAL_CTRL" VM-exit control.
    kHostIa32PerfGlobalCtrlFull = 0x00002C04,
    kHostIa32PerfGlobalCtrlHigh = 0x00002C05,

    // Index: 0x000000003
    // Exists if:
    //   * 1-setting of the "load PKRS" VM-exit control.
    kHostIa32PkrsFull = 0x00002C06,
    kHostIa32PkrsHigh = 0x00002C07,

    /**
     * 32 bit Control Fields
     */

    // Index: 0x000000000
    kControlPinBasedVmExecutionControls = 0x00004000,

    // Index: 0x000000001
    kControlPrimaryProcessorVmExecutionControls = 0x00004002,

    // Index: 0x000000002
    kControlExceptionBitmap = 0x00004004,

    // Index: 0x000000003
    kControlPageFaultErrorCodeMask = 0x00004006,

    // Index: 0x000000004
    kControlPageFaultErrorCodeMatch = 0x00004008,

    // Index: 0x000000005
    kControlCr3TargetCount = 0x0000400A,

    // Index: 0x000000006
    kControlVmExitControls = 0x0000400C,

    // Index: 0x000000007
    kControlVmExitMsrStoreCount = 0x0000400E,

    // Index: 0x000000008
    kControlVmExitMsrLoadCount = 0x00004010,

    // Index: 0x000000009
    kControlVmEntryControls = 0x00004012,

    // Index: 0x00000000A
    kControlVmEntryMsrLoadCount = 0x00004014,

    // Index: 0x00000000B
    kControlVmEntryInterruptionInformationField = 0x00004016,

    // Index: 0x00000000C
    kControlVmEntryExceptionErrorCode = 0x00004018,

    // Index: 0x00000000D
    kControlVmEntryInstructionLength = 0x0000401A,

    // Index: 0x00000000E
    // Exists if:
    //   * 1-setting of the "use TPR shadow" VM-execution control.
    kControlTprThreshold = 0x0000401C,

    // Index: 0x00000000F
    // Exists if:
    //   * 1-setting of the "activate secondary controls" VM-execution control.
    kControlSecondaryProcessorVmExecutionControls = 0x0000401E,

    // Index: 0x000000010
    // Exists if:
    //   * 1-setting of the "PAUSE-loop exiting" VM-execution control.
    kControlPleGap = 0x00004020,

    // Index: 0x000000011
    // Exists if:
    //   * 1-setting of the "PAUSE-loop exiting" VM-execution control.
    kControlPleWindow = 0x00004022,

    /**
     * 32 bit VM Exit Information (Read Only)
     */

    // Index: 0x000000000
    kVmInstructionError = 0x00004400,

    // Index: 0x000000001
    kExitReason = 0x00004402,

    // Index: 0x000000002
    kVmExitInterruptionInformation = 0x00004404,

    // Index: 0x000000003
    kVmExitInterruptionErrorCode = 0x00004406,

    // Index: 0x000000004
    kIdtVectoringInformationField = 0x00004408,

    // Index: 0x000000005
    kIdtVectoringErrorCode = 0x0000440A,

    // Index: 0x000000005
    kVmExitInstructionLength = 0x0000440C,

    // Index: 0x000000006
    kVmExitInstructionInformation = 0x0000440E,

    /**
     * 32 bit Guest State Fields
     */

    // Index: 0x00000000
    kGuestEsLimit = 0x00004800,

    // Index: 0x00000001
    kGuestCsLimit = 0x00004802,

    // Index: 0x00000002
    kGuestSsLimit = 0x00004804,

    // Index: 0x00000003
    kGuestDsLimit = 0x00004806,

    // Index: 0x00000004
    kGuestFsLimit = 0x00004808,

    // Index: 0x00000005
    kGuestGsLimit = 0x0000480A,

    // Index: 0x00000006
    kGuestLdtrLimit = 0x0000480C,

    // Index: 0x00000007
    kGuestTrLimit = 0x0000480E,

    // Index: 0x00000008
    kGuestGdtrLimit = 0x00004810,

    // Index: 0x00000009
    kGuestIdtrLimit = 0x00004812,

    // Index: 0x0000000A
    kGuestEsAccessRights = 0x00004814,

    // Index: 0x0000000B
    kGuestCsAccessRights = 0x00004816,

    // Index: 0x0000000C
    kGuestSsAccessRights = 0x00004818,

    // Index: 0x0000000D
    kGuestDsAccessRights = 0x0000481A,

    // Index: 0x0000000E
    kGuestFsAccessRights = 0x0000481C,

    // Index: 0x0000000F
    kGuestGsAccessRights = 0x0000481E,

    // Index: 0x00000010
    kGuestLdtrAccessRights = 0x00004820,

    // Index: 0x00000011
    kGuestTrAccessRights = 0x00004822,

    // Index: 0x00000012
    kGuestInterruptibilityState = 0x00004824,

    // Index: 0x00000013
    kGuestActivityState = 0x00004826,

    // Index: 0x00000014
    kGuestSmbase = 0x00004828,

    // Index: 0x00000015
    kGuestIa32SysenterCs = 0x0000482A,

    // Index: 0x00000016
    // Exists if:
    //   * 1-setting of the "activate VMX-preemption timer" VM-execution control.
    kGuestVmxPreemptionTimerValue = 0x0000482C,

    /**
     * 32 bit Host State Fields
     */

    // Index: 0x00000000
    kHostIa32SysenterCs = 0x00004C00,

    /**
     * Natural Width Control Fields
     */

    // Index: 0x00000000
    kControlCr0Mask = 0x00006000,

    // Index: 0x00000001
    kControlCr4Mask = 0x00006002,

    // Index: 0x00000002
    kControlCr0ReadShadow = 0x00006004,

    // Index: 0x00000003
    kControlCr4ReadShadow = 0x00006006,

    // Index: 0x00000004
    kControlCr3TargetValue0 = 0x00006008,

    // Index: 0x00000005
    kControlCr3TargetValue1 = 0x0000600A,

    // Index: 0x00000006
    kControlCr3TargetValue2 = 0x0000600C,

    // Index: 0x00000007
    kControlCr3TargetValue3 = 0x0000600E,

    // If there are more 4 CR3 target values then they will be placed here in
    // increments of 2 again

    /**
     * Natural Width VM Exit Information (Read Only)
     */

    // Index: 0x00000000
    kVmExitQualification = 0x00006400,

    // Index: 0x00000001
    kVmExitIoRcx = 0x00006402,

    // Index: 0x00000002
    kVmExitIoRsi = 0x00006404,

    // Index: 0x00000003
    kVmExitIoRdi = 0x00006406,

    // Index: 0x00000004
    kVmExitIoRip = 0x00006408,

    // Index: 0x00000005
    kVmExitGuestLinearAddr = 0x0000640A,

    /**
     * Natural Width Guest State Fields
     */

    // Index: 0x00000000
    kGuestCr0 = 0x00006800,

    // Index: 0x00000001
    kGuestCr3 = 0x00006802,

    // Index: 0x00000002
    kGuestCr4 = 0x00006804,

    // Index: 0x00000003
    kGuestEsBase = 0x00006806,

    // Index: 0x00000004
    kGuestCsBase = 0x00006808,

    // Index: 0x00000005
    kGuestSsBase = 0x0000680A,

    // Index: 0x00000006
    kGuestDsBase = 0x0000680C,

    // Index: 0x00000007
    kGuestFsBase = 0x0000680E,

    // Index: 0x00000008
    kGuestGsBase = 0x00006810,

    // Index: 0x00000009
    kGuestLdtrBase = 0x00006812,

    // Index: 0x0000000A
    kGuestTrBase = 0x00006814,

    // Index: 0x0000000B
    kGuestGdtrBase = 0x00006816,

    // Index: 0x0000000C
    kGuestIdtrBase = 0x00006818,

    // Index: 0x0000000D
    kGuestDr7 = 0x0000681A,

    // Index: 0x0000000E
    kGuestRsp = 0x0000681C,

    // Index: 0x0000000F
    kGuestRip = 0x0000681E,

    // Index: 0x00000010
    kGuestRFlags = 0x00006820,

    // Index: 0x00000011
    kGuestPendingDebugExceptions = 0x00006822,

    // Index: 0x00000012
    kGuestIa32SysenterEsp = 0x00006824,

    // Index: 0x00000013
    kGuestIa32SysenterEip = 0x00006826,

    // Index: 0x00000014
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kGuestIa32SCet = 0x00006828,

    // Index: 0x00000015
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kGuestIa32Ssp = 0x0000682A,

    // Index: 0x00000016
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kGuestIa32InterruptSspTableAddr = 0x0000682C,

    /**
     * Natural Width Host State Fields
     */

    // Index: 0x00000000
    kHostCr0 = 0x00006C00,

    // Index: 0x00000001
    kHostCr3 = 0x00006C02,

    // Index: 0x00000002
    kHostCr4 = 0x00006C04,

    // Index: 0x00000003
    kHostFsBase = 0x00006C06,

    // Index: 0x00000004
    kHostGsBase = 0x00006C08,

    // Index: 0x00000005
    kHostTrBase = 0x00006C0A,

    // Index: 0x00000006
    kHostGdtrBase = 0x00006C0C,

    // Index: 0x00000007
    kHostIdtrBase = 0x00006C0E,

    // Index: 0x00000008
    kHostIa32SysenterEsp = 0x00006C10,

    // Index: 0x00000009
    kHostIa32SysenterEip = 0x00006C12,

    // Index: 0x0000000A
    kHostRsp = 0x00006C14,

    // Index: 0x0000000B
    kHostRip = 0x00006C16,

    // Index: 0x0000000C
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kHostIa32SCet = 0x00006C18,

    // Index: 0x0000000D
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kHostIa32Ssp = 0x00006C1A,

    // Index: 0x0000000E
    // Exists if:
    //   * 1-setting of the “load CET state” VM-entry control.
    kHostIa32InterruptSspTableAddr = 0x00006C1C,

} vmcs_field_encoding_e;
