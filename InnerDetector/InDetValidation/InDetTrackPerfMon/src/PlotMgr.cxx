/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    PlotMgr.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 **/

/// local include(s)
#include "PlotMgr.h"

/// Gaudi include(s)
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// STD include(s)
#include <cmath> // for std::isnan


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::PlotMgr::PlotMgr(
    const std::string& dirName,
    const std::string& anaTag,
    PlotMgr* pParent ) :
        PlotBase( pParent, dirName ),
        AthMessaging( "PlotMgr"+anaTag ),
        m_anaTag( anaTag ),
        m_plotsDefSvc( nullptr ) { }


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::PlotMgr::initialize()
{
  /// intialize PlotBase
  PlotBase::initialize();

  /// load plotDefSvc 
  if( not m_plotsDefSvc ) {
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    ATH_CHECK( svcLoc->service( "PlotsDefSvc"+m_anaTag, m_plotsDefSvc ) );
  }

  return StatusCode::SUCCESS;
}


/// --------------------------
/// --- retrieveDefinition ---
/// --------------------------
IDTPM::SinglePlotDefinition IDTPM::PlotMgr::retrieveDefinition(
    const std::string& identifier, 
    const std::string& folderOverride, 
    const std::string& nameOverride ) const
{
  /// Retrieve copy of SinglePlotDefinition
  SinglePlotDefinition sDef = m_plotsDefSvc->definition( identifier );

  /// Check if definition is empty or non-valid 
  if( sDef.isEmpty() or !sDef.isValid() )  return sDef;

  /// Override directory?
  if( not folderOverride.empty() )  sDef.folder( folderOverride );

  /// Override name?
  if( not nameOverride.empty() )  sDef.name( nameOverride );

  return sDef;
}


/// -------------------------------
/// --- Book histograms methods ---
/// -------------------------------
/// Book a TH1 histogram
StatusCode IDTPM::PlotMgr::book(
    TH1*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TH1 plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = Book1D( def.name(), def.titleDigest(),
                   def.nBinsX(), def.xLow(), def.xHigh(),
                   false );
  return StatusCode::SUCCESS;
}


/// Book a TH2 histogram
StatusCode IDTPM::PlotMgr::book(
    TH2*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TH2 plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = Book2D( def.name(), def.titleDigest(),
                   def.nBinsX(), def.xLow(), def.xHigh(),
                   def.nBinsY(), def.yLow(), def.yHigh(),
                   false );
  return StatusCode::SUCCESS;
}


/// Book a TH3 histogram
StatusCode IDTPM::PlotMgr::book(
    TH3*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TH3 plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = Book3D( def.name(), def.titleDigest(),
                   def.nBinsX(), def.xLow(), def.xHigh(),
                   def.nBinsY(), def.yLow(), def.yHigh(),
                   def.nBinsZ(), def.zLow(), def.zHigh(),
                   false );
  return StatusCode::SUCCESS;
}


/// Book a TProfile histogram
StatusCode IDTPM::PlotMgr::book(
    TProfile*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TProfile plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = BookTProfile( def.name(), def.titleDigest(),
                         def.nBinsX(), def.xLow(), def.xHigh(),
                         def.yLow(), def.yHigh(),
                         false );
  return StatusCode::SUCCESS;
}


/// Book a TProfile2D histogram
StatusCode IDTPM::PlotMgr::book(
    TProfile2D*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TProfile2D plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = BookTProfile2D( def.name(), def.titleDigest(),
                           def.nBinsX(), def.xLow(), def.xHigh(),
                           def.nBinsY(), def.yLow(), def.yHigh(),
                           false );
  return StatusCode::SUCCESS;
}


/// Book a (1D or 2D) TEfficiency histogram
StatusCode IDTPM::PlotMgr::book(
    TEfficiency*& pHisto, const IDTPM::SinglePlotDefinition& def )
{
  if( not def.isValid() ) {
    ATH_MSG_ERROR( "Non-valid TEfficiency plot : " << def.identifier() );
    return StatusCode::FAILURE;
  }
  pHisto = ( def.nBinsY() == 0 ) ?
           BookTEfficiency( def.name(), def.titleDigest(),
                            def.nBinsX(), def.xLow(), def.xHigh(),
                            false ) :
           BookTEfficiency( def.name(), def.titleDigest(),
                            def.nBinsX(), def.xLow(), def.xHigh(),
                            def.nBinsY(), def.yLow(), def.yHigh(),
                            false );
  return StatusCode::SUCCESS;
}


