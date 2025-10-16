/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_trackIsolationDecorator_H
#define DERIVATIONFRAMEWORK_trackIsolationDecorator_H

#include<string>
#include<vector>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"

namespace DerivationFramework {
  /** @class trackIsolationDecorator
      @author christopher.young@cern.ch
  */
  class trackIsolationDecorator : public extends<AthAlgTool, IAugmentationTool> {

  public:
    /** Constructor with parameters */
    trackIsolationDecorator( const std::string& t, const std::string& n, const IInterface* p);

    /** Destructor */
    ~trackIsolationDecorator();

    // Athena algtool's Hooks
    StatusCode  initialize();
    StatusCode  finalize();

    virtual StatusCode addBranches() const;

  private:
    Gaudi::Property<std::string> m_containerName{this, "TargetContainer", "InDetTrackParticles", "Container to be decorated"};
    Gaudi::Property<std::string> m_prefix{this, "Prefix", "", ""};

    /// Athena configured tools
    PublicToolHandle<xAOD::ITrackIsolationTool> m_trackIsolationTool{this, "TrackIsolationTool", ""};

    std::vector<xAOD::Iso::IsolationType> m_ptconeTypes;
    Gaudi::Property<std::vector< int >> m_ptcones{this, "ptcones", {}, ""};
    xAOD::TrackCorrection m_trkCorrList;

    std::vector< SG::Decorator< float >* > m_decorators;
  };
}
#endif //
