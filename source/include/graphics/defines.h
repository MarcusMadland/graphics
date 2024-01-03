/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

/*
 *
 * AUTO GENERATED FROM IDL! DO NOT EDIT! (source : temp.defines.h)
 *
 * More info about IDL:
 * https://gist.github.com/bkaradzic/05a1c86a6dd57bf86e2d828878e88dc2#graphics-is-switching-to-idl-to-generate-api
 *
 */

#ifndef GRAPHICS_DEFINES_H_HEADER_GUARD
#define GRAPHICS_DEFINES_H_HEADER_GUARD

#define GRAPHICS_API_VERSION UINT32_C(121)

/**
 * Color RGB/alpha/depth write. When it's not specified write will be disabled.
 *
 */
#define GRAPHICS_STATE_WRITE_R                        UINT64_C(0x0000000000000001) //!< Enable R write.
#define GRAPHICS_STATE_WRITE_G                        UINT64_C(0x0000000000000002) //!< Enable G write.
#define GRAPHICS_STATE_WRITE_B                        UINT64_C(0x0000000000000004) //!< Enable B write.
#define GRAPHICS_STATE_WRITE_A                        UINT64_C(0x0000000000000008) //!< Enable alpha write.
#define GRAPHICS_STATE_WRITE_Z                        UINT64_C(0x0000004000000000) //!< Enable depth write.
/// Enable RGB write.
#define GRAPHICS_STATE_WRITE_RGB (0 \
	| GRAPHICS_STATE_WRITE_R \
	| GRAPHICS_STATE_WRITE_G \
	| GRAPHICS_STATE_WRITE_B \
	)

/// Write all channels mask.
#define GRAPHICS_STATE_WRITE_MASK (0 \
	| GRAPHICS_STATE_WRITE_RGB \
	| GRAPHICS_STATE_WRITE_A \
	| GRAPHICS_STATE_WRITE_Z \
	)


/**
 * Depth test state. When `GRAPHICS_STATE_DEPTH_` is not specified depth test will be disabled.
 *
 */
#define GRAPHICS_STATE_DEPTH_TEST_LESS                UINT64_C(0x0000000000000010) //!< Enable depth test, less.
#define GRAPHICS_STATE_DEPTH_TEST_LEQUAL              UINT64_C(0x0000000000000020) //!< Enable depth test, less or equal.
#define GRAPHICS_STATE_DEPTH_TEST_EQUAL               UINT64_C(0x0000000000000030) //!< Enable depth test, equal.
#define GRAPHICS_STATE_DEPTH_TEST_GEQUAL              UINT64_C(0x0000000000000040) //!< Enable depth test, greater or equal.
#define GRAPHICS_STATE_DEPTH_TEST_GREATER             UINT64_C(0x0000000000000050) //!< Enable depth test, greater.
#define GRAPHICS_STATE_DEPTH_TEST_NOTEQUAL            UINT64_C(0x0000000000000060) //!< Enable depth test, not equal.
#define GRAPHICS_STATE_DEPTH_TEST_NEVER               UINT64_C(0x0000000000000070) //!< Enable depth test, never.
#define GRAPHICS_STATE_DEPTH_TEST_ALWAYS              UINT64_C(0x0000000000000080) //!< Enable depth test, always.
#define GRAPHICS_STATE_DEPTH_TEST_SHIFT               4                            //!< Depth test state bit shift
#define GRAPHICS_STATE_DEPTH_TEST_MASK                UINT64_C(0x00000000000000f0) //!< Depth test state bit mask

/**
 * Use GRAPHICS_STATE_BLEND_FUNC(_src, _dst) or GRAPHICS_STATE_BLEND_FUNC_SEPARATE(_srcRGB, _dstRGB, _srcA, _dstA)
 * helper macros.
 *
 */
#define GRAPHICS_STATE_BLEND_ZERO                     UINT64_C(0x0000000000001000) //!< 0, 0, 0, 0
#define GRAPHICS_STATE_BLEND_ONE                      UINT64_C(0x0000000000002000) //!< 1, 1, 1, 1
#define GRAPHICS_STATE_BLEND_SRC_COLOR                UINT64_C(0x0000000000003000) //!< Rs, Gs, Bs, As
#define GRAPHICS_STATE_BLEND_INV_SRC_COLOR            UINT64_C(0x0000000000004000) //!< 1-Rs, 1-Gs, 1-Bs, 1-As
#define GRAPHICS_STATE_BLEND_SRC_ALPHA                UINT64_C(0x0000000000005000) //!< As, As, As, As
#define GRAPHICS_STATE_BLEND_INV_SRC_ALPHA            UINT64_C(0x0000000000006000) //!< 1-As, 1-As, 1-As, 1-As
#define GRAPHICS_STATE_BLEND_DST_ALPHA                UINT64_C(0x0000000000007000) //!< Ad, Ad, Ad, Ad
#define GRAPHICS_STATE_BLEND_INV_DST_ALPHA            UINT64_C(0x0000000000008000) //!< 1-Ad, 1-Ad, 1-Ad ,1-Ad
#define GRAPHICS_STATE_BLEND_DST_COLOR                UINT64_C(0x0000000000009000) //!< Rd, Gd, Bd, Ad
#define GRAPHICS_STATE_BLEND_INV_DST_COLOR            UINT64_C(0x000000000000a000) //!< 1-Rd, 1-Gd, 1-Bd, 1-Ad
#define GRAPHICS_STATE_BLEND_SRC_ALPHA_SAT            UINT64_C(0x000000000000b000) //!< f, f, f, 1; f = min(As, 1-Ad)
#define GRAPHICS_STATE_BLEND_FACTOR                   UINT64_C(0x000000000000c000) //!< Blend factor
#define GRAPHICS_STATE_BLEND_INV_FACTOR               UINT64_C(0x000000000000d000) //!< 1-Blend factor
#define GRAPHICS_STATE_BLEND_SHIFT                    12                           //!< Blend state bit shift
#define GRAPHICS_STATE_BLEND_MASK                     UINT64_C(0x000000000ffff000) //!< Blend state bit mask

/**
 * Use GRAPHICS_STATE_BLEND_EQUATION(_equation) or GRAPHICS_STATE_BLEND_EQUATION_SEPARATE(_equationRGB, _equationA)
 * helper macros.
 *
 */
