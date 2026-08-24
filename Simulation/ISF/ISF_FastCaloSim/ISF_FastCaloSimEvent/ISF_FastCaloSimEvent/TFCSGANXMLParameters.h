/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TFCSGANDetectorRegion.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ISF_TFCSGANXMLPARAMETERS_H
#define ISF_TFCSGANXMLPARAMETERS_H 1

#include <map>
#include <vector>

#include "ISF_FastCaloSimEvent/MLogging.h"
#include "TH2D.h"
;

class TFCSGANXMLParameters : public ISF_FCS::MLogging {
 public:
  typedef std::map<int, TH2D> Binning;

  TFCSGANXMLParameters();
  virtual ~TFCSGANXMLParameters();

  void InitialiseFromXML(int pid, int etaMid,
                         const std::string &FastCaloGANInputFolderName);
  void Print() const;

  const std::vector<int>& GetRelevantLayers() const { return m_relevantlayers; };
  const Binning &GetBinning() const { return m_binning; };
  int GetLatentSpaceSize() const { return m_latentDim; };
  int GetGANVersion() const { return m_ganVersion; };
  bool IsSymmetrisedAlpha() const { return m_symmetrisedAlpha; };
  const std::string& GetInputFolder() const { return m_fastCaloGANInputFolderName; };

  // Called immediately after being read to fix histograms that erroneously
  // have kIsOnHeap set.  See ATLASSIM-7031.
  void fixHists();

 private:
  bool m_symmetrisedAlpha{};
  Binning m_binning;
  std::vector<int> m_relevantlayers;
  int m_ganVersion{};
  int m_latentDim{};
  std::string m_fastCaloGANInputFolderName;

  ClassDef(TFCSGANXMLParameters, 2)  // TFCSGANXMLParameters
};

#endif  //> !ISF_TFCSGANXMLPARAMETERS_H
