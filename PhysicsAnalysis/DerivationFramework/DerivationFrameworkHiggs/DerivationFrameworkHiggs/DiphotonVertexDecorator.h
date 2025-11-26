/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// DiphotonVertexDecorator.h
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_DiphotonVertexDecorator_H
#define DERIVATIONFRAMEWORK_DiphotonVertexDecorator_H

#include <string>
#include <vector>
#include <algorithm>

// Gaudi & Athena basics
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"

// DerivationFramework includes
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
// xAOD header files
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTracking/VertexContainer.h"

#include "xAODPFlow/FlowElementContainer.h"

#include "PhotonVertexSelection/IPhotonVertexSelectionTool.h"

namespace DerivationFramework {

  /** @class DiphotonVertexDecorator
      @author Bruno Lenzi
      @author Leo Cerda
      @author magdac@cern.ch
  */
  class DiphotonVertexDecorator : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    // Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:

    ///////////////
    ///// TOOLS
    ToolHandle<CP::IPhotonVertexSelectionTool> m_photonVertexSelectionTool{this,  "PhotonVertexSelectionTool", "", ""};

    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexKey{this, "PrimaryVertexName", "PrimaryVertices", "" };
    SG::ReadHandleKey<xAOD::PhotonContainer> m_photonKey{this, "PhotonKey", "Photons", "" };
    SG::WriteHandleKey<xAOD::VertexContainer> m_diphotonVertexKey{this, "DiphotonVertexName", "HggPrimaryVertices", "" };
    SG::ReadHandleKey<xAOD::FlowElementContainer> m_FEContainerHandleKey{this,"PFOContainerName","JetETMissChargedParticleFlowObjects","ReadHandleKey for the PFO container"};
    ///////////////
    ///// SETTINGS

    Gaudi::Property<double> m_minPhotonPt{this, "MinimumPhotonPt",  20.*Gaudi::Units::GeV};
    Gaudi::Property<bool> m_removeCrack{this, "RemoveCrack", true};
    Gaudi::Property<double> m_maxEta{this, "MaxEta", 2.37};
    Gaudi::Property<bool> m_ignoreConv{this, "IgnoreConvPointing", false};
    Gaudi::Property<double> m_tcMatch_dR{this, "TCMatchDeltaR", 0.1};
    Gaudi::Property<double> m_tcMatch_maxRat{this, "TCMatchMaxRat", 1.5};

    bool  PhotonPreselect(const xAOD::Photon *ph) const;
    StatusCode matchFlowElement(const xAOD::Photon* eg,const xAOD::FlowElementContainer *pfoCont) const;
    static inline bool greaterPtFlowElement(const xAOD::FlowElement* part1, const xAOD::FlowElement* part2) {
      if (part1->charge()==0 && part2->charge()!=0) return false;
      if (part1->charge()!=0 && part2->charge()==0) return true;
      return part1->pt()>part2->pt();
    }

  };

}

#endif // DERIVATIONFRAMEWORK_DiphotonVertexDecorator_H