#define GRAPHICS_STATE_BLEND_EQUATION_ADD             UINT64_C(0x0000000000000000) //!< Blend add: src + dst.
#define GRAPHICS_STATE_BLEND_EQUATION_SUB             UINT64_C(0x0000000010000000) //!< Blend subtract: src - dst.
#define GRAPHICS_STATE_BLEND_EQUATION_REVSUB          UINT64_C(0x0000000020000000) //!< Blend reverse subtract: dst - src.
#define GRAPHICS_STATE_BLEND_EQUATION_MIN             UINT64_C(0x0000000030000000) //!< Blend min: min(src, dst).
#define GRAPHICS_STATE_BLEND_EQUATION_MAX             UINT64_C(0x0000000040000000) //!< Blend max: max(src, dst).
#define GRAPHICS_STATE_BLEND_EQUATION_SHIFT           28                           //!< Blend equation bit shift
#define GRAPHICS_STATE_BLEND_EQUATION_MASK            UINT64_C(0x00000003f0000000) //!< Blend equation bit mask

/**
 * Cull state. When `GRAPHICS_STATE_CULL_*` is not specified culling will be disabled.
 *
 */
#define GRAPHICS_STATE_CULL_CW                        UINT64_C(0x0000001000000000) //!< Cull clockwise triangles.
#define GRAPHICS_STATE_CULL_CCW                       UINT64_C(0x0000002000000000) //!< Cull counter-clockwise triangles.
#define GRAPHICS_STATE_CULL_SHIFT                     36                           //!< Culling mode bit shift
#define GRAPHICS_STATE_CULL_MASK                      UINT64_C(0x0000003000000000) //!< Culling mode bit mask

/**
 * Alpha reference value.
 *
 */
#define GRAPHICS_STATE_ALPHA_REF_SHIFT                40                           //!< Alpha reference bit shift
#define GRAPHICS_STATE_ALPHA_REF_MASK                 UINT64_C(0x0000ff0000000000) //!< Alpha reference bit mask
#define GRAPHICS_STATE_ALPHA_REF(v) ( ( (uint64_t)(v)<<GRAPHICS_STATE_ALPHA_REF_SHIFT )&GRAPHICS_STATE_ALPHA_REF_MASK)

#define GRAPHICS_STATE_PT_TRISTRIP                    UINT64_C(0x0001000000000000) //!< Tristrip.
#define GRAPHICS_STATE_PT_LINES                       UINT64_C(0x0002000000000000) //!< Lines.
#define GRAPHICS_STATE_PT_LINESTRIP                   UINT64_C(0x0003000000000000) //!< Line strip.
#define GRAPHICS_STATE_PT_POINTS                      UINT64_C(0x0004000000000000) //!< Points.
#define GRAPHICS_STATE_PT_SHIFT                       48                           //!< Primitive type bit shift
#define GRAPHICS_STATE_PT_MASK                        UINT64_C(0x0007000000000000) //!< Primitive type bit mask

/**
 * Point size value.
 *
 */
#define GRAPHICS_STATE_POINT_SIZE_SHIFT               52                           //!< Point size bit shift
#define GRAPHICS_STATE_POINT_SIZE_MASK                UINT64_C(0x00f0000000000000) //!< Point size bit mask
#define GRAPHICS_STATE_POINT_SIZE(v) ( ( (uint64_t)(v)<<GRAPHICS_STATE_POINT_SIZE_SHIFT )&GRAPHICS_STATE_POINT_SIZE_MASK)

/**
 * Enable MSAA write when writing into MSAA frame buffer.
 * This flag is ignored when not writing into MSAA frame buffer.
 *
 */
#define GRAPHICS_STATE_MSAA                           UINT64_C(0x0100000000000000) //!< Enable MSAA rasterization.
#define GRAPHICS_STATE_LINEAA                         UINT64_C(0x0200000000000000) //!< Enable line AA rasterization.
#define GRAPHICS_STATE_CONSERVATIVE_RASTER            UINT64_C(0x0400000000000000) //!< Enable conservative rasterization.
#define GRAPHICS_STATE_NONE                           UINT64_C(0x0000000000000000) //!< No state.
#define GRAPHICS_STATE_FRONT_CCW                      UINT64_C(0x0000008000000000) //!< Front counter-clockwise (default is clockwise).
#define GRAPHICS_STATE_BLEND_INDEPENDENT              UINT64_C(0x0000000400000000) //!< Enable blend independent.
#define GRAPHICS_STATE_BLEND_ALPHA_TO_COVERAGE        UINT64_C(0x0000000800000000) //!< Enable alpha to coverage.
/// Default state is write to RGB, alpha, and depth with depth test less enabled, with clockwise
/// culling and MSAA (when writing into MSAA frame buffer, otherwise this flag is ignored).
#define GRAPHICS_STATE_DEFAULT (0 \
	| GRAPHICS_STATE_WRITE_RGB \
	| GRAPHICS_STATE_WRITE_A \
	| GRAPHICS_STATE_WRITE_Z \
	| GRAPHICS_STATE_DEPTH_TEST_LESS \
	| GRAPHICS_STATE_CULL_CW \
	| GRAPHICS_STATE_MSAA \
	)

#define GRAPHICS_STATE_MASK                           UINT64_C(0xffffffffffffffff) //!< State bit mask

/**
 * Do not use!
 *
 */
#define GRAPHICS_STATE_RESERVED_SHIFT                 61

#define GRAPHICS_STATE_RESERVED_MASK                  UINT64_C(0xe000000000000000)

/**
 * Set stencil ref value.
 *
 */
#define GRAPHICS_STENCIL_FUNC_REF_SHIFT               0

#define GRAPHICS_STENCIL_FUNC_REF_MASK                UINT32_C(0x000000ff)
#define GRAPHICS_STENCIL_FUNC_REF(v) ( ( (uint32_t)(v)<<GRAPHICS_STENCIL_FUNC_REF_SHIFT )&GRAPHICS_STENCIL_FUNC_REF_MASK)

/**
 * Set stencil rmask value.
 *
 */
#define GRAPHICS_STENCIL_FUNC_RMASK_SHIFT             8

#define GRAPHICS_STENCIL_FUNC_RMASK_MASK              UINT32_C(0x0000ff00)
#define GRAPHICS_STENCIL_FUNC_RMASK(v) ( ( (uint32_t)(v)<<GRAPHICS_STENCIL_FUNC_RMASK_SHIFT )&GRAPHICS_STENCIL_FUNC_RMASK_MASK)

