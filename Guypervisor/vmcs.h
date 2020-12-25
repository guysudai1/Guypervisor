#ifndef __VMCS_H
#define __VMCS_H

#include <wdm.h>

#include "processor.h"

typedef struct {
	UINT32 segmentType : 3;
	// 0 = system, 1 = code or data
	UINT32 descriptorType : 1;

	// CPL required to access this descriptor
	UINT32 descriptorPrivilegeLevel : 2;

	// 0 = absent, 1 = present
	UINT32 segmentPresent : 1;

	UINT32 reserved1 : 4;
	
	// Available for use by system software
	UINT32 availableBySystem : 1;

	union {
		// Not CS segment
		UINT32 reserved2 : 1;
		// Only for CS segment
		UINT32 bit_mode_64 : 1;
	};

	// 0 = 16 bit segment, 1 = 32 bit segment
	UINT32 defaultOperationSize : 1;

	// 0 = limit is in bytes, 1 = limit is in pages (4KB)
	UINT32 granuality : 1;

	// 0 = usable, 1 = unusable
	UINT32 segmentUnusable : 1;

	UINT32 reserved3 : 16;
} AccessRights;

typedef struct {
	UINT64 baseAddress;
	UINT32 limit;
} descriptorTableRegister;

typedef struct {
	UINT16 selector;
	UINT64 baseAddress;
	UINT32 segmentLimit; // In bytes
	AccessRights accessRights;
} segmentRegister;

typedef struct {

	// 0 = Doesn't block interrupts
	// 1 = Blocks interrupts for one instruction after its execution.
	UINT32 blockedSti : 1;

	// 0 = Doesn't block interrupts
	// 1 = Blocks interrupts for one instruction after MOV to SS or POP to SS
	UINT32 blockedMovSS : 1;

	// 0 = Doesn't block SMIs
	// 1 = Blocks SMIs in effect
	UINT32 blockedSMI : 1;

	// 0 = Doesn't block NMI
	// 1 = Blocks NMIs in effect
	UINT32 blockedNMI : 1;

	// VM exit sets this bit as 1 to indicate the VM 
	// exit was incident to enclave mode
	UINT32 enclaveInterruption : 1;

	UINT32 reserved : 27;
} InterruptibilityState;

typedef struct {
	UINT64 pdpte0;
	UINT64 pdpte1;
	UINT64 pdpte2;
	UINT64 pdpte3;
} PDPTE;

/*

VMCS Structure:

 - Guest State (Saves data about the guest)
 - Host State  (Saves data about the host)
 - VM-execution control
 - VM-exit control
 - VM-entry control
 - VM-exit information (read-only)

*/

typedef struct  {
	/*
	 *  Registers saved in the guest state
	 */
	struct {
		struct {
			processor::Cr0 cr0;
			processor::Cr3 cr3;
			processor::Cr4 cr4;
		} controlRegisters;

		struct {
			UINT64 dr7;
		} debugRegisters;

		struct {
			UINT64 rsp;
			UINT64 rip;
			UINT64 rflags;
		} contextRegisters;

		struct {
			segmentRegister cs;
			segmentRegister ss;
			segmentRegister ds;
			segmentRegister es;
			segmentRegister fs;
			segmentRegister gs;
			segmentRegister ldtr;
			segmentRegister tr;
		} segmentRegisters;

		struct {
			descriptorTableRegister gdtr;
			descriptorTableRegister idtr;
		} descriptorTableRegisters;

		struct {
			UINT64 IA32_DEBUGCTL;
			UINT32 IA32_SYSENTER_CS;
			UINT64 IA32_SYSENTER_ESP;
			UINT64 IA32_SYSENTER_EIP;
			UINT64 IA32_PERF_GLOBAL_CTRL;
			UINT64 IA32_PAT;
			UINT64 IA32_EFER;
			UINT64 IA32_BNDCFGS;
		} modelSpecificRegisters;

		UINT32 smbase;
	} guestRegisterState;

	struct {
		// — 0: Active. The logical processor is executing instructions normally.
		// — 1: HLT. The logical processor is inactive because it executed the HLT instruction.
		// — 2: Shutdown. The logical processor is inactive because it incurred a triple fault1 or some other serious error
		// — 3: Wait-for-SIPI. The logical processor is inactive because it is waiting for a startup-IPI (SIPI).
		UINT32 activityState;

		InterruptibilityState intState;

		UINT64 pendingDebugExceptions;

		// If the “VMCS shadowing” VM-execution control is 1, the VMREAD and VMWRITE
		// instructions access the VMCS referenced by this pointer(see Section 24.10).Otherwise, software should set
		// this field to FFFFFFFF_FFFFFFFFH to avoid VM - entry failures(see Section 26.3.1.5)
		UINT64 vmcsLinkPointer;

		UINT32 preemptiveTimer;

		PDPTE pageDirectoryPtrTblEntries;

		struct {
			unsigned char requestVirtualInterrupt;
			unsigned char servicingVirtualInterrupt;
		} guestInterruptStatus;

		unsigned short PMLIndex;
	} guestNonRegisterState;
} GuestState;

