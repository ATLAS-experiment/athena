/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_THINNINGTOOLEXAMPLE_H
#define DERIVATIONFRAMEWORK_THINNINGTOOLEXAMPLE_H

#include <string>
#include <atomic>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ThinningHandleKey.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/IThinningTool.h"


namespace DerivationFramework {

  /** @class ThinningToolExample
  
      @author James Catmore -at- cern.ch
     */
  class ThinningToolExample : public extends<AthAlgTool, IThinningTool> {
    
  public: 
    /** Use constructor from base class */
    using base_class::base_class;

    /** Destructor */
    virtual ~ThinningToolExample();
    
    // Athena algtool's Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    
    /** Check that the current event passes this filter */
    virtual StatusCode doThinning() const override;
 
  private:
    Gaudi::Property<std::string> m_streamName
      { this, "StreamName", "", "Name of the stream being thinned" };

    Gaudi::Property<double> m_trackPtCut
      { this, "TrackPtCut", 20.0, "Track p_T cut in GeV" };

    SG::ThinningHandleKey<xAOD::TrackParticleContainer> m_inDetSGKey
      { this, "InDetTrackParticlesKey", "InDetTrackParticles", "Key for track particle container" };

    mutable std::atomic<unsigned int> m_ntot{0};
    mutable std::atomic<unsigned int> m_npass{0};

  }; 
  
}

#endif
