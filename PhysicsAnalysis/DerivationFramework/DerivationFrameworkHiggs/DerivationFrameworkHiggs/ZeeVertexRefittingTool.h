/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGVertexRefittingTool.h
// author: ioannis.nomidis@cern.ch
///////////////////////////////////////////////////////////////////

/**
 * refit the primary vertex after removing the tracks of electrons from a Z->ee decay
 * to imitate the primary vertex reconstruction in H->yy events with unconverted photons
 */

#ifndef DERIVATIONFRAMEWORK_ZEEVERTEXREFITTINGTOOL_H
#define DERIVATIONFRAMEWORK_ZEEVERTEXREFITTINGTOOL_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODEgamma/ElectronContainer.h"
#include "GaudiKernel/ToolHandle.h"
#include "JpsiUpsilonTools/PrimaryVertexRefitter.h"
#include "ExpressionEvaluation/ExpressionParser.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

  class ZeeVertexRefittingTool : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode finalize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    Gaudi::Property<std::string> m_expression{this, "ObjectRequirements",  "true"};
    Gaudi::Property<float> m_massCut{this, "LowMassCut", 0.f};

    SG::ReadHandleKey<xAOD::VertexContainer> m_primaryVertexKey{this, "PVContainerName", "PrimaryVertices", "" };
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronKey { this, "ElectronContainerName", "Electrons", "" };
    SG::WriteHandleKey<xAOD::VertexContainer> m_refitpvKey{this, "RefittedPVContainerName", "HggPrimaryVertices", "" };
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey { this, "EventInfoKey", "EventInfo", "" };

    Gaudi::Property<std::vector<unsigned int> > m_MCSamples{this, "MCSamples", {} };

    ToolHandle < Analysis::PrimaryVertexRefitter > m_pvrefitter{this, "PrimaryVertexRefitterTool", "Analysis::PrimaryVertexRefitter"};

    StatusCode makeZeePairs( const xAOD::ElectronContainer *particles, std::vector<std::vector<unsigned int> > &ZeePairs) const;
  };
}

#endif // DERIVATIONFRAMEWORK_ZEEVERTEXREFITTINGTOOL_H
