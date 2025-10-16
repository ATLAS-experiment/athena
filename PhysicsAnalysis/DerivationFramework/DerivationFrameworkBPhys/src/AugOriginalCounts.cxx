/* 
   Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file   AugOriginalCounts.cxx
 *
 */

#include "AugOriginalCounts.h"
#include <StoreGate/WriteDecorHandle.h>
#include "GaudiKernel/EventContext.h"

using namespace xAOD;
namespace DerivationFramework {
 
  AugOriginalCounts::AugOriginalCounts(const std::string& t,
                                       const std::string& n,
                                       const IInterface* p) :
    base_class(t,n,p),
    m_TrackContainername("InDetTrackParticles"),
    m_TrackContainerLRTname("InDetLargeD0TrackParticles"),
    m_PVContainername("PrimaryVertices")
  {

    declareProperty("TrackContainer", m_TrackContainername);
    declareProperty("TrackLRTContainer", m_TrackContainerLRTname);
    declareProperty("VertexContainer", m_PVContainername);
    declareProperty("AddPVCountsByType", m_addPVCountsByType = false);
    // decorate PVs with track counts and/or sqrt(sum(pt^2))
    // (needed if track collection will be thinned)
    declareProperty("AddNTracksToPVs", m_addNTracksToPVs = false);
    declareProperty("AddSqrtPt2SumToPVs", m_addSqrtPt2SumToPVs = false);
  }

  StatusCode AugOriginalCounts::initialize()
  {
  	 ATH_CHECK(m_TrackContainername.initialize(SG::AllowEmpty));
     ATH_CHECK(m_TrackContainerLRTname.initialize(SG::AllowEmpty));
  	 ATH_CHECK(m_PVContainername.initialize(SG::AllowEmpty));
  	 
     if(!m_PVContainername.empty()){
        std::string pvstring = "EventInfo.OriginalCount_";
        pvstring += m_PVContainername.key();
        m_OrigPVNTracks = std::move(pvstring);
        ATH_CHECK(m_OrigPVNTracks.initialize());
     }
     if ( m_addPVCountsByType ) {
        std::string pv0string = "EventInfo.OriginalCount_type0_"+m_PVContainername.key();
        std::string pv1string = "EventInfo.OriginalCount_type1_"+m_PVContainername.key();
        std::string pv2string = "EventInfo.OriginalCount_type2_"+m_PVContainername.key();
        std::string pv3string = "EventInfo.OriginalCount_type3_"+m_PVContainername.key();
        std::string pvUstring = "EventInfo.OriginalCount_typeUnknown_"+m_PVContainername.key();
        m_OrigNtype0 = std::move(pv0string);
        m_OrigNtype1 = std::move(pv1string);
        m_OrigNtype2 = std::move(pv2string);
        m_OrigNtype3 = std::move(pv3string);
        m_OrigNtypeUnknown = std::move(pvUstring);
        ATH_CHECK(m_OrigNtype0.initialize());
        ATH_CHECK(m_OrigNtype1.initialize());
        ATH_CHECK(m_OrigNtype2.initialize());
        ATH_CHECK(m_OrigNtype3.initialize());
        ATH_CHECK(m_OrigNtypeUnknown.initialize());
     }
     if ( m_addSqrtPt2SumToPVs ) {
     	std::string trackcon  = m_PVContainername.key();
     	trackcon += ".OriginalCount_";
     	trackcon += m_TrackContainername.key();
        m_OrigSqrtPt2Sum = std::move(trackcon);
        ATH_CHECK(m_OrigSqrtPt2Sum.initialize());
     }
     if ( m_addNTracksToPVs ) {
        std::string name = m_PVContainername.key();
        name+= ".OrigNTracks";
        m_d_nPVTracks = std::move(name);
        ATH_CHECK(m_d_nPVTracks.initialize());
     }
     if(!m_TrackContainername.empty()){
        m_OrigNTracksKeys = "EventInfo.OriginalCount_" + m_TrackContainername.key();
        ATH_CHECK(m_OrigNTracksKeys.initialize());
     }
     if(!m_TrackContainerLRTname.empty()){
        m_OrigNTracksLRTKeys = "EventInfo.OriginalCount_" + m_TrackContainerLRTname.key();
        ATH_CHECK(m_OrigNTracksLRTKeys.initialize());
     }
     return StatusCode::SUCCESS;
  }
 
