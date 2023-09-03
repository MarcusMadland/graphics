#pragma once

#include <../3rdparty/bgfx/bgfx/include/bgfx/bgfx.h>
	
#define MRENDER_STATE_WRITE_R BGFX_STATE_WRITE_R
#define MRENDER_STATE_WRITE_G BGFX_STATE_WRITE_G
#define MRENDER_STATE_WRITE_B BGFX_STATE_WRITE_B
#define MRENDER_STATE_WRITE_A BGFX_STATE_WRITE_A
#define MRENDER_STATE_WRITE_Z BGFX_STATE_WRITE_Z

#define MRENDER_STATE_WRITE_RGB BGFX_STATE_WRITE_RGB
#define MRENDER_STATE_WRITE_MASK BGFX_STATE_WRITE_MASK

#define MRENDER_STATE_DEPTH_TEST_LESS BGFX_STATE_DEPTH_TEST_LESS
#define MRENDER_STATE_DEPTH_TEST_LEQUAL BGFX_STATE_DEPTH_TEST_LEQUAL
#define MRENDER_STATE_DEPTH_TEST_EQUAL BGFX_STATE_DEPTH_TEST_EQUAL
#define MRENDER_STATE_DEPTH_TEST_GEQUAL BGFX_STATE_DEPTH_TEST_GEQUAL
#define MRENDER_STATE_DEPTH_TEST_GREATER BGFX_STATE_DEPTH_TEST_GREATER
#define MRENDER_STATE_DEPTH_TEST_NOTEQUAL BGFX_STATE_DEPTH_TEST_NOTEQUAL
#define MRENDER_STATE_DEPTH_TEST_NEVER BGFX_STATE_DEPTH_TEST_NEVER
#define MRENDER_STATE_DEPTH_TEST_ALWAYS BGFX_STATE_DEPTH_TEST_ALWAYS

#define MRENDER_STATE_BLEND_ZERO BGFX_STATE_BLEND_ZERO
#define MRENDER_STATE_BLEND_ONE BGFX_STATE_BLEND_ONE
#define MRENDER_STATE_BLEND_SRC_COLOR BGFX_STATE_BLEND_SRC_COLOR
#define MRENDER_STATE_BLEND_INV_SRC_COLOR BGFX_STATE_BLEND_INV_SRC_COLOR
#define MRENDER_STATE_BLEND_SRC_ALPHA BGFX_STATE_BLEND_SRC_ALPHA
#define MRENDER_STATE_BLEND_INV_SRC_ALPHA BGFX_STATE_BLEND_INV_SRC_ALPHA
#define MRENDER_STATE_BLEND_DST_ALPHA BGFX_STATE_BLEND_DST_ALPHA
#define MRENDER_STATE_BLEND_INV_DST_ALPHA BGFX_STATE_BLEND_INV_DST_ALPHA
#define MRENDER_STATE_BLEND_DST_COLOR BGFX_STATE_BLEND_DST_COLOR
#define MRENDER_STATE_BLEND_INV_DST_COLOR BGFX_STATE_BLEND_INV_DST_COLOR
#define MRENDER_STATE_BLEND_SRC_ALPHA_SAT BGFX_STATE_BLEND_SRC_ALPHA_SAT
#define MRENDER_STATE_BLEND_FACTOR BGFX_STATE_BLEND_FACTOR
#define MRENDER_STATE_BLEND_INV_FACTOR BGFX_STATE_BLEND_INV_FACTOR

#define MRENDER_STATE_BLEND_EQUATION_ADD BGFX_STATE_BLEND_EQUATION_ADD
#define MRENDER_STATE_BLEND_EQUATION_SUB BGFX_STATE_BLEND_EQUATION_SUB
#define MRENDER_STATE_BLEND_EQUATION_REVSUB BGFX_STATE_BLEND_EQUATION_REVSUB
#define MRENDER_STATE_BLEND_EQUATION_MIN BGFX_STATE_BLEND_EQUATION_MIN
#define MRENDER_STATE_BLEND_EQUATION_MAX BGFX_STATE_BLEND_EQUATION_MAX

