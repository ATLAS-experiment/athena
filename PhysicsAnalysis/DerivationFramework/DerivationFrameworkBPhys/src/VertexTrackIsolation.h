/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// VertexTrackIsolation.h,
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_VertexTrackIsolation_H
#define DERIVATIONFRAMEWORK_VertexTrackIsolation_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "RecoToolInterfaces/ITrackIsolationTool.h"
#include <vector>
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

/** THE reconstruction tool
 */
namespace DerivationFramework {

  class VertexTrackIsolation : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      VertexTrackIsolation(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      
      virtual StatusCode addBranches(const EventContext& ctx) const;

      bool isSame(const xAOD::Vertex* theVtx1, const xAOD::Vertex* theVtx2) const;
      bool isContainedIn(const xAOD::Vertex* theVtx, const std::vector<const xAOD::Vertex*> &theColl) const;
      
    private:

	ToolHandle<xAOD::ITrackIsolationTool> m_trackIsoTool;

    std::string m_trackContainerName; // FIXME Use Handles
	std::string m_vertexContainerName; // FIXME Use Handles
	std::vector<unsigned int> m_cones;
	std::vector<std::string> m_passFlags;
	int m_vertexType; 	//Which type of primary vertices should be used? (7 = 0b111 are all at the moment)

        bool m_doIsoPerTrk;
        int m_removeDuplicate;

  bool m_fixElecExclusion;
  bool m_includeV0;
  }; 
}

#endif
