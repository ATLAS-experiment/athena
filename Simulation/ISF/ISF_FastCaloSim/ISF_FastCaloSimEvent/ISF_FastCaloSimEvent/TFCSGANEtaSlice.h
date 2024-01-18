/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TFCSGANEtaSlice.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_TFCSGANETASLICE_H
#define ISF_TFCSGANETASLICE_H 1

// ISF includes
#include <map>
#include <vector>

#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimEvent/TFCSGANXMLParameters.h"
#include "ISF_FastCaloSimEvent/TFCSGANLWTNNHandler.h"
#include "ISF_FastCaloSimEvent/MLogging.h"

#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"

#include <fstream>


class TFCSGANEtaSlice : public ISF_FCS::MLogging {
public:
  TFCSGANEtaSlice();
  TFCSGANEtaSlice(int pid, int etaMin, int etaMax,
                  const TFCSGANXMLParameters &param);
  virtual ~TFCSGANEtaSlice();

  typedef std::map<int, std::vector<double>> FitResultsPerLayer;
  typedef std::map<int, double> ExtrapolatorWeights;
  typedef std::map<std::string, std::map<std::string, double>> NetworkInputs;
  typedef std::map<std::string, double> NetworkOutputs;

  bool LoadGAN();
  void CalculateMeanPointFromDistributionOfR();
  void ExtractExtrapolatorMeansFromInputs();

  NetworkOutputs GetNetworkOutputs(const TFCSTruthState *truth,
                                   const TFCSExtrapolationState *extrapol,
                                   TFCSSimulationState simulstate) const;

  bool IsGanCorrectlyLoaded() const;
  FitResultsPerLayer GetFitResults() const { return m_allFitResults; }
  ExtrapolatorWeights GetExtrapolatorWeights() { return m_extrapolatorWeights; }

  void Print() const;

private:
  int m_pid;
  int m_etaMin;
  int m_etaMax;

  std::string m_inputFolderName;

  FitResultsPerLayer m_allFitResults;
  ExtrapolatorWeights m_extrapolatorWeights;

  TFCSGANLWTNNHandler *m_gan_all = nullptr;
  TFCSGANLWTNNHandler *m_gan_low = nullptr;
  TFCSGANLWTNNHandler *m_gan_high = nullptr;

  bool LoadGANNoRange(std::string inputFileName);
  bool LoadGANFromRange(std::string inputFileName, std::string energyRange);

  TFCSGANXMLParameters m_param;

  ClassDef(TFCSGANEtaSlice, 4) // TFCSGANEtaSlice
};

#endif //> !ISF_TFCSGANETASLICE_H
