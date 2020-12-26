#include "error_codes.h"

const char* InstructionErrorToString(UINT32 errorCode)
{
	// Instruction error to string function
	// Documented in page 4123 / 5052
	switch (errorCode) {
	case 1:
		return "VMCALL executed in VMX root operation";
	case 2:
		return "VMCLEAR with invalid physical address";
	case 3:
		return "VMCLEAR with VMXON pointer";
	case 4:
		return "VMLAUNCH with non-clear VMCS";
	case 5:
		return "VMRESUME with non-launched VMCS";
	case 6:
		return "VMRESUME after VMXOFF (VMXOFF and VMXON between VMLAUNCH and VMRESUME)";
	case 7: 
		return "VM entry with invalid control field(s)";
	case 8:
		return "VM entry with invalid host-state field(s)";
	case 9:
		return "VMPTRLD with invalid physical address";
	case 10:
		return "VMPTRLD with VMXON pointer";
	case 11:
		return "VMPTRLD with incorrect VMCS revision identifier";
	case 12:
		return "VMREAD/VMWRITE from/to unsupported VMCS component";
	case 13:
		return "VMWRITE to read-only VMCS component";
	case 15:
		return "VMXON executed in VMX root operation";
	case 16:
		return "VM entry with invalid executive-VMCS pointer";
	case 17:
		return "VM entry with non-launched executive VMCS";
	case 18:
		return "VM entry with executive-VMCS pointer not VMXON pointer (when attempting to deactivate the dual-monitor treatment of SMIs and SMM)";
	case 19:
		return "VMCALL with non-clear VMCS (when attempting to activate the dual-monitor treatment of SMIs and SMM)";
	case 20:
		return "VMCALL with invalid VM-exit control fields";
	case 22:
		return "VMCALL with incorrect MSEG revision identifier (when attempting to activate the dual-monitor treatment of SMIs and SMM)";
	case 23:
		return "VMXOFF under dual-monitor treatment of SMIs and SMM";
	case 24:
		return "VMCALL with invalid SMM-monitor features (when attempting to activate the dual-monitor treatment of SMIs and SMM)";
	case 25:
		return "VM entry with invalid VM-execution control fields in executive VMCS (when attempting to return from SMM)";
	case 26:
		return "VM entry with events blocked by MOV SS.";
	case 28:
		return "Invalid operand to INVEPT/INVVPID.";
	default:
		return "Invalid error code detected";
	}
}
const char* BasicExitReasonToString(UINT32 basicExitReason) {
	switch (basicExitReason) {
	case 0:
		return "Exception or non-maskable interrupt (NMI).";
	case 1:
		return "External interrupt. An external interrupt arrived and the 'external-interrupt exiting' VM-execution control was 1.";
	case 2:
		return "Triple fault. The logical processor encountered an exception while attempting to call the double-fault handler and that exception did not itself cause a VM exit due to the exception bitmap.";
	case 3:
		return "INIT signal. An INIT signal arrived";
	case 4:
		return "Start-up IPI (SIPI). A SIPI arrived while the logical processor was in the 'wait-for-SIPI' state.";
	case 5:
		return "I/O system-management interrupt (SMI). An SMI arrived immediately after retirement of an I/O instruction and caused an SMM VM exit (see Section 34.15.2).";
	case 6:
		return "Other SMI. An SMI arrived and caused an SMM VM exit (see Section 34.15.2) but not immediately after retirement of an I/O instruction.";
	case 7:
		return "Interrupt window. At the beginning of an instruction, RFLAGS.IF was 1; events were not blocked by STI or by MOV SS; and the 'interrupt-window exiting' VM-execution control was 1.";
	case 8:
		return "NMI window. At the beginning of an instruction, there was no virtual-NMI blocking; events were not blocked by MOV SS; and the 'NMI-window exiting' VM-execution control was 1.";
	case 9:
		return "Task switch. Guest software attempted a task switch.";
	case 10:
		return "CPUID. Guest software attempted to execute CPUID.";
	case 11:
		return "GETSEC. Guest software attempted to execute GETSEC.";
	case 12:
		return "HLT. Guest software attempted to execute HLT and the 'HLT exiting' VM-execution control was 1.";
	case 13:
		return "INVD. Guest software attempted to execute INVD.";
	case 14:
		return "INVLPG. Guest software attempted to execute INVLPG and the 'INVLPG exiting' VM-execution control was 1.";
	case 15:
		return "RDPMC. Guest software attempted to execute RDPMC and the 'RDPMC exiting' VM-execution control was 1.";
	case 16:
		return "RDTSC. Guest software attempted to execute RDTSC and the 'RDTSC exiting' VM-execution control was 1.";
	case 17:
		return "RSM. Guest software attempted to execute RSM in SMM.";
	case 18:
		return "VMCALL. VMCALL was executed either by guest software (causing an ordinary VM exit) or by the executive monitor (causing an SMM VM exit; see Section 34.15.2).";
	case 19:
		return "VMCLEAR. Guest software attempted to execute VMCLEAR.";
	case 20:
		return "VMLAUNCH. Guest software attempted to execute VMLAUNCH.";
	case 21:
		return "VMPTRLD. Guest software attempted to execute VMPTRLD.";
	case 22:
		return "VMPTRST. Guest software attempted to execute VMPTRST.";
	case 23:
		return "VMREAD. Guest software attempted to execute VMREAD.";
	case 24:
		return "VMRESUME. Guest software attempted to execute VMRESUME.";
	case 25:
		return "VMWRITE. Guest software attempted to execute VMWRITE.";
	case 26:
		return "VMXOFF. Guest software attempted to execute VMXOFF.";
	case 27:
		return "VMXON. Guest software attempted to execute VMXON.";
	case 28:
		return "Control-register accesses. Guest software attempted to access CR0, CR3, CR4, or CR8 using CLTS, LMSW, or MOV CR and the VM-execution control fields indicate that a VM exit should occur (see Section 25.1 for details). This";
	case 29:
		return "MOV DR. Guest software attempted a MOV to or from a debug register and the 'MOV-DR exiting' VM-execution control was 1.";
	case 30:
		return "I/O instruction. Guest software attempted to execute an I/O instruction";
	case 31:
		return "RDMSR. Guest software attempted to execute RDMSR";
	case 32:
		return "WRMSR. Guest software attempted to execute WRMSR ";
	case 33:
		return "VM-entry failure due to invalid guest state. A VM entry failed one of the checks identified in Section 26.3.1.";
	case 34:
		return "VM-entry failure due to MSR loading. A VM entry failed in an attempt to load MSRs. See Section 26.4.";
	case 36:
		return "MWAIT. Guest software attempted to execute MWAIT and the 'MWAIT exiting' VM-execution control was 1.";
	case 37:
		return "Monitor trap flag. A VM exit occurred due to the 1-setting of the 'monitor trap flag' VM-execution control (see Section 25.5.2) or VM entry injected a pending MTF VM exit as part of VM entry (see Section 26.6.2).";
	case 39:
		return "MONITOR. Guest software attempted to execute MONITOR and the 'MONITOR exiting' VM-execution control was 1.";
	case 40:
		return "PAUSE. Either guest software attempted to execute PAUSE and the 'PAUSE exiting' VM-execution control was 1 or the 'PAUSE-loop exiting' VM-execution control was 1 and guest software executed a PAUSE loop with execution time exceeding PLE_Window (see Section 25.1.3).";
	case 41:
		return "VM-entry failure due to machine-check event. A machine-check event occurred during VM entry (see Section 26.9).";
	case 43:
		return "TPR below threshold. The logical processor determined that the value of bits 7:4 of the byte at offset 080H on the virtual-APIC page was below that of the TPR threshold VM-execution control field while the 'use TPR shadow' VMexecution control was 1 either as part of TPR virtualization (Section 29.1.2) or VM entry (Section 26.7.7).";
	case 44:
		return "APIC access. Guest software attempted to access memory at a physical address on the APIC-access page and the 'virtualize APIC accesses' VM-execution control was 1 (see Section 29.4).";
	case 45:
		return "Virtualized EOI. EOI virtualization was performed for a virtual interrupt whose vector indexed a bit set in the EOIexit bitmap.";
	case 46:
		return "Access to GDTR or IDTR. Guest software attempted to execute LGDT, LIDT, SGDT, or SIDT and the 'descriptor-table exiting' VM-execution control was 1.";
	case 47:
		return "Access to LDTR or TR. Guest software attempted to execute LLDT, LTR, SLDT, or STR and the 'descriptor-table exiting' VM-execution control was 1.";
	case 48:
		return "EPT violation. An attempt to access memory with a guest-physical address was disallowed by the configuration of the EPT paging structures.";
	case 49:
		return "EPT misconfiguration. An attempt to access memory with a guest-physical address encountered a misconfigured EPT paging-structure entry.";
	case 50:
		return "INVEPT. Guest software attempted to execute INVEPT.";
	case 51:
		return "RDTSCP. Guest software attempted to execute RDTSCP and the 'enable RDTSCP' and 'RDTSC exiting' VM-execution controls were both 1.";
	case 52:
		return "VMX-preemption timer expired. The preemption timer counted down to zero.";
	case 53:
		return "INVVPID. Guest software attempted to execute INVVPID.";
	case 54:
		return "WBINVD. Guest software attempted to execute WBINVD and the 'WBINVD exiting' VM-execution control was 1.";
	case 55:
		return "XSETBV. Guest software attempted to execute XSETBV.";
	case 56:
		return "APIC write. Guest software completed a write to the virtual-APIC page that must be virtualized by VMM software (see Section 29.4.3.3).";
	case 57:
		return "RDRAND. Guest software attempted to execute RDRAND and the 'RDRAND exiting' VM-execution control was 1.";
	case 58:
		return "INVPCID. Guest software attempted to execute INVPCID and the 'enable INVPCID' and 'INVLPG exiting' VM-execution controls were both 1.";
	case 59:
		return "VMFUNC. Guest software invoked a VM function with the VMFUNC instruction and the VM function either was not enabled or generated a function-specific condition causing a VM exit.";
	case 60:
		return "ENCLS. Guest software attempted to execute ENCLS and 'enable ENCLS exiting' VM-execution control was 1 and either (1) EAX < 63 and the corresponding bit in the ENCLS-exiting bitmap is 1; or (2) EAX ? 63 and bit 63 in the ENCLS-exiting bitmap is 1.";
	case 61:
		return "RDSEED. Guest software attempted to execute RDSEED and the 'RDSEED exiting' VM-execution control was 1.";
	case 62:
		return "Page-modification log full. The processor attempted to create a page-modification log entry and the value of the PML index was not in the range 0–511.";
	case 63:
		return "XSAVES. Guest software attempted to execute XSAVES, the 'enable XSAVES/XRSTORS' was 1, and a bit was set in the logical-AND of the following three values: EDX:EAX, the IA32_XSS MSR, and the XSS-exiting bitmap.";
	case 64:
		return "XRSTORS. Guest software attempted to execute XRSTORS, the 'enable XSAVES/XRSTORS' was 1, and a bit was set in the logical-AND of the following three values: EDX:EAX, the IA32_XSS MSR, and the XSS-exiting bitmap.";
	case 66:
		return "SPP-related event. The processor attempted to determine an access’s sub-page write permission and encountered an SPP miss or an SPP misconfiguration. See Section 28.2.4.2.";
	case 67:
		return "UMWAIT. Guest software attempted to execute UMWAIT and the 'enable user wait and pause' and 'RDTSC exiting' VM-execution controls were both 1.";
	case 68:
		return "TPAUSE. Guest software attempted to execute TPAUSE and the 'enable user wait and pause' and 'RDTSC exiting' VM-execution controls were both 1.";
	default:
		return "No basic exit reason found.";
	}
}