#define GRAPHICS_STENCIL_NONE                         UINT32_C(0x00000000)
#define GRAPHICS_STENCIL_MASK                         UINT32_C(0xffffffff)
#define GRAPHICS_STENCIL_DEFAULT                      UINT32_C(0x00000000)

#define GRAPHICS_STENCIL_TEST_LESS                    UINT32_C(0x00010000) //!< Enable stencil test, less.
#define GRAPHICS_STENCIL_TEST_LEQUAL                  UINT32_C(0x00020000) //!< Enable stencil test, less or equal.
#define GRAPHICS_STENCIL_TEST_EQUAL                   UINT32_C(0x00030000) //!< Enable stencil test, equal.
#define GRAPHICS_STENCIL_TEST_GEQUAL                  UINT32_C(0x00040000) //!< Enable stencil test, greater or equal.
#define GRAPHICS_STENCIL_TEST_GREATER                 UINT32_C(0x00050000) //!< Enable stencil test, greater.
#define GRAPHICS_STENCIL_TEST_NOTEQUAL                UINT32_C(0x00060000) //!< Enable stencil test, not equal.
#define GRAPHICS_STENCIL_TEST_NEVER                   UINT32_C(0x00070000) //!< Enable stencil test, never.
#define GRAPHICS_STENCIL_TEST_ALWAYS                  UINT32_C(0x00080000) //!< Enable stencil test, always.
#define GRAPHICS_STENCIL_TEST_SHIFT                   16                   //!< Stencil test bit shift
#define GRAPHICS_STENCIL_TEST_MASK                    UINT32_C(0x000f0000) //!< Stencil test bit mask

#define GRAPHICS_STENCIL_OP_FAIL_S_ZERO               UINT32_C(0x00000000) //!< Zero.
#define GRAPHICS_STENCIL_OP_FAIL_S_KEEP               UINT32_C(0x00100000) //!< Keep.
#define GRAPHICS_STENCIL_OP_FAIL_S_REPLACE            UINT32_C(0x00200000) //!< Replace.
#define GRAPHICS_STENCIL_OP_FAIL_S_INCR               UINT32_C(0x00300000) //!< Increment and wrap.
#define GRAPHICS_STENCIL_OP_FAIL_S_INCRSAT            UINT32_C(0x00400000) //!< Increment and clamp.
#define GRAPHICS_STENCIL_OP_FAIL_S_DECR               UINT32_C(0x00500000) //!< Decrement and wrap.
#define GRAPHICS_STENCIL_OP_FAIL_S_DECRSAT            UINT32_C(0x00600000) //!< Decrement and clamp.
#define GRAPHICS_STENCIL_OP_FAIL_S_INVERT             UINT32_C(0x00700000) //!< Invert.
#define GRAPHICS_STENCIL_OP_FAIL_S_SHIFT              20                   //!< Stencil operation fail bit shift
#define GRAPHICS_STENCIL_OP_FAIL_S_MASK               UINT32_C(0x00f00000) //!< Stencil operation fail bit mask

#define GRAPHICS_STENCIL_OP_FAIL_Z_ZERO               UINT32_C(0x00000000) //!< Zero.
#define GRAPHICS_STENCIL_OP_FAIL_Z_KEEP               UINT32_C(0x01000000) //!< Keep.
#define GRAPHICS_STENCIL_OP_FAIL_Z_REPLACE            UINT32_C(0x02000000) //!< Replace.
#define GRAPHICS_STENCIL_OP_FAIL_Z_INCR               UINT32_C(0x03000000) //!< Increment and wrap.
#define GRAPHICS_STENCIL_OP_FAIL_Z_INCRSAT            UINT32_C(0x04000000) //!< Increment and clamp.
#define GRAPHICS_STENCIL_OP_FAIL_Z_DECR               UINT32_C(0x05000000) //!< Decrement and wrap.
#define GRAPHICS_STENCIL_OP_FAIL_Z_DECRSAT            UINT32_C(0x06000000) //!< Decrement and clamp.
#define GRAPHICS_STENCIL_OP_FAIL_Z_INVERT             UINT32_C(0x07000000) //!< Invert.
#define GRAPHICS_STENCIL_OP_FAIL_Z_SHIFT              24                   //!< Stencil operation depth fail bit shift
#define GRAPHICS_STENCIL_OP_FAIL_Z_MASK               UINT32_C(0x0f000000) //!< Stencil operation depth fail bit mask

#define GRAPHICS_STENCIL_OP_PASS_Z_ZERO               UINT32_C(0x00000000) //!< Zero.
#define GRAPHICS_STENCIL_OP_PASS_Z_KEEP               UINT32_C(0x10000000) //!< Keep.
#define GRAPHICS_STENCIL_OP_PASS_Z_REPLACE            UINT32_C(0x20000000) //!< Replace.
#define GRAPHICS_STENCIL_OP_PASS_Z_INCR               UINT32_C(0x30000000) //!< Increment and wrap.
#define GRAPHICS_STENCIL_OP_PASS_Z_INCRSAT            UINT32_C(0x40000000) //!< Increment and clamp.
#define GRAPHICS_STENCIL_OP_PASS_Z_DECR               UINT32_C(0x50000000) //!< Decrement and wrap.
#define GRAPHICS_STENCIL_OP_PASS_Z_DECRSAT            UINT32_C(0x60000000) //!< Decrement and clamp.
#define GRAPHICS_STENCIL_OP_PASS_Z_INVERT             UINT32_C(0x70000000) //!< Invert.
#define GRAPHICS_STENCIL_OP_PASS_Z_SHIFT              28                   //!< Stencil operation depth pass bit shift
#define GRAPHICS_STENCIL_OP_PASS_Z_MASK               UINT32_C(0xf0000000) //!< Stencil operation depth pass bit mask

