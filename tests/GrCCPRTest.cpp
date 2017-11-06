/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrClip.h"
#include "GrDrawingManager.h"
#include "GrPathRenderer.h"
#include "GrPaint.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrShape.h"
#include "SkMatrix.h"
#include "SkPathPriv.h"
#include "SkRect.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "mock/GrMockTypes.h"
#include <cmath>

static constexpr int kCanvasSize = 100;

class CCPRPathDrawer {
public:
    CCPRPathDrawer(GrContext* ctx, skiatest::Reporter* reporter)
            : fCtx(ctx)
            , fCCPR(fCtx->contextPriv().drawingManager()->getCoverageCountingPathRenderer())
            , fRTC(fCtx->makeDeferredRenderTargetContext(SkBackingFit::kExact, kCanvasSize,
                                                         kCanvasSize, kRGBA_8888_GrPixelConfig,
                                                         nullptr)) {
        if (!fCCPR) {
            ERRORF(reporter, "ccpr not enabled in GrContext for ccpr tests");
        }
        if (!fRTC) {
            ERRORF(reporter, "failed to create GrRenderTargetContext for ccpr tests");
        }
    }

    bool valid() const { return fCCPR && fRTC; }
    void clear() const { fRTC->clear(nullptr, 0, true); }
    void abandonGrContext() { fCtx = nullptr; fCCPR = nullptr; fRTC = nullptr; }

    void drawPath(SkPath path, GrColor4f color = GrColor4f(0, 1, 0, 1)) const {
        SkASSERT(this->valid());

        GrPaint paint;
        paint.setColor4f(color);

        GrNoClip noClip;
        SkIRect clipBounds = SkIRect::MakeWH(kCanvasSize, kCanvasSize);

        SkMatrix matrix = SkMatrix::I();

        path.setIsVolatile(true);
        GrShape shape(path);

        fCCPR->drawPath({fCtx, std::move(paint), &GrUserStencilSettings::kUnused, fRTC.get(),
                         &noClip, &clipBounds, &matrix, &shape, GrAAType::kCoverage, false});
    }

    void flush() const {
        SkASSERT(this->valid());
        fCtx->flush();
    }

private:
    GrContext*                        fCtx;
    GrCoverageCountingPathRenderer*   fCCPR;
    sk_sp<GrRenderTargetContext>      fRTC;
};

class CCPRTest {
public:
    void run(skiatest::Reporter* reporter) {
        GrMockOptions mockOptions;
        mockOptions.fInstanceAttribSupport = true;
        mockOptions.fMapBufferFlags = GrCaps::kCanMap_MapFlag;
        mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fRenderable[0] = true;
        mockOptions.fConfigOptions[kAlpha_half_GrPixelConfig].fTexturable = true;
        mockOptions.fGeometryShaderSupport = true;
        mockOptions.fTexelBufferSupport = true;
        mockOptions.fIntegerSupport = true;
        mockOptions.fFlatInterpolationSupport = true;
        mockOptions.fMaxVertexSamplers = 1;

        GrContextOptions ctxOptions;
        ctxOptions.fAllowPathMaskCaching = false;
        ctxOptions.fGpuPathRenderers = GpuPathRenderers::kCoverageCounting;

        fMockContext = GrContext::MakeMock(&mockOptions, ctxOptions);
        if (!fMockContext) {
            ERRORF(reporter, "could not create mock context");
            return;
        }
        if (!fMockContext->unique()) {
            ERRORF(reporter, "mock context is not unique");
            return;
        }

        CCPRPathDrawer ccpr(fMockContext.get(), reporter);
        if (!ccpr.valid()) {
            return;
        }

        fPath.moveTo(0, 0);
        fPath.cubicTo(50, 50, 0, 50, 50, 0);
        this->onRun(reporter, ccpr);
    }

    virtual ~CCPRTest() {}

protected:
    virtual void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) = 0;

    sk_sp<GrContext>   fMockContext;
    SkPath             fPath;
};

#define DEF_CCPR_TEST(name) \
    DEF_GPUTEST(name, reporter, factory) { \
        name test; \
        test.run(reporter); \
    }

class GrCCPRTest_cleanup : public CCPRTest {
    void onRun(skiatest::Reporter* reporter, CCPRPathDrawer& ccpr) override {
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure paths get unreffed.
        for (int i = 0; i < 10; ++i) {
            ccpr.drawPath(fPath);
        }
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        ccpr.flush();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));

        // Ensure paths get unreffed when we delete the context without flushing.
        for (int i = 0; i < 10; ++i) {
            ccpr.drawPath(fPath);
        }
        ccpr.abandonGrContext();
        REPORTER_ASSERT(reporter, !SkPathPriv::TestingOnly_unique(fPath));
        fMockContext.reset();
        REPORTER_ASSERT(reporter, SkPathPriv::TestingOnly_unique(fPath));
    }
};
DEF_CCPR_TEST(GrCCPRTest_cleanup)

class CCPRRenderingTest {
public:
    void run(skiatest::Reporter* reporter, GrContext* ctx) const {
        if (!ctx->contextPriv().drawingManager()->getCoverageCountingPathRenderer()) {
            return; // CCPR is not enabled on this GPU.
        }
        CCPRPathDrawer ccpr(ctx, reporter);
        if (!ccpr.valid()) {
            return;
        }
        this->onRun(reporter, ccpr);
    }

    virtual ~CCPRRenderingTest() {}

protected:
    virtual void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const = 0;
};

#define DEF_CCPR_RENDERING_TEST(name) \
    DEF_GPUTEST_FOR_RENDERING_CONTEXTS(name, reporter, ctxInfo) { \
        name test; \
        test.run(reporter, ctxInfo.grContext()); \
    }

class GrCCPRTest_busyPath : public CCPRRenderingTest {
    void onRun(skiatest::Reporter* reporter, const CCPRPathDrawer& ccpr) const override {
        static constexpr int kNumBusyVerbs = 1 << 17;
        ccpr.clear();
        SkPath busyPath;
        busyPath.moveTo(0, 0); // top left
        busyPath.lineTo(kCanvasSize, kCanvasSize); // bottom right
        for (int i = 2; i < kNumBusyVerbs; ++i) {
            float offset = i * ((float)kCanvasSize / kNumBusyVerbs);
            busyPath.lineTo(kCanvasSize - offset, kCanvasSize + offset); // offscreen
        }
        ccpr.drawPath(busyPath);

        ccpr.flush(); // If this doesn't crash, the test passed.
                      // If it does, maybe fiddle with fMaxInstancesPerDrawArraysWithoutCrashing in
                      // your platform's GrGLCaps.
    }
};
DEF_CCPR_RENDERING_TEST(GrCCPRTest_busyPath)

#endif