/// -------------------------------
/// --- Fill histograms methods ---
/// -------------------------------
/// Fill a TH1 histogram
StatusCode IDTPM::PlotMgr::fill(
    TH1* pTh1, float value, float weight ) const
{
  if( not pTh1 ) {
    ATH_MSG_ERROR( "Trying to fill non-definded TH1" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( value ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for TH1:" << pTh1->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  pTh1->Fill( value, weight );
  return StatusCode::SUCCESS;
}


/// Fill a TH2 histogram
StatusCode IDTPM::PlotMgr::fill(
    TH2* pTh2, float xval, float yval, float weight ) const
{
  if( not pTh2 ) {
    ATH_MSG_ERROR( "Trying to fill non-definded TH2" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( xval ) or std::isnan( yval ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for TH2:" << pTh2->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  pTh2->Fill( xval, yval, weight );
  return StatusCode::SUCCESS;
}


/// Fill a TH3 histogram
StatusCode IDTPM::PlotMgr::fill(
    TH3* pTh3, float xval, float yval, float zval, float weight ) const
{
  if( not pTh3 ) {
    ATH_MSG_ERROR( "Trying to fill non-definded TH3" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( xval ) or std::isnan( yval ) or
      std::isnan( zval ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for TH3:" << pTh3->GetName() );
    return StatusCode::FAILURE;
  }


  /// fill the plot
  pTh3->Fill( xval, yval, zval, weight );
  return StatusCode::SUCCESS;
}


/// Fill a TProfile histogram
StatusCode IDTPM::PlotMgr::fill(
    TProfile* pTprofile, float xval, float yval, float weight ) const
{
  if( not pTprofile ) {
    ATH_MSG_ERROR( "Trying to fill non-definded TProfile" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( xval ) or std::isnan( yval ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for TProfile:" << pTprofile->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  pTprofile->Fill( xval, yval, weight );
  return StatusCode::SUCCESS;
}


/// Fill a TProfile2D histogram
StatusCode IDTPM::PlotMgr::fill(
    TProfile2D* pTprofile, float xval, float yval, float zval, float weight ) const
{
  if( not pTprofile ) {
    ATH_MSG_ERROR( "Trying to fill non-definded TProfile2D" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( xval ) or std::isnan( yval ) or
      std::isnan( zval ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for TProfile2D:" << pTprofile->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  pTprofile->Fill( xval, yval, zval, weight );
  return StatusCode::SUCCESS;
}


/// Fill a (1D) TEfficiency histogram
StatusCode IDTPM::PlotMgr::fill(
    TEfficiency* pTeff, float value, bool accepted, float weight ) const
{
  if( not pTeff ) {
    ATH_MSG_ERROR( "Trying to fill non-definded 1D TEfficiency" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( value ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for 1D TEfficiency:" << pTeff->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  /// To get proper error estimate when possible
  if( weight==1.) pTeff->Fill( accepted, value );
  else            pTeff->FillWeighted( accepted, weight, value );
  return StatusCode::SUCCESS;
}


/// Fill a (2D) TEfficiency histogram
StatusCode IDTPM::PlotMgr::fill(
    TEfficiency* pTeff2d, float xvalue, float yvalue, bool accepted, float weight ) const
{
  if( not pTeff2d ) {
    ATH_MSG_ERROR( "Trying to fill non-definded 2D TEfficiency" );
    return StatusCode::FAILURE;
  }

  if( std::isnan( xvalue ) or std::isnan( yvalue ) or std::isnan( weight ) ) {
    ATH_MSG_ERROR( "Non-valid fill arguments for 2D TEfficiency:" << pTeff2d->GetName() );
    return StatusCode::FAILURE;
  }

  /// fill the plot
  /// To get proper error estimate when possible
  if( weight==1.) pTeff2d->Fill( accepted, xvalue, yvalue );
  else            pTeff2d->FillWeighted( accepted, weight, xvalue, yvalue );
  return StatusCode::SUCCESS;
}