#define GRAPHICS_CLEAR_NONE                           UINT16_C(0x0000) //!< No clear flags.
#define GRAPHICS_CLEAR_COLOR                          UINT16_C(0x0001) //!< Clear color.
#define GRAPHICS_CLEAR_DEPTH                          UINT16_C(0x0002) //!< Clear depth.
#define GRAPHICS_CLEAR_STENCIL                        UINT16_C(0x0004) //!< Clear stencil.
#define GRAPHICS_CLEAR_DISCARD_COLOR_0                UINT16_C(0x0008) //!< Discard frame buffer attachment 0.
#define GRAPHICS_CLEAR_DISCARD_COLOR_1                UINT16_C(0x0010) //!< Discard frame buffer attachment 1.
#define GRAPHICS_CLEAR_DISCARD_COLOR_2                UINT16_C(0x0020) //!< Discard frame buffer attachment 2.
#define GRAPHICS_CLEAR_DISCARD_COLOR_3                UINT16_C(0x0040) //!< Discard frame buffer attachment 3.
#define GRAPHICS_CLEAR_DISCARD_COLOR_4                UINT16_C(0x0080) //!< Discard frame buffer attachment 4.
#define GRAPHICS_CLEAR_DISCARD_COLOR_5                UINT16_C(0x0100) //!< Discard frame buffer attachment 5.
#define GRAPHICS_CLEAR_DISCARD_COLOR_6                UINT16_C(0x0200) //!< Discard frame buffer attachment 6.
#define GRAPHICS_CLEAR_DISCARD_COLOR_7                UINT16_C(0x0400) //!< Discard frame buffer attachment 7.
#define GRAPHICS_CLEAR_DISCARD_DEPTH                  UINT16_C(0x0800) //!< Discard frame buffer depth attachment.
#define GRAPHICS_CLEAR_DISCARD_STENCIL                UINT16_C(0x1000) //!< Discard frame buffer stencil attachment.
#define GRAPHICS_CLEAR_DISCARD_COLOR_MASK (0 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_0 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_1 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_2 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_3 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_4 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_5 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_6 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_7 \
	)

#define GRAPHICS_CLEAR_DISCARD_MASK (0 \
	| GRAPHICS_CLEAR_DISCARD_COLOR_MASK \
	| GRAPHICS_CLEAR_DISCARD_DEPTH \
	| GRAPHICS_CLEAR_DISCARD_STENCIL \
	)


/**
 * Rendering state discard. When state is preserved in submit, rendering states can be discarded
 * on a finer grain.
 *
 */
#define GRAPHICS_DISCARD_NONE                         UINT8_C(0x00) //!< Preserve everything.
#define GRAPHICS_DISCARD_BINDINGS                     UINT8_C(0x01) //!< Discard texture sampler and buffer bindings.
#define GRAPHICS_DISCARD_INDEX_BUFFER                 UINT8_C(0x02) //!< Discard index buffer.
#define GRAPHICS_DISCARD_INSTANCE_DATA                UINT8_C(0x04) //!< Discard instance data.
#define GRAPHICS_DISCARD_STATE                        UINT8_C(0x08) //!< Discard state and uniform bindings.
#define GRAPHICS_DISCARD_TRANSFORM                    UINT8_C(0x10) //!< Discard transform.
#define GRAPHICS_DISCARD_VERTEX_STREAMS               UINT8_C(0x20) //!< Discard vertex streams.
#define GRAPHICS_DISCARD_ALL                          UINT8_C(0xff) //!< Discard all states.

#define GRAPHICS_DEBUG_NONE                           UINT32_C(0x00000000) //!< No debug.
#define GRAPHICS_DEBUG_WIREFRAME                      UINT32_C(0x00000001) //!< Enable wireframe for all primitives.

/// Enable infinitely fast hardware test. No draw calls will be submitted to driver.
/// It's useful when profiling to quickly assess bottleneck between CPU and GPU.
#define GRAPHICS_DEBUG_IFH                            UINT32_C(0x00000002)
#define GRAPHICS_DEBUG_STATS                          UINT32_C(0x00000004) //!< Enable statistics display.
#define GRAPHICS_DEBUG_TEXT                           UINT32_C(0x00000008) //!< Enable debug text display.
#define GRAPHICS_DEBUG_PROFILER                       UINT32_C(0x00000010) //!< Enable profiler. This causes per-view statistics to be collected, available through `graphics::Stats::ViewStats`. This is unrelated to the profiler functions in `graphics::CallbackI`.

#define GRAPHICS_BUFFER_COMPUTE_FORMAT_8X1            UINT16_C(0x0001) //!< 1 8-bit value
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_8X2            UINT16_C(0x0002) //!< 2 8-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_8X4            UINT16_C(0x0003) //!< 4 8-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_16X1           UINT16_C(0x0004) //!< 1 16-bit value
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_16X2           UINT16_C(0x0005) //!< 2 16-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_16X4           UINT16_C(0x0006) //!< 4 16-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_32X1           UINT16_C(0x0007) //!< 1 32-bit value
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_32X2           UINT16_C(0x0008) //!< 2 32-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_32X4           UINT16_C(0x0009) //!< 4 32-bit values
#define GRAPHICS_BUFFER_COMPUTE_FORMAT_SHIFT          0

#define GRAPHICS_BUFFER_COMPUTE_FORMAT_MASK           UINT16_C(0x000f)

#define GRAPHICS_BUFFER_COMPUTE_TYPE_INT              UINT16_C(0x0010) //!< Type `int`.
#define GRAPHICS_BUFFER_COMPUTE_TYPE_UINT             UINT16_C(0x0020) //!< Type `uint`.
#define GRAPHICS_BUFFER_COMPUTE_TYPE_FLOAT            UINT16_C(0x0030) //!< Type `float`.
#define GRAPHICS_BUFFER_COMPUTE_TYPE_SHIFT            4

#define GRAPHICS_BUFFER_COMPUTE_TYPE_MASK             UINT16_C(0x0030)

#define GRAPHICS_BUFFER_NONE                          UINT16_C(0x0000)
#define GRAPHICS_BUFFER_COMPUTE_READ                  UINT16_C(0x0100) //!< Buffer will be read by shader.
#define GRAPHICS_BUFFER_COMPUTE_WRITE                 UINT16_C(0x0200) //!< Buffer will be used for writing.
#define GRAPHICS_BUFFER_DRAW_INDIRECT                 UINT16_C(0x0400) //!< Buffer will be used for storing draw indirect commands.
#define GRAPHICS_BUFFER_ALLOW_RESIZE                  UINT16_C(0x0800) //!< Allow dynamic index/vertex buffer resize during update.
#define GRAPHICS_BUFFER_INDEX32                       UINT16_C(0x1000) //!< Index buffer contains 32-bit indices.
#define GRAPHICS_BUFFER_COMPUTE_READ_WRITE (0 \
	| GRAPHICS_BUFFER_COMPUTE_READ \
	| GRAPHICS_BUFFER_COMPUTE_WRITE \
	)


