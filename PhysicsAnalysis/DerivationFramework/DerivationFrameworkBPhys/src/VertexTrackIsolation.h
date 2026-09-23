/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VertexTrackIsolation.h,
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_VertexTrackIsolation_H
#define DERIVATIONFRAMEWORK_VertexTrackIsolation_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include <vector>
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

/** THE reconstruction tool
 */
namespace DerivationFramework {

  class VertexTrackIsolation : public AthReentrantAlgorithm {
    public:
      using AthReentrantAlgorithm::AthReentrantAlgorithm;

      StatusCode initialize();

      virtual StatusCode execute(const EventContext& ctx) const;

      bool isSame(const xAOD::Vertex* theVtx1, const xAOD::Vertex* theVtx2) const;
      bool isContainedIn(const xAOD::Vertex* theVtx, const std::vector<const xAOD::Vertex*> &theColl) const;

    private:

    PublicToolHandle<xAOD::ITrackIsolationTool> m_trackIsoTool{this, "TrackIsoTool", "xAOD::TrackIsolationTool"};

    Gaudi::Property<std::string> m_trackContainerName{this, "TrackContainer", "InDetTrackParticles"}; // FIXME Use Handles
    Gaudi::Property<std::string> m_vertexContainerName{this, "InputVertexContainer", "NONE"}; // FIXME Use Handles
    Gaudi::Property<std::vector<unsigned int>> m_cones{this, "IsolationTypes", {}};
    Gaudi::Property<std::vector<std::string>> m_passFlags{this, "PassFlags", {}};
    Gaudi::Property<int> m_vertexType{this, "DoVertexTypes", 7};       //Which type of primary vertices should be used? (7 = 0b111 are all at the moment)

    Gaudi::Property<bool> m_doIsoPerTrk{this, "DoIsoPerTrk", false,
      "New property to deal with track isolation per track, the default option "
      "(m_doIsoPerTrk=false) preserves the old behavior"};
    Gaudi::Property<int> m_removeDuplicate{this, "RemoveDuplicate", 2, "Used with DoIsoPerTrk"};

    Gaudi::Property<bool> m_fixElecExclusion{this, "FixElecExclusion", false};
    Gaudi::Property<bool> m_includeV0{this, "IncludeV0", false};
  };
}

#endif
