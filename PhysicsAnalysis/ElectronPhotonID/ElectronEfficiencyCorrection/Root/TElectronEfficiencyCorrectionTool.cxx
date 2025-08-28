/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
  @class TElectronEfficiencyCorrectionTool
  @brief Calculate the egamma scale factors in pure ROOT
  @date   July 2012
  @author Karsten Koeneke, Felix Buehrer (initial version)
  @author Kristin Lohwasser added TOY and Run-2 updates
  @author Christos Anastopoulos MT cleanup

  */

// This class header
#include "ElectronEfficiencyCorrection/TElectronEfficiencyCorrectionTool.h"
// STL includes
#include <cmath>
#include <iostream>
#include <memory>
#include <cstring>
// ROOT includes
#include "TClass.h"
#include "TFile.h"
#include "TKey.h"
#include "TMD5.h"
#include "TObjString.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

namespace mapkey {
// The "keys" below are indices for the vectors
//(of size mapkey::end)
// we use to look up the numbers we need
enum key
{
  sf = 0,
  stat = 1,
  eig = 2,
  uncorr = 3,
  sys = 4,
  end = 5
};
const char*
keytostring(int input)
{
  switch (input) {
    case (sf):
      return "sf";
    case (stat):
      return "stat";
    case (eig):
      return "eig";
    case (uncorr):
      return "uncorr";
    case (sys):
      return "sys";
  }
  return "";
}
}

namespace {
char const * const LowPt_string = "LowPt" ;
const std::vector<int> s_keys = { mapkey::sf,
                                  mapkey::stat,
                                  mapkey::eig,
                                  mapkey::uncorr };
}

Root::TElectronEfficiencyCorrectionTool::TElectronEfficiencyCorrectionTool(
  const char* name)
  : asg::AsgMessaging(std::string(name))
  , m_doToyMC(false)
  , m_doCombToyMC(false)
  , m_nToyMC(0)
  , m_seed(0)
  , m_nSysMax(0)
  , m_histList{ mapkey::end }
  , m_fastHistList{ mapkey::end }
  , m_Rndm()
{
}

int
Root::TElectronEfficiencyCorrectionTool::initialize()
{
  // use an int as a StatusCode
  int sc(1);
  // Check if files are present
  if (m_corrFileNameList.empty()) {
    ATH_MSG_ERROR(" No file added!");
    return 0;
  }
  ATH_MSG_DEBUG("Initializing tool with " << m_corrFileNameList.size()
                                          << " configuration file(s)");

  if (m_doToyMC && m_doCombToyMC) {
    ATH_MSG_ERROR(" Both regular and combined toy MCs booked!"
                  << " Only use one!");
    return 0;
  }
  /*
   * initialize the random number generator if toyMC propagation booked
   * Use the 1st 4 bytes of the CheckSum of the reccomendation file as seed
   */
  if (m_doToyMC || m_doCombToyMC) {
    if (m_seed == 0) {
      // Use the name of the correction  for auto-setting of the seed based on
      // the md5-sum of the file
      const std::unique_ptr<char[]> fname(
        gSystem->ExpandPathName(m_corrFileNameList[0].c_str()));
      std::unique_ptr<TMD5> tmd = std::make_unique<TMD5>();
      const char* tmd_as_string = tmd->FileChecksum(fname.get())->AsString();
      m_seed = *(reinterpret_cast<const unsigned long int*>(tmd_as_string));
      ATH_MSG_DEBUG("Seed (automatically) set to " << m_seed);
    } else {
      ATH_MSG_DEBUG("Seed set to " << m_seed);
    }
    m_Rndm = TRandom3(m_seed);
  }
  /*
   * Load the needed histograms
   */
  if (0 == getHistograms()) {
    ATH_MSG_ERROR(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                             << "! Problem when calling getHistograms()");
    return 0;
  }
  const unsigned int nRunNumbersFull = m_begRunNumberList.size();
  const unsigned int nRunNumbersFast = m_begRunNumberListFastSim.size();

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Found " << nRunNumbersFast
                           << " run number ranges for fast sim with a total of "
                           << m_fastHistList[mapkey::sf].size()
                           << " scale factor histograms.");

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Found " << nRunNumbersFull
                           << " run number ranges for full sim with a total of "
                           << m_histList[mapkey::sf].size()
                           << " scale factor histograms.");

  ATH_MSG_DEBUG("Tool succesfully initialized!");

  return sc;
}