#define MRENDER_STATE_CULL_CW BGFX_STATE_CULL_CW
#define MRENDER_STATE_CULL_CCW BGFX_STATE_CULL_CCW

#define MRENDER_STATE_ALPHA_REF_SHIFT BGFX_STATE_ALPHA_REF_SHIFT
#define MRENDER_STATE_ALPHA_REF_MASK BGFX_STATE_ALPHA_REF_MASK

#define MRENDER_STATE_PT_TRISTRIP BGFX_STATE_PT_TRISTRIP
#define MRENDER_STATE_PT_LINES BGFX_STATE_PT_LINES
#define MRENDER_STATE_PT_LINESTRIP BGFX_STATE_PT_LINESTRIP
#define MRENDER_STATE_PT_POINTS BGFX_STATE_PT_POINTS

#define MRENDER_STATE_POINT_SIZE_SHIFT BGFX_STATE_POINT_SIZE_SHIFT
#define MRENDER_STATE_POINT_SIZE_MASK BGFX_STATE_POINT_SIZE_MASK

#define MRENDER_STATE_BLEND_ADD BGFX_STATE_BLEND_ADD
#define MRENDER_STATE_MSAA BGFX_STATE_MSAA
#define MRENDER_STATE_LINEAA BGFX_STATE_LINEAA
#define MRENDER_STATE_CONSERVATIVE_RASTER BGFX_STATE_CONSERVATIVE_RASTER
#define MRENDER_STATE_NONE BGFX_STATE_NONE
#define MRENDER_STATE_FRONT_CCW BGFX_STATE_FRONT_CCW
#define MRENDER_STATE_BLEND_INDEPENDENT BGFX_STATE_BLEND_INDEPENDENT
#define MRENDER_STATE_BLEND_ALPHA_TO_COVERAGE BGFX_STATE_BLEND_ALPHA_TO_COVERAGE
#define MRENDER_STATE_DEFAULT BGFX_STATE_DEFAULT
#define MRENDER_STATE_MASK BGFX_STATE_MASK

#define MRENDER_STATE_RESERVED_SHIFT BGFX_STATE_RESERVED_SHIFT
#define MRENDER_STATE_RESERVED_MASK BGFX_STATE_RESERVED_MASK

#define MRENDER_STENCIL_FUNC_REF_SHIFT BGFX_STENCIL_FUNC_REF_SHIFT
#define MRENDER_STENCIL_FUNC_REF_MASK BGFX_STENCIL_FUNC_REF_MASK

#define MRENDER_STENCIL_FUNC_RMASK_SHIFT BGFX_STENCIL_FUNC_RMASK_SHIFT
#define MRENDER_STENCIL_FUNC_RMASK_MASK BGFX_STENCIL_FUNC_RMASK_MASK

#define MRENDER_STENCIL_NONE BGFX_STENCIL_NONE
#define MRENDER_STENCIL_MASK BGFX_STENCIL_MASK
#define MRENDER_STENCIL_DEFAULT BGFX_STENCIL_DEFAULT

#define MRENDER_STENCIL_TEST_LESS BGFX_STENCIL_TEST_LESS
#define MRENDER_STENCIL_TEST_LEQUAL BGFX_STENCIL_TEST_LEQUAL
#define MRENDER_STENCIL_TEST_EQUAL BGFX_STENCIL_TEST_EQUAL
#define MRENDER_STENCIL_TEST_GEQUAL BGFX_STENCIL_TEST_GEQUAL
#define MRENDER_STENCIL_TEST_GREATER BGFX_STENCIL_TEST_GREATER
#define MRENDER_STENCIL_TEST_NOTEQUAL BGFX_STENCIL_TEST_NOTEQUAL
#define MRENDER_STENCIL_TEST_NEVER BGFX_STENCIL_TEST_NEVER
#define MRENDER_STENCIL_TEST_ALWAYS BGFX_STENCIL_TEST_ALWAYS
#define MRENDER_STENCIL_TEST_SHIFT BGFX_STENCIL_TEST_SHIFT
#define MRENDER_STENCIL_TEST_MASK BGFX_STENCIL_TEST_MASK

