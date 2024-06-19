/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSegmentFittingAlg.h"
#include <Math/Functor.h>
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"

using namespace MuonR4;

MuonSegmentFittingAlg::MuonSegmentFittingAlg(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode MuonSegmentFittingAlg::initialize() {
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_inHoughSegmentSeedKey.initialize());
    ATH_CHECK(m_outSegments.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFittingAlg::execute(const EventContext& ctx) const {

    MuonSegmentFitterEventData data; 
    ATH_CHECK(prepareEventData(ctx, data)); 
    const StationHoughSegmentSeedContainer* segmentSeeds=nullptr; 
    ATH_CHECK(retrieveContainer(ctx, m_inHoughSegmentSeedKey, segmentSeeds));
    SG::WriteHandle<MuonSegmentContainer> writeSegments(
        m_outSegments, ctx);
    ATH_CHECK(writeSegments.record(
        std::make_unique<MuonSegmentContainer>()));
    for (const StationHoughSegmentSeeds & stationWithMax : *segmentSeeds){
        for (auto & seed : stationWithMax.getMaxima()){
            ATH_CHECK(prepareSegmentFit(seed,data)); 
            ATH_CHECK(fitSegment(data)); 
            if (data.foundMin){
                writeSegments->push_back(buildSegment(data)); 
            }
        }
    }

    return StatusCode::SUCCESS; 
}

template <class ContainerType>
StatusCode MuonSegmentFittingAlg::retrieveContainer(
    const EventContext& ctx, const SG::ReadHandleKey<ContainerType>& key,
    const ContainerType*& contToPush) const {
    contToPush = nullptr;
    if (key.empty()) {
        ATH_MSG_VERBOSE("No key has been parsed for object "
                        << typeid(ContainerType).name());
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle<ContainerType> readHandle{key, ctx};
    ATH_CHECK(readHandle.isPresent());
    contToPush = readHandle.cptr();
    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFittingAlg::prepareSegmentFit(const HoughSegmentSeed & seed,  MuonSegmentFitterEventData & data) const{
    data.chi2 = -1;
    data.measurementsToFit = seed.getHitsInMax(); 
    data.foundMin = false;
    data.x0 = seed.interceptX();
    data.y0 = seed.interceptY();
    data.tanTheta = seed.tanTheta();
    data.tanPhi = seed.tanPhi();
    data.minimizer->SetVariable((int)MuonSegmentFitterEventData::parameterIndices::y0,"y0",data.y0,1.e-5);
    data.minimizer->SetVariable((int)MuonSegmentFitterEventData::parameterIndices::x0,"x0",data.x0,1.e-5);
    data.minimizer->SetVariable((int)MuonSegmentFitterEventData::parameterIndices::tanTheta,"tanTheta",data.tanTheta,1.e-5);
    data.minimizer->SetVariable((int)MuonSegmentFitterEventData::parameterIndices::tanPhi,"tanPhi",data.tanPhi,1.e-5);
    data.minimizer->SetVariableLimits((int)MuonSegmentFitterEventData::parameterIndices::y0,data.y0 - 600, data.y0 + 600);
    data.minimizer->SetVariableLimits((int)MuonSegmentFitterEventData::parameterIndices::x0,data.x0 - 600, data.x0 + 600);
    data.minimizer->SetVariableLimits((int)MuonSegmentFitterEventData::parameterIndices::tanTheta,data.tanTheta - 0.6, data.tanTheta + 0.6);
    data.minimizer->SetVariableLimits((int)MuonSegmentFitterEventData::parameterIndices::tanPhi,data.tanPhi - 0.6, data.tanPhi + 0.6);
    data.minimizer->SetMaxFunctionCalls(1e8);  // for Minuit/Minuit2
    data.minimizer->SetMaxIterations(1e8);       // for GSL
    data.minimizer->SetTolerance(0.0001);
    data.minimizer->SetStrategy(1); 
    data.minimizer->SetPrintLevel(0);
    data.chi2_per_measurement.clear();
    data.chi2_per_measurement.resize(2 * data.measurementsToFit.size());
    return StatusCode::SUCCESS;
}
StatusCode MuonSegmentFittingAlg::prepareEventData(const EventContext & ctx, MuonSegmentFitterEventData & data) const {
    const ActsGeometryContext* gctx{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctx));
    data.gctx = gctx;
    return StatusCode::SUCCESS;
}

StatusCode MuonSegmentFittingAlg::fitSegment(MuonSegmentFitterEventData & data) const{
    ROOT::Math::Functor c2f(std::bind(SegmentFitHelpers::segmentChiSquare, std::placeholders::_1, data.measurementsToFit, data.chi2_per_measurement),4);
    data.minimizer->SetFunction(c2f);
    // do the minimization
    if (!data.minimizer->Minimize()){
        data.minimizer->SetStrategy(2); 
        data.minimizer->Minimize();
    }
    data.minimizer->Hesse();
    const double* xs = data.minimizer->X();
    const double* errs = data.minimizer->Errors();
    SegmentFitHelpers::segmentChiSquare(xs, data.measurementsToFit, data.chi2_per_measurement);   // updates the event data's chi2 per layer 
    data.x0 = xs[(int)MuonSegmentFitterEventData::parameterIndices::x0];
    data.y0 = xs[(int)MuonSegmentFitterEventData::parameterIndices::y0];
    data.sigmaX0 = errs[(int)MuonSegmentFitterEventData::parameterIndices::x0];
    data.sigmaY0 = errs[(int)MuonSegmentFitterEventData::parameterIndices::y0];
    data.tanTheta = xs[(int)MuonSegmentFitterEventData::parameterIndices::tanTheta];
    data.tanPhi = xs[(int)MuonSegmentFitterEventData::parameterIndices::tanPhi];
    data.sigmaTanTheta = errs[(int)MuonSegmentFitterEventData::parameterIndices::tanTheta];
    data.sigmaTanPhi = errs[(int)MuonSegmentFitterEventData::parameterIndices::tanPhi];
    data.chi2 = c2f(xs); 
    data.foundMin = true; 

    return StatusCode::SUCCESS;
}

MuonR4::MuonSegment MuonSegmentFittingAlg::buildSegment(MuonSegmentFitterEventData & data) const{
    // dummy 
    std::vector<const xAOD::UncalibratedMeasurement*> measurements;  
    for (auto hit : data.measurementsToFit){
        if (hit->primaryMeasurement()) measurements.push_back(hit->primaryMeasurement());
        if (hit->secondaryMeasurement()) measurements.push_back(hit->secondaryMeasurement());
    }
    MuonSegment segment(data.y0, data.x0, data.tanTheta, data.tanPhi, measurements); 
    segment.setChi2(data.chi2);
    segment.setChi2PerMeasurement(data.chi2_per_measurement);
    return segment;
}
