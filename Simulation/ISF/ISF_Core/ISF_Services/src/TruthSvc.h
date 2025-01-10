/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_SERVICES_TRUTHSVC_H
#define ISF_SERVICES_TRUTHSVC_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

// ISF include
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_HepMC_Interfaces/ITruthStrategy.h"
#include "ISF_Event/ITruthIncident.h"

// DetectorDescription
#include "AtlasDetDescr/AtlasRegion.h"

// Barcode
#include "BarcodeInterfaces/IBarcodeSvc.h"

// McEventCollection
#include "GeneratorObjects/McEventCollection.h"

// forward declarations
class StoreGateSvc;

#include "AtlasHepMC/GenEvent_fwd.h"
#include "AtlasHepMC/GenVertex.h"

namespace ISFTesting {
  class TruthSvc_test;
}

namespace ISF {

  /** @class TruthSvc

      HepMC based version of the ISF::ITruthSvc,
      currently it takes an ITruthIncident base class


      @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
  */
  class TruthSvc final : public extends<AthService, ITruthSvc> {

    // allow test to access private data
    friend ISFTesting::TruthSvc_test;

  public:

    //** Constructor with parameters */
    TruthSvc( const std::string& name, ISvcLocator* pSvcLocator );

    /** Destructor */
    virtual ~TruthSvc() = default;

    /** Athena algorithm's interface method initialize() */
    StatusCode  initialize() override;
    /** Athena algorithm's interface method finalize() */
    StatusCode  finalize() override;

    /** Register a truth incident */
    void registerTruthIncident( ITruthIncident& truthincident, bool saveAllChildren=false) const override;

    /** Initialize the Truth Svc at the beginning of each event */
    StatusCode initializeTruthCollection(int largestGeneratedParticleBC=0, int largestGeneratedVertexBC=0) override;

    /** Finalize the Truth Svc at the end of each event*/
    StatusCode releaseEvent() override;

  private:
    /** Record the given truth incident to the MC Truth */
    void recordIncidentToMCTruth( ITruthIncident& truthincident, bool passWholeVertex) const;
    /** Record and end vertex to the MC Truth for the parent particle */
    HepMC::GenVertexPtr  createGenVertexFromTruthIncident( ITruthIncident& truthincident) const;

    ServiceHandle<Barcode::IBarcodeSvc> m_barcodeSvc{this, "BarcodeSvc", "BarcodeSvc", ""};           //!< The Barcode service

    /** the truth strategies applied (as AthenaToolHandle Array) */
    ToolHandleArray<ITruthStrategy> m_truthStrategies{this, "TruthStrategies", {}, ""};
    /** for faster access: using an internal pointer to the actual ITruthStrategy instances */
    ITruthStrategy**              m_geoStrategies[AtlasDetDescr::fNumAtlasRegions];
    unsigned short                 m_numStrategies[AtlasDetDescr::fNumAtlasRegions];

    /** MCTruth steering */
    Gaudi::Property<bool>   m_skipIfNoChildren{this, "SkipIfNoChildren", true, ""};       //!< do not record incident if numChildren==0
    Gaudi::Property<bool>   m_skipIfNoParentId{this, "SkipIfNoParentId", true, ""};  //!< do not record if parentId == HepMC::UNDEFINED_ID
    Gaudi::Property<bool>   m_ignoreUndefinedBarcodes{this, "IgnoreUndefinedBarcodes",  false, ""};//!< do/don't abort if retrieve an undefined barcode

    Gaudi::Property<bool>   m_passWholeVertex{this, "PassWholeVertices", true, ""};

    Gaudi::Property<std::vector<unsigned int> >  m_forceEndVtxRegionsVec{this, "ForceEndVtxInRegions", {}, ""}; //!< property containing AtlasRegions for which
    //   to write end-vtx
    std::array<bool, AtlasDetDescr::fNumAtlasRegions> m_forceEndVtx{}; //!< attach end vertex to
    //   all parent particles if they die

    Gaudi::Property<bool>   m_quasiStableParticlesIncluded{this, "QuasiStableParticlesIncluded", false, ""}; //!< does this job simulate quasi-stable particles.
    Gaudi::Property<bool>   m_quasiStableParticleOverwrite{this, "QuasiStableParticleOverwrite", true, ""}; //!< should the job be overwriting QS particles

  };
}


#endif //> !ISF_SERVICES_TRUTHSVC_H
