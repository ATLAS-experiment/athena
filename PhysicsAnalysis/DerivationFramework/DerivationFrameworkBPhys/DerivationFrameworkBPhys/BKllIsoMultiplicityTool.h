#ifndef DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H
#define DERIVATIONFRAMEWORK_BKllIsoMultiplicityTool_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTracking/Vertex.h"
#include "xAODBPhys/BPhysHelper.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "DerivationFrameworkBPhys/BPhysVertexTrackBase.h"
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

    std::string m_name;
    std::vector<std::string> m_cones;
    bool m_onlyInVertex;
    std::vector<std::string> m_vertexPassFlags;
	  std::string m_vertexContainerName;
	  std::string m_trackContainerName;
    ToolHandle < Trk::ITrackSelectorTool > m_trkSelector;
    std::vector<std::string> m_trkSelectionCuts;
    float m_trkPVAssociationChi2Cut;
    float m_trackPtCut;
    float m_trackEtaCut;
    std::string  m_elContainerKey;
    std::string  m_elTrackContainerKey;
    std::vector<std::string> m_elTrkSelectionCuts;
    float  m_elTrackPtCut;
    float  m_elTrackEtaCut;
    std::string  m_elLHCut;
    std::string  m_muContainerKey;
    std::string m_muTrackContainerKey;
    std::vector<std::string> m_muTrkSelectionCuts;
    float m_muTrackPtCut;
    float m_muTrackEtaCut;
    int m_muQualityCut; 
    ToolHandle<CP::IMuonSelectionTool> m_muSelectionTool;
    bool  m_recordTrackMult;
    bool  m_recordElMult;
    bool  m_recordMuMult;
    
    std::vector< InDet::InDetTrackSelectionTool* > m_addTrkSelTools;
    std::vector< InDet::InDetTrackSelectionTool* > m_addElTrkSelTools;
    std::vector< InDet::InDetTrackSelectionTool* > m_addMuTrkSelTools;

  }; 
}

#endif