bool Root::TElectronEfficiencyCorrectionTool::uncorrEmpty(const PATCore::ParticleDataType::DataType dataType)
{
  const bool isFastSim = dataType == PATCore::ParticleDataType::Fast;
  const auto& currentmap = (isFastSim) ? m_fastHistList : m_histList;

  return  currentmap.at(mapkey::stat).empty() && currentmap.at(mapkey::uncorr).empty();
}

int
Root::TElectronEfficiencyCorrectionTool::calculate(
  const PATCore::ParticleDataType::DataType dataType,
  const unsigned int runnumber,
  const double cluster_eta,
  const double et, /* in MeV */
  Result& result,
  const bool onlyTotal) const
{
  // Set up the non-0 defaults
  result.SF = -999;
  result.Total = 1;
  /*
   * Determine Simulation flavour and find the run period
   */
  const bool isFastSim = dataType == PATCore::ParticleDataType::Fast;
  int runnumberIndex = -1;
  if (isFastSim) {
    for (unsigned int i = 0; i < m_begRunNumberListFastSim.size(); ++i) {
      if (m_begRunNumberListFastSim[i] <= runnumber &&
          runnumber <= m_endRunNumberListFastSim[i]) {
        runnumberIndex = i;
        break;
      }
    }
  } else {
    for (unsigned int i = 0; i < m_begRunNumberList.size(); ++i) {
      if (m_begRunNumberList[i] <= runnumber &&
          runnumber <= m_endRunNumberList[i]) {
        runnumberIndex = i;
        break;
      }
    }
  }
  if (runnumberIndex < 0) {
    return 0;
  }
  /* What we have is a vector<vector<HistArray>> acting as a map:
   * Key/Index is SF,Stat,Eigen,UnCorr
   * The Entry in this index is a vector<HistArray>
   * Each vector<HistArray>  has as many entries as supported Run periods.
   * Each HistArray has 2D histos (could be standard, low-et, or forward
   * electrons) The 2D Histo then has the number we want. What follows is the
   * logic to get to this number.
   */
  const auto& currentmap = (isFastSim) ? m_fastHistList : m_histList;
  const std::vector<HistArray>& sfVector = currentmap.at(mapkey::sf);
  /*
   * See if we can find a  vector for key SF in the map
   * and then if we can get the  corresponding HisArray
   * for the run period.
   */
  if (sfVector.empty() || runnumberIndex >= static_cast<int>(sfVector.size())) {
    return 0;
  }
  /*
   * At this stage we have found the relevant period
   * So we need to locate the right histogram.
   */
  const HistArray& sfObjectArray = sfVector[runnumberIndex];
  const auto& edges = (isFastSim) ? m_fastHistEdges[runnumberIndex]
                                  : m_histEdges[runnumberIndex];
  const int entries = edges.size();
  /*
   * Now the logic of finding the histogram
   * Some parts of the code can be perhaps improved ...
   */
  double xValue(et);
  double yValue(cluster_eta);
  int smallEt(0);
  int etaCov(0);
  int nSF(0);
  bool invalid = false;
  bool changedEt = false;
  int index = -1;

  for (int i = 0; i < entries; ++i) {
    invalid = false;
    const HistEdge& histEdge = edges[i];
    // invalid if we are below minimum et
    if (et < histEdge.etMin) {
      smallEt++;
      invalid = true;
    }
    // invalid if we are above max eta
    if (std::abs(yValue) >= histEdge.etaMax) {
      etaCov++;
      invalid = true;
    }
    // invalid if we are less than minimum eta (forward electrons)
    if (std::abs(yValue) < histEdge.etaMin) {
      etaCov++;
      invalid = true;
    }
    /*
     * Invalid if above max et and is a low Et histogram.
     * If not low Et histogram then change the xValue to the maximum
     * availabe Et of ths histogram. As we assume that the  SF stays the same
     * for very high Et
     */
    if (et > histEdge.etMax) {
      if (histEdge.isLowPt) {
        invalid = true;
      } else {
        xValue = histEdge.etMax - 1000 ; //1000 is 1 GeV
        changedEt = true;
      }
    }
    /*
     * Get the histogram index in the TObjArray
     * Also mark how many times we found something
     * as SF should be unique
     */
    if (!invalid) {
      index = i;
      if (!changedEt) {
        nSF++;
      }
    }
  }
  if (smallEt == entries) {
    return 0;
  }
  if (etaCov == entries) {
    return 0;
  }
  if (nSF > 1) {
    ATH_MSG_WARNING("More than 1 SF found for eta="
                    << yValue << " , et = " << et << " , run number = "
                    << runnumber << ". Please check your input files!");
  }
  if (index < 0) {
    return 0;
  }

  /*
   * Now we have the index of the histogram
   */
  const HistEdge& currentEdge = edges[index];
  /*
   * If SF is only given in Abs(eta) convert eta input to std::abs()
   */
  constexpr double epsilon = 1e-6;
  if (currentEdge.etaMin >= (0 - epsilon)) {
    yValue = std::abs(yValue);
  }

  const TH2* currentHist = static_cast<TH2*>(sfObjectArray[index].get()) ;
  const int globalBinNumber = currentHist->FindFixBin(xValue, yValue);
  const double scaleFactor = currentHist->GetBinContent(globalBinNumber);
  const double scaleFactorErr = currentHist->GetBinError(globalBinNumber);
  /*
   * Write the retrieved values to the output
   */
  result.SF= scaleFactor;
  result.Total = scaleFactorErr;
  result.histIndex = index;
  result.histBinNum = globalBinNumber;
  /*
   * if we only wanted the Total we can exit here
   */
  if (onlyTotal) {
    return 1;
  }
  /*
   * Do the stat error using the available info from the above (SF)
   */
  double statErr = -999;
  const std::vector<HistArray>& statVector = currentmap.at(mapkey::stat);
  if (!statVector.empty()) {
    if (!statVector[runnumberIndex].empty()) {
      statErr = static_cast<TH1*>(statVector[runnumberIndex][index].get())
                  ->GetBinContent(globalBinNumber);
      result.Stat = statErr;
    }
  }
  /*
   * Do the Uncorr  uncertainty
   */
  double val = statErr;
  const std::vector<HistArray>& uncorrVector = currentmap.at(mapkey::uncorr);
  if (!uncorrVector.empty()) {
    if (!uncorrVector.at(runnumberIndex).empty()) {
      const double valAdd =
        static_cast<TH1*>(uncorrVector[runnumberIndex][index].get())
          ->GetBinContent(globalBinNumber);
      val = sqrt(val * val + valAdd * valAdd);
    }
  }
  result.UnCorr = val;
  /*
   * Do the correlated part
   * For the N~16 systematic variations
   * we keep them in a vector of vector of HistArray
   * The first vector index being the runnumber
   * The second the systematic
   * And them the HistArray for high low etc.
   */
  result.Corr.resize(m_nSysMax);
  const std::vector<std::vector<HistArray>>& sysList =
      (isFastSim) ? m_fastSysList : m_sysList;
  if (sysList.size() > static_cast<unsigned int>(index)) {
    if (sysList.at(index).size() > static_cast<unsigned int>(runnumberIndex)) {
      const int sys_entries = sysList.at(index).at(runnumberIndex).size();
      for (int sys = 0; sys < sys_entries; ++sys) {
        double sysVal =
            static_cast<TH2*>(sysList[index][runnumberIndex][sys].get())
                ->GetBinContent(globalBinNumber);
        result.Corr[sys] = sysVal;
      }
    }
  }
  /*
   * Do the toys if requested
   */
  if (m_doToyMC || m_doCombToyMC) {
    result.toys.resize(static_cast<size_t>(m_nToyMC));
    const std::vector<std::vector<HistArray>>& toyMCList =
        ((isFastSim) ? m_uncorrToyMCSystFast : m_uncorrToyMCSystFull);
    if (toyMCList.size() > (unsigned int)runnumberIndex) {
      for (int toy = 0; toy < m_nToyMC; ++toy) {
        if (toyMCList[runnumberIndex][toy].size() >
            static_cast<unsigned int>(index)) {
          result.toys[toy] =
              static_cast<TH2*>(toyMCList[runnumberIndex][toy][index].get())
                  ->GetBinContent(globalBinNumber);
        }
      }
    }
  }
  return 1;
}
/*
 * Build the toyMC tables from inputs
 * Ownership should be tranfered to the map of the tables
 * and the proper delete happens in the dtor
 */
