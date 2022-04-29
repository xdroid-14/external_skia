/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlUtils_DEFINED
#define skgpu_graphite_MtlUtils_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/sksl/ir/SkSLProgram.h"

#import <Metal/Metal.h>

namespace skgpu {
class ShaderErrorHandler;
}

namespace skgpu::graphite {
class MtlGpu;

bool MtlFormatIsDepthOrStencil(MTLPixelFormat);
bool MtlFormatIsDepth(MTLPixelFormat);
bool MtlFormatIsStencil(MTLPixelFormat);

MTLPixelFormat MtlDepthStencilFlagsToFormat(SkEnumBitMask<DepthStencilFlags>);

/**
 * Produces MSL code generated by SkSLC
 */
bool SkSLToMSL(const MtlGpu* gpu,
               const std::string& sksl,
               SkSL::ProgramKind kind,
               const SkSL::Program::Settings& settings,
               std::string* msl,
               SkSL::Program::Inputs* outInputs,
               ShaderErrorHandler* errorHandler);

sk_cfp<id<MTLLibrary>> MtlCompileShaderLibrary(const MtlGpu* gpu,
                                               const std::string& msl,
                                               ShaderErrorHandler* errorHandler);

#ifdef SK_BUILD_FOR_IOS
bool MtlIsAppInBackground();
#endif
} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlUtils_DEFINED
