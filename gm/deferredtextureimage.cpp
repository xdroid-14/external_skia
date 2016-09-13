/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <vector>

#include "gm.h"
#include "GrContext.h"
#include "SkMipMap.h"
#include "Resources.h"

#if SK_SUPPORT_GPU

// Helper function that uploads the given SkImage using MakdeFromDeferredTextureImageData and then
// draws the uploaded version at the specified coordinates.
static void DrawDeferredTextureImageData(SkCanvas* canvas,
                                         SkImage::DeferredTextureImageUsageParams* params) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }
    SkAutoTUnref<GrContextThreadSafeProxy> proxy(context->threadSafeProxy());



    sk_sp<SkImage> encodedImage = GetResourceAsImage("mandrill_512.png");
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    size_t requiredMemoryInBytes = encodedImage->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, SkSourceGammaTreatment::kRespect);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    std::vector<uint8_t> memory;
    memory.resize(requiredMemoryInBytes);
    encodedImage->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), SkSourceGammaTreatment::kRespect);
    sk_sp<SkImage> uploadedEncodedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    canvas->drawImage(uploadedEncodedImage, 10, 10);



    SkBitmap bitmap;
    if (!GetResourceAsBitmap("mandrill_512.png", &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }
    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    requiredMemoryInBytes = decodedImage->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, SkSourceGammaTreatment::kRespect);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    memory.resize(requiredMemoryInBytes);
    decodedImage->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), SkSourceGammaTreatment::kRespect);
    sk_sp<SkImage> uploadedDecodedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    canvas->drawImage(uploadedDecodedImage, 512 + 20, 10);
}

static void DrawDeferredTextureImageMipMapTree(SkCanvas* canvas, SkImage* image,
                                               SkImage::DeferredTextureImageUsageParams* params) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }
    SkAutoTUnref<GrContextThreadSafeProxy> proxy(context->threadSafeProxy());

    SkPaint paint;
    paint.setFilterQuality(params->fQuality);

    int mipLevelCount = SkMipMap::ComputeLevelCount(512, 512);
    size_t requiredMemoryInBytes = image->getDeferredTextureImageData(
        *proxy, params, 1, nullptr, SkSourceGammaTreatment::kRespect);
    if (requiredMemoryInBytes == 0) {
        SkDebugf("\nCould not create DeferredTextureImageData.\n");
        return;
    }

    std::vector<uint8_t> memory;
    memory.resize(requiredMemoryInBytes);
    image->getDeferredTextureImageData(
        *proxy, params, 1, memory.data(), SkSourceGammaTreatment::kRespect);
    sk_sp<SkImage> uploadedImage = SkImage::MakeFromDeferredTextureImageData(
        context, memory.data(), SkBudgeted::kNo);

    // draw a column using deferred texture images
    SkScalar offsetHeight = 10.f;
    // handle base mipmap level
    canvas->save();
    canvas->translate(10.f, offsetHeight);
    canvas->drawImage(uploadedImage, 0, 0, &paint);
    canvas->restore();
    offsetHeight += 512 + 10;
    // handle generated mipmap levels
    for (int i = 0; i < mipLevelCount; i++) {
        SkISize mipSize = SkMipMap::ComputeLevelSize(512, 512, i);
        canvas->save();
        canvas->translate(10.f, offsetHeight);
        canvas->scale(mipSize.width() / 512.f, mipSize.height() / 512.f);
        canvas->drawImage(uploadedImage, 0, 0, &paint);
        canvas->restore();
        offsetHeight += mipSize.height() + 10;
    }

    // draw a column using SkImage
    offsetHeight = 10;
    // handle base mipmap level
    canvas->save();
    canvas->translate(512.f + 20.f, offsetHeight);
    canvas->drawImage(image, 0, 0, &paint);
    canvas->restore();
    offsetHeight += 512 + 10;
    // handle generated mipmap levels
    for (int i = 0; i < mipLevelCount; i++) {
        SkISize mipSize = SkMipMap::ComputeLevelSize(512, 512, i);
        canvas->save();
        canvas->translate(512.f + 20.f, offsetHeight);
        canvas->scale(mipSize.width() / 512.f, mipSize.height() / 512.f);
        canvas->drawImage(image, 0, 0, &paint);
        canvas->restore();
        offsetHeight += mipSize.height() + 10;
    }
}

DEF_SIMPLE_GM(deferred_texture_image_none, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kNone_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, &params);
}

DEF_SIMPLE_GM(deferred_texture_image_low, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kLow_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, &params);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_encoded, canvas, 512 + 512 + 30, 1110) {
    sk_sp<SkImage> encodedImage = GetResourceAsImage("mandrill_512.png");
    if (!encodedImage) {
        SkDebugf("\nCould not load resource.\n");
        return;
    }

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, encodedImage.get(), &params);
}

DEF_SIMPLE_GM(deferred_texture_image_medium_decoded, canvas, 512 + 512 + 30, 1110) {
    SkBitmap bitmap;
    if (!GetResourceAsBitmap("mandrill_512.png", &bitmap)) {
        SkDebugf("\nCould not decode resource.\n");
        return;
    }
    sk_sp<SkImage> decodedImage = SkImage::MakeFromBitmap(bitmap);

    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(0.25f, 0.25f),
                                                           kMedium_SkFilterQuality, 0);
    DrawDeferredTextureImageMipMapTree(canvas, decodedImage.get(), &params);
}

DEF_SIMPLE_GM(deferred_texture_image_high, canvas, 512 + 512 + 30, 512 + 20) {
    auto params = SkImage::DeferredTextureImageUsageParams(SkMatrix::MakeScale(1.f, 1.f),
                                                           kHigh_SkFilterQuality, 0);
    DrawDeferredTextureImageData(canvas, &params);
}

#endif