std::vector<TH2*>
Root::TElectronEfficiencyCorrectionTool::buildSingleToyMC(
  const TH2* sf,
  const TH2* stat,
  const TH2* uncorr,
  const std::vector<TH1*>& corr,
  int& randomCounter)
{
  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")! "
                           << "Entering function buildSingleToyMC");
  std::vector<TH2*> tmpHists;
  int nBins = (stat->GetNbinsX() + 2) * (stat->GetNbinsY() + 2);
  tmpHists.reserve(m_nToyMC);
  for (int toy = 0; toy < m_nToyMC; toy++) {
    tmpHists.push_back((TH2*)corr.at(0)->Clone());
  }
  // Loop over all bins
  for (int bin = 0; bin < nBins; bin++) {
    double val = stat->GetBinContent(bin);

    // Add uncorrelated systematics
    if (uncorr != nullptr) {
      double valAdd = uncorr->GetBinContent(bin);
      val = sqrt(val * val + valAdd * valAdd);
    }
    for (int toy = 0; toy < m_nToyMC; toy++) {
      tmpHists.at(toy)->SetBinContent(
        bin, (val * m_Rndm.Gaus(0, 1)) + sf->GetBinContent(bin));
      randomCounter++;
      tmpHists.at(toy)->SetDirectory(nullptr);
    }
  }
  return tmpHists;
}
/*
 * Build the combined toyMC tables from inputs
 * Ownership should be tranfered to the unique_ptr
 * in the lookup tables and the proper delete
 * happens in the dtor
 */
