/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTMGR_H
#define INDETTRACKPERFMON_PLOTMGR_H

/**
 * @file    PlotMgr.h
 * @brief   Derived class to give extra capabilities to TrkValHistUtils/PlotBase.h
 *          such as ATH_MSG and an easier booking interface, as well
 *          as access to the PlotsDefinitionSvc
 * @author  Marco Aparo <marco.aparo@cern.ch>, Shaun Roe <shaun.roe@cern.ch>
 * @date    26 April 2023
 **/

/// Athena include(s)
#include "TrkValHistUtils/PlotBase.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaBaseComps/AthMessaging.h"

/// local include(s)
#include "InDetTrackPerfMon/SinglePlotDefinition.h"

/// STL include(s)
#include <string>
#include <vector>


namespace IDTPM {

  class PlotMgr : public PlotBase, public AthMessaging {

  public:

    /// Constructor taking parent node and directory name for plots
    /// pParent = nullptr by default to book plots in top directory
    PlotMgr( const std::string& dirName,
             const std::string& anaTag,
             PlotMgr* pParent = nullptr );

    /// Destructor
    virtual ~PlotMgr() = default;

    /// initialize
    StatusCode initialize();

    /// Retrieve a single histogram definition, given the unique string identifier
    SinglePlotDefinition retrieveDefinition(
        const std::string& identifier,
        const std::string& folderOverride = "",
        const std::string& nameOverride = "" ) const;

    /// --------------------------
    /// --- Book plots methods ---
    /// --------------------------
    /// Helper method to book plots using an identifier string 
    /// @param pHisto: Pointer to the histogram to be booked (assumed to be initialized to nullptr)
    /// @param histoIdentifier: string identifier of the plot (looked up from PlotsDefinitionSvc)
    /// @param nameOverride: Allows to override the histo name 
    /// @param folderOverride: Allows to override the folder of the histo
    template < class P >
    StatusCode retrieveAndBook(
        P*& pHisto,
        const std::string& identifier,
        const std::string& folderOverride = "",
        const std::string& nameOverride = "" )
    {
      const SinglePlotDefinition& def =
          retrieveDefinition( identifier, folderOverride, nameOverride );
      if( def.isEmpty() || ! def.isValid() ) {
        ATH_MSG_WARNING( "Trying to book empty or non-valid plot : " << identifier );
        return StatusCode::RECOVERABLE;
      }
      ATH_CHECK( book( pHisto, def ) );
      return StatusCode::SUCCESS;
    }

    /// Book a TH1 histogram
    StatusCode book( TH1*& pHisto, const SinglePlotDefinition& def );

    /// Book a TH2 histogram
    StatusCode book( TH2*& pHisto, const SinglePlotDefinition& def );

    /// Book a TH3 histogram
    StatusCode book( TH3*& pHisto, const SinglePlotDefinition& def );

    /// Book a TProfile histogram
    StatusCode book( TProfile*& pHisto, const SinglePlotDefinition& def );

    /// Book a TProfile2D histogram
    StatusCode book( TProfile2D*& pHisto, const SinglePlotDefinition& def );

    /// Book a (1D or 2D) TEfficiency histogram
    StatusCode book( TEfficiency*& pHisto, const SinglePlotDefinition& def );

    /// --------------------------
    /// --- Fill plots methods ---
    /// --------------------------
    /// Fill a TH1 histogram
    StatusCode fill(
        TH1* pTh1, float value, float weight=1. ) const;

    /// Fill a TH2 histogram
    StatusCode fill(
        TH2* pTh2, float xval, float yval, float weight=1. ) const;

    /// Fill a TH3 histogram
    StatusCode fill(
        TH3* pTh3, float xval, float yval, float zval, float weight=1. ) const;

    /// Fill a TProfile histogram
    /// weight allows weighted-averaging in the profile
    StatusCode fill(
        TProfile* pTprofile, float xval, float yval, float weight=1. ) const;

    /// Fill a TProfile2D histogram
    /// weight allows weighted-averaging in the profile
    StatusCode fill(
        TProfile2D* pTprofile, float xval, float yval, float zval, float weight=1. ) const;

    /// Fill a (1D) TEfficiency histogram
    StatusCode fill(
        TEfficiency* pTeff, float value, bool accepted, float weight=1. ) const;

    /// Fill a (2D) TEfficiency histogram
    StatusCode fill(
        TEfficiency* pTeff2d, float xvalue, float yvalue, bool accepted, float weight=1. ) const;

  protected:

