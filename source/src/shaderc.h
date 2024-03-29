/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef SHADERC_H_HEADER_GUARD
#define SHADERC_H_HEADER_GUARD

namespace bgfx
{
	extern bool g_verbose;
}

#ifndef SHADERC_CONFIG_HLSL
#	define SHADERC_CONFIG_HLSL BASE_PLATFORM_WINDOWS
#endif // SHADERC_CONFIG_HLSL

#include <alloca.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>

#include <base/base.h>
#include <base/debug.h>
#include <base/commandline.h>
#include <base/endian.h>
#include <base/uint32_t.h>
#include <base/string.h>
#include <base/hash.h>
#include <base/file.h>

#include "vertexlayout.h"

namespace shaderc {
	extern bool g_verbose;

	base::StringView nextWord(base::StringView& _parse);

	constexpr uint8_t kUniformFragmentBit = 0x10;
	constexpr uint8_t kUniformSamplerBit = 0x20;
	constexpr uint8_t kUniformReadOnlyBit = 0x40;
	constexpr uint8_t kUniformCompareBit = 0x80;
	constexpr uint8_t kUniformMask = 0
		| kUniformFragmentBit
		| kUniformSamplerBit
		| kUniformReadOnlyBit
		| kUniformCompareBit
		;

	const char* getUniformTypeName(graphics::UniformType::Enum _enum);
	graphics::UniformType::Enum nameToUniformTypeEnum(const char* _name);

	struct Uniform
	{
		Uniform()
			: type(graphics::UniformType::Count)
			, num(0)
			, regIndex(0)
			, regCount(0)
			, texComponent(0)
			, texDimension(0)
			, texFormat(0)
		{
		}

		std::string name;
		graphics::UniformType::Enum type;
		uint8_t num;
		uint16_t regIndex;
		uint16_t regCount;
		uint8_t texComponent;
		uint8_t texDimension;
		uint16_t texFormat;
	};

	struct Options
	{
		Options();

		void dump();

		char shaderType;
		std::string platform;
		std::string profile;

		std::string	inputFilePath;
		std::string	outputFilePath;

		std::vector<std::string> includeDirs;
		std::vector<std::string> defines;
		std::vector<std::string> dependencies;

		bool disasm;
		bool raw;
		bool preprocessOnly;
		bool depends;

		bool debugInformation;

		bool avoidFlowControl;
		bool noPreshader;
		bool partialPrecision;
		bool preferFlowControl;
		bool backwardsCompatibility;
		bool warningsAreErrors;
		bool keepIntermediate;

		bool optimize;
		uint32_t optimizationLevel;
	};

	class File
	{
	public:
		File()
			: m_data(NULL)
			, m_size(0)
		{
		}

		~File()
		{
			delete[] m_data;
		}

		void load(const base::FilePath& _filePath)
		{
			base::FileReader reader;
			if (base::open(&reader, _filePath))
			{
				m_size = (uint32_t)base::getSize(&reader);
				m_data = new char[m_size + 1];
				m_size = (uint32_t)base::read(&reader, m_data, m_size, base::ErrorAssert{});
				base::close(&reader);

				if (m_data[0] == '\xef'
					&& m_data[1] == '\xbb'
					&& m_data[2] == '\xbf')
				{
					base::memMove(m_data, &m_data[3], m_size - 3);
					m_size -= 3;
				}

				m_data[m_size] = '\0';
			}
		}

		const char* getData() const
		{
			return m_data;
		}

		uint32_t getSize() const
		{
			return m_size;
		}

	private:
		char* m_data;
		uint32_t m_size;
	};

	typedef std::vector<Uniform> UniformArray;

	void printCode(const char* _code, int32_t _line = 0, int32_t _start = 0, int32_t _end = INT32_MAX, int32_t _column = -1);
	void strReplace(char* _str, const char* _find, const char* _replace);
	int32_t writef(base::WriterI* _writer, const char* _format, ...);
	void writeFile(const char* _filePath, const void* _data, int32_t _size);

	bool compileGLSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _writer, base::WriterI* _messages);
	bool compileHLSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _writer, base::WriterI* _messages);
	bool compileMetalShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _writer, base::WriterI* _messages);
	bool compilePSSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _writer, base::WriterI* _messages);
	bool compileSPIRVShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _writer, base::WriterI* _messages);

	bool compileShader(const char* _varying, const char* _comment, char* _shader, uint32_t _shaderLen, Options& _options, base::WriterI* _shaderWriter, base::WriterI* _messageWriter);
	//int compileShader(int _argc, const char* _argv[]);
	const graphics::Memory* compileShader(int _argc, const char* _argv[]);

	const char* getPsslPreamble();

} // namespace shaderc

#endif // SHADERC_H_HEADER_GUARD