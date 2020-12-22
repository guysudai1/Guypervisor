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
