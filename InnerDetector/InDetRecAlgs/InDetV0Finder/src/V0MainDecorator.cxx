/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "InDetV0Finder/V0MainDecorator.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorModules/GenData.h"
namespace InDet
{
V0MainDecorator::V0MainDecorator(const std::string& t, const std::string& n, const IInterface* p)
  :
  AthAlgTool(t,n,p){}

V0MainDecorator::~V0MainDecorator() = default;

StatusCode V0MainDecorator::initKey(const std::string &containerKey,
                              SG::WriteDecorHandleKey<xAOD::VertexContainer> &decokey) const {
   decokey = containerKey + decokey.key();
   ATH_MSG_DEBUG(" : " << decokey.key());
   ATH_CHECK(decokey.initialize(!containerKey.empty()));
   return StatusCode::SUCCESS;
}

StatusCode V0MainDecorator::initialize(){
   
   ATH_CHECK( m_V0Tools.retrieve() );
   ATH_CHECK(initKey(m_v0Key, m_decorKsMass));
   ATH_CHECK(initKey(m_v0Key, m_decorKsMassErr));
   ATH_CHECK(initKey(m_v0Key, m_decorLaMass));
   ATH_CHECK(initKey(m_v0Key, m_decorLaMassErr));
   ATH_CHECK(initKey(m_v0Key, m_decorLbMass));
   ATH_CHECK(initKey(m_v0Key, m_decorLbMassErr));

   ATH_CHECK(initKey( m_ksKey, m_decorMass_ks) );
   ATH_CHECK(initKey( m_laKey, m_decorMass_la) );
   ATH_CHECK(initKey( m_lbKey, m_decorMass_lb) );
   ATH_CHECK(initKey( m_ksKey, m_decorMassErr_ks) );
   ATH_CHECK(initKey( m_laKey, m_decorMassErr_la) );
   ATH_CHECK(initKey( m_lbKey, m_decorMassErr_lb) );

   ATH_CHECK(initKey( m_v0Key, m_decorPt_v0));
   ATH_CHECK(initKey( m_ksKey, m_decorPt_ks));
   ATH_CHECK(initKey( m_laKey, m_decorPt_la));
   ATH_CHECK(initKey( m_lbKey, m_decorPt_lb));
   ATH_CHECK(initKey( m_v0Key, m_decorPtErr_v0 ));
   ATH_CHECK(initKey( m_ksKey, m_decorPtErr_ks ));
   ATH_CHECK(initKey( m_laKey, m_decorPtErr_la ));
   ATH_CHECK(initKey( m_lbKey, m_decorPtErr_lb ));
   ATH_CHECK(initKey( m_v0Key, m_decorRxy_v0) );
   ATH_CHECK(initKey( m_ksKey, m_decorRxy_ks) );
   ATH_CHECK(initKey( m_laKey, m_decorRxy_la) );
   ATH_CHECK(initKey( m_lbKey, m_decorRxy_lb) );
   ATH_CHECK(initKey( m_v0Key, m_decorRxyErr_v0 ));
   ATH_CHECK(initKey( m_ksKey, m_decorRxyErr_ks ));
   ATH_CHECK(initKey( m_laKey, m_decorRxyErr_la ));
   ATH_CHECK(initKey( m_lbKey, m_decorRxyErr_lb ));
   ATH_CHECK(initKey( m_v0Key, m_decorPx_v0));
   ATH_CHECK(initKey( m_ksKey, m_decorPx_ks));
   ATH_CHECK(initKey( m_laKey, m_decorPx_la));
   ATH_CHECK(initKey( m_lbKey, m_decorPx_lb));
   ATH_CHECK(initKey( m_v0Key, m_decorPy_v0));
   ATH_CHECK(initKey( m_ksKey, m_decorPy_ks));
   ATH_CHECK(initKey( m_laKey, m_decorPy_la));
   ATH_CHECK(initKey( m_lbKey, m_decorPy_lb));
   ATH_CHECK(initKey( m_v0Key, m_decorPz_v0));
   ATH_CHECK(initKey( m_ksKey, m_decorPz_ks));
   ATH_CHECK(initKey( m_laKey, m_decorPz_la));
   ATH_CHECK(initKey( m_lbKey, m_decorPz_lb));



  if (m_masses == 1) {
   auto gendata = std::make_shared<GenData>();
   m_masspi     = gendata->particleMass(MC::PIPLUS).value();
   m_massp      = gendata->particleMass(MC::PROTON).value();
   m_masse      = gendata->particleMass(MC::ELECTRON).value();
   m_massK0S    = gendata->particleMass(MC::K0S).value();
   m_massLambda = gendata->particleMass(MC::LAMBDA0).value();
  }
   return StatusCode::SUCCESS;
}

StatusCode V0MainDecorator::decorateV0(xAOD::VertexContainer *container, const EventContext& ctx) const
{

    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorKsMass(m_decorKsMass, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorLaMass(m_decorLaMass, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorLbMass(m_decorLbMass, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorKsMassErr(m_decorKsMassErr, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorLaMassErr(m_decorLaMassErr, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorLbMassErr(m_decorLbMassErr, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPt_v0(m_decorPt_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPtErr_v0(m_decorPtErr_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxy_v0(m_decorRxy_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxyErr_v0(m_decorRxyErr_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPx_v0(m_decorPx_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPy_v0(m_decorPy_v0, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPz_v0(m_decorPz_v0, ctx );

    for ( auto unconstrV0 : *container )
    {
      double mass_ks = m_V0Tools->invariantMass(unconstrV0,m_masspi,m_masspi);
      double mass_error_ks = m_V0Tools->invariantMassError(unconstrV0,m_masspi,m_masspi);
      double mass_la = m_V0Tools->invariantMass(unconstrV0,m_massp,m_masspi);
      double mass_error_la = m_V0Tools->invariantMassError(unconstrV0,m_massp,m_masspi);
      double mass_lb = m_V0Tools->invariantMass(unconstrV0,m_masspi,m_massp);
      double mass_error_lb = m_V0Tools->invariantMassError(unconstrV0,m_masspi,m_massp);
      double pt = m_V0Tools->pT(unconstrV0);
      double ptError = m_V0Tools->pTError(unconstrV0);
      double rxy = m_V0Tools->rxy(unconstrV0);
      double rxyError = m_V0Tools->rxyError(unconstrV0);
      Amg::Vector3D momentum = m_V0Tools->V0Momentum(unconstrV0);

      decorKsMass( *unconstrV0 ) = mass_ks;
      decorLaMass( *unconstrV0 ) = mass_la;
      decorLbMass( *unconstrV0 ) = mass_lb;
      decorKsMassErr( *unconstrV0 ) = mass_error_ks;
      decorLaMassErr( *unconstrV0 ) = mass_error_la;
      decorLbMassErr( *unconstrV0 ) = mass_error_lb;
      decorPt_v0( *unconstrV0 ) = pt;
      decorPtErr_v0( *unconstrV0 ) =ptError;
      decorRxy_v0( *unconstrV0 ) = rxy;
      decorRxyErr_v0( *unconstrV0 ) =rxyError;
      decorPx_v0( *unconstrV0 ) = momentum.x();
      decorPy_v0( *unconstrV0 ) = momentum.y();
      decorPz_v0( *unconstrV0 ) = momentum.z();
    }
    return StatusCode::SUCCESS;
}

StatusCode V0MainDecorator::decorateks(xAOD::VertexContainer *container, const EventContext& ctx) const
{

    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMass_ks(m_decorMass_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMassErr_ks(m_decorMassErr_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPt_ks(m_decorPt_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPtErr_ks(m_decorPtErr_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxy_ks(m_decorRxy_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxyErr_ks(m_decorRxyErr_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPx_ks(m_decorPx_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPy_ks(m_decorPy_ks, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPz_ks(m_decorPz_ks, ctx );

    for ( auto ksV0 : *container )
    {
      double mass_ks = m_V0Tools->invariantMass(ksV0,m_masspi,m_masspi);
      double mass_error_ks = m_V0Tools->invariantMassError(ksV0,m_masspi,m_masspi);
      double pt = m_V0Tools->pT(ksV0);
      double ptError = m_V0Tools->pTError(ksV0);
      double rxy = m_V0Tools->rxy(ksV0);
      double rxyError = m_V0Tools->rxyError(ksV0);
      Amg::Vector3D momentum = m_V0Tools->V0Momentum(ksV0);

      decorMass_ks( *ksV0 ) = mass_ks;
      decorMassErr_ks( *ksV0 ) = mass_error_ks;
      decorPt_ks( *ksV0 ) = pt;
      decorPtErr_ks( *ksV0 ) = ptError;
      decorRxy_ks( *ksV0 ) = rxy;
      decorRxyErr_ks( *ksV0 ) = rxyError;
      decorPx_ks( *ksV0 ) = momentum.x();
      decorPy_ks( *ksV0 ) = momentum.y();
      decorPz_ks( *ksV0 ) = momentum.z();
    }
    return StatusCode::SUCCESS;
}

StatusCode V0MainDecorator::decoratela(xAOD::VertexContainer *container, const EventContext& ctx) const
{
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMass_la(m_decorMass_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMassErr_la(m_decorMassErr_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPt_la(m_decorPt_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPtErr_la(m_decorPtErr_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxy_la(m_decorRxy_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxyErr_la(m_decorRxyErr_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPx_la(m_decorPx_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPy_la(m_decorPy_la, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPz_la(m_decorPz_la, ctx );

    for ( auto laV0 : *container )
    {
      double mass_la = m_V0Tools->invariantMass(laV0,m_massp,m_masspi);
      double mass_error_la = m_V0Tools->invariantMassError(laV0,m_massp,m_masspi);
      double pt = m_V0Tools->pT(laV0);
      double ptError = m_V0Tools->pTError(laV0);
      double rxy = m_V0Tools->rxy(laV0);
      double rxyError = m_V0Tools->rxyError(laV0);
      Amg::Vector3D momentum = m_V0Tools->V0Momentum(laV0);

      decorMass_la( *laV0 ) = mass_la;
      decorMassErr_la( *laV0 ) = mass_error_la;
      decorPt_la( *laV0 ) = pt;
      decorPtErr_la( *laV0 ) = ptError;
      decorRxy_la( *laV0 ) = rxy;
      decorRxyErr_la( *laV0 ) = rxyError;
      decorPx_la( *laV0 ) = momentum.x();
      decorPy_la( *laV0 ) = momentum.y();
      decorPz_la( *laV0 ) = momentum.z();
    }
   return StatusCode::SUCCESS;
}

StatusCode V0MainDecorator::decoratelb(xAOD::VertexContainer *container, const EventContext& ctx) const
{
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMass_lb(m_decorMass_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorMassErr_lb(m_decorMassErr_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPt_lb(m_decorPt_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPtErr_lb(m_decorPtErr_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxy_lb(m_decorRxy_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorRxyErr_lb(m_decorRxyErr_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPx_lb(m_decorPx_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPy_lb(m_decorPy_lb, ctx );
    SG::WriteDecorHandle<xAOD::VertexContainer, float> decorPz_lb(m_decorPz_lb, ctx );

    for ( auto lbV0 : *container )
    {
      double mass_lb = m_V0Tools->invariantMass(lbV0,m_masspi,m_massp);
      double mass_error_lb = m_V0Tools->invariantMassError(lbV0,m_masspi,m_massp);
      double pt = m_V0Tools->pT(lbV0);
      double ptError = m_V0Tools->pTError(lbV0);
      double rxy = m_V0Tools->rxy(lbV0);
      double rxyError = m_V0Tools->rxyError(lbV0);
      Amg::Vector3D momentum = m_V0Tools->V0Momentum(lbV0);

      decorMass_lb( *lbV0 ) = mass_lb;
      decorMassErr_lb( *lbV0 ) = mass_error_lb;
      decorPt_lb( *lbV0 ) = pt;
      decorPtErr_lb( *lbV0 ) = ptError;
      decorRxy_lb( *lbV0 ) = rxy;
      decorRxyErr_lb( *lbV0 ) = rxyError;
      decorPx_lb( *lbV0 ) = momentum.x();
      decorPy_lb( *lbV0 ) = momentum.y();
      decorPz_lb( *lbV0 ) = momentum.z();
    }
    return StatusCode::SUCCESS;
}

}
