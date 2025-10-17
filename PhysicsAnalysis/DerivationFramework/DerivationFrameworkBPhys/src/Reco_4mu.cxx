/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// Reco_4mu.cxx
///////////////////////////////////////////////////////////////////
// Author: James Catmore <james.catmore@cern.ch>

#include "Reco_4mu.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "BPhysPVTools.h"
#include "xAODBPhys/BPhysHypoHelper.h"
#include "AthContainers/ConstAccessor.h"
#include "TruthUtils/ParticleConstants.h"

namespace DerivationFramework {

  Reco_4mu::Reco_4mu(const std::string& t,
                     const std::string& n,
                     const IInterface* p) :
    base_class(t,n,p)
  {
  }

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

  StatusCode Reco_4mu::initialize()
  {

    ATH_MSG_DEBUG("in initialize()");
    ATH_CHECK(m_pvContainerName.initialize());
    ATH_CHECK(m_refPVContainerName.initialize());
    ATH_CHECK(m_pairName.initialize());
    ATH_CHECK(m_quadName.initialize());
    // retrieve V0 tools
    ATH_CHECK( m_v0Tools.retrieve() );

    // get the JpsiFinder tool
    ATH_CHECK( m_fourMuonTool.retrieve() );

    // get the PrimaryVertexRefitter tool
    ATH_CHECK( m_pvRefitter.retrieve() );

    return StatusCode::SUCCESS;

  }

  // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