#define GRAPHICS_TEXTURE_NONE                         UINT64_C(0x0000000000000000)
#define GRAPHICS_TEXTURE_MSAA_SAMPLE                  UINT64_C(0x0000000800000000) //!< Texture will be used for MSAA sampling.
#define GRAPHICS_TEXTURE_RT                           UINT64_C(0x0000001000000000) //!< Render target no MSAA.
#define GRAPHICS_TEXTURE_COMPUTE_WRITE                UINT64_C(0x0000100000000000) //!< Texture will be used for compute write.
#define GRAPHICS_TEXTURE_SRGB                         UINT64_C(0x0000200000000000) //!< Sample texture as sRGB.
#define GRAPHICS_TEXTURE_BLIT_DST                     UINT64_C(0x0000400000000000) //!< Texture will be used as blit destination.
#define GRAPHICS_TEXTURE_READ_BACK                    UINT64_C(0x0000800000000000) //!< Texture will be used for read back from GPU.

#define GRAPHICS_TEXTURE_RT_MSAA_X2                   UINT64_C(0x0000002000000000) //!< Render target MSAAx2 mode.
#define GRAPHICS_TEXTURE_RT_MSAA_X4                   UINT64_C(0x0000003000000000) //!< Render target MSAAx4 mode.
#define GRAPHICS_TEXTURE_RT_MSAA_X8                   UINT64_C(0x0000004000000000) //!< Render target MSAAx8 mode.
#define GRAPHICS_TEXTURE_RT_MSAA_X16                  UINT64_C(0x0000005000000000) //!< Render target MSAAx16 mode.
#define GRAPHICS_TEXTURE_RT_MSAA_SHIFT                36

#define GRAPHICS_TEXTURE_RT_MSAA_MASK                 UINT64_C(0x0000007000000000)

#define GRAPHICS_TEXTURE_RT_WRITE_ONLY                UINT64_C(0x0000008000000000) //!< Render target will be used for writing
#define GRAPHICS_TEXTURE_RT_SHIFT                     36

#define GRAPHICS_TEXTURE_RT_MASK                      UINT64_C(0x000000f000000000)

/**
 * Sampler flags.
 *
 */
#define GRAPHICS_SAMPLER_U_MIRROR                     UINT32_C(0x00000001) //!< Wrap U mode: Mirror
#define GRAPHICS_SAMPLER_U_CLAMP                      UINT32_C(0x00000002) //!< Wrap U mode: Clamp
#define GRAPHICS_SAMPLER_U_BORDER                     UINT32_C(0x00000003) //!< Wrap U mode: Border
#define GRAPHICS_SAMPLER_U_SHIFT                      0

#define GRAPHICS_SAMPLER_U_MASK                       UINT32_C(0x00000003)

#define GRAPHICS_SAMPLER_V_MIRROR                     UINT32_C(0x00000004) //!< Wrap V mode: Mirror
#define GRAPHICS_SAMPLER_V_CLAMP                      UINT32_C(0x00000008) //!< Wrap V mode: Clamp
#define GRAPHICS_SAMPLER_V_BORDER                     UINT32_C(0x0000000c) //!< Wrap V mode: Border
#define GRAPHICS_SAMPLER_V_SHIFT                      2

#define GRAPHICS_SAMPLER_V_MASK                       UINT32_C(0x0000000c)

#define GRAPHICS_SAMPLER_W_MIRROR                     UINT32_C(0x00000010) //!< Wrap W mode: Mirror
#define GRAPHICS_SAMPLER_W_CLAMP                      UINT32_C(0x00000020) //!< Wrap W mode: Clamp
#define GRAPHICS_SAMPLER_W_BORDER                     UINT32_C(0x00000030) //!< Wrap W mode: Border
#define GRAPHICS_SAMPLER_W_SHIFT                      4

#define GRAPHICS_SAMPLER_W_MASK                       UINT32_C(0x00000030)

#define GRAPHICS_SAMPLER_MIN_POINT                    UINT32_C(0x00000040) //!< Min sampling mode: Point
#define GRAPHICS_SAMPLER_MIN_ANISOTROPIC              UINT32_C(0x00000080) //!< Min sampling mode: Anisotropic
#define GRAPHICS_SAMPLER_MIN_SHIFT                    6

#define GRAPHICS_SAMPLER_MIN_MASK                     UINT32_C(0x000000c0)

#define GRAPHICS_SAMPLER_MAG_POINT                    UINT32_C(0x00000100) //!< Mag sampling mode: Point
#define GRAPHICS_SAMPLER_MAG_ANISOTROPIC              UINT32_C(0x00000200) //!< Mag sampling mode: Anisotropic
#define GRAPHICS_SAMPLER_MAG_SHIFT                    8

#define GRAPHICS_SAMPLER_MAG_MASK                     UINT32_C(0x00000300)

#define GRAPHICS_SAMPLER_MIP_POINT                    UINT32_C(0x00000400) //!< Mip sampling mode: Point
#define GRAPHICS_SAMPLER_MIP_SHIFT                    10

#define GRAPHICS_SAMPLER_MIP_MASK                     UINT32_C(0x00000400)

#define GRAPHICS_SAMPLER_COMPARE_LESS                 UINT32_C(0x00010000) //!< Compare when sampling depth texture: less.
#define GRAPHICS_SAMPLER_COMPARE_LEQUAL               UINT32_C(0x00020000) //!< Compare when sampling depth texture: less or equal.
#define GRAPHICS_SAMPLER_COMPARE_EQUAL                UINT32_C(0x00030000) //!< Compare when sampling depth texture: equal.
#define GRAPHICS_SAMPLER_COMPARE_GEQUAL               UINT32_C(0x00040000) //!< Compare when sampling depth texture: greater or equal.
#define GRAPHICS_SAMPLER_COMPARE_GREATER              UINT32_C(0x00050000) //!< Compare when sampling depth texture: greater.
#define GRAPHICS_SAMPLER_COMPARE_NOTEQUAL             UINT32_C(0x00060000) //!< Compare when sampling depth texture: not equal.
#define GRAPHICS_SAMPLER_COMPARE_NEVER                UINT32_C(0x00070000) //!< Compare when sampling depth texture: never.
#define GRAPHICS_SAMPLER_COMPARE_ALWAYS               UINT32_C(0x00080000) //!< Compare when sampling depth texture: always.
#define GRAPHICS_SAMPLER_COMPARE_SHIFT                16