TH2*
Root::TElectronEfficiencyCorrectionTool::buildSingleCombToyMC(
  const TH2* sf,
  const TH2* stat,
  const TH2* uncorr,
  const std::vector<TH1*>& corr,
  const int nSys,
  int& randomCounter)
{

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Entering function buildSingleCombToyMC");

  TH2* tmpHist;
  const int nBins = (stat->GetNbinsX() + 2) * (stat->GetNbinsY() + 2);
  tmpHist = (TH2*)corr.at(0)->Clone();
  // Create random numbers for the corr. uncertainties
  std::vector<double> rnd(nSys, 0);
  for (int s = 0; s < nSys; ++s) {
    rnd[s] = m_Rndm.Gaus(0, 1);
    randomCounter++;
  }
  // Loop over all bins
  for (int bin = 0; bin < nBins; ++bin) {
    double val = stat->GetBinContent(bin);

    // Add uncorrelated systematics
    if (uncorr != nullptr) {
      double valAdd = uncorr->GetBinContent(bin);
      val = sqrt(val * val + valAdd * valAdd);
    }
    val = val * m_Rndm.Gaus(0, 1);
    randomCounter++;
    // Add larger correlated systematics
    for (int s = 0; s < nSys; ++s) {
      if (corr.at(s) != nullptr) {
        val += static_cast<TH2*>(corr.at(s))->GetBinContent(bin) * rnd[s];
      }
    }
    tmpHist->SetBinContent(bin, val + sf->GetBinContent(bin));
  }
  tmpHist->SetDirectory(nullptr);
  return tmpHist;
}
/*
 * Build the toyMC tables from inputs
 */
