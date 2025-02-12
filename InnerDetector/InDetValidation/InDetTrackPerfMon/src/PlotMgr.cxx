/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    PlotMgr.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 **/

/// local include(s)
#include "PlotMgr.h"
#include "InDetTrackPerfMon/IPlotsDefinitionSvc.h"

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
        m_anaTag( anaTag ) { }


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::PlotMgr::initialize()
{
  /// intialize PlotBase
  PlotBase::initialize();
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
  /// Loading PlotsDefinitionSvc
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  SmartIF<IPlotsDefinitionSvc> plotsDefSvc(svcLoc->service( "PlotsDefSvc"+m_anaTag ));
  ATH_CHECK( plotsDefSvc.isValid(), {} );

  /// retrieve a copy of the plot definition
  SinglePlotDefinition sDef = plotsDefSvc->definition( identifier );

  /// Check if definition is empty or non-valid 
  if( sDef.isEmpty() or not sDef.isValid() )  return sDef;

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBins( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( not def.xBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.xBinLabelsVec(), 'X' ) );
  }

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doLogLinBinsY() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsY(), def.yLow(), def.yHigh(), 'Y' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBins( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( def.doVarBinsY() ) {
    ATH_CHECK( setVariableBins( pHisto, def.yBinsVec(), 'Y' ) );
  }

  if( not def.xBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.xBinLabelsVec(), 'X' ) );
  }

  if( not def.yBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.yBinLabelsVec(), 'Y' ) );
  }

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doLogLinBinsY() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsY(), def.yLow(), def.yHigh(), 'Y' ) );
  }

  if( def.doLogLinBinsZ() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsZ(), def.zLow(), def.zHigh(), 'Z' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBins( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( def.doVarBinsY() ) {
    ATH_CHECK( setVariableBins( pHisto, def.yBinsVec(), 'Y' ) );
  }

  if( def.doVarBinsZ() ) {
    ATH_CHECK( setVariableBins( pHisto, def.zBinsVec(), 'Z' ) );
  }

  if( not def.xBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.xBinLabelsVec(), 'X' ) );
  }

  if( not def.yBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.yBinLabelsVec(), 'Y' ) );
  }

  if( not def.zBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.zBinLabelsVec(), 'Z' ) );
  }

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBins( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( not def.xBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.xBinLabelsVec(), 'X' ) );
  }

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doLogLinBinsY() ) {
    ATH_CHECK( setLogLinearBins( pHisto, def.nBinsY(), def.yLow(), def.yHigh(), 'Y' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBins( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( def.doVarBinsY() ) {
    ATH_CHECK( setVariableBins( pHisto, def.yBinsVec(), 'Y' ) );
  }

  if( not def.xBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.xBinLabelsVec(), 'X' ) );
  }

  if( not def.yBinLabelsVec().empty() ) {
    ATH_CHECK( setBinLabels( pHisto, def.yBinLabelsVec(), 'Y' ) );
  }

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

  if( def.doLogLinBinsX() ) {
    ATH_CHECK( setLogLinearBinsEff( pHisto, def.nBinsX(), def.xLow(), def.xHigh(), 'X' ) );
  }

  if( def.doLogLinBinsY() and def.nBinsY() != 0 ) {
    ATH_CHECK( setLogLinearBinsEff( pHisto, def.nBinsY(), def.yLow(), def.yHigh(), 'Y' ) );
  }

  if( def.doVarBinsX() ) {
    ATH_CHECK( setVariableBinsEff( pHisto, def.xBinsVec(), 'X' ) );
  }

  if( def.doVarBinsY() and def.nBinsY() != 0 ) {
    ATH_CHECK( setVariableBinsEff( pHisto, def.yBinsVec(), 'Y' ) );
  }

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


/// Get Log-Linear binning vector
std::vector<float> IDTPM::PlotMgr::getLogLinearBins(
  unsigned int nBins, float absMin, float absMax, bool symmetriseAroundZero )
{
  std::vector<float> emptyVec;
  /// some checks to ensure the user is requesting something sensible 
  if( absMin<=0 or absMax<=0 ) {
    ATH_MSG_WARNING( "absMin or absMax argument to getLogLinearBins is out of range" );
    return emptyVec;
  } else if( nBins==0 ) {
    ATH_MSG_WARNING( "nBins argument to getLogLinearBins is zero" );
    return emptyVec;
  } 
  /// reserve the vector space
  unsigned int asymVecSize = nBins + 1; 
  std::vector<float> theBinning( asymVecSize, 0.);
  /// define our starting bin edge and step size in log space
  float logStart = std::log( absMin );
  float logDist = std::log( absMax ) - logStart;
  float logStep = logDist / (float) nBins;
  /// then populate the bin array
  float thisLog{ logStart };
  for( float& thisBin : theBinning ) {
    thisBin = std::exp( thisLog );
    thisLog += logStep;
  }
  if( symmetriseAroundZero ) {
    /// FIXME: currently disabled
    return emptyVec;
  }
  return theBinning;
}
