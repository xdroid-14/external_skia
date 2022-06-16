/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlTrampoline.h"

#include "src/gpu/graphite/mtl/MtlGpu.h"

namespace skgpu::graphite {
sk_sp<skgpu::graphite::Gpu> MtlTrampoline::MakeGpu(const MtlBackendContext& backendContext,
                                                   const ContextOptions& options) {
    return MtlGpu::Make(backendContext, options);
}

}  // namespace skgpu::graphite