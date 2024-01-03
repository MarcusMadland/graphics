/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include "shaderc.h"
#if (0)
#include "glsl_optimizer.h"

namespace graphics { namespace glsl
{
	static bool compile(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _shaderWriter, base::WriterI* _messageWriter)
	{
		base::ErrorAssert messageErr;

		char ch = _options.shaderType;
		const glslopt_shader_type type = ch == 'f'
			? kGlslOptShaderFragment
			: (ch == 'c' ? kGlslOptShaderCompute : kGlslOptShaderVertex);

		glslopt_target target = kGlslTargetOpenGL;
		if(_version == BASE_MAKEFOURCC('M', 'T', 'L', 0))
		{
			target = kGlslTargetMetal;
		} else if(_version < 0x80000000) {
			target = kGlslTargetOpenGL;
		}
		else {
			_version &= ~0x80000000;
			target = (_version >= 300) ? kGlslTargetOpenGLES30 : kGlslTargetOpenGLES20;
		}

		glslopt_ctx* ctx = glslopt_initialize(target);

		glslopt_shader* shader = glslopt_optimize(ctx, type, _code.c_str(), 0);

		if (!glslopt_get_status(shader) )
		{
			const char* log = glslopt_get_log(shader);
			int32_t source  = 0;
			int32_t line    = 0;
			int32_t column  = 0;
			int32_t start   = 0;
			int32_t end     = INT32_MAX;

			bool found = false
				|| 3 == sscanf(log, "%u:%u(%u):", &source, &line, &column)
				|| 2 == sscanf(log, "(%u,%u):", &line, &column)
				;

			if (found
			&&  0 != line)
			{
				start = base::uint32_imax(1, line-10);
				end   = start + 20;
			}

			printCode(_code.c_str(), line, start, end, column);
			base::write(_messageWriter, &messageErr, "Error: %s\n", log);
			glslopt_shader_delete(shader);
			glslopt_cleanup(ctx);
			return false;
		}

		const char* optimizedShader = glslopt_get_output(shader);

		std::string out;
		// Trim all directives.
		while ('#' == *optimizedShader)
		{
			optimizedShader = base::strFindNl(optimizedShader).getPtr();
		}

		out.append(optimizedShader, strlen(optimizedShader));
		optimizedShader = out.c_str();

		{
			char* code = const_cast<char*>(optimizedShader);
			strReplace(code, "gl_FragDepthEXT", "gl_FragDepth");

			strReplace(code, "textureLodEXT", "texture2DLod");
			strReplace(code, "textureGradEXT", "texture2DGrad");

			strReplace(code, "texture2DLodARB", "texture2DLod");
			strReplace(code, "texture2DLodEXT", "texture2DLod");
			strReplace(code, "texture2DGradARB", "texture2DGrad");
			strReplace(code, "texture2DGradEXT", "texture2DGrad");

			strReplace(code, "textureCubeLodARB", "textureCubeLod");
			strReplace(code, "textureCubeLodEXT", "textureCubeLod");
			strReplace(code, "textureCubeGradARB", "textureCubeGrad");
			strReplace(code, "textureCubeGradEXT", "textureCubeGrad");

			strReplace(code, "texture2DProjLodARB", "texture2DProjLod");
			strReplace(code, "texture2DProjLodEXT", "texture2DProjLod");
			strReplace(code, "texture2DProjGradARB", "texture2DProjGrad");
			strReplace(code, "texture2DProjGradEXT", "texture2DProjGrad");

			strReplace(code, "shadow2DARB", "shadow2D");
			strReplace(code, "shadow2DEXT", "shadow2D");
			strReplace(code, "shadow2DProjARB", "shadow2DProj");
			strReplace(code, "shadow2DProjEXT", "shadow2DProj");
		}

		UniformArray uniforms;

		if (target != kGlslTargetMetal)
		{
			base::StringView parse(optimizedShader);

			while (!parse.isEmpty() )
			{
				parse = base::strLTrimSpace(parse);
				base::StringView eol = base::strFind(parse, ';');
				if (!eol.isEmpty() )
				{
					base::StringView qualifier = nextWord(parse);

					if (0 == base::strCmp(qualifier, "precision", 9) )
					{
						// skip precision
						parse.set(eol.getPtr() + 1, parse.getTerm() );
						continue;
					}

					if (0 == base::strCmp(qualifier, "attribute", 9)
					||  0 == base::strCmp(qualifier, "varying",   7)
					||  0 == base::strCmp(qualifier, "in",        2)
					||  0 == base::strCmp(qualifier, "out",       3)
					   )
					{
						// skip attributes and varyings.
						parse.set(eol.getPtr() + 1, parse.getTerm() );
						continue;
					}

					if (0 == base::strCmp(qualifier, "flat", 4)
					||  0 == base::strCmp(qualifier, "smooth", 6)
					||  0 == base::strCmp(qualifier, "noperspective", 13)
					||  0 == base::strCmp(qualifier, "centroid", 8)
					   )
					{
						// skip interpolation qualifiers
						parse.set(eol.getPtr() + 1, parse.getTerm() );
						continue;
					}

					if (0 == base::strCmp(parse, "tmpvar", 6) )
					{
						// skip temporaries
						parse.set(eol.getPtr() + 1, parse.getTerm() );
						continue;
					}

					if (0 != base::strCmp(qualifier, "uniform", 7) )
					{
						// end if there is no uniform keyword.
						parse.clear();
						continue;
					}

					base::StringView precision;
					base::StringView typen = nextWord(parse);

					if (0 == base::strCmp(typen, "lowp", 4)
					||  0 == base::strCmp(typen, "mediump", 7)
					||  0 == base::strCmp(typen, "highp", 5) )
					{
						precision = typen;
						typen = nextWord(parse);
					}

					BASE_UNUSED(precision);

					char uniformType[256];

					if (0 == base::strCmp(typen, "sampler", 7)
					||  0 == base::strCmp(typen, "isampler", 8)
					||  0 == base::strCmp(typen, "usampler", 8) )
					{
						base::strCopy(uniformType, BASE_COUNTOF(uniformType), "int");
					}
					else
					{
						base::strCopy(uniformType, BASE_COUNTOF(uniformType), typen);
					}

					base::StringView name = nextWord(parse);

					uint8_t num = 1;
					base::StringView array = base::strSubstr(parse, 0, 1);
					if (0 == base::strCmp(array, "[", 1) )
					{
						parse = base::strLTrimSpace(base::StringView(parse.getPtr() + 1, parse.getTerm() ) );

						uint32_t tmp;
						base::fromString(&tmp, parse);
						num = uint8_t(tmp);
					}

					Uniform un;
					un.type = nameToUniformTypeEnum(uniformType);

					if (UniformType::Count != un.type)
					{
						un.name.assign(name.getPtr(), name.getTerm());

						BASE_TRACE("name: %s (type %d, num %d)", un.name.c_str(), un.type, num);

						un.num = num;
						un.regIndex = 0;
						un.regCount = num;
						switch (un.type)
						{
						case UniformType::Mat3:
							un.regCount *= 3;
							break;
						case UniformType::Mat4:
							un.regCount *= 4;
							break;
						default:
							break;
						}

						uniforms.push_back(un);
					}

					parse = base::strLTrimSpace(base::strFindNl(base::StringView(eol.getPtr(), parse.getTerm() ) ) );
				}
			}
		}
		else
		{
			const base::StringView optShader(optimizedShader);
			base::StringView parse = base::strFind(optimizedShader, "struct xlatMtlShaderUniform {");
			base::StringView end   = parse;
			if (!parse.isEmpty() )
			{
				parse.set(parse.getPtr() + base::strLen("struct xlatMtlShaderUniform {"), optShader.getTerm() );
				end = base::strFind(parse, "};");
			}

			while ( parse.getPtr() < end.getPtr()
			&&     !parse.isEmpty() )
			{
				parse.set(base::strLTrimSpace(parse).getPtr(), optShader.getTerm() );
				const base::StringView eol = base::strFind(parse, ';');
				if (!eol.isEmpty() )
				{
					const char* typen = parse.getPtr();

					char uniformType[256];
					parse = base::strWord(parse);
					base::strCopy(uniformType, parse.getLength()+1, typen);
					parse.set(parse.getPtr()+parse.getLength(),optShader.getTerm());
					const char* name = base::strLTrimSpace(parse).getPtr();
					parse.set(name, optShader.getTerm() );

					char uniformName[256];
					uint8_t num = 1;
					base::StringView array = base::strFind(base::StringView(name, int32_t(eol.getPtr()-parse.getPtr() ) ), "[");
					if (!array.isEmpty() )
					{
						base::strCopy(uniformName, int32_t(array.getPtr()-name+1), name);

						char arraySize[32];
						base::StringView arrayEnd = base::strFind(base::StringView(array.getPtr(), int32_t(eol.getPtr()-array.getPtr() ) ), "]");
						base::strCopy(arraySize, int32_t(arrayEnd.getPtr()-array.getPtr() ), array.getPtr()+1);
						uint32_t tmp;
						base::fromString(&tmp, arraySize);
						num = uint8_t(tmp);
					}
					else
					{
						base::strCopy(uniformName, int32_t(eol.getPtr()-name+1), name);
					}

					Uniform un;
					un.type = nameToUniformTypeEnum(uniformType);

					if (UniformType::Count != un.type)
					{
						BASE_TRACE("name: %s (type %d, num %d)", uniformName, un.type, num);

						un.name = uniformName;
						un.num = num;
						un.regIndex = 0;
						un.regCount = num;
						uniforms.push_back(un);
					}

					parse = eol.getPtr() + 1;
				}
			}

			base::StringView mainEntry("xlatMtlShaderOutput xlatMtlMain (");
			parse = base::strFind(optimizedShader, mainEntry);
			end = parse;
			if (!parse.isEmpty())
			{
				parse.set(parse.getPtr() + mainEntry.getLength(), optShader.getTerm());
				end = base::strFind(parse, "{");
			}

			while (parse.getPtr() < end.getPtr()
				&& !parse.isEmpty())
			{
				parse.set(base::strLTrimSpace(parse).getPtr(), optShader.getTerm());
				const base::StringView textureNameMark("[[texture(");
				const base::StringView textureName = base::strFind(parse, textureNameMark);

				if (!textureName.isEmpty())
				{
					Uniform un;
					un.type = nameToUniformTypeEnum("int");	// int for sampler
					const char* varNameEnd = textureName.getPtr() - 1;
					parse.set(parse.getPtr(), varNameEnd - 1);
					const char* varNameBeg = parse.getPtr();
					for (int ii = parse.getLength() - 1; 0 <= ii; --ii)
					{
						if (varNameBeg[ii] == ' ')
						{
							parse.set(varNameBeg + ii + 1, varNameEnd);
							break;
						}
					}
					char uniformName[256];
					base::strCopy(uniformName, parse.getLength() + 1, parse);
					un.name = uniformName;
					const char* regIndexBeg = textureName.getPtr() + textureNameMark.getLength();
					base::StringView regIndex = base::strFind(regIndexBeg, ")");

					regIndex.set(regIndexBeg, regIndex.getPtr());
					uint32_t tmp;
					base::fromString(&tmp, regIndex);
					un.regIndex = uint16_t(tmp);
					un.num = 1;
					un.regCount = 1;

					uniforms.push_back(un);

					parse = regIndex.getPtr() + 1;
				}
				else
				{
					parse = textureName;
				}
			}
		}

		base::ErrorAssert err;

		uint16_t count = (uint16_t)uniforms.size();
		base::write(_shaderWriter, count, &err);

		for (UniformArray::const_iterator it = uniforms.begin(); it != uniforms.end(); ++it)
		{
			const Uniform& un = *it;
			uint8_t nameSize = (uint8_t)un.name.size();
			base::write(_shaderWriter, nameSize, &err);
			base::write(_shaderWriter, un.name.c_str(), nameSize, &err);
			uint8_t uniformType = uint8_t(un.type);
			base::write(_shaderWriter, uniformType, &err);
			base::write(_shaderWriter, un.num, &err);
			base::write(_shaderWriter, un.regIndex, &err);
			base::write(_shaderWriter, un.regCount, &err);
			base::write(_shaderWriter, un.texComponent, &err);
			base::write(_shaderWriter, un.texDimension, &err);
			base::write(_shaderWriter, un.texFormat, &err);

			BASE_TRACE("%s, %s, %d, %d, %d"
				, un.name.c_str()
				, getUniformTypeName(un.type)
				, un.num
				, un.regIndex
				, un.regCount
				);
		}

		uint32_t shaderSize = (uint32_t)base::strLen(optimizedShader);
		base::write(_shaderWriter, shaderSize, &err);
		base::write(_shaderWriter, optimizedShader, shaderSize, &err);
		uint8_t nul = 0;
		base::write(_shaderWriter, nul, &err);

		if (_options.disasm )
		{
			std::string disasmfp = _options.outputFilePath + ".disasm";
			writeFile(disasmfp.c_str(), optimizedShader, shaderSize);
		}

		glslopt_shader_delete(shader);
		glslopt_cleanup(ctx);

		return true;
	}

} // namespace glsl

	bool compileGLSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _shaderWriter, base::WriterI* _messageWriter)
	{
		return glsl::compile(_options, _version, _code, _shaderWriter, _messageWriter);
	}

} // namespace graphics

#else
namespace shaderc {

	// @todo Implement GLSL support
	bool compileGLSLShader(const Options& _options, uint32_t _version, const std::string& _code, base::WriterI* _shaderWriter, base::WriterI* _messageWriter)
	{
		return false;
	}
}

#endif