typedef struct {
	struct {
		processor::Cr0 cr0;
		processor::Cr3 cr3;
		processor::Cr4 cr4;
	} controlRegisters;
	
	struct {
		UINT64 rsp;
		UINT64 rip;
	} contextRegisters;

	struct {
		UINT16 csSelector;
		UINT16 ssSelector;
		UINT16 dsSelector;
		UINT16 esSelector;
		UINT16 fsSelector;
		UINT16 gsSelector;
		UINT16 trSelector;
	} segmentRegisters;

	struct {
		UINT64 fs;
		UINT64 gs;
		UINT64 tr;
		UINT64 gdtr;
		UINT64 idtr;
	} baseAddresses;

	struct {
		UINT32 IA32_SYSENTER_CS;
		UINT64 IA32_SYSENTER_ESP;
		UINT64 IA32_SYSENTER_EIP;
		UINT64 IA32_PERF_GLOBAL_CTRL;
		UINT64 IA32_PAT;
		UINT64 IA32_EFER;
	} modelSpecificRegisters;
} HostState;

typedef union {
	struct {
		// If this control is 1, external interrupts cause VM exits
		UINT32 externalInterruptExit : 1;

		UINT32 reserved1 : 2;

		// If this control is 1, non-maskable interrupts (NMIs) cause VM exits
		UINT32 nmiExiting : 1;

		UINT32 reserved2 : 1;

		// NMIs never blocked
		UINT32 virtualNMI : 1;

		// preemption timer counts down, and an exit occurs when timer counts down to 0
		UINT32 vmxPreemptionTimer : 1;

		// processor treats interrupts with the posted-interrupt notification vector
		UINT32 processInterrupts : 1;

		UINT32 reserved3 : 25;
	} fields;
	UINT32 all;
} PinBasedControls;

typedef union
{
	struct {
		UINT32 reserved1 : 2;

		// VM exit occurs at any instruction if interrupts are enabled
		UINT32 interruptExit : 1;

		// Allows time modification
		UINT32 tscOffset : 1;

		UINT32 reserved2 : 3;

		// HLT causes vmexit
		UINT32 hltExiting : 1;

		UINT32 reserved3 : 1;

		// INVLPG causes vmexit
		UINT32 invlpgExit : 1;
		// MWAIT causes vmexit
		UINT32 mwaitExit : 1;
		// RDSTC and RDTSCP cause vmexits
		UINT32 rdstcExit : 1;

		UINT32 reserved4 : 2;

		// decides whether MOVs to CR3 cause exit
		UINT32 cr3LoadExit : 1;
		// decides whether MOVs from CR3 cause exit
		UINT32 cr3StoreExit : 1;

		UINT32 reserved5 : 2;

		// decides whether MOVs to CR8 cause exit
		UINT32 cr8LoadExit : 1;
		// decides whether MOVs from CR3 cause exit
		UINT32 cr8StoreExit : 1;

		// enables TPR virtualization, and APIC virtualization
		UINT32 useTprShadow : 1;

		// exits if no virtual NMI blocking
		UINT32 nmiWindowExit : 1;

		// MOV DR causes exit
		UINT32 movDrExit : 1;

		// exits if IN, INS/INSB/INSW/INSD, OUT, OUTS/OUTSB/OUTSW/OUTSD executed
		UINT32 unconditionalIoExit : 1;

		// 0 => don't use bitmaps, 1 => use bitmaps
		UINT32 virtualizeIoBitmaps : 1;

		UINT32 reserved6 : 1;

		// monitor trap flag debugging features enabled
		UINT32 monitorTrapFlag : 1;

		// virtualize MSR bitmaps - RDMSR / WRMSR
		UINT32 virtualizeMsrBitmaps : 1;

		// MONITOR Exiting 
		UINT32 monitorExit : 1;
		// PAUSE Exiting 
		UINT32 pauseExit : 1;

		// Activate secondary controls (whether to use a second control structure) - we will use this
		UINT32 secondControls : 1;
	} fields;
	UINT32 all;
} VMExecCtrlFields;

