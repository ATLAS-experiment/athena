#ifndef DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H
#define DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/Vertex.h"
#include <vector>

namespace Trk {
    class ITrackSelectorTool;
}

/** THE reconstruction tool
 */
namespace DerivationFramework {

  class BKllIsoMultiplicityTool : public AthAlgTool, public IAugmentationTool {
    public: 
      BKllIsoMultiplicityTool(const std::string& t, const std::string& n, const IInterface* p);

      StatusCode initialize();
      StatusCode finalize();
      
      virtual StatusCode addBranches() const;
      bool isTrackInVertex(const xAOD::Vertex* theVtx, const xAOD::TrackParticle* thePart) const;
      bool setIsoVar(const xAOD::Vertex* theVtx, std::vector<std::vector<float>> isolationsAllLegsAllCones, std::string isoName ) const;
 
    private:

    std::vector<std::string> m_cones;
	  std::string m_vertexContainerName;
	  std::string m_trackContainerName;
    ToolHandle < Trk::ITrackSelectorTool > m_trkSelector;
    float m_trackPtCut;
    float m_trackEtaCut;
    std::string  m_elContainerKey;
    std::string  m_elTrackContainerKey;
    float  m_elTrackPtCut;
    float  m_elTrackEtaCut;
    std::string  m_elLHCut;
    bool  m_recordTrackMult;
    bool  m_recordElMult;
    bool  m_recordMuMult;
  }; 
}

#endif