#define MRENDER_STENCIL_OP_FAIL_S_ZERO BGFX_STENCIL_OP_FAIL_S_ZERO
#define MRENDER_STENCIL_OP_FAIL_S_KEEP BGFX_STENCIL_OP_FAIL_S_KEEP
#define MRENDER_STENCIL_OP_FAIL_S_REPLACE BGFX_STENCIL_OP_FAIL_S_REPLACE
#define MRENDER_STENCIL_OP_FAIL_S_INCR BGFX_STENCIL_OP_FAIL_S_INCR
#define MRENDER_STENCIL_OP_FAIL_S_INCRSAT BGFX_STENCIL_OP_FAIL_S_INCRSAT
#define MRENDER_STENCIL_OP_FAIL_S_DECR BGFX_STENCIL_OP_FAIL_S_DECR
#define MRENDER_STENCIL_OP_FAIL_S_DECRSAT BGFX_STENCIL_OP_FAIL_S_DECRSAT
#define MRENDER_STENCIL_OP_FAIL_S_INVERT BGFX_STENCIL_OP_FAIL_S_INVERT
#define MRENDER_STENCIL_OP_FAIL_S_SHIFT BGFX_STENCIL_OP_FAIL_S_SHIFT
#define MRENDER_STENCIL_OP_FAIL_S_MASK BGFX_STENCIL_OP_FAIL_S_MASK

#define MRENDER_STENCIL_OP_FAIL_Z_ZERO			BGFX_STENCIL_OP_FAIL_Z_ZERO
#define MRENDER_STENCIL_OP_FAIL_Z_KEEP			BGFX_STENCIL_OP_FAIL_Z_KEEP
#define MRENDER_STENCIL_OP_FAIL_Z_REPLACE		BGFX_STENCIL_OP_FAIL_Z_REPLACE
#define MRENDER_STENCIL_OP_FAIL_Z_INCR			BGFX_STENCIL_OP_FAIL_Z_INCR
#define MRENDER_STENCIL_OP_FAIL_Z_INCRSAT		BGFX_STENCIL_OP_FAIL_Z_INCRSAT
#define MRENDER_STENCIL_OP_FAIL_Z_DECR			BGFX_STENCIL_OP_FAIL_Z_DECR
#define MRENDER_STENCIL_OP_FAIL_Z_DECRSAT		BGFX_STENCIL_OP_FAIL_Z_DECRSAT
#define MRENDER_STENCIL_OP_FAIL_Z_INVERT		BGFX_STENCIL_OP_FAIL_Z_INVERT
#define MRENDER_STENCIL_OP_FAIL_Z_SHIFT			BGFX_STENCIL_OP_FAIL_Z_SHIFT
#define MRENDER_STENCIL_OP_FAIL_Z_MASK			BGFX_STENCIL_OP_FAIL_Z_MASK

#define MRENDER_STENCIL_OP_PASS_Z_ZERO			BGFX_STENCIL_OP_PASS_Z_ZERO
#define MRENDER_STENCIL_OP_PASS_Z_KEEP			BGFX_STENCIL_OP_PASS_Z_KEEP
#define MRENDER_STENCIL_OP_PASS_Z_REPLACE       BGFX_STENCIL_OP_PASS_Z_REPLACE
#define MRENDER_STENCIL_OP_PASS_Z_INCR          BGFX_STENCIL_OP_PASS_Z_INCR
#define MRENDER_STENCIL_OP_PASS_Z_INCRSAT       BGFX_STENCIL_OP_PASS_Z_INCRSAT
#define MRENDER_STENCIL_OP_PASS_Z_DECR          BGFX_STENCIL_OP_PASS_Z_DECR
#define MRENDER_STENCIL_OP_PASS_Z_DECRSAT       BGFX_STENCIL_OP_PASS_Z_DECRSAT
#define MRENDER_STENCIL_OP_PASS_Z_INVERT        BGFX_STENCIL_OP_PASS_Z_INVERT