std::vector<Root::TElectronEfficiencyCorrectionTool::HistArray>
Root::TElectronEfficiencyCorrectionTool::buildToyMCTable(
  const std::vector<TH1*>& sf,
  const std::vector<TH1*>& eig,
  const std::vector<TH1*>& stat,
  const std::vector<TH1*>& uncorr,
  const std::vector<std::vector<TH1*>>& corr)
{

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Entering function buildToyMCTable");

  int nSys{};
  int randomCounter(0);
  std::vector<HistArray> tmpVec;
  const int stat_entries = stat.size();
  if (m_doCombToyMC) {
    for (int toyMC = 0; toyMC < m_nToyMC; toyMC++) {
      HistArray tmpArray;
      for (int i = 0; i < stat_entries; ++i) {
        if (!eig.empty() && !uncorr.empty()) {
          nSys = (static_cast<TH1*>(eig.at(i)))->GetNbinsX() - 1;
          tmpArray.emplace_back(buildSingleCombToyMC(static_cast<TH2*>(sf.at(i)),
                                                     static_cast<TH2*>(stat.at(i)),
                                                     static_cast<TH2*>(uncorr.at(i)),
                                                     corr.at(i),
                                                     nSys,
                                                     randomCounter));
        } else {
          tmpArray.emplace_back(buildSingleCombToyMC(static_cast<TH2*>(sf.at(i)),
                                                     static_cast<TH2*>(stat.at(i)),
                                                     nullptr,
                                                     corr.at(i),
                                                     nSys,
                                                     randomCounter));
        }
      }
      tmpVec.emplace_back(std::move(tmpArray));
    }
  } else {
    std::vector<std::vector<TH2*>> tmpVec2;
    for (int i = 0; i < stat_entries; ++i) {
      nSys = (static_cast<TH1*>(eig.at(i)))->GetNbinsX() - 1;
      tmpVec2.push_back(buildSingleToyMC(static_cast<TH2*>(sf.at(i)),
                                         static_cast<TH2*>(stat.at(i)),
                                         static_cast<TH2*>(uncorr.at(i)),
                                         corr.at(i),
                                         randomCounter));
    }
    for (int toy = 0; toy < m_nToyMC; toy++) {
      HistArray tmpArray;
      for (auto& i : tmpVec2) {
        tmpArray.emplace_back(i.at(toy));
      }
      tmpVec.emplace_back(std::move(tmpArray));
    }
  }
  return tmpVec;
}
/*
 * Helper function to retrieve number of uncorrelated bins
 */
int
Root::TElectronEfficiencyCorrectionTool::getNbins(
  std::map<float, std::vector<float>>& pt_eta1) const
{
  // Get sf histograms
  const std::vector<HistArray>& tmpVec = (!m_histList[mapkey::sf].empty())
                                             ? m_histList[mapkey::sf]
                                             : m_fastHistList[mapkey::sf];

  int nbinsTotal = 0;
  pt_eta1.clear();
  std::vector<float> eta1;
  eta1.clear();

  // Loop over the different Run range (one TObjeArray for each)
  for (const auto& ikey : tmpVec) {
    // Loop over the histograms for a given run numbers
    for (const auto& entries : ikey) {
      eta1.clear();
      // Get number of bins
      TH2* h_tmp = ((TH2*)entries.get());
      int nbinsX = h_tmp->GetNbinsX();
      int nbinsY = h_tmp->GetNbinsY();
      // fill in the eta pushing back
      for (int biny = 1; biny <= nbinsY; ++biny) {
        eta1.push_back(h_tmp->GetYaxis()->GetBinLowEdge(biny));
      }
      // associate each pt (bin) with the corresponding/available eta ones
      for (int binx = 1; binx <= nbinsX; ++binx) {
        pt_eta1[h_tmp->GetXaxis()->GetBinLowEdge(binx)] = eta1;
      }
    }
  }
  for (auto& i : pt_eta1) {
    nbinsTotal += i.second.size();
  }
  return nbinsTotal;
}
/*
 * Get the  histograms from the input files
 */
