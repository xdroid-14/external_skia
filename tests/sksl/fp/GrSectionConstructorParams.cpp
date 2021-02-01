

/**************************************************************************************************
 *** This file was autogenerated from GrSectionConstructorParams.fp; do not modify.
 **************************************************************************************************/
#include "GrSectionConstructorParams.h"

#include "src/core/SkUtils.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLSectionConstructorParams : public GrGLSLFragmentProcessor {
public:
    GrGLSLSectionConstructorParams() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrSectionConstructorParams& _outer = args.fFp.cast<GrSectionConstructorParams>();
        (void) _outer;
        auto w = _outer.w;
        (void) w;
        fragBuilder->codeAppendf(
R"SkSL(return half4(1.0);
)SkSL"
);
    }
private:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrFragmentProcessor& _proc) override {
    }
};
GrGLSLFragmentProcessor* GrSectionConstructorParams::onCreateGLSLInstance() const {
    return new GrGLSLSectionConstructorParams();
}
void GrSectionConstructorParams::onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const {
}
bool GrSectionConstructorParams::onIsEqual(const GrFragmentProcessor& other) const {
    const GrSectionConstructorParams& that = other.cast<GrSectionConstructorParams>();
    (void) that;
    if (w != that.w) return false;
    return true;
}
GrSectionConstructorParams::GrSectionConstructorParams(const GrSectionConstructorParams& src)
: INHERITED(kGrSectionConstructorParams_ClassID, src.optimizationFlags())
, w(src.w) {
        this->cloneAndRegisterAllChildProcessors(src);
}
std::unique_ptr<GrFragmentProcessor> GrSectionConstructorParams::clone() const {
    return std::make_unique<GrSectionConstructorParams>(*this);
}
#if GR_TEST_UTILS
SkString GrSectionConstructorParams::onDumpInfo() const {
    return SkStringPrintf("(w=%f)", w);
}
#endif