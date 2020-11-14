#include "irp_handlers.h"

#include "print.h"

NTSTATUS irp_handlers::GeneralHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	// Handle general IRP 
	NTSTATUS status = STATUS_SUCCESS;

	MDbgPrint("Used general IRP: %ul", static_cast<ULONG>(Irp->Type));

	return status;
}

NTSTATUS irp_handlers::CreateHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	// Handle IRP_MJ_CREATE and start hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Received IRP_MJ_CREATE");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Initialized hypervisor");

	return status;
}

NTSTATUS irp_handlers::WriteHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	// Handle IRP_MJ_CREATE and start hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Received IRP_MJ_CREATE");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Initialized hypervisor");

	return status;
}


NTSTATUS irp_handlers::CloseHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	// Handle IRP_MJ_CLOSE and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Entered IRP_MJ_CLOSE");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Ended hypervisor");

	return status;
}

NTSTATUS irp_handlers::IOCTLHandlerIRP(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	PIO_STACK_LOCATION IrpStack;
	// Handle IRP_MJ_DEVICE_CONTROL and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Entered IRP_MJ_DEVICE_CONTROL");

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
		case IOCTL_SPOT:
			status = irp_handlers::ioctl::IoctlSpotHandler(
				pDeviceObject,
						Irp
			);
			break;
	}

	Irp->IoStatus.Status = status;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	MDbgPrint("Ended hypervisor");
	return status;
}

NTSTATUS irp_handlers::ioctl::IoctlSpotHandler(DEVICE_OBJECT *pDeviceObject, IRP *Irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	// Handle IRP_MJ_CLOSE and stop hypervisor
	NTSTATUS status = STATUS_SUCCESS;
	MDbgPrint("Entered IRP_MJ_CLOSE");

	/*
	Hypervisor code here
	*/

	MDbgPrint("Ended hypervisor");

	return status;
}
