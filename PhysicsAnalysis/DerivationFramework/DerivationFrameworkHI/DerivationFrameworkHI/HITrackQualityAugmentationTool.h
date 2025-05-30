/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//// HITrackQualityAugmentationTool.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_HITrackQualityAugmentationTool_H
#define DERIVATIONFRAMEWORK_HITrackQualityAugmentationTool_H
 
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

#include <string>

namespace DerivationFramework {
 
  class HITrackQualityAugmentationTool : public extends<AthAlgTool, IAugmentationTool> {
    public:
      enum{
        PP_MIN_BIAS=1<<1, //2
        
        HI_LOOSE=1<<2, // 4
        HI_LOOSE_7SCT_HITS    =1<<5, // 32
        HI_LOOSE_TIGHT_D0_Z0  =1<<7,  // 128 
        HI_LOOSE_TIGHTER_D0_Z0=1<<8,  // 256
        
        HI_TIGHT=1<<3,  //8
        HI_TIGHT_TIGHTER_D0_Z0=1<<4, //16
        HI_TIGHT_LOOSE_D0_Z0  =1<<6, //64
      };

      HITrackQualityAugmentationTool(const std::string& t, const std::string& n, const IInterface* p);
      virtual StatusCode initialize() override;
      virtual StatusCode addBranches() const;

    private:
      unsigned short GetTrackQuality   (const xAOD::TrackParticle* track,float z_vtx           ) const;
      unsigned short GetTrackQualityNew(const xAOD::TrackParticle* track,const xAOD::Vertex* pv) const;

      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticlesName{this, "TrackParticlesName", "InDetTrackParticles", ""};
      SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerName {this, "VertexContainerName", "PrimaryVertices", ""};

      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_decorator{ this, "TrackQuality", "TrackQuality", ""};

      ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelTool_pp {this, "TrackSelectionTool_pp", "", ""};
      ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelTool_hi_loose {this, "TrackSelectionTool_hi_loose", "", ""};
      ToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelTool_hi_tight {this, "TrackSelectionTool_hi_tight", "", ""};
  };
}
 
#endif // DERIVATIONFRAMEWORK_HITrackQualityAugmentationTool_H
