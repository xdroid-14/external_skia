

/**************************************************************************************************
 *** This file was autogenerated from GrSectionClass.fp; do not modify.
 **************************************************************************************************/
#ifndef GrSectionClass_DEFINED
#define GrSectionClass_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkTypes.h"


#include "src/gpu/GrFragmentProcessor.h"

class GrSectionClass : public GrFragmentProcessor {
public:
class section    static std::unique_ptr<GrFragmentProcessor> Make() {
        return std::unique_ptr<GrFragmentProcessor>(new GrSectionClass());
    }
    GrSectionClass(const GrSectionClass& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "SectionClass"; }
private:
    GrSectionClass()
    : INHERITED(kGrSectionClass_ClassID, kNone_OptimizationFlags) {
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    using INHERITED = GrFragmentProcessor;
};
#endif