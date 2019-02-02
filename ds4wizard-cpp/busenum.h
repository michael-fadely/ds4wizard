#pragma once

#include <minwindef.h>

#define FILE_DEVICE_BUSENUM         FILE_DEVICE_BUS_EXTENDER

#ifndef BUSENUM_IOCTL
#define BUSENUM_IOCTL(_index_) \
	CTL_CODE (FILE_DEVICE_BUSENUM, _index_, METHOD_BUFFERED, FILE_READ_DATA)
#endif

#define IOCTL_BUSENUM_PLUGIN_HARDWARE               BUSENUM_IOCTL (0x0)
#define IOCTL_BUSENUM_UNPLUG_HARDWARE               BUSENUM_IOCTL (0x1)
#define IOCTL_BUSENUM_EJECT_HARDWARE                BUSENUM_IOCTL (0x2)
#define IOCTL_TOASTER_DONT_DISPLAY_IN_UI_DEVICE     BUSENUM_IOCTL (0x3)

typedef struct _BUSENUM_UNPLUG_HARDWARE {

	IN ULONG Size;

	IN ULONG SerialNo;

	IN ULONG Flags;

	ULONG Reserved[1];

	static _BUSENUM_UNPLUG_HARDWARE create(int i)
	{
		_BUSENUM_UNPLUG_HARDWARE result {};

		result.Size        = sizeof(_BUSENUM_UNPLUG_HARDWARE);
		result.SerialNo    = i + 1;
		result.Flags       = 0;
		result.Reserved[0] = 0;

		return result;
	}

} BUSENUM_UNPLUG_HARDWARE, *PBUSENUM_UNPLUG_HARDWARE;