  StatusCode AugOriginalCounts::addBranches(const EventContext& ctx) const
  {

    
    if(!m_PVContainername.empty()){

      SG::WriteDecorHandle<xAOD::EventInfo, int> PV_count(m_OrigPVNTracks, ctx);
      SG::ReadHandle<xAOD::VertexContainer> vertices(m_PVContainername, ctx);
      if(!PV_count.isAvailable()) PV_count(0) = vertices->size();
      
      if ( m_addPVCountsByType ) {
      	SG::WriteDecorHandle<xAOD::EventInfo, int> PV0_count(m_OrigNtype0, ctx);
      	SG::WriteDecorHandle<xAOD::EventInfo, int> PV1_count(m_OrigNtype1, ctx);
      	SG::WriteDecorHandle<xAOD::EventInfo, int> PV2_count(m_OrigNtype2, ctx);
      	SG::WriteDecorHandle<xAOD::EventInfo, int> PV3_count(m_OrigNtype3, ctx);
      	SG::WriteDecorHandle<xAOD::EventInfo, int> PVUnk_count(m_OrigNtypeUnknown, ctx);

        // now count
        constexpr int nvtypes = 5;
        int nvtc[] = {0, 0, 0, 0, 0};
        for (auto vtx : *vertices) {
          VxType::VertexType vt = vtx->vertexType();
          if ( vt >=0 && vt < nvtypes ) {
            nvtc[vt]++; // vertex types 0 - 3
          } else {
            nvtc[nvtypes-1]++; // unknown
          }
        }
        if(!PV0_count.isAvailable()) PV0_count(0) = nvtc[0];
        if(!PV1_count.isAvailable()) PV1_count(0) = nvtc[1];
        if(!PV2_count.isAvailable()) PV2_count(0) = nvtc[2];
        if(!PV3_count.isAvailable()) PV3_count(0) = nvtc[3];
        if(!PVUnk_count.isAvailable()) PVUnk_count(0) = nvtc[4];
      } // m_addPVCountsByType

      // decorate PVs with track counts
      // (needed if track collection will be thinned)
      if ( m_addNTracksToPVs ) {
        SG::WriteDecorHandle<xAOD::VertexContainer, int> d_nPVTracks(m_d_nPVTracks, ctx);
        if(!d_nPVTracks.isAvailable()){
          for (auto vtx : *vertices) {
            d_nPVTracks(*vtx) = (int)vtx->nTrackParticles();
          }
        }
      } // m_addNTracksToPVs
      
      // decorate PVs with sqrt(sum(pt^2)) of tracks
      // (needed if track collection will be thinned)
      if ( m_addSqrtPt2SumToPVs ) {
      	SG::WriteDecorHandle<xAOD::VertexContainer, float> d_pvSqrtPt2Sum(m_OrigSqrtPt2Sum, ctx);
        if(!d_pvSqrtPt2Sum.isAvailable()){
          for (auto vtx : *vertices) {
            float sqrtPt2Sum(0.);
            for (auto tp : vtx->trackParticleLinks()) {
              sqrtPt2Sum += std::sqrt(pow((*tp)->pt(),2));
            }
            d_pvSqrtPt2Sum(*vtx) = sqrtPt2Sum;
          }
        }
      } // m_addSqrtPt2SumToPVs
    }
    
    if(!m_TrackContainername.empty()){
      SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_TrackContainername, ctx);
      SG::WriteDecorHandle<xAOD::EventInfo, int> track_count(m_OrigNTracksKeys, ctx);
      if(!track_count.isAvailable()) track_count(0) = tracks->size();
    }
    if(!m_TrackContainerLRTname.empty()){
      SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_TrackContainerLRTname, ctx);
      SG::WriteDecorHandle<xAOD::EventInfo, int> track_count(m_OrigNTracksLRTKeys, ctx);
      if(!track_count.isAvailable()) track_count(0) = tracks->size();
    }

    return StatusCode::SUCCESS;
  }
}
