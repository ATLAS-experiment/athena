/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETV0FINDER_V0MAINDECORATOR_H
#define INDETV0FINDER_V0MAINDECORATOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/VertexContainerFwd.h"
#include "TrkVertexAnalysisUtils/V0Tools.h"
#include "GaudiKernel/IPartPropSvc.h"

namespace InDet
{

  class V0MainDecorator:  public AthAlgTool
  {
    public:
     V0MainDecorator(const std::string& t, const std::string& n, const IInterface*  p);
    ~V0MainDecorator();
    StatusCode initialize();
    StatusCode decorateV0(xAOD::VertexContainer *container, const EventContext& ctx) const;
    StatusCode decorateks(xAOD::VertexContainer *container, const EventContext& ctx) const;
    StatusCode decoratela(xAOD::VertexContainer *container, const EventContext& ctx) const;
    StatusCode decoratelb(xAOD::VertexContainer *container, const EventContext& ctx) const;
    private:

    IntegerProperty m_masses{this, "masses", 1};              //!< = 1 if using PDG values, = 2 if user set (1)
    DoubleProperty m_masspi{this, "masspi", ParticleConstants::chargedPionMassInMeV};          //!< pion mass (139.57 MeV)
    DoubleProperty m_massp{this, "massp", ParticleConstants::protonMassInMeV};           //!< proton mass (938.272 MeV)
    DoubleProperty m_masse{this, "masse", ParticleConstants::electronMassInMeV};          //!< electron mass (0.510999 MeV)
    DoubleProperty m_massK0S{this, "massK0s", ParticleConstants::KZeroMassInMeV};       //!< Kshort mass (497.672 MeV)
    DoubleProperty m_massLambda{this, "massLambda", ParticleConstants::lambdaMassInMeV}; //!< Lambda mass (1115.68 MeV)
    ToolHandle<Trk::V0Tools> m_V0Tools {this, "V0Tools", "Trk::V0Tools", "V0 tools to calculate things like Lxy"};
    StatusCode initKey(const std::string&, SG::WriteDecorHandleKey<xAOD::VertexContainer> &decokey) const;
    
    Gaudi::Property<std::string>   m_v0Key
                    { this, "V0ContainerName", "V0Candidates", "V0 container name (same calling alg)" };
    Gaudi::Property<std::string>       m_ksKey { this, "KshortContainerName", "KshortCandidates", "Ks container" };
    Gaudi::Property<std::string>       m_laKey { this, "LambdaContainerName", "LambdaCandidates",
                                                              "Lambda container" };
    Gaudi::Property<std::string>       m_lbKey { this, "LambdabarContainerName", "LambdabarCandidates", 
                                                              "Lambdabar container" };


    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorKsMass 
                    { this, "KsMass_v0", ".Kshort_mass", "Ks mass for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorKsMassErr 
                    { this, "KsMassErr_v0", ".Kshort_massError", "Ks mass error for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorLaMass 
                    { this, "LaMass_v0", ".Lambda_mass", "Lambda mass for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorLaMassErr 
                    { this, "LaMassErr_v0", ".Lambda_massError", "Lambda mass error for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorLbMass 
                    { this, "LbMass_v0", ".Lambdabar_mass", "Lambdabar mass for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorLbMassErr 
                    { this, "LbMassErr_v0", ".Lambdabar_massError", "Lambdabar mass error for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPt_v0 
                    { this, "Pt_v0", ".pT", "Transverse momentum for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPtErr_v0 
                    { this, "PtErr_v0", ".pTError", "Transverse momentum error for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxy_v0 
                    { this, "Rxy_v0", ".Rxy", "Rxy for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxyErr_v0 
                    { this, "RxyErr_v0", ".RxyError", "Rxy error for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPx_v0 
                    { this, "Px_v0", ".px", "Px for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPy_v0 
                    { this, "Py_v0", ".py", "Py for v0" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPz_v0 
                    { this, "Pz_v0", ".pz", "Pz for v0" };

    // Ks decorators
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMass_ks 
                    { this, "Mass_ks", ".mass", "mass for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMassErr_ks 
                    { this, "MassErr_ks", ".massError", "mass error for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPt_ks
                    { this, "Pt_ks", ".pT", "Transverse momentum for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPtErr_ks
                    { this, "PtErr_ks", ".pTError", "Transverse momentum error for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxy_ks 
                    { this, "Rxy_ks", ".Rxy", "Rxy for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxyErr_ks 
                    { this, "RxyErr_ks", ".RxyError", "Rxy error for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPx_ks 
                    { this, "Px_ks", ".px", "Px for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPy_ks
                    { this, "Py_ks", ".py", "Py for Ks" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPz_ks 
                    { this, "Pz_ks", ".pz", "Pz for Ks" };

    // Lambda decorators
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMass_la 
                    { this, "Mass_la", ".mass", "mass for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMassErr_la 
                    { this, "MassErr_la", ".massError", "mass error for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPt_la 
                    { this, "Pt_la", ".pT", "Transverse momentum for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPtErr_la 
                    { this, "PtErr_la", ".pTError", "Transverse momentum error for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxy_la
                    { this, "Rxy_la", ".Rxy", "Rxy for la" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxyErr_la
                    { this, "RxyErr_la", ".RxyError", "Rxy error for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPx_la 
                    { this, "Px_la", ".px", "Px for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPy_la 
                    { this, "Py_la", ".py", "Py for Lambda" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPz_la 
                    { this, "Pz_la", ".pz", "Pz for Lambda" };

    // Lambdabar decorators
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMass_lb 
                    { this, "Mass_lb", ".mass", "mass for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorMassErr_lb 
                    { this, "MassErr_lb", ".massError", "mass error for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPt_lb 
                    { this, "Pt_lb", ".pT", "Transverse momentum for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPtErr_lb 
                    { this, "PtErr_lb", ".pTError", "Transverse momentum error for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxy_lb 
                    { this, "Rxy_lb", ".Rxy", "Rxy for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorRxyErr_lb 
                    { this, "RxyErr_lb", ".RxyError", "Rxy error for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPx_lb
                    { this, "Px_lb", ".px", "Px for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPy_lb 
                    { this, "Py_lb", ".py", "Py for Lambdabar" };
    SG::WriteDecorHandleKey<xAOD::VertexContainer>  m_decorPz_lb 
                    { this, "Pz_lb", ".pz", "Pz for Lambdabar" };

    ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};
  };

}


#endif 