#define GRAPHICS_SAMPLER_COMPARE_MASK                 UINT32_C(0x000f0000)

#define GRAPHICS_SAMPLER_BORDER_COLOR_SHIFT           24

#define GRAPHICS_SAMPLER_BORDER_COLOR_MASK            UINT32_C(0x0f000000)
#define GRAPHICS_SAMPLER_BORDER_COLOR(v) ( ( (uint32_t)(v)<<GRAPHICS_SAMPLER_BORDER_COLOR_SHIFT )&GRAPHICS_SAMPLER_BORDER_COLOR_MASK)

#define GRAPHICS_SAMPLER_RESERVED_SHIFT               28

#define GRAPHICS_SAMPLER_RESERVED_MASK                UINT32_C(0xf0000000)

#define GRAPHICS_SAMPLER_NONE                         UINT32_C(0x00000000)
#define GRAPHICS_SAMPLER_SAMPLE_STENCIL               UINT32_C(0x00100000) //!< Sample stencil instead of depth.
#define GRAPHICS_SAMPLER_POINT (0 \
	| GRAPHICS_SAMPLER_MIN_POINT \
	| GRAPHICS_SAMPLER_MAG_POINT \
	| GRAPHICS_SAMPLER_MIP_POINT \
	)

#define GRAPHICS_SAMPLER_UVW_MIRROR (0 \
	| GRAPHICS_SAMPLER_U_MIRROR \
	| GRAPHICS_SAMPLER_V_MIRROR \
	| GRAPHICS_SAMPLER_W_MIRROR \
	)

#define GRAPHICS_SAMPLER_UVW_CLAMP (0 \
	| GRAPHICS_SAMPLER_U_CLAMP \
	| GRAPHICS_SAMPLER_V_CLAMP \
	| GRAPHICS_SAMPLER_W_CLAMP \
	)

#define GRAPHICS_SAMPLER_UVW_BORDER (0 \
	| GRAPHICS_SAMPLER_U_BORDER \
	| GRAPHICS_SAMPLER_V_BORDER \
	| GRAPHICS_SAMPLER_W_BORDER \
	)

#define GRAPHICS_SAMPLER_BITS_MASK (0 \
	| GRAPHICS_SAMPLER_U_MASK \
	| GRAPHICS_SAMPLER_V_MASK \
	| GRAPHICS_SAMPLER_W_MASK \
	| GRAPHICS_SAMPLER_MIN_MASK \
	| GRAPHICS_SAMPLER_MAG_MASK \
	| GRAPHICS_SAMPLER_MIP_MASK \
	| GRAPHICS_SAMPLER_COMPARE_MASK \
	)


#define GRAPHICS_RESET_MSAA_X2                        UINT32_C(0x00000010) //!< Enable 2x MSAA.
#define GRAPHICS_RESET_MSAA_X4                        UINT32_C(0x00000020) //!< Enable 4x MSAA.
#define GRAPHICS_RESET_MSAA_X8                        UINT32_C(0x00000030) //!< Enable 8x MSAA.
#define GRAPHICS_RESET_MSAA_X16                       UINT32_C(0x00000040) //!< Enable 16x MSAA.
#define GRAPHICS_RESET_MSAA_SHIFT                     4

#define GRAPHICS_RESET_MSAA_MASK                      UINT32_C(0x00000070)

#define GRAPHICS_RESET_NONE                           UINT32_C(0x00000000) //!< No reset flags.
#define GRAPHICS_RESET_FULLSCREEN                     UINT32_C(0x00000001) //!< Not supported yet.
#define GRAPHICS_RESET_VSYNC                          UINT32_C(0x00000080) //!< Enable V-Sync.
#define GRAPHICS_RESET_MAXANISOTROPY                  UINT32_C(0x00000100) //!< Turn on/off max anisotropy.
#define GRAPHICS_RESET_CAPTURE                        UINT32_C(0x00000200) //!< Begin screen capture.
#define GRAPHICS_RESET_FLUSH_AFTER_RENDER             UINT32_C(0x00002000) //!< Flush rendering after submitting to GPU.

/// This flag specifies where flip occurs. Default behaviour is that flip occurs
/// before rendering new frame. This flag only has effect when `GRAPHICS_CONFIG_MULTITHREADED=0`.
#define GRAPHICS_RESET_FLIP_AFTER_RENDER              UINT32_C(0x00004000)
#define GRAPHICS_RESET_SRGB_BACKBUFFER                UINT32_C(0x00008000) //!< Enable sRGB backbuffer.
#define GRAPHICS_RESET_HDR10                          UINT32_C(0x00010000) //!< Enable HDR10 rendering.
#define GRAPHICS_RESET_HIDPI                          UINT32_C(0x00020000) //!< Enable HiDPI rendering.
#define GRAPHICS_RESET_DEPTH_CLAMP                    UINT32_C(0x00040000) //!< Enable depth clamp.
#define GRAPHICS_RESET_SUSPEND                        UINT32_C(0x00080000) //!< Suspend rendering.
#define GRAPHICS_RESET_TRANSPARENT_BACKBUFFER         UINT32_C(0x00100000) //!< Transparent backbuffer. Availability depends on: `GRAPHICS_CAPS_TRANSPARENT_BACKBUFFER`.

#define GRAPHICS_RESET_FULLSCREEN_SHIFT               0

#define GRAPHICS_RESET_FULLSCREEN_MASK                UINT32_C(0x00000001)

#define GRAPHICS_RESET_RESERVED_SHIFT                 31                   //!< Internal bit shift
#define GRAPHICS_RESET_RESERVED_MASK                  UINT32_C(0x80000000) //!< Internal bit mask