    /// SetVariableBins
    template < class P >
    StatusCode setVariableBins(
      P*& pHisto, const std::vector<float>& binning, char axis )
    {
      if( binning.empty() ) {
        ATH_MSG_ERROR( "Non-valid variable plot binning : " << pHisto->GetName() );
        return StatusCode::FAILURE;
      }
      if( axis == 'X' ) pHisto->GetXaxis()->Set( binning.size()-1, binning.data() );
      if( axis == 'Y' ) pHisto->GetYaxis()->Set( binning.size()-1, binning.data() );
      if( axis == 'Z' ) pHisto->GetZaxis()->Set( binning.size()-1, binning.data() );
      return StatusCode::SUCCESS;
    }

    /// SetVariableBins (for Efficiencies)
    template < class P >
    StatusCode setVariableBinsEff(
      P*& pHisto, const std::vector<float>& binning, char axis )
    {
      if( binning.empty() ) {
        ATH_MSG_ERROR( "Non-valid variable plot binning : " << pHisto->GetName() );
        return StatusCode::FAILURE;
      }
      std::vector<double> binningD( binning.begin(), binning.end() );
      if( axis == 'X' ) pHisto->SetBins( binningD.size()-1, binningD.data() );
      if( axis == 'Y' ) {
        pHisto->SetBins(
          pHisto->GetTotalHistogram()->GetNbinsX(),
          pHisto->GetTotalHistogram()->GetXaxis()->GetXbins()->GetArray(),
          binningD.size()-1, binningD.data() );
      }
      return StatusCode::SUCCESS;
    }

    /// Set Log-Linear axis
    template < class P >
    StatusCode setLogLinearBins(
      P*& pHisto, unsigned int nBins, float absMin, float absMax, char axis )
    {
      std::vector<float> binning = getLogLinearBins( nBins, absMin, absMax );
      ATH_CHECK( setVariableBins( pHisto, binning, axis ) );
      return StatusCode::SUCCESS;
    }

    /// Set Log-Linear axis (for Efficiencies)
    template < class P >
    StatusCode setLogLinearBinsEff(
      P*& pHisto, unsigned int nBins, float absMin, float absMax, char axis )
    {
      std::vector<float> binning = getLogLinearBins( nBins, absMin, absMax );
      ATH_CHECK( setVariableBinsEff( pHisto, binning, axis ) );
      return StatusCode::SUCCESS;
    }

    /// Get Log-Linear binning vector
    /// inherited from InDetPhysValMonitoring/src/logLinearBinning.h
    std::vector<float> getLogLinearBins(
        unsigned int nBins, float absMin, float absMax,
        bool symmetriseAroundZero = false );

    /// SetBinLabels (for TH* and TProfile* only)
    template < class P >
    StatusCode setBinLabels(
      P*& pHisto, const std::vector< std::string >& binLabels, char axis )
    {
      if( axis == 'X' ) {
        size_t nBinsX = pHisto->GetXaxis()->GetNbins();
        if( nBinsX != binLabels.size() ) {
          ATH_MSG_ERROR( "Mismstch in number of X bin labels for : " << pHisto->GetName() );
          return StatusCode::FAILURE;
        }
        for( size_t bin = 0; bin < nBinsX; bin++ ) {
          pHisto->GetXaxis()->SetBinLabel( bin+1, binLabels[ bin ].c_str() );
        }
      }
      if( axis == 'Y' ) {
        size_t nBinsY = pHisto->GetYaxis()->GetNbins();
        if( nBinsY != binLabels.size() ) {
          ATH_MSG_ERROR( "Mismstch in number of Y bin labels for : " << pHisto->GetName() );
          return StatusCode::FAILURE;
        }
        for( size_t bin = 0; bin < nBinsY; bin++ ) {
          pHisto->GetYaxis()->SetBinLabel( bin+1, binLabels[ bin ].c_str() );
        }
      }
      if( axis == 'Z' ) {
        size_t nBinsZ = pHisto->GetZaxis()->GetNbins();
        if( nBinsZ != binLabels.size() ) {
          ATH_MSG_ERROR( "Mismstch in number of Z bin labels for : " << pHisto->GetName() );
          return StatusCode::FAILURE;
        }
        for( size_t bin = 0; bin < nBinsZ; bin++ ) {
          pHisto->GetZaxis()->SetBinLabel( bin+1, binLabels[ bin ].c_str() );
        }
      }
      return StatusCode::SUCCESS;
    }

    std::string m_anaTag;

  }; // class PlotMgr

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTMGR_H
