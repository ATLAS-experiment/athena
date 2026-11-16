/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBNOISEWRITE_H
#define TBREC_TBNOISEWRITE_H

// 
// class TBNoiseWrite 
// Write ROOT Tree with noise in randomly triggered events
//
#include "AthenaBaseComps/AthAlgorithm.h"

class TFile;
class TTree;
class CaloCell_ID;
class IToolSvc;
class LArDigitContainer;

class TBNoiseWrite: public AthAlgorithm {
 public:    
  
  TBNoiseWrite(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBNoiseWrite();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private: 
  void clear();
  /** Get Xcryo and Ytable from a text file */
  StatusCode getXcryoYtable(float &x, float &y, float &eBeam);
  
  int m_nEvent{0};                  // counter
  int m_nEventRandomTrigger{0};     // counter
  bool m_first{true};               // First event flag

  // Run header
  struct NoiseHeader{
    int m_nRun{0};            /** Run number */
    float m_beamMom{0};       /** Beam momentum */
    float m_xCryo{0};         /** CryoX */
    float m_yTable{0};        /** TableY */
  };
  NoiseHeader m_header;

  // Variables to be in the TTree
  //--------------------------
  // Cell parameters
  std::vector<unsigned int>* m_cell_id{nullptr};
  std::vector<float>* m_cell_energy{nullptr};
  
  // Names and pointers
  std::string m_digitContainerName;
  Gaudi::Property<std::string> m_caloCellContainerName{this, "CellContainer", "AllCalo", "Cell container name"};
  std::string m_headerTreeName{"HeaderTree"};           // Header Tree name
  Gaudi::Property<std::string> m_noiseTreeName{this, "NoiseTreeName", "NoiseTree", "Noise Tree name"};
  Gaudi::Property<std::string> m_rootfile_name{this, "RootFileName", "tbh6tree_noise.root", "name of the ROOT file with TBTree"};
  TFile* m_rootfile{nullptr};             // and its pointer
  TTree* m_header_tree{nullptr};          // Header Tree pointer
  TTree* m_tree{nullptr};                 // Noise Tree pointer

  /** Text file containing xCryo and yTable */
  std::string m_txtFileWithXY{"xcryo_ytable.txt"};
};

#endif