  StatusCode Reco_4mu::addBranches(const EventContext & ctx) const
  {
    // Output containers and its auxilliary store
    SG::WriteHandle<xAOD::VertexContainer> pairContainer{m_pairName, ctx};
    ATH_CHECK(pairContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
    SG::WriteHandle<xAOD::VertexContainer> quadContainer{m_quadName, ctx};
    ATH_CHECK(quadContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
    bool acceptEvent = false; // this is a dummy
    //----------------------------------------------------
    // call  finder
    //----------------------------------------------------
    if( !m_fourMuonTool->performSearch(pairContainer.ptr(), quadContainer.ptr(), acceptEvent, ctx).isSuccess() ) {
      ATH_MSG_FATAL("4mu tool (" << m_fourMuonTool << ") failed.");
      return StatusCode::FAILURE;
    }

    //----------------------------------------------------
    // retrieve primary vertices
    //----------------------------------------------------
    SG::ReadHandle<xAOD::VertexContainer> pvContainer{m_pvContainerName, ctx};
    ATH_CHECK(!pvContainer.isValid());
    //----------------------------------------------------
    // Refit primary vertices
    //----------------------------------------------------
    SG::WriteHandle<xAOD::VertexContainer> refPvContainer;
    if (m_refitPV) {
      // refitted PV container does not exist. Create a new one.
      refPvContainer = SG::makeHandle(m_refPVContainerName, ctx);
      ATH_CHECK(refPvContainer.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
    }

    BPhysPVTools helper(&(*m_v0Tools));//Give the helper class the ptr to v0tools to use

    if (m_refitPV) {
      if (quadContainer->size() >0) {
        if (helper.FillCandwithRefittedVertices(quadContainer.ptr(),  pvContainer.cptr(), refPvContainer.ptr(), &(*m_pvRefitter) , m_PV_max, m_DoVertexType).isFailure()){
          ATH_MSG_FATAL("refitting failed - check the vertices you passed");
          return StatusCode::FAILURE;
        }
      }
      if (pairContainer->size()>0) {
        if (helper.FillCandwithRefittedVertices(pairContainer.ptr(),  pvContainer.cptr(), refPvContainer.ptr(), &(*m_pvRefitter) , m_PV_max, m_DoVertexType).isFailure()){
          ATH_MSG_FATAL("refitting failed - check the vertices you passed");
          return StatusCode::FAILURE;
        }
      }
    } else {
      if (quadContainer->size() >0) {
        auto sc = helper.FillCandExistingVertices(quadContainer.ptr(), pvContainer.cptr(), m_DoVertexType);
        sc.ignore();
      }
      if (pairContainer->size() >0) {
        auto sc = helper.FillCandExistingVertices(pairContainer.ptr(), pvContainer.cptr(), m_DoVertexType);
        sc.ignore();
      }
    }

    //----------------------------------------------------
    // Mass-hypothesis dependent quantities
    //----------------------------------------------------

    std::vector<double> muonPairMasses = std::vector<double>(2, ParticleConstants::muonMassInMeV);
    std::vector<double> muonQuadMasses = std::vector<double>(4, ParticleConstants::muonMassInMeV);

    bool doPt   = (m_DoVertexType & 1) != 0;
    bool doA0   = (m_DoVertexType & 2) != 0;
    bool doZ0   = (m_DoVertexType & 4) != 0;
    bool doZ0BA = (m_DoVertexType & 8) != 0;

    // loop over pairs
    ATH_MSG_DEBUG("Indices/masses of pairs follows....");
    for(xAOD::Vertex* pairVtx : *pairContainer) {
      // create BPhysHypoHelper
      xAOD::BPhysHypoHelper pairHelper("PAIR", pairVtx);

      //----------------------------------------------------
      // decorate the vertex
      //----------------------------------------------------
      // a) invariant mass and error
      if( !pairHelper.setMass(muonPairMasses) ) ATH_MSG_WARNING("Decoration pair.setMass failed");

      double massErr = m_v0Tools->invariantMassError(pairHelper.vtx(), muonPairMasses);
      if( !pairHelper.setMassErr(massErr) ) ATH_MSG_WARNING("Decoration pair.setMassErr failed");

      // b) proper decay time and error:
      // retrieve the refitted PV (or the original one, if the PV refitting was turned off)
      if(doPt) ProcessVertex(pairHelper, xAOD::BPhysHelper::PV_MAX_SUM_PT2, muonPairMasses);
      if(doA0) ProcessVertex(pairHelper, xAOD::BPhysHelper::PV_MIN_A0, muonPairMasses);
      if(doZ0) ProcessVertex(pairHelper, xAOD::BPhysHelper::PV_MIN_Z0, muonPairMasses);
      if(doZ0BA) ProcessVertex(pairHelper, xAOD::BPhysHelper::PV_MIN_Z0_BA, muonPairMasses);
      static const SG::ConstAccessor<std::string> CombinationCodeAcc("CombinationCode");
      ATH_MSG_DEBUG(CombinationCodeAcc(*pairVtx) << " : " << pairHelper.mass() << " +/- " << pairHelper.massErr());
    }

    // loop over quadruplets
    ATH_MSG_DEBUG("Indices/masses of quadruplets follows....");
    for (xAOD::Vertex* quadVtx: *quadContainer) {
      // create BPhysHypoHelper
      xAOD::BPhysHypoHelper quadHelper("QUAD", quadVtx);

      //----------------------------------------------------
      // decorate the vertex
      //----------------------------------------------------
      // a) invariant mass and error
      if( !quadHelper.setMass(muonQuadMasses) ) ATH_MSG_WARNING("Decoration quad.setMass failed");

      double massErr = m_v0Tools->invariantMassError(quadHelper.vtx(), muonQuadMasses);
      if( !quadHelper.setMassErr(massErr) ) ATH_MSG_WARNING("Decoration quad.setMassErr failed");

      // b) proper decay time and error:
      // retrieve the refitted PV (or the original one, if the PV refitting was turned off)
      if(doPt) ProcessVertex(quadHelper, xAOD::BPhysHelper::PV_MAX_SUM_PT2, muonQuadMasses);
      if(doA0) ProcessVertex(quadHelper, xAOD::BPhysHelper::PV_MIN_A0, muonQuadMasses);
      if(doZ0) ProcessVertex(quadHelper, xAOD::BPhysHelper::PV_MIN_Z0, muonQuadMasses);
      if(doZ0BA) ProcessVertex(quadHelper, xAOD::BPhysHelper::PV_MIN_Z0_BA, muonQuadMasses);
      static const SG::ConstAccessor<std::string> CombinationCodeAcc("CombinationCode");
      ATH_MSG_DEBUG(CombinationCodeAcc(*quadVtx) << " : " << quadHelper.mass() << " +/- " << quadHelper.massErr());
    }

    return StatusCode::SUCCESS;
  }


  void Reco_4mu::ProcessVertex(xAOD::BPhysHypoHelper &hypoHelper, xAOD::BPhysHelper::pv_type pv_t, std::vector<double> trackMasses) const{

    const xAOD::Vertex* pv = hypoHelper.pv(pv_t);
    if(pv) {
      // decorate the vertex.

      BPHYS_CHECK( hypoHelper.setTau( m_v0Tools->tau(hypoHelper.vtx(), pv,  trackMasses),
                                      pv_t,
                                      xAOD::BPhysHypoHelper::TAU_INV_MASS) );

      BPHYS_CHECK( hypoHelper.setTauErr( m_v0Tools->tauError(hypoHelper.vtx(), pv,  trackMasses),
                                         pv_t,
                                         xAOD::BPhysHypoHelper::TAU_INV_MASS) );

      //enum pv_type {PV_MAX_SUM_PT2, PV_MIN_A0, PV_MIN_Z0, PV_MIN_Z0BA};
    }else{

      const float errConst = -9999999;
      BPHYS_CHECK( hypoHelper.setTau( errConst,
                                      pv_t,
                                      xAOD::BPhysHypoHelper::TAU_INV_MASS) );

      BPHYS_CHECK( hypoHelper.setTauErr( errConst,
                                         pv_t,
                                         xAOD::BPhysHypoHelper::TAU_INV_MASS) );
    }

    return;
  }

}