#define GRAPHICS_CAPS_ALPHA_TO_COVERAGE               UINT64_C(0x0000000000000001) //!< Alpha to coverage is supported.
#define GRAPHICS_CAPS_BLEND_INDEPENDENT               UINT64_C(0x0000000000000002) //!< Blend independent is supported.
#define GRAPHICS_CAPS_COMPUTE                         UINT64_C(0x0000000000000004) //!< Compute shaders are supported.
#define GRAPHICS_CAPS_CONSERVATIVE_RASTER             UINT64_C(0x0000000000000008) //!< Conservative rasterization is supported.
#define GRAPHICS_CAPS_DRAW_INDIRECT                   UINT64_C(0x0000000000000010) //!< Draw indirect is supported.
#define GRAPHICS_CAPS_FRAGMENT_DEPTH                  UINT64_C(0x0000000000000020) //!< Fragment depth is available in fragment shader.
#define GRAPHICS_CAPS_FRAGMENT_ORDERING               UINT64_C(0x0000000000000040) //!< Fragment ordering is available in fragment shader.
#define GRAPHICS_CAPS_GRAPHICS_DEBUGGER               UINT64_C(0x0000000000000080) //!< Graphics debugger is present.
#define GRAPHICS_CAPS_HDR10                           UINT64_C(0x0000000000000100) //!< HDR10 rendering is supported.
#define GRAPHICS_CAPS_HIDPI                           UINT64_C(0x0000000000000200) //!< HiDPI rendering is supported.
#define GRAPHICS_CAPS_IMAGE_RW                        UINT64_C(0x0000000000000400) //!< Image Read/Write is supported.
#define GRAPHICS_CAPS_INDEX32                         UINT64_C(0x0000000000000800) //!< 32-bit indices are supported.
#define GRAPHICS_CAPS_INSTANCING                      UINT64_C(0x0000000000001000) //!< Instancing is supported.
#define GRAPHICS_CAPS_OCCLUSION_QUERY                 UINT64_C(0x0000000000002000) //!< Occlusion query is supported.
#define GRAPHICS_CAPS_RENDERER_MULTITHREADED          UINT64_C(0x0000000000004000) //!< Renderer is on separate thread.
#define GRAPHICS_CAPS_SWAP_CHAIN                      UINT64_C(0x0000000000008000) //!< Multiple windows are supported.
#define GRAPHICS_CAPS_TEXTURE_2D_ARRAY                UINT64_C(0x0000000000010000) //!< 2D texture array is supported.
#define GRAPHICS_CAPS_TEXTURE_3D                      UINT64_C(0x0000000000020000) //!< 3D textures are supported.
#define GRAPHICS_CAPS_TEXTURE_BLIT                    UINT64_C(0x0000000000040000) //!< Texture blit is supported.
#define GRAPHICS_CAPS_TRANSPARENT_BACKBUFFER          UINT64_C(0x0000000000080000) //!< Transparent back buffer supported.
#define GRAPHICS_CAPS_TEXTURE_COMPARE_RESERVED        UINT64_C(0x0000000000100000)
#define GRAPHICS_CAPS_TEXTURE_COMPARE_LEQUAL          UINT64_C(0x0000000000200000) //!< Texture compare less equal mode is supported.
#define GRAPHICS_CAPS_TEXTURE_CUBE_ARRAY              UINT64_C(0x0000000000400000) //!< Cubemap texture array is supported.
#define GRAPHICS_CAPS_TEXTURE_DIRECT_ACCESS           UINT64_C(0x0000000000800000) //!< CPU direct access to GPU texture memory.
#define GRAPHICS_CAPS_TEXTURE_READ_BACK               UINT64_C(0x0000000001000000) //!< Read-back texture is supported.
#define GRAPHICS_CAPS_VERTEX_ATTRIB_HALF              UINT64_C(0x0000000002000000) //!< Vertex attribute half-float is supported.
#define GRAPHICS_CAPS_VERTEX_ATTRIB_UINT10            UINT64_C(0x0000000004000000) //!< Vertex attribute 10_10_10_2 is supported.
#define GRAPHICS_CAPS_VERTEX_ID                       UINT64_C(0x0000000008000000) //!< Rendering with VertexID only is supported.
#define GRAPHICS_CAPS_VIEWPORT_LAYER_ARRAY            UINT64_C(0x0000000010000000) //!< Viewport layer is available in vertex shader.
#define GRAPHICS_CAPS_DRAW_INDIRECT_COUNT             UINT64_C(0x0000000020000000) //!< Draw indirect with indirect count is supported.
/// All texture compare modes are supported.
#define GRAPHICS_CAPS_TEXTURE_COMPARE_ALL (0 \
	| GRAPHICS_CAPS_TEXTURE_COMPARE_RESERVED \
	| GRAPHICS_CAPS_TEXTURE_COMPARE_LEQUAL \
	)


#define GRAPHICS_CAPS_FORMAT_TEXTURE_NONE             UINT32_C(0x00000000) //!< Texture format is not supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_2D               UINT32_C(0x00000001) //!< Texture format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_2D_SRGB          UINT32_C(0x00000002) //!< Texture as sRGB format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_2D_EMULATED      UINT32_C(0x00000004) //!< Texture format is emulated.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_3D               UINT32_C(0x00000008) //!< Texture format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_3D_SRGB          UINT32_C(0x00000010) //!< Texture as sRGB format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_3D_EMULATED      UINT32_C(0x00000020) //!< Texture format is emulated.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE             UINT32_C(0x00000040) //!< Texture format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_SRGB        UINT32_C(0x00000080) //!< Texture as sRGB format is supported.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_CUBE_EMULATED    UINT32_C(0x00000100) //!< Texture format is emulated.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_VERTEX           UINT32_C(0x00000200) //!< Texture format can be used from vertex shader.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_READ       UINT32_C(0x00000400) //!< Texture format can be used as image and read from.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_IMAGE_WRITE      UINT32_C(0x00000800) //!< Texture format can be used as image and written to.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER      UINT32_C(0x00001000) //!< Texture format can be used as frame buffer.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_FRAMEBUFFER_MSAA UINT32_C(0x00002000) //!< Texture format can be used as MSAA frame buffer.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_MSAA             UINT32_C(0x00004000) //!< Texture can be sampled as MSAA.
#define GRAPHICS_CAPS_FORMAT_TEXTURE_MIP_AUTOGEN      UINT32_C(0x00008000) //!< Texture format supports auto-generated mips.

#define GRAPHICS_RESOLVE_NONE                         UINT8_C(0x00) //!< No resolve flags.
#define GRAPHICS_RESOLVE_AUTO_GEN_MIPS                UINT8_C(0x01) //!< Auto-generate mip maps on resolve.

