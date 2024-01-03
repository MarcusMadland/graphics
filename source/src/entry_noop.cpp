/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "entry_p.h"

#if ENTRY_CONFIG_USE_NOOP

namespace entry
{
	const Event* poll()
	{
		return NULL;
	}

	const Event* poll(WindowHandle _handle)
	{
		BASE_UNUSED(_handle);
		return NULL;
	}

	void release(const Event* _event)
	{
		BASE_UNUSED(_event);
	}

	WindowHandle createWindow(int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, uint32_t _flags, const char* _title)
	{
		BASE_UNUSED(_x, _y, _width, _height, _flags, _title);
		WindowHandle handle = { UINT16_MAX };
		return handle;
	}

	void destroyWindow(WindowHandle _handle)
	{
		BASE_UNUSED(_handle);
	}

	void setWindowPos(WindowHandle _handle, int32_t _x, int32_t _y)
	{
		BASE_UNUSED(_handle, _x, _y);
	}

	void setWindowSize(WindowHandle _handle, uint32_t _width, uint32_t _height)
	{
		BASE_UNUSED(_handle, _width, _height);
	}

	void setWindowTitle(WindowHandle _handle, const char* _title)
	{
		BASE_UNUSED(_handle, _title);
	}

	void setWindowFlags(WindowHandle _handle, uint32_t _flags, bool _enabled)
	{
		BASE_UNUSED(_handle, _flags, _enabled);
	}

	void toggleFullscreen(WindowHandle _handle)
	{
		BASE_UNUSED(_handle);
	}

	void setMouseLock(WindowHandle _handle, bool _lock)
	{
		BASE_UNUSED(_handle, _lock);
	}

	void* getNativeWindowHandle(WindowHandle _handle)
	{
		BASE_UNUSED(_handle);
		return NULL;
	}

	void* getNativeDisplayHandle()
	{
		return NULL;
	}

} // namespace graphics

int main(int _argc, const char* const* _argv)
{
	return graphics::main(_argc, _argv);
}

#endif // ENTRY_CONFIG_USE_NOOP
