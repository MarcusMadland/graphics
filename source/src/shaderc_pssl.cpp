/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "shaderc.h"

namespace shaderc
{
	bool compilePSSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _shaderWriter, base::WriterI* _messageWriter)
	{
		BASE_UNUSED(_options, _version, _code, _shaderWriter);
		base::ErrorAssert messageErr;
		base::write(_messageWriter, &messageErr, "PSSL compiler is not supported.\n");
		return false;
	}

	const char* getPsslPreamble()
	{
		return "";
	}

} // namespace graphics