#define MRENDER_CLEAR_NONE                      BGFX_CLEAR_NONE
#define MRENDER_CLEAR_COLOR                     BGFX_CLEAR_COLOR
#define MRENDER_CLEAR_DEPTH                     BGFX_CLEAR_DEPTH
#define MRENDER_CLEAR_STENCIL                   BGFX_CLEAR_STENCIL
#define MRENDER_CLEAR_DISCARD_COLOR_0           BGFX_CLEAR_DISCARD_COLOR_0
#define MRENDER_CLEAR_DISCARD_COLOR_1           BGFX_CLEAR_DISCARD_COLOR_1
#define MRENDER_CLEAR_DISCARD_COLOR_2           BGFX_CLEAR_DISCARD_COLOR_2
#define MRENDER_CLEAR_DISCARD_COLOR_3           BGFX_CLEAR_DISCARD_COLOR_3
#define MRENDER_CLEAR_DISCARD_COLOR_4           BGFX_CLEAR_DISCARD_COLOR_4
#define MRENDER_CLEAR_DISCARD_COLOR_5           BGFX_CLEAR_DISCARD_COLOR_5
#define MRENDER_CLEAR_DISCARD_COLOR_6           BGFX_CLEAR_DISCARD_COLOR_6
#define MRENDER_CLEAR_DISCARD_COLOR_7           BGFX_CLEAR_DISCARD_COLOR_7
#define MRENDER_CLEAR_DISCARD_DEPTH             BGFX_CLEAR_DISCARD_DEPTH
#define MRENDER_CLEAR_DISCARD_STENCIL           BGFX_CLEAR_DISCARD_STENCIL

#define MRENDER_DISCARD_NONE                    BGFX_DISCARD_NONE
#define MRENDER_DISCARD_BINDINGS                BGFX_DISCARD_BINDINGS
#define MRENDER_DISCARD_INDEX_BUFFER            BGFX_DISCARD_INDEX_BUFFER
#define MRENDER_DISCARD_INSTANCE_DATA           BGFX_DISCARD_INSTANCE_DATA
#define MRENDER_DISCARD_STATE                   BGFX_DISCARD_STATE
#define MRENDER_DISCARD_TRANSFORM               BGFX_DISCARD_TRANSFORM
#define MRENDER_DISCARD_VERTEX_STREAMS          BGFX_DISCARD_VERTEX_STREAMS
#define MRENDER_DISCARD_ALL                     BGFX_DISCARD_ALL

#define MRENDER_DEBUG_NONE                      BGFX_DEBUG_NONE
#define MRENDER_DEBUG_WIREFRAME                 BGFX_DEBUG_WIREFRAME
#define MRENDER_DEBUG_IFH                       BGFX_DEBUG_IFH
#define MRENDER_DEBUG_STATS                     BGFX_DEBUG_STATS
#define MRENDER_DEBUG_TEXT                      BGFX_DEBUG_TEXT
#define MRENDER_DEBUG_PROFILER                  BGFX_DEBUG_PROFILER