typedef union {
	struct {
		UINT32 virtualizeApic : 1;
		UINT32 enableEPT : 1;

		// LGDT, LIDT, LLDT, LTR, SGDT, SIDT, SLDT, STR cause exits
		UINT32 descTableExit : 1;

		UINT32 enableRdtscp : 1;

		// If this control is 1, the logical processor treats specially RDMSR and WRMSR to APIC MSRs (in the range 800H–8FFH)
		UINT32 virtualizeAPICMsr : 1;

		// If this control is 1, cached translations of linear addresses are associated with a virtualprocessor identifier
		UINT32 enableVPID : 1;

		UINT32 wbinvdExit : 1;
		// Guest may run in unpaged protected mode or in real-mode
		UINT32 unrestrictedGuest : 1;

		UINT32 apicRegisterVirt : 1;

		// Enables evaluation and delivery of pending virtual interrupts
		UINT32 virtualInterruptDelivery : 1;

		// This control determines whether a series of executions of PAUSE can cause a VM exit 
		UINT32 pauseLoopExiting : 1;

		UINT32 rdrandExit : 1;

		// If this is 0, invpcid causes #UD
		UINT32 enableINVPCID : 1;

		UINT32 enableVMFUNC : 1;

		UINT32 vmcsShadowing : 1;

		// If this control is 1, executions of ENCLS consult the ENCLS-exiting bitmap to determine whether
		// the instruction causes a VM exit
		UINT32 enableENCLSExit : 1;

		UINT32 rdseedExit : 1;

		// enable page modification log
		UINT32 enablePML : 1;

		// If this control is 1, EPT violations may cause virtualization exceptions (#VE) instead of VM exits. 
		UINT32 EPTViolation : 1;

		// If this control is 1, Intel Processor Trace suppresses data packets that indicate the use of virtualization
		UINT32 concealVirtualizationFromPT : 1;

		// If this control is 0, any execution of XSAVES or XRSTORS causes a #UD.
		UINT32 enableXSaves_XStores : 1;

		UINT32 reserved1 : 1;

		// If this control is 1, EPT execute permissions are based on whether the linear address being
		// accessed is supervisor mode or user mode
		UINT32 executeCtrlEPT : 1;

		UINT32 reserved2 : 2;

		// Scale time by multiply factor
		UINT32 useTSCScaling : 1;

		UINT32 reserved3 : 6;
	} fields;
	UINT32 all;
} SecondaryVMExecCtrls;

typedef struct {
	// Enabled if the "RDTSC Exiting " CTRL is 0
	UINT64 tscOffset; // Enabled if the use TSC offseting exists
	UINT64 tscScaling; // Enabled if the use TSC scaling exists
} TimeScale;

typedef struct { char _bitmap[4 * 1024]; } Bitmap;

typedef struct {
	/* 
	A logical processor uses these bitmaps if and 
	only if the “use I/O bitmaps”  */
	Bitmap A;
	Bitmap B;
} IOBitmap;

/** 
 * The EPT is documented in page 3960
 */
typedef struct {
	/*
	Possible memory types:
	 *	0 = Uncacheable
	 *  6 = Write-back
	*/
	UINT64 memoryType : 3;

	// This value is 1 less than the EPT page-walk length (see Section 28.2.2)
	UINT64 walkPathMinusOne : 3;

	// Setting this control to 1 enables accessed and dirty flags for EPT (see Section 28.2.4)2
	//  Not all processors support accessed and dirty flags for EPT. Software should read the VMX capability MSR
	// IA32_VMX_EPT_VPID_CAP(see Appendix A.10) to determine whether the processor supports this feature
	UINT64 dirtyAccessFlags : 1;

	UINT64 reserved1 : 5;

	//  N is the physical-address width supported by the logical processor. Software can determine a processor’s physical-address width by
	// executing CPUID with 80000008H in EAX.The physical - address width is returned in bits 7:0 of EAX.
	UINT64 addressPlusReserved : 52;
} EPTP;