int
Root::TElectronEfficiencyCorrectionTool::getHistograms()
{

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Entering function getHistograms");
  // Cache the current directory in the root file
  TDirectory* origDir = gDirectory;
  /*
   * Get the name of the first input ROOT file and
   * interpret from that what we have:
   * efficiency vs. efficiencySF; offline vs. trigger; medium, loose,...
   */
  if (!m_corrFileNameList.empty()) {
    TString firstFileNameAndPath = m_corrFileNameList[0].c_str();
    std::unique_ptr<TObjArray> myStringList(firstFileNameAndPath.Tokenize("/"));
    int lastIdx = myStringList->GetLast();
    TString fileName = ((TObjString*)myStringList->At(lastIdx))->GetString();
    std::unique_ptr<TObjArray> myFileNameTokensList(fileName.Tokenize("."));

    if (myFileNameTokensList->GetLast() < 3) {
      ATH_MSG_ERROR("input file name has wrong format!");
      return 0;
    }
  }
  /*
   * Get all ROOT files and histograms
   */
  for (auto& ifile : m_corrFileNameList) {
    // Load the ROOT file
    const std::unique_ptr<char[]> fname(gSystem->ExpandPathName(ifile.c_str()));
    std::unique_ptr<TFile> rootFile(TFile::Open(fname.get(), "READ"));
    if (!rootFile) {
      ATH_MSG_ERROR("No ROOT file found here: " << ifile);
      return 0;
    }
    // Loop over all directories inside the root file (correspond to the run
    // number ranges
    TIter nextdir(rootFile->GetListOfKeys());
    TKey* dir = nullptr;
    TObject* obj = nullptr;
    while ((dir = (TKey*)nextdir())) {
      obj = dir->ReadObj();
      if (obj->IsA()->InheritsFrom("TDirectory")) {
        // splits string by delimiter --> e.g RunNumber1_RunNumber2
        std::unique_ptr<TObjArray> dirNameArray(
          TString(obj->GetName()).Tokenize("_"));
        // returns index of last string --> if one, the directory name does not
        // contain any run numbers
        int lastIdx = dirNameArray->GetLast();
        if (lastIdx != 1) {
          ATH_MSG_ERROR(
            "The folder name seems to have the wrong format! Directory name:"
            << obj->GetName());
          return 0;
        }
        rootFile->cd(obj->GetName());
        if (0 == this->setupHistogramsInFolder(*dirNameArray, lastIdx)) {
          ATH_MSG_ERROR("Unable to setup the histograms in directory "
                        << dir->GetName() << "in file " << ifile);
          return 0;
        }
      } else {
        ATH_MSG_ERROR("Wrong file content! Expected only Directories "
                      << gDirectory->cd());
        return 0;
      }
      // Return to the original ROOT directory
      gDirectory = origDir;
    } // End: directory loop
  }   // End: file loop
  return 1;
}
/*
 * Get the input histograms from a given folder/run number range
 */
