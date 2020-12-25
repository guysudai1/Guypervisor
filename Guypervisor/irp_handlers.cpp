#include "irp_handlers.h"

#include <intrin.h>

#include "print.h"
#include "virtualization.h"

// TODO: Remove later, this is just for testing
#include "virtual_addr_helpers.h"

NTSTATUS irp_handlers::GeneralHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	// Handle general IRP 
	NTSTATUS status = STATUS_SUCCESS;

	MDbgPrint("Used general IRP: %ul\n", static_cast<ULONG>(Irp->Type));

	return status;
}

NTSTATUS irp_handlers::CreateHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	// Handle IRP_MJ_CREATE and start hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Received IRP_MJ_CREATE\n");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Initialized hypervisor\n");

	return status;
}

NTSTATUS irp_handlers::WriteHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	// Handle IRP_MJ_CREATE and start hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Received IRP_MJ_CREATE\n");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Initialized hypervisor\n");

	return status;
}


NTSTATUS irp_handlers::CloseHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	// Handle IRP_MJ_CLOSE and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Entered IRP_MJ_CLOSE\n");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Ended hypervisor\n");

	return status;
}

NTSTATUS irp_handlers::IOCTLHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	PIO_STACK_LOCATION IrpStack;
	// Handle IRP_MJ_DEVICE_CONTROL and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Entered IRP_MJ_DEVICE_CONTROL\n");

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_ENTER_VMX:
			status = irp_handlers::ioctl::EnterVmxHandler(
				pDeviceObject,
						Irp
			);
			break;
	}

	Irp->IoStatus.Status = status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MDbgPrint("Ended hypervisor\n");
	return status;
}

NTSTATUS irp_handlers::ioctl::EnterVmxHandler(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	// Handle IRP_MJ_CLOSE and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	bool entered_vmx = false;
	bool created_active_vmcs = false;

	MDbgPrint("Entered EnterVmxHandler\n");

	status = virtualization::EnterVmxonMode();
	if (!NT_SUCCESS(status))
	{
		MDbgPrint("Instruction VMXON failed with status: %d\n", status);
		goto cleanup;
	}

	entered_vmx = true;

	status = virtualization::InitializeVMCS();
	if (!NT_SUCCESS(status))
	{
		MDbgPrint("Initializing VMCS (after VMXON) failed with status: %d\n", status);
		goto cleanup;
	}
	created_active_vmcs = true;

	// Populate VMCS here
	status = virtualization::PopulateActiveVMCS();
	if (!NT_SUCCESS(status))
	{
		MDbgPrint("Populating the VMCS (after VMPTRLD) failed with status: %d\n", status);
		goto cleanup;
	}

	// VMLaunch here
	status = virtualization::LaunchGuest();
	if (!NT_SUCCESS(status))
	{
		MDbgPrint("VMLaunch (after VMCS population) failed with status: %d\n", status);
		goto cleanup;
	}

cleanup:
	if (created_active_vmcs) {
		/**
		 * To prevent such corruption of a VMCS that may be used either after a return to VMX 
		 * operation or on another logical processor, software should execute VMCLEAR for that 
		 * VMCS before executing the VMXOFF instruction or removing power from the processor.
		 */
		virtualization::ClearActiveVMCS();
	}
	if (entered_vmx) {
		__vmx_off();
	}
	return status;
}