#define MRENDER_BUFFER_COMPUTE_FORMAT_8X1       BGFX_BUFFER_COMPUTE_FORMAT_8X1
#define MRENDER_BUFFER_COMPUTE_FORMAT_8X2       BGFX_BUFFER_COMPUTE_FORMAT_8X2
#define MRENDER_BUFFER_COMPUTE_FORMAT_8X4       BGFX_BUFFER_COMPUTE_FORMAT_8X4
#define MRENDER_BUFFER_COMPUTE_FORMAT_16X1      BGFX_BUFFER_COMPUTE_FORMAT_16X1
#define MRENDER_BUFFER_COMPUTE_FORMAT_16X2      BGFX_BUFFER_COMPUTE_FORMAT_16X2
#define MRENDER_BUFFER_COMPUTE_FORMAT_16X4      BGFX_BUFFER_COMPUTE_FORMAT_16X4
#define MRENDER_BUFFER_COMPUTE_FORMAT_32X1      BGFX_BUFFER_COMPUTE_FORMAT_32X1
#define MRENDER_BUFFER_COMPUTE_FORMAT_32X2      BGFX_BUFFER_COMPUTE_FORMAT_32X2
#define MRENDER_BUFFER_COMPUTE_FORMAT_32X4      BGFX_BUFFER_COMPUTE_FORMAT_32X4

#define MRENDER_BUFFER_COMPUTE_TYPE_INT         BGFX_BUFFER_COMPUTE_TYPE_INT
#define MRENDER_BUFFER_COMPUTE_TYPE_UINT        BGFX_BUFFER_COMPUTE_TYPE_UINT
#define MRENDER_BUFFER_COMPUTE_TYPE_FLOAT       BGFX_BUFFER_COMPUTE_TYPE_FLOAT

#define MRENDER_BUFFER_NONE                     BGFX_BUFFER_NONE
#define MRENDER_BUFFER_COMPUTE_READ             BGFX_BUFFER_COMPUTE_READ
#define MRENDER_BUFFER_COMPUTE_WRITE            BGFX_BUFFER_COMPUTE_WRITE
#define MRENDER_BUFFER_DRAW_INDIRECT            BGFX_BUFFER_DRAW_INDIRECT
#define MRENDER_BUFFER_ALLOW_RESIZE             BGFX_BUFFER_ALLOW_RESIZE
#define MRENDER_BUFFER_INDEX32                  BGFX_BUFFER_INDEX32
#define MRENDER_BUFFER_COMPUTE_READ_WRITE       BGFX_BUFFER_COMPUTE_READ_WRITE

#define MRENDER_TEXTURE_NONE                    BGFX_TEXTURE_NONE
#define MRENDER_TEXTURE_MSAA_SAMPLE             BGFX_TEXTURE_MSAA_SAMPLE
#define MRENDER_TEXTURE_RT                      BGFX_TEXTURE_RT
#define MRENDER_TEXTURE_COMPUTE_WRITE           BGFX_TEXTURE_COMPUTE_WRITE
#define MRENDER_TEXTURE_SRGB                    BGFX_TEXTURE_SRGB
#define MRENDER_TEXTURE_BLIT_DST                BGFX_TEXTURE_BLIT_DST
#define MRENDER_TEXTURE_READ_BACK               BGFX_TEXTURE_READ_BACK
#define MRENDER_TEXTURE_RT_MSAA_X2              BGFX_TEXTURE_RT_MSAA_X2
#define MRENDER_TEXTURE_RT_MSAA_X4              BGFX_TEXTURE_RT_MSAA_X4
#define MRENDER_TEXTURE_RT_MSAA_X8              BGFX_TEXTURE_RT_MSAA_X8
#define MRENDER_TEXTURE_RT_MSAA_X16             BGFX_TEXTURE_RT_MSAA_X16
#define MRENDER_TEXTURE_RT_WRITE_ONLY           BGFX_TEXTURE_RT_WRITE_ONLY

