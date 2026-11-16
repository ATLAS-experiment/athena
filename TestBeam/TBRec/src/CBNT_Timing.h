/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNT_TIMING_H
#define TBREC_CBNT_TIMING_H

#include "GaudiKernel/ToolHandle.h"
#include "CBNT_TBRecBase.h"
#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/LArEM_ID.h"
 #include "LArCabling/LArOnOffIdMapping.h"
#include "CLHEP/Units/SystemOfUnits.h"

class LArOnlineID;

#include <fstream>
#include <string>


class CBNT_Timing : public CBNT_TBRecBase
{
 public:
  CBNT_Timing(const std::string & name, ISvcLocator * pSvcLocator);

  virtual ~CBNT_Timing();

  //standard algorithm methods
  virtual StatusCode CBNT_initialize() override;
  virtual StatusCode CBNT_execute() override;
  virtual StatusCode CBNT_finalize() override;
  virtual StatusCode CBNT_clear() override;

 private: 
  static const int NOTIME = -999;
  static const int NOENERGY = -999;

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  const LArOnlineID* m_onlineHelper{nullptr};

  Gaudi::Property<double> m_energy_cut{this, "EnergyCut", 2.*CLHEP::GeV, "Cell energy cut"};
  bool m_first_event{true};

  Gaudi::Property<std::string> m_tbphase{this, "TBPhase", "TBPhase", "SG key of TBPhase"};
  Gaudi::Property<std::vector<std::string> > m_sampling_names{this, "IncludedSamplings", {}, "Names of included calorimeter samplings"};
  Gaudi::Property<std::string> m_caloCellName{this, "CellContainerName", "AllCalo", "SG key of CaloCellContainer"};

 /*! \brief stores */
  //  const CaloCell_ID* m_caloCellHelper;
  std::vector<CaloSampling::CaloSample> m_samplingIndices;
  std::map< CaloCell_ID::SUBCALO, std::vector<CaloSampling::CaloSample> > m_calosAndSamplings;
  std::vector<HWIdentifier> m_febIDs;

  /*! \brief internal helpers */
  StatusCode setupLookupTables();
  std::map<int, std::string>                               m_slotToFebNameLookup;
  std::map<CaloCell_ID::SUBCALO, std::string>              m_caloToNameLookup;
  std::map<std::string, CaloCell_ID::SUBCALO>              m_caloFromNameLookup;
  std::map<CaloSampling::CaloSample, std::string>          m_samplingToNameLookup;
  std::map<std::string, CaloSampling::CaloSample>          m_samplingFromNameLookup;
  std::map<CaloSampling::CaloSample, CaloCell_ID::SUBCALO> m_caloLookup;


  // per cell:
  std::vector<long>  *m_layer_cell{nullptr}, *m_eta_cell{nullptr}, *m_phi_cell{nullptr}, *m_febId_cell{nullptr};
  std::vector<float> *m_energy_cell{nullptr}, *m_time_cell{nullptr};
  // per layer:
  std::vector<float> *m_time_layer{nullptr};
  std::vector<long>  *m_layer_layer{nullptr};

  // per feb
  std::vector<long>  *m_febId_feb{nullptr};
  std::vector<float> *m_time_feb{nullptr};
  std::vector<long> *m_slot_feb{nullptr};
  std::vector<long> *m_feedthrough_feb{nullptr};

  // per event:
  float  m_energy{0}, m_time{0}, m_tdc_phase{0}; // time = calculated cell time (cubic), clock = phase (TBRec)
};

#endif
