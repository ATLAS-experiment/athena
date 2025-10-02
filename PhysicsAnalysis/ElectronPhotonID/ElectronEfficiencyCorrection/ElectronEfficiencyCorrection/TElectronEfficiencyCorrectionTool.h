// Dear emacs, this is -*- c++ -*-
/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ELECTRONEFFICIENCYCORRECTION_TELECTRONEFFICIENCYCORRECTIONTOOL_H
#define ELECTRONEFFICIENCYCORRECTION_TELECTRONEFFICIENCYCORRECTIONTOOL_H

/**
  @class TElectronEfficiencyCorrectionTool
  @brief Calculate the egamma scale factors
  Implementation class for the e/gamma Efficiency Scale Factors. This code
  implements the underlying logic of accessing the ROOT files containing the
  recommendations.
  @authors Kristin Lohwasser, Karsten Koeneke, Felix Buehrer
  @updated by Christos Anastopoulos 2017-2022
  */

// STL includes
#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>
// ROOT includes
#include "TH1.h"
#include "TH2.h"
#include "TObjArray.h"
#include "TRandom3.h"
// Core includes
#include "AsgMessaging/AsgMessaging.h"
#include "PATCore/PATCoreEnums.h"

namespace Root {
class TElectronEfficiencyCorrectionTool : public asg::AsgMessaging
{
public:
  //We want to have unique ownership of the Histos
  using HistArray = std::vector<std::unique_ptr<TH1>>;

  // The Result
  struct Result {
    double SF = 0;
    double Total = 0;
    double Stat = 0;
    double UnCorr = 0;
    std::vector<double> Corr{};
    std::vector<double> toys{};
    int histIndex = -1;
    int histBinNum = -1;
  };

  /** Standard constructor */
  TElectronEfficiencyCorrectionTool(
    const char* name = "TElectronEfficiencyCorrectionTool");

  /** Standard destructor */
  ~TElectronEfficiencyCorrectionTool() = default;

  /// This is more of an utility
  /// so the initialize is different wrt
  /// to an athena component
  ///
  ///  Add an input file with the auxiliary measurement
  ///  needed before we call initialize
  inline void addFileName(const std::string& val) {
    m_corrFileNameList.push_back(val);
  }
  /// Running these before the initialize
  /// will setup the booking of toys
  inline void bookToyMCScaleFactors(const int nToyMC) {
    m_doToyMC = true;
    m_nToyMC = nToyMC;
  }
  inline void bookCombToyMCScaleFactors(const int nToyMC) {
    m_doCombToyMC = true;
    m_nToyMC = nToyMC;
  }
  /// Initialize this class
  int initialize();

  /** The main calculate method:
   *  @c dataType PATCore::ParticleDataType::DataType (e.g DATA,FULL etc)
   *  @ runnumber the run number. 1st dimension of the stored measurements
   *  @ cluster_eta the cluster eta. 2nd dimension of the stored measurements
   *  @ electron et. third dimension of the stored measurments
   *  @ result struct filled with
   *  SF, Total uncertainty, Stat uncertainty, Uncorr uncertainty
   *  @ onlyTotal do only the "total" systematic
   *  returns 0 in failure
   *
   *  Toy production is controlled by internal flags
   *  set by the Asg Tool. As toys are special.
   */
  int calculate(const PATCore::ParticleDataType::DataType dataType,
                const unsigned int runnumber, 
                const double cluster_eta,
                const double et, /* in MeV */
                Result& result,
                const bool onlyTotal = false) const;

  /// Helpers to get the binning of the uncertainties
  /// in a std::map (pt, eta)
  int getNbins(std::map<float, std::vector<float>>& ptEta) const;

  /// get number of systematics
  inline int getNSyst() const { return m_nSysMax; }

  /// Set the Random Seed
  inline void setSeed(const unsigned long int seed) { m_seed = seed; }

  /// Check if stat+uncorr has input
  bool uncorrEmpty(const PATCore::ParticleDataType::DataType dataType);

private:
  // Private methods
  /// Load all histograms from the input file(s)
  int getHistograms();

  int setupHistogramsInFolder(const TObjArray& dirNameArray, int lastIdx);

  bool setupUncorrToySyst(std::vector<std::vector<TH1*>>& objs,
                          std::vector<std::vector<TH1*>>& sysObjs,
                          std::vector<std::vector<HistArray>>& uncorrToyMCSyst);

  std::vector<HistArray> buildToyMCTable(
    const std::vector<TH1*>& sf,
    const std::vector<TH1*>& eig,
    const std::vector<TH1*>& stat,
    const std::vector<TH1*>& uncorr,
    const std::vector<std::vector<TH1*>>& corr);

  std::vector<TH2*> buildSingleToyMC(const TH1* sf,
                                     const TH1* stat,
                                     const TH1* uncorr,
                                     const std::vector<TH1*>& corr,
                                     int& randomCounter);

  TH2* buildSingleCombToyMC(const TH1* sf,
                            const TH1* stat,
                            const TH1* uncorr,
                            const std::vector<TH1*>& corr,
                            const int nSys,
                            int& randomCounter);

  void setupTempMapsHelper(TH1* obj,
                           std::vector<std::vector<TH1*>>& objs,
                           std::vector<std::vector<TH1*>>& sysObjs,
                           int& seenSystematics);

  /// Fill and interpret the setup, depending on which histograms are found in
  /// the input file(s)
  int setup(const std::vector<TH1*>& hists,
            std::vector<HistArray>& histList,
            std::vector<unsigned int>& beginRunNumberList,
            std::vector<unsigned int>& endRunNumberList,
            const int runNumBegin,
            const int runNumEnd) const;

  struct HistEdge {
    double etaMax = 0;
    double etaMin = 0;
    double etMax = 0;
    double etMin = 0;
    bool isLowPt = false;
  };

  static void fillHistEdges(const std::vector<HistArray>& sfPerPeriodHist,
                     std::vector<std::vector<HistEdge>>& sfPerPeriodEdges) ;

    /// Flag to control Toys
    bool m_doToyMC;
    bool m_doCombToyMC;
    /// The number of toys
    int m_nToyMC;
    /// The Random seed
    unsigned long int m_seed;
    /// Maximum number of systematics
    int m_nSysMax;
    // The representation of the prepared toy SF tables
    std::vector<std::vector<HistArray>> m_uncorrToyMCSystFull;
    std::vector<std::vector<HistArray>> m_uncorrToyMCSystFast;
    /// The list of file name(s)
    std::vector<std::string> m_corrFileNameList;
    /// List of run numbers where histograms become valid for full simulation
    std::vector<unsigned int> m_begRunNumberList;
    /// List of run numbers where histograms stop being valid for full
    /// simulation
    std::vector<unsigned int> m_endRunNumberList;
    /// List of run numbers where histograms become valid for fast simulation
    std::vector<unsigned int> m_begRunNumberListFastSim;
    /// List of run numbers where histograms stop being valid for fast
    /// simulation
    std::vector<unsigned int> m_endRunNumberListFastSim;
    /// List of histograms for full Geant4 simulation
    std::vector<std::vector<HistArray>> m_histList;
    std::vector<std::vector<HistEdge>> m_histEdges;
    std::vector<std::vector<HistArray>> m_sysList;
    /// List of histograms for fast simulation
    std::vector<std::vector<HistArray>> m_fastHistList;
    std::vector<std::vector<HistEdge>> m_fastHistEdges;
    std::vector<std::vector<HistArray>> m_fastSysList;
    // The Random generator class
    TRandom3 m_Rndm;
  };  // End: class definition
} // End: namespace Root

#endif