#define GRAPHICS_PCI_ID_NONE                          UINT16_C(0x0000) //!< Autoselect adapter.
#define GRAPHICS_PCI_ID_SOFTWARE_RASTERIZER           UINT16_C(0x0001) //!< Software rasterizer.
#define GRAPHICS_PCI_ID_AMD                           UINT16_C(0x1002) //!< AMD adapter.
#define GRAPHICS_PCI_ID_APPLE                         UINT16_C(0x106b) //!< Apple adapter.
#define GRAPHICS_PCI_ID_INTEL                         UINT16_C(0x8086) //!< Intel adapter.
#define GRAPHICS_PCI_ID_NVIDIA                        UINT16_C(0x10de) //!< nVidia adapter.
#define GRAPHICS_PCI_ID_MICROSOFT                     UINT16_C(0x1414) //!< Microsoft adapter.
#define GRAPHICS_PCI_ID_ARM                           UINT16_C(0x13b5) //!< ARM adapter.

#define GRAPHICS_CUBE_MAP_POSITIVE_X                  UINT8_C(0x00) //!< Cubemap +x.
#define GRAPHICS_CUBE_MAP_NEGATIVE_X                  UINT8_C(0x01) //!< Cubemap -x.
#define GRAPHICS_CUBE_MAP_POSITIVE_Y                  UINT8_C(0x02) //!< Cubemap +y.
#define GRAPHICS_CUBE_MAP_NEGATIVE_Y                  UINT8_C(0x03) //!< Cubemap -y.
#define GRAPHICS_CUBE_MAP_POSITIVE_Z                  UINT8_C(0x04) //!< Cubemap +z.
#define GRAPHICS_CUBE_MAP_NEGATIVE_Z                  UINT8_C(0x05) //!< Cubemap -z.


/// Blend function separate.
#define GRAPHICS_STATE_BLEND_FUNC_SEPARATE(_srcRGB, _dstRGB, _srcA, _dstA) (UINT64_C(0) \
	| ( ( (uint64_t)(_srcRGB)|( (uint64_t)(_dstRGB)<<4) )   )                       \
	| ( ( (uint64_t)(_srcA  )|( (uint64_t)(_dstA  )<<4) )<<8)                       \
	)

/// Blend equation separate.
#define GRAPHICS_STATE_BLEND_EQUATION_SEPARATE(_equationRGB, _equationA) ( (uint64_t)(_equationRGB)|( (uint64_t)(_equationA)<<3) )

/// Blend function.
#define GRAPHICS_STATE_BLEND_FUNC(_src, _dst)    GRAPHICS_STATE_BLEND_FUNC_SEPARATE(_src, _dst, _src, _dst)

/// Blend equation.
#define GRAPHICS_STATE_BLEND_EQUATION(_equation) GRAPHICS_STATE_BLEND_EQUATION_SEPARATE(_equation, _equation)

/// Utility predefined blend modes.

/// Additive blending.
#define GRAPHICS_STATE_BLEND_ADD (0                                         \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_ONE, GRAPHICS_STATE_BLEND_ONE) \
	)

/// Alpha blend.
#define GRAPHICS_STATE_BLEND_ALPHA (0                                                       \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_SRC_ALPHA, GRAPHICS_STATE_BLEND_INV_SRC_ALPHA) \
	)

/// Selects darker color of blend.
#define GRAPHICS_STATE_BLEND_DARKEN (0                                      \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_ONE, GRAPHICS_STATE_BLEND_ONE) \
	| GRAPHICS_STATE_BLEND_EQUATION(GRAPHICS_STATE_BLEND_EQUATION_MIN)          \
	)

/// Selects lighter color of blend.
#define GRAPHICS_STATE_BLEND_LIGHTEN (0                                     \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_ONE, GRAPHICS_STATE_BLEND_ONE) \
	| GRAPHICS_STATE_BLEND_EQUATION(GRAPHICS_STATE_BLEND_EQUATION_MAX)          \
	)

/// Multiplies colors.
#define GRAPHICS_STATE_BLEND_MULTIPLY (0                                           \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_DST_COLOR, GRAPHICS_STATE_BLEND_ZERO) \
	)

/// Opaque pixels will cover the pixels directly below them without any math or algorithm applied to them.
#define GRAPHICS_STATE_BLEND_NORMAL (0                                                \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_ONE, GRAPHICS_STATE_BLEND_INV_SRC_ALPHA) \
	)

/// Multiplies the inverse of the blend and base colors.
#define GRAPHICS_STATE_BLEND_SCREEN (0                                                \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_ONE, GRAPHICS_STATE_BLEND_INV_SRC_COLOR) \
	)

/// Decreases the brightness of the base color based on the value of the blend color.
#define GRAPHICS_STATE_BLEND_LINEAR_BURN (0                                                 \
	| GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_DST_COLOR, GRAPHICS_STATE_BLEND_INV_DST_COLOR) \
	| GRAPHICS_STATE_BLEND_EQUATION(GRAPHICS_STATE_BLEND_EQUATION_SUB)                          \
	)

///
#define GRAPHICS_STATE_BLEND_FUNC_RT_x(_src, _dst) (0         \
	| ( (uint32_t)( (_src)>>GRAPHICS_STATE_BLEND_SHIFT)       \
	| ( (uint32_t)( (_dst)>>GRAPHICS_STATE_BLEND_SHIFT)<<4) ) \
	)

///
#define GRAPHICS_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation) (0         \
	| GRAPHICS_STATE_BLEND_FUNC_RT_x(_src, _dst)                          \
	| ( (uint32_t)( (_equation)>>GRAPHICS_STATE_BLEND_EQUATION_SHIFT)<<8) \
	)

#define GRAPHICS_STATE_BLEND_FUNC_RT_1(_src, _dst)  (GRAPHICS_STATE_BLEND_FUNC_RT_x(_src, _dst)<< 0)
#define GRAPHICS_STATE_BLEND_FUNC_RT_2(_src, _dst)  (GRAPHICS_STATE_BLEND_FUNC_RT_x(_src, _dst)<<11)
#define GRAPHICS_STATE_BLEND_FUNC_RT_3(_src, _dst)  (GRAPHICS_STATE_BLEND_FUNC_RT_x(_src, _dst)<<22)

#define GRAPHICS_STATE_BLEND_FUNC_RT_1E(_src, _dst, _equation) (GRAPHICS_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<< 0)
#define GRAPHICS_STATE_BLEND_FUNC_RT_2E(_src, _dst, _equation) (GRAPHICS_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<11)
#define GRAPHICS_STATE_BLEND_FUNC_RT_3E(_src, _dst, _equation) (GRAPHICS_STATE_BLEND_FUNC_RT_xE(_src, _dst, _equation)<<22)


#endif // GRAPHICS_DEFINES_H_HEADER_GUARD
