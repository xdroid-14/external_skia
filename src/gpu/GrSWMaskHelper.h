/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSWMaskHelper_DEFINED
#define GrSWMaskHelper_DEFINED

#include "SkAutoPixmapStorage.h"
#include "GrColor.h"
#include "SkBitmap.h"
#include "SkDraw.h"
#include "SkMatrix.h"
#include "SkRasterClip.h"
#include "SkRegion.h"
#include "SkTypes.h"

class GrClip;
class GrPaint;
class GrShape;
class GrTextureProvider;
class GrStyle;
class GrTexture;
struct GrUserStencilSettings;

/**
 * The GrSWMaskHelper helps generate clip masks using the software rendering
 * path. It is intended to be used as:
 *
 *   GrSWMaskHelper helper(context);
 *   helper.init(...);
 *
 *      draw one or more paths/rects specifying the required boolean ops
 *
 *   toTexture();   // to get it from the internal bitmap to the GPU
 *
 * The result of this process will be the final mask (on the GPU) in the
 * upper left hand corner of the texture.
 */
class GrSWMaskHelper : SkNoncopyable {
public:
    GrSWMaskHelper(GrTextureProvider* texProvider) : fTexProvider(texProvider) { }

    // set up the internal state in preparation for draws. Since many masks
    // may be accumulated in the helper during creation, "resultBounds"
    // allows the caller to specify the region of interest - to limit the
    // amount of work.
    bool init(const SkIRect& resultBounds, const SkMatrix* matrix);

    // Draw a single rect into the accumulation bitmap using the specified op
    void drawRect(const SkRect& rect, SkRegion::Op op, bool antiAlias, uint8_t alpha);

    // Draw a single path into the accumuation bitmap using the specified op
    void drawShape(const GrShape&, SkRegion::Op op, bool antiAlias, uint8_t alpha);

    // Move the mask generation results from the internal bitmap to the gpu.
    void toTexture(GrTexture* texture);

    // Convert mask generation results to a signed distance field
    void toSDF(unsigned char* sdf);

    // Reset the internal bitmap
    void clear(uint8_t alpha) {
        fPixels.erase(SkColorSetARGB(alpha, 0xFF, 0xFF, 0xFF));
    }

    // Canonical usage utility that draws a single path and uploads it
    // to the GPU. The result is returned.
    static GrTexture* DrawShapeMaskToTexture(GrTextureProvider*,
                                             const GrShape&,
                                             const SkIRect& resultBounds,
                                             bool antiAlias,
                                             const SkMatrix* matrix);

    // This utility routine is used to add a shape's mask to some other draw.
    // The GrClipStackClip uses it to accumulate clip masks while the
    // GrSoftwarePathRenderer uses it to fulfill a drawPath call.
    // It draws with "texture" as a path mask into "target" using "rect" as
    // geometry and the current drawState. The current drawState is altered to
    // accommodate the mask.
    // Note that this method assumes that the GrPaint::kTotalStages slot in
    // the draw state can be used to hold the mask texture stage.
    // This method is really only intended to be used with the
    // output of DrawPathMaskToTexture.
    static void DrawToTargetWithShapeMask(GrTexture* texture,
                                          GrDrawContext*,
                                          const GrPaint& paint,
                                          const GrUserStencilSettings& userStencilSettings,
                                          const GrClip&,
                                          const SkMatrix& viewMatrix,
                                          const SkIRect& rect);

private:
    // Helper function to get a scratch texture suitable for capturing the
    // result (i.e., right size & format)
    GrTexture* createTexture();

    GrTextureProvider*  fTexProvider;
    SkMatrix            fMatrix;
    SkAutoPixmapStorage fPixels;
    SkDraw              fDraw;
    SkRasterClip        fRasterClip;

    typedef SkNoncopyable INHERITED;
};

#endif // GrSWMaskHelper_DEFINED
