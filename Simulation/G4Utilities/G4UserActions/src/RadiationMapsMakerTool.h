/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA__RADIATIONMAPSMAKERTOOL_H 
#define G4USERACTIONS_G4UA__RADIATIONMAPSMAKERTOOL_H 

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "RadiationMapsMaker.h"

namespace G4UA
{ 

  /// 	
  /// @class RadiationMapsMakerTool
  /// @brief Tool which manages the RadiationMapsMaker action.
  ///
  /// Create the RadiationMapsMaker  for each worker thread
  ///
  /// @author Sven Menke (based on Andrea Di Simone's FluxRecorderTool)
  ///

class RadiationMapsMakerTool: 
  public UserActionToolBase<RadiationMapsMaker>
  {

  public:
    using UserActionToolBase<RadiationMapsMaker>::UserActionToolBase;

    /// Initialize configurable properties 
    virtual StatusCode initialize() override final;

    /// Finalize and merge results from all threads
    virtual StatusCode finalize() override final;

  protected:

    /// create action for this thread
    virtual std::unique_ptr<RadiationMapsMaker> makeAndFillAction(G4AtlasUserActions&) override final;

  private:
    /// Output Filename for the Radiation Maps
    Gaudi::Property<std::string> m_radMapsFileName{this, "RadMapsFileName", "RadMaps.root"};
    /// Name of the materials to make radiation maps for (take all if empty)
    Gaudi::Property<std::vector<std::string>> m_materials{this, "Materials", {}};
    /// Name of the file to store activated unstable isotopes (don't save if empty)
    Gaudi::Property<std::string> m_activationFileName{this, "ActivationFileName", ""};
    /// If true consider hits with y>0 only -- useful for shafts
    Gaudi::Property<bool> m_posYOnly{this, "PositiveYOnly", false};
    /// map granularities
    /// number of bins in r and z for all 2D maps
    Gaudi::Property<int> m_nBinsr{this, "NBinsR", 120};
    Gaudi::Property<int> m_nBinsz{this, "NBinsZ", 240};
    /// number of bins in logE for energy spectra of neutrons in 2D grids
    Gaudi::Property<int> m_nBinslogEn{this, "NBinsLogEn", 90};
    /// number of bins in logE for energy spectra of other particles in 2D grids
    Gaudi::Property<int> m_nBinslogEo{this, "NBinsLogEo", 45};
    /// number of bins in dphi for dphi x theta dependent energy spectra
    Gaudi::Property<int> m_nBinsdphi{this, "NBinsDPhi", 18};
    /// number of bins in theta for dphi x theta dependent energy spectra
    Gaudi::Property<int> m_nBinstheta{this, "NBinsTheta", 9};
    /// number of bins in r, z and phi for all 3D maps
    Gaudi::Property<int> m_nBinsr3d{this, "NBinsR3D", 30};
    Gaudi::Property<int> m_nBinsz3d{this, "NBinsZ3D", 60};
    Gaudi::Property<int> m_nBinsphi3d{this, "NBinsPhi3D", 32};
    /// number of bins in logTimeCut for time dependent TID and H_T 2D maps
    Gaudi::Property<int> m_nBinslogT{this, "NBinsLogTimeCut", 20};
    /// map ranges
    /// for Zoomed area in 2D and 3D
    Gaudi::Property<double> m_rMinZoom{this, "RMinZoom", 0.};
    Gaudi::Property<double> m_rMaxZoom{this, "RMaxZoom", 480.};
    Gaudi::Property<double> m_zMinZoom{this, "ZMinZoom", 0.};
    Gaudi::Property<double> m_zMaxZoom{this, "ZMaxZoom", 960.};
    /// for Full detector in 2D
    Gaudi::Property<double> m_rMinFull{this, "RMinFull", 0.};
    Gaudi::Property<double> m_rMaxFull{this, "RMaxFull", 1200.};
    Gaudi::Property<double> m_zMinFull{this, "ZMinFull", 0.};
    Gaudi::Property<double> m_zMaxFull{this, "ZMaxFull", 2400.};
    /// for Zoomed area in 3D
    Gaudi::Property<double> m_phiMinZoom{this, "PhiMinZoom", -180.};
    Gaudi::Property<double> m_phiMaxZoom{this, "PhiMaxZoom", 180.};
    /// for logE of neutrons in 2D spectra
    Gaudi::Property<double> m_logEMinn{this, "LogEMinn", -11.};
    Gaudi::Property<double> m_logEMaxn{this, "LogEMaxn", 7.};
    /// for logE of other particles in 2D spectra
    Gaudi::Property<double> m_logEMino{this, "LogEMino", -2.};
    Gaudi::Property<double> m_logEMaxo{this, "LogEMaxo", 7.};
    /// for Theta in 2D spectra
    Gaudi::Property<double> m_thetaMin{this, "ThetaMin", 0.};
    Gaudi::Property<double> m_thetaMax{this, "ThetaMax", 90.};
    /// for logT in time-dependent TID and H_T 2D maps
    Gaudi::Property<double> m_logTMin{this, "LogTMin", -9.};
    Gaudi::Property<double> m_logTMax{this, "LogTMax", 11.};
    /// for elements mass fracion 2D maps
    Gaudi::Property<int> m_elemZMin{this, "ElemZMin", 1};
    Gaudi::Property<int> m_elemZMax{this, "ElemZMax", 92};

    /// Radiation Map ranges and granularities, filled from the properties in initialize()
    RadiationMapsMaker::Config m_config;

  }; // class RadiationMapsMakerTool
 
} // namespace G4UA 
#endif