int
Root::TElectronEfficiencyCorrectionTool::setupHistogramsInFolder(
  const TObjArray& dirNameArray,
  int lastIdx)
{

  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Entering funtion setupHistogramsInFolder");

  int runNumBegin(-1);
  TString myBegRunNumString =
    ((TObjString*)dirNameArray.At(lastIdx - 1))->GetString();
  if (myBegRunNumString.IsDigit()) {
    runNumBegin = myBegRunNumString.Atoi();
  }
  int runNumEnd(-1);
  TString myEndRunNumString =
    ((TObjString*)dirNameArray.At(lastIdx))->GetString();
  if (myEndRunNumString.IsDigit()) {
    runNumEnd = myEndRunNumString.Atoi();
  }
  if (runNumBegin < 0 || runNumEnd < 0 || runNumEnd < runNumBegin) {
    ATH_MSG_ERROR(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                             << "Could NOT interpret the run number range: "
                             << runNumBegin << " - " << runNumEnd);
    return 0;
  }
  /// setup obj arrays at specific indices
  //--> e.g. "sf" index 0 ,
  std::vector<std::vector<TH1*>> objsFull(mapkey::end);
  std::vector<std::vector<TH1*>> objsFast(mapkey::end);
  // Vector to hold the N~16 systematic variations
  std::vector<std::vector<TH1*>> sysObjsFull;
  std::vector<std::vector<TH1*>> sysObjsFast;
  TIter nextkey(gDirectory->GetListOfKeys());
  TKey* key = nullptr;
  TObject* obj = nullptr;
  int seenSystematics = 0;
  // Loop of the keys
  while ((key = (TKey*)nextkey())) {
    obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom("TH1")) {
      // The histogram containing the scale factors need to end with _sf and
      // need to contain either the string "FullSim" or "AtlFast"!
      if (std::strstr(obj->GetName(), "FullSim") != nullptr) {
        setupTempMapsHelper(
          static_cast<TH1*>(obj), objsFull, sysObjsFull, seenSystematics);
      } else if (std::strstr(obj->GetName(), "AtlFast") != nullptr) {
        setupTempMapsHelper(
          static_cast<TH1*>(obj), objsFast, sysObjsFast, seenSystematics);
      } else {
        ATH_MSG_ERROR("Could NOT interpret if the histogram: "
                      << obj->GetName() << " is full or fast simulation!");
        return 0;
      }
    }
  }
  ATH_MSG_DEBUG(" (file: " << __FILE__ << ", line: " << __LINE__ << ")\n"
                           << "Setting up histograms for Run range  "
                           << runNumEnd);
  // Copy from the temporaries to the actual member variables
  // via the setup function
  for (int key : s_keys) {
    if (!objsFull.at(key).empty()) {
      if (0 == setup(objsFull.at(key),
                     m_histList[key],
                     m_begRunNumberList,
                     m_endRunNumberList,
                     runNumBegin,
                     runNumEnd)) {
        ATH_MSG_ERROR("! Could NOT setup histogram " << key
                                                     << " for full sim!");
        return 0;
      }
    }
    if (!objsFast.at(key).empty()) {
      if (0 == setup(objsFast.at(key),
                     m_fastHistList[key],
                     m_begRunNumberListFastSim,
                     m_endRunNumberListFastSim,
                     runNumBegin,
                     runNumEnd)) {
        ATH_MSG_ERROR("! Could NOT setup histogram " << key << " for fast sim");
        return 0;
      }
    }
  }
  m_fastSysList.resize(sysObjsFast.size());
  for (unsigned int sys = 0; sys < sysObjsFast.size(); sys++) {
    if (0 == setup(sysObjsFast.at(sys),
                   m_fastSysList[sys],
                   m_begRunNumberListFastSim,
                   m_endRunNumberListFastSim,
                   runNumBegin,
                   runNumEnd)) {
      ATH_MSG_ERROR("! Could NOT setup systematic histograms for fast sim");
      return 0;
    }
  }
  m_sysList.resize(sysObjsFull.size());
  for (unsigned int sys = 0; sys < sysObjsFull.size(); sys++) {
    if (0 == setup(sysObjsFull.at(sys),
                   m_sysList[sys],
                   m_begRunNumberList,
                   m_endRunNumberList,
                   runNumBegin,
                   runNumEnd)) {
      ATH_MSG_ERROR("! Could NOT setup systematic histograms for fast sim");
      return 0;
    }
  }
  //Histogram edges
  fillHistEdges(m_histList.at(mapkey::sf), m_histEdges);
  fillHistEdges(m_fastHistList.at(mapkey::sf), m_fastHistEdges);

  // Toys
  if (m_doToyMC || m_doCombToyMC) {
    bool fullToysBooked =
      setupUncorrToySyst(objsFull, sysObjsFull, m_uncorrToyMCSystFull);
    bool fastToysBooked =
      setupUncorrToySyst(objsFast, sysObjsFast, m_uncorrToyMCSystFast);
    if (fullToysBooked || fastToysBooked) {
      if (m_doToyMC) {
        ATH_MSG_DEBUG("Created tables for " << m_nToyMC
                                            << " ToyMC systematics ");
      }
      if (m_doCombToyMC) {
        ATH_MSG_DEBUG("Created tables for " << m_nToyMC
                                            << " combined ToyMC systematics ");
      }
    }
  }
  return 1;
}
/*
 * Helper for Setting up the temporary/intermediate maps
 * from the histos
 */
void
Root::TElectronEfficiencyCorrectionTool::setupTempMapsHelper(
  TH1* obj,
  std::vector<std::vector<TH1*>>& objs,
  std::vector<std::vector<TH1*>>& sysObjs,
  int& seenSystematics)
{
  // Add all except the correlated
  for (int key : s_keys) {
    if (TString(obj->GetName())
          .EndsWith("_" + TString(mapkey::keytostring(key)))) {
      objs.at(key).emplace_back(obj);
    }
  }

  const TString tmpName(obj->GetName());
  // Special treatment , this is only for photons
  if (tmpName.EndsWith("_sys")) {
    objs.at(mapkey::sys).emplace_back(obj);
    std::vector<TH1*> tmpArray;
    // clone
    tmpArray.emplace_back(static_cast<TH1*>(obj->Clone()));
    sysObjs.emplace_back(tmpArray);
    seenSystematics++;
  }
  // See if we are dealing with correlated
  if (tmpName.Contains("_corr")) {
    /*
     * corr0 in the name triggers a few things
     * We assume that 0 is the 1st
     * histogram in a series of corr(i) that
     * we see for each of the vector entries that
     * can be one for LowPt,Standard,Forward etc
     */
    if (tmpName.EndsWith("corr0")) {
      // 1st create a TObjectArray
      std::vector<TH1*> tmpArray;
      // Register it to the vector
      sysObjs.emplace_back(tmpArray);
      // Reset the counter here
      seenSystematics = 0;
    }
    /*
     * Now we can add to the TObjeArray
     * This can be Low Pt or high Pt
     */
    sysObjs.back().emplace_back(obj);
    //Increase the counter
    seenSystematics++;
  }

  if (seenSystematics > m_nSysMax) {
    m_nSysMax = seenSystematics;
  }
}
/*
 * Helper for Setting up the uncorrelated syst for the toys
 */