typedef struct {
	UINT32 reserved1 : 2;

	UINT32 saveDebugCtrls : 1;

	UINT32 reserved2 : 6;

	// Must be 1 on intel x64
	UINT32 hostAddrSpaceSize : 1;

	UINT32 reserved3 : 2;

	// Load IA32_PERF_GLOBAL_CTRL on VMExit
	UINT32 loadPerfGlobalCtrlOnExit : 1;

	UINT32 reserved4 : 2;

	/*
	This control affects VM exits due to external interrupts:
		• If such a VM exit occurs and this control is 1, the logical processor acknowledges the
		interrupt controller, acquiring the interrupt’s vector. The vector is stored in the VM-exit
		interruption-information field, which is marked valid.
		• If such a VM exit occurs and this control is 0, the interrupt is not acknowledged and the
		VM-exit interruption-information field is marked invalid.
	*/
	UINT32 acknowledgeInterruptOnExit : 1;

	UINT32 reserved5 : 2;

	UINT32 savePATMsrOnExit : 1;
	UINT32 loadPATMsrOnExit : 1;

	UINT32 saveEFERMsrOnExit : 1;
	UINT32 loadEFERMsrOnExit : 1;

	UINT32 savePreemptionTimerOnExit : 1;

	UINT32 clearBNDCFGSOnExit : 1;

	UINT32 concealVMExitPt : 1;

	UINT32 reserved6 : 7;
} VMExitCtrlFields;

typedef struct {
	UINT32 reserved1 : 2;

	UINT32 loadDebugCtrls : 1;

	UINT32 reserved2 : 6;

	// Allows x64 bit processors to enter x32 bit mode after tentry
	UINT32 ia32ModeGuest : 1;

	// Entry to SMM Mode
	UINT32 entryToSMM : 1;

	// If set to 1, the default treatment of SMIs and SMM is in effect after the VM entry
	UINT32 deactivateDualMonitor : 1;

	UINT32 reserved3 : 1;

	// Load IA32_PERF_GLOBAL_CTRL on VMExit
	UINT32 loadPerfGlobalCtrlOnEntry : 1;

	UINT32 loadPATMsrOnEntry : 1;
	UINT32 loadEFERMsrOnEntry : 1;
	UINT32 loadBNDCFGSOnEntry : 1;

	UINT32 concealVMEntryPt : 1;

	UINT32 reserved4 : 14;
} VMEntryCtrlFields;


/* BASIC VM EXIT INFORMATION */

typedef struct {
	UINT32 basicExitReason : 16;

	UINT32 reserved1: 11;

	UINT32 VMExitInclaveMode : 1;

	UINT32 pendingMonitorTrapFlagVMExit : 1;

	UINT32 VMExitFromRootOp : 1;
	UINT32 reserved2 : 1;

	// 0 = true VM exit
	// 1 = VM Entry Failure
	UINT32 VMEntryFailure : 1;
} ExitReason;

typedef struct {
	ExitReason exitReason;

	/*
	This field contains additional information about the cause of VM exits due to the following: debug exceptions; page-fault
	exceptions; start-up IPIs (SIPIs); task switches; INVEPT; INVLPG;INVVPID; LGDT; LIDT; LLDT; LTR; SGDT;
	SIDT; SLDT; STR; VMCLEAR; VMPTRLD; VMPTRST; VMREAD; VMWRITE; VMXON; control-register accesses;
	MOV DR; I/O instructions; and MWAIT. The format of the field depends on the cause of the VM exit.
	*/
	UINT64 exitQualification;

	/*
	This field is used in the following cases:
		— VM exits due to attempts to execute LMSW with a memory operand.
		— VM exits due to attempts to execute INS or OUTS.
		— VM exits due to system-management interrupts (SMIs) that arrive immediately after retirement of I/O
		instructions.
		— Certain VM exits due to EPT violations
	*/
	UINT64 guestLinearAddress;

	//  This field is used VM exits due to EPT violations and EPT misconfigurations. 
	UINT64 guestPhysicalAddress;
} BasicVMExitInformation;

/* BASIC VM EXIT INFORMATION END */

/* EXIT VECTORED EVENTS */

