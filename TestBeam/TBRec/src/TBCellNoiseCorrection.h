/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBCELLNOISECORRECTION_H
#define TBREC_TBCELLNOISECORRECTION_H
/********************************************************************
NAME:     TBCellNoiseCorrection.cxx
PACKAGE:  offline/TestBeam/TBRec

PURPOSE:  Read the weighted noise events and add them

********************************************************************/
#include "CaloUtils/CaloCellCorrection.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "CLHEP/Random/RandomEngine.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentListener.h"

class LArCell;
class MsgStream;
class TFile;
class TChain;

class TBCellNoiseCorrection : public CaloCellCorrection, virtual public IIncidentListener
{

 public:

    TBCellNoiseCorrection (const std::string& type, const std::string& name, 
		const IInterface* parent);

    virtual ~TBCellNoiseCorrection();
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    void MakeCorrection (CaloCell* theCell,
                         const EventContext& ctx) const override;
    virtual void handle(const Incident&) override;

 private: 

  // Properties:  
  Gaudi::Property<bool> m_noise_correction{this, "NoiseAdd", false, "Add noise to cells"};
  Gaudi::Property<std::vector<std::string>>  m_noise_file{this, "NoiseFile", {}, "Noise ROOT files"};

  // Data members:
  ServiceHandle <IAthRNGSvc> m_rndmSvc
    { this, "RndmSvc", "AthRNGSvc", "" };
  ATHRNG::RNGWrapper* m_engine{nullptr};
  //TFile *m_root; // File with noise tree
  TChain *m_tree{nullptr};
  Gaudi::Property<float> m_xcryo{this, "NoiseX", 0., "CryoX; if given, try to find run number from xcryo_ytable.txt file"};
  Gaudi::Property<float> m_ytable{this, "NoiseY", 0., "TableY; then a run number in file name is changed to this one"};
  Gaudi::Property<float> m_energy{this, "NoiseE", -1., "Beam energy"};
  // Cell parameters
  std::vector<unsigned int>* m_cell_id{nullptr};
  std::vector<float>* m_cell_energy{nullptr};
  unsigned int m_entries{0};
  // should check the event number
  //int m_myevtnum;
};

#endif     




