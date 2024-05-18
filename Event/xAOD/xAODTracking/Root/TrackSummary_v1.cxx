/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackSummary_v1.h"

#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/AuxAccessorMacro.h"


namespace xAOD {

AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackSummary_v1, std::vector<double>, 
                                  params, setParams)

AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackSummary_v1, std::vector<double>,
                                  covParams, setCovParams)

DEFINE_API(TrackSummary_v1, unsigned int, nMeasurements, setnMeasurements)

DEFINE_API(TrackSummary_v1, unsigned int, nHoles, setnHoles)

DEFINE_API(TrackSummary_v1, float, chi2f, setChi2f)

DEFINE_API(TrackSummary_v1, unsigned int, ndf, setNdf)

DEFINE_API(TrackSummary_v1, unsigned int, nOutliers, setnOutliers)

DEFINE_API(TrackSummary_v1, unsigned int, nSharedHits, setnSharedHits)

DEFINE_API(TrackSummary_v1, unsigned int, tipIndex, setTipIndex)

DEFINE_API(TrackSummary_v1, unsigned int, stemIndex, setStemIndex)

AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackSummary_v1, unsigned int, surfaceIndex, setSurfaceIndex)


AUXSTORE_OBJECT_SETTER_AND_GETTER(TrackSummary_v1, uint8_t, particleHypothesis, setParticleHypothesis)


const SG::AuxElement::Accessor<std::vector<double> >
    xAOD::TrackSummary_v1::s_paramsAcc{"params"};
const SG::AuxElement::Accessor<std::vector<double> >
    xAOD::TrackSummary_v1::s_covParamsAcc{"covParams"};

void TrackSummary_v1::resize(size_t sz) {
  s_paramsAcc(*this).resize(sz);
  s_covParamsAcc(*this).resize(sz * sz);
}

size_t TrackSummary_v1::size() const {
  return s_paramsAcc(*this).size();
}
}  // namespace xAOD