/*
This field receives basic information associated with the event
	causing the VM exit
*/
typedef struct {
	UINT32 vectorOfInterrupt: 8;

	/*
	0: External interrupt
	1: Not used
	2: Non-maskable interrupt (NMI)
	3: Hardware exception
	4 – 5: Not used
	6: Software exception
	7: Not used
	*/
	UINT32 interruptionType : 3;

	// 0 = Invalid, 1 = Valid
	UINT32 errorCodeInvalid : 1;

	UINT32 NMIUnblockingDueToIRET : 1;

	UINT32 reserved1 : 18;
	UINT32 valid : 1;
} VMExitInterruptionInformation;

/*
Event-specific information is provided for VM exits due to the following vectored events: exceptions (including
those generated by the instructions INT3, INTO, BOUND, and UD2); external interrupts that occur while the
“acknowledge interrupt on exit” VM-exit control is 1; and non-maskable interrupts (NMIs).
*/
typedef struct {
	VMExitInterruptionInformation exitInterruptionInformation;
	UINT32 interruptionErrorCode;
} VMExitVectoredEvents;

/* EXIT VECTORED EVENTS END */


/* EXIT OCCUR DURING EVENT DELIVERY */

typedef struct {
	UINT32 vectorOfInterrupt : 8;

	/*
	0: External interrupt
	1: Not used
	2: Non-maskable interrupt (NMI)
	3: Hardware exception
	4: Software interrupt
	5: Privileged software exception
	6: Software exception
	7: Not used
	*/
	UINT32 interruptionType : 3;

	// 0 = Invalid, 1 = Valid
	UINT32 errorCodeInvalid : 1;

	UINT32 undefined : 1;

	UINT32 reserved1 : 18;
	UINT32 valid : 1;
} IDTVectoringInformation;

/*
Additional information is provided for VM exits that occur during event delivery in VMX non-root operation
*/
typedef struct {
	IDTVectoringInformation idtInformation;
	UINT32 idtErrorCode;
} VMExitDuringEventDelivery;

/* EXIT OCCUR DURING EVENT DELIVERY END */

/* EXIT DUE TO INSTRUCTION EXECUTION */

typedef struct {
	UINT32 instructionLength;
	
	/*
	This field is used for VM exits due to attempts to execute INS,
	INVEPT, INVVPID, LIDT, LGDT, LLDT, LTR, OUTS, SIDT, SGDT, SLDT, STR, VMCLEAR, VMPTRLD, VMPTRST,
	VMREAD, VMWRITE, or VMXON.
	*/
	UINT32 instructionInformation;

	/*
	• I/O RCX. The value of RCX before the I/O instruction started.
	• I/O RSI. The value of RSI before the I/O instruction started.
	• I/O RDI. The value of RDI before the I/O instruction started.
	• I/O RIP. The value of RIP before the I/O instruction started (the RIP that addressed the I/O instruction).
	*/
	UINT64 ioRcx;
	UINT64 ioRsi;
	UINT64 ioRdi;
	UINT64 ioRip;
} VMExitDuringInstruction;

/* EXIT DUE TO INSTRUCTION EXECUTION EXIT */

typedef struct {
	BasicVMExitInformation basicInformation;
	VMExitVectoredEvents vectoredEvents;
	VMExitDuringEventDelivery eventDeliveryInformation;
	VMExitDuringInstruction instructionInformation;
	UINT32 instructionErrorField;
} VMExitInformationFields;

typedef struct {
	// Revision identifier should be taken from the kIa32VmxBasic MSR
	UINT32 revisionIdentifier : 31;
	// Should be 1 if the VMCS is a shadow VMCS
	UINT32 shadowVMCS : 1;

	/*
	 *   The contents of these bits do not control processor operation in any way. 
	 *   A logical processor writes a non-zero value into these bits if a VMX abort 
	 *   occurs (see Section 27.7). Software may also write into this field
	 */
	UINT32 abortIndicator;

	GuestState guestState;
	HostState hostState;
	
	VMExecCtrlFields executionCtrl;
	SecondaryVMExecCtrls secondExecCtrl;

	// UINT32 exceptionBitmap;
	// IOBitmap ioBitmaps;
	// TimeScale timeFields;
	EPTP eptPtr;
	UINT16 vpid;

	VMExitCtrlFields exitCtrl;
	VMEntryCtrlFields entryCtrl;
	VMExitInformationFields exitInformation;
} VMCS;

#endif /* __VMCS_H */