bool
Root::TElectronEfficiencyCorrectionTool::setupUncorrToySyst(
  std::vector<std::vector<TH1*>>& objs,
  std::vector<std::vector<TH1*>>& sysObjs,
  std::vector<std::vector<HistArray>>& uncorrToyMCSyst)
{
  bool toysBooked = false;
  if (!m_histList[mapkey::sf].empty()) {
    if (objs.at(mapkey::eig).empty() || objs.at(mapkey::stat).empty() ||
        objs.at(mapkey::uncorr).empty()) {

      if (objs.at(mapkey::stat).size() > 1 || objs.at(mapkey::sys).size() > 1) {

        uncorrToyMCSyst.push_back(buildToyMCTable(
          objs.at(mapkey::sf), {}, objs.at(mapkey::stat), {}, sysObjs));
        toysBooked = true;
      }
    } else {
      uncorrToyMCSyst.push_back(buildToyMCTable(objs.at(mapkey::sf),
                                                objs.at(mapkey::eig),
                                                objs.at(mapkey::stat),
                                                objs.at(mapkey::uncorr),
                                                sysObjs));
      toysBooked = true;
    }
  }
  return toysBooked;
}
/*
 * Fill and interpret the setup, depending
 * on which histograms are found in the input file(s)
 */
int
Root::TElectronEfficiencyCorrectionTool::setup(
  const std::vector<TH1*>& hists,
  std::vector<HistArray>& histList,
  std::vector<unsigned int>& beginRunNumberList,
  std::vector<unsigned int>& endRunNumberList,
  const int runNumBegin,
  const int runNumEnd) const
{
  if (hists.empty()) {
    ATH_MSG_ERROR("! Could NOT find histogram with name *_sf in folder");
    return 0;
  }
  TH1* tmpHist(nullptr);
  HistArray tmpArray;
  for (const auto& hist : hists) {
    tmpHist = static_cast<TH1*>(hist);
    tmpHist->SetDirectory(nullptr);
    tmpArray.emplace_back(tmpHist);
  }
  histList.emplace_back(std::move(tmpArray));
  // Now, we have all the needed info. Fill the vectors accordingly
  if (!beginRunNumberList.empty()) {
    if (runNumBegin != (int)beginRunNumberList.back()) {
      beginRunNumberList.push_back(runNumBegin);
    }
  } else {
    beginRunNumberList.push_back(runNumBegin);
  }
  if (!endRunNumberList.empty()) {
    if (runNumEnd != (int)endRunNumberList.back()) {
      endRunNumberList.push_back(runNumEnd);
    }
  } else {
    endRunNumberList.push_back(runNumEnd);
  }
  return 1;
}


void Root::TElectronEfficiencyCorrectionTool::fillHistEdges(
    const std::vector<HistArray>& sfPerPeriodHist,
    std::vector<std::vector<HistEdge>>& sfPerPeriodEdges) {

  for (const auto& vec : sfPerPeriodHist) {
    const size_t vecSize = vec.size();
    std::vector<HistEdge> periodVec;
    periodVec.reserve(vecSize);
    for (size_t i = 0; i < vecSize; ++i) {
      const auto* tmpHist = static_cast<TH2*>(vec[i].get());
      const auto* const xAxis = tmpHist->GetXaxis();
      const auto* yAxis = tmpHist->GetYaxis();
      HistEdge histEdge;
      histEdge.etaMax = yAxis->GetXmax();
      histEdge.etaMin = yAxis->GetXmin();
      histEdge.etMax = xAxis->GetXmax();
      histEdge.etMin = xAxis->GetXmin();
      histEdge.isLowPt =
          (std::strstr(tmpHist->GetName(), LowPt_string) != nullptr);

      periodVec.emplace_back(histEdge);
    }
    sfPerPeriodEdges.emplace_back(std::move(periodVec));
  }
}

