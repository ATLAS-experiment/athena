/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#ifndef TAURECTOOLS_TAUTRACKCALOEXTRAPOLATOR_H
#define TAURECTOOLS_TAUTRACKCALOEXTRAPOLATOR_H

#include "tauRecTools/TauRecToolBase.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"

#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"

#include "ActsEvent/ContextUtility.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////

/** 
 * @brief Tau Tracks to Calo extrapolation.
 * 
 * 
 * @author Antonio De Maria
 */

class TauTrackCaloExtrapolator : public TauRecToolBase {
public:
    //-------------------------------------------------------------
    //! Constructor and Destructor
    //-------------------------------------------------------------
    TauTrackCaloExtrapolator(const std::string& name);
    ASG_TOOL_CLASS2(TauTrackCaloExtrapolator, TauRecToolBase, ITauToolBase);
    ~TauTrackCaloExtrapolator();

    //-------------------------------------------------------------
    //! Algorithm functions
    //-------------------------------------------------------------
    virtual StatusCode initialize() override;

    using TauRecToolBase::executeTool;
    virtual StatusCode executeTool(xAOD::TauJet& pTau,
				   const EventContext& ctx,
				   xAOD::TauTrackContainer& tauTrackCon) const override;
    
private:

    //-------------------------------------------------------------
    //! tools
    //-------------------------------------------------------------
    ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtensionTool {this, "ParticleCaloExtensionTool", "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool", "Tool for the extrapolation of charged tracks"};
    SG::ReadHandleKey<CaloExtensionCollection> m_ParticleCacheKey {this,"tauParticleCache", "ParticleCaloExtension", "Name of the particle measurement extrapolation cache for TauTrackCaloExtrapolator"};
    Gaudi::Property<bool> m_extrapOnlyChargedTracks{this,"ExtrapOnlyChargedTracks", false};        

    std::set<CaloSampling::CaloSample> m_EMSamplings;
    std::set<CaloSampling::CaloSample> m_HadSamplings;

    // ACTS Extrapolator
    Gaudi::Property<bool> m_useACTSExtrapolator{this, "useACTSExtrapolator", false};
    ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool {this, "ExtrapolationTool", "ExtrapolationTool", "Tool to run propagation in an ACTS tracking geometry" };
    ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extensionReadDecorKey{this, "ExtensionDecoration", "InDetTrackParticles.caloExtensionLink"};
   
    /** The ContextUtility will give us access to the Acts geometry context */
    ActsTrk::ContextUtility m_ctxProvider{this};

    //Map each identifier volume and layer pair onto a calorimeter sampling layer.
    std::map<std::pair<std::size_t, std::size_t>, std::string> m_caloVolumeLayerMap;

};

#endif // TAURECTOOLS_TAUTRACKCALOEXTRAPOLATOR_H
#endif // XAOD_ANALYSIS
