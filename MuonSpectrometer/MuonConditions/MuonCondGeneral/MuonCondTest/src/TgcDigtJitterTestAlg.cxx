/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcDigtJitterTestAlg.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"


// Constructor
TgcDigtJitterTestAlg::TgcDigtJitterTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
    AthAlgorithm(name, pSvcLocator) {}

// Destructor
TgcDigtJitterTestAlg::~TgcDigtJitterTestAlg() = default;

// Initialize
StatusCode TgcDigtJitterTestAlg::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_rndmSvc.retrieve());
    return StatusCode::SUCCESS;
}

// Execute
StatusCode TgcDigtJitterTestAlg::execute(const EventContext& ctx) {  
    ATH_MSG_INFO("Calling execute");   
    SG::ReadCondHandle<TgcDigitJitterData> readHandle{m_readKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    } 
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
    rngWrapper->setSeed(name(), ctx);
    CLHEP::HepRandomEngine* rndmEngine = rngWrapper->getEngine(ctx);
    for (unsigned int trial = 0; trial < m_testJitters; ++trial){
        ///
        const Amg::Vector3D dir{CLHEP::RandFlat::shoot(rndmEngine,-1., 1.) * Amg::Vector3D::UnitX()+
                                CLHEP::RandFlat::shoot(rndmEngine,-1., 1.) *Amg::Vector3D::UnitZ()};
        const double jitter = readHandle->drawJitter(dir, rndmEngine);
        ATH_MSG_ALWAYS("Direction "<<Amg::toString(dir,2)<<"  drawn jitter: "<<jitter);
    }
       
    return StatusCode::SUCCESS;
}
