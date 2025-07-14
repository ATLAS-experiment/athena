/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVALG_H
#define ACTSTRKFINDING_TRACKTOTRACKPARTICLECNVALG_H 1

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "Gaudi/Property.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/CondHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

#include "ActsEvent/TrackContainer.h"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"

#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Definitions/PdgParticle.hpp"
#include "xAODTracking/TrackingPrimitives.h"

#include "xAODTracking/VertexContainer.h"

// expected layer
#include "Acts/Surfaces/CylinderSurface.hpp"

#include "Acts/Propagator/EigenStepper.hpp"
#include "Acts/Propagator/Propagator.hpp"
#include "Acts/Propagator/Navigator.hpp"

class ActsGeometryContext;

namespace ActsTrk
{

  class TrackToTrackParticleCnvAlg : public AthReentrantAlgorithm
  {

  public:
    TrackToTrackParticleCnvAlg(const std::string &name,
                               ISvcLocator *pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
     // propagator used to get path derivatives
     using Stepper = Acts::EigenStepper<>;
     using Navigator = Acts::Navigator;
     using Propagator = Acts::Propagator<Stepper, Navigator>;

     static std::shared_ptr<Acts::PerigeeSurface> makePerigeeSurface(const InDet::BeamSpotData *beamspotptr);    
     static std::shared_ptr<Acts::PerigeeSurface> makePerigeeSurface(const xAOD::Vertex&);
     Acts::BoundTrackParameters parametersAtPerigee(const EventContext &ctx,
						    const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
						    const Acts::PerigeeSurface &perigee_surface) const;

    ToolHandle<IActsExtrapolationTool> m_extrapolationTool
       {this, "ExtrapolationTool", ""};
   
   PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", "ActsTrackingGeometryTool"};


    SG::ReadHandleKeyArray<ActsTrk::TrackContainer> m_tracksContainerKey
       {this, "ACTSTracksLocation", {},"Track collection (ActsTrk variant)"};
    SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey
       {this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot or empty." };
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey
       {this,"AtlasFieldCacheCondObj","fieldCondObj", "Name of the Magnetic Field conditions object key" };

    SG::ReadCondHandleKeyArray<InDetDD::SiDetectorElementCollection> m_siDetEleCollKey
       {this, "SiDetectorElementCollections", {}, "Pixel and strip element collections to get geometry information about measurements."};
    Gaudi::Property<std::vector<unsigned int> >  m_siDetEleCollToMeasurementType
       {this, "SiDetEleCollToMeasurementType",{}, "One value per si detector collection: Pixel = 1, Strip = 2"};

    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexHandle
       {this, "VertexContainerKey", "", "Name of the Primary Vertex Container"};
    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_trackParticlesOutKey
       {this, "TrackParticlesOutKey","", "Name of the produced track particle collection" };

    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_decorator_actsTracks
      {this, "ActsTrackLink", "actsTrack"};
    
    Gaudi::Property<double>  m_paramExtrapolationParLimit
       {this, "ExtrapolationPathLimit",std::numeric_limits<double>::max(), "PathLimit for extrapolating track parameters." }; // @TODO (unit?mm?)
    Gaudi::Property<bool>  m_firstAndLastParamOnly
       {this, "FirstAndLastParameterOnly",true, "Only convert the first and the last parameter." };
    Gaudi::Property<bool>  m_computeExpectedLayerPattern
       {this, "ComputeExpectedLayerPattern",true, "Compute the expected layer pattern. CPU expensive" };
    Gaudi::Property<bool>  m_expectIfPixelContributes
       {this, "expectIfPixelContribution",true, "Only expect pixel hits if there are pixel hits on track." };

     Gaudi::Property<double>  m_pixelExpectLayerPathLimitInMM
       {this, "PixelExpectLayerPathLimitInMM",1000,
        "PathLimit for extrapolating to get the expected pixel layer pattern in mm." };
    
      Gaudi::Property<std::string> m_perigeeExpression{this, "PerigeeExpression", "BeamLine"};

    enum class expressionStrategy {BeamLine, Vertex};
    expressionStrategy m_expression_strategy {expressionStrategy::BeamLine};
    
     std::unique_ptr<Propagator> m_propagator;

     static std::vector<std::pair<Acts::PdgParticle, xAOD::ParticleHypothesis> > s_actsHypothesisToxAOD ATLAS_THREAD_SAFE;
     static xAOD::ParticleHypothesis convertParticleHypothesis(Acts::PdgParticle abs_pdg_id);
     static void initParticleHypothesisMap();
  };

}

inline xAOD::ParticleHypothesis ActsTrk::TrackToTrackParticleCnvAlg::convertParticleHypothesis(Acts::PdgParticle abs_pdg_id) {
   std::vector<std::pair<Acts::PdgParticle, xAOD::ParticleHypothesis> >::const_iterator
      iter = std::find_if(s_actsHypothesisToxAOD.begin(),
                          s_actsHypothesisToxAOD.end(),
                          [abs_pdg_id](const std::pair<Acts::PdgParticle, xAOD::ParticleHypothesis> &elm) {
                             return abs_pdg_id == elm.first;
                          });
   return  (iter != s_actsHypothesisToxAOD.end() ? iter->second : xAOD::noHypothesis);
}

#endif