#define MRENDER_SAMPLER_U_MIRROR                BGFX_SAMPLER_U_MIRROR
#define MRENDER_SAMPLER_U_CLAMP                 BGFX_SAMPLER_U_CLAMP
#define MRENDER_SAMPLER_U_BORDER                BGFX_SAMPLER_U_BORDER
#define MRENDER_SAMPLER_V_MIRROR                BGFX_SAMPLER_V_MIRROR
#define MRENDER_SAMPLER_V_CLAMP                 BGFX_SAMPLER_V_CLAMP
#define MRENDER_SAMPLER_V_BORDER                BGFX_SAMPLER_V_BORDER
#define MRENDER_SAMPLER_W_MIRROR                BGFX_SAMPLER_W_MIRROR
#define MRENDER_SAMPLER_W_CLAMP                 BGFX_SAMPLER_W_CLAMP
#define MRENDER_SAMPLER_W_BORDER                BGFX_SAMPLER_W_BORDER
#define MRENDER_SAMPLER_MIN_POINT               BGFX_SAMPLER_MIN_POINT
#define MRENDER_SAMPLER_MIN_ANISOTROPIC         BGFX_SAMPLER_MIN_ANISOTROPIC
#define MRENDER_SAMPLER_MAG_POINT               BGFX_SAMPLER_MAG_POINT
#define MRENDER_SAMPLER_MAG_ANISOTROPIC         BGFX_SAMPLER_MAG_ANISOTROPIC
#define MRENDER_SAMPLER_MIP_POINT               BGFX_SAMPLER_MIP_POINT
#define MRENDER_SAMPLER_COMPARE_LEQUAL          BGFX_SAMPLER_COMPARE_LEQUAL
#define MRENDER_SAMPLER_COMPARE_LESS            BGFX_SAMPLER_COMPARE_LESS
#define MRENDER_SAMPLER_COMPARE_EQUAL           BGFX_SAMPLER_COMPARE_EQUAL
#define MRENDER_SAMPLER_COMPARE_GREATER         BGFX_SAMPLER_COMPARE_GREATER
#define MRENDER_SAMPLER_COMPARE_GEQUAL          BGFX_SAMPLER_COMPARE_GEQUAL
#define MRENDER_SAMPLER_COMPARE_NOTEQUAL        BGFX_SAMPLER_COMPARE_NOTEQUAL
#define MRENDER_SAMPLER_COMPARE_ALWAYS          BGFX_SAMPLER_COMPARE_ALWAYS
#define MRENDER_SAMPLER_BORDER_COLOR            BGFX_SAMPLER_BORDER_COLOR

#define MRENDER_RESET_NONE                      BGFX_RESET_NONE
#define MRENDER_RESET_FULLSCREEN                BGFX_RESET_FULLSCREEN
#define MRENDER_RESET_FULLSCREEN_SHIFT          BGFX_RESET_FULLSCREEN_SHIFT
#define MRENDER_RESET_FULLSCREEN_MASK           BGFX_RESET_FULLSCREEN_MASK
#define MRENDER_RESET_MSAA_X2                   BGFX_RESET_MSAA_X2
#define MRENDER_RESET_MSAA_X4                   BGFX_RESET_MSAA_X4
#define MRENDER_RESET_MSAA_X8                   BGFX_RESET_MSAA_X8
#define MRENDER_RESET_MSAA_X16                  BGFX_RESET_MSAA_X16
#define MRENDER_RESET_VSYNC                     BGFX_RESET_VSYNC
#define MRENDER_RESET_MAXANISOTROPY_SHIFT       BGFX_RESET_MAXANISOTROPY_SHIFT
#define MRENDER_RESET_MAXANISOTROPY_MASK        BGFX_RESET_MAXANISOTROPY_MASK
#define MRENDER_RESET_CAPTURE                   BGFX_RESET_CAPTURE
#define MRENDER_RESET_FLUSH_AFTER_RENDER        BGFX_RESET_FLUSH_AFTER_RENDER
#define MRENDER_RESET_FLIP_AFTER_RENDER         BGFX_RESET_FLIP_AFTER_RENDER
#define MRENDER_RESET_SRGB_BACKBUFFER           BGFX_RESET_SRGB_BACKBUFFER
#define MRENDER_RESET_HIDPI                     BGFX_RESET_HIDPI
#define MRENDER_RESET_DEPTH_CLAMP               BGFX_RESET_DEPTH_CLAMP
#define MRENDER_RESET_SUSPEND                   BGFX_RESET_SUSPEND
#define MRENDER_RESET_COUNT                     BGFX_RESET_COUNT