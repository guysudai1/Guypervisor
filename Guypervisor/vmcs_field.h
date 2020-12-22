#ifndef __VMCS_FIELD_H
#define __VMCS_FIELD_H

#include <wdm.h>

namespace virtualization {

	/*
	 * Contains size of field
	 */
	enum AccessType {
		kFullAccess = 0x0, // 16-bit / 32-bit / natural-width bit field
		kHighAccess = 0x1  // 64-bit field access
	};

	/*
	 * Contains which component the field was in
	 */
	enum ComponentType {
		kControlComponent = 0x0,	// Control
		kExitInfoComponent = 0x1,	// VM-exit information
		kGuestStateComponent = 0x2, // Guest State
		kHostStateComponent = 0x3, // Host State
	};

	/*
	 * Field width
	 */
	enum WidthType {
		k16Bit = 0x0, // 16 Bit field width 
		k64Bit = 0x1, // 64 Bit field width
		k32Bit = 0x2, // Guest State
		kNaturalWidth = 0x3, // Host State
	};

	typedef union _FieldEncoding {
		struct {
			UINT32 access_type : 1;
			UINT32 index : 8;
			UINT32 component_type : 2;
			UINT32 reserved1 : 1;  // Must be set to zero
			UINT32 field_width : 2;
			UINT32 reserved2 : 18; // Must be set to zero
		} fields;
		SIZE_T all;
	} FieldEncoding;
}

#endif /* __VMCS_FIELD_H */