/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILESIMUTILS_TILECALIBHITCNTNTUP_H
#define TILESIMUTILS_TILECALIBHITCNTNTUP_H

/********************************************************************
 *
 * NAME:     TileCalibHitCntNtup
 * PACKAGE:  offline/TileCalorimeter/TileSimUtils
 *
 * AUTHOR :  A. Solodkov    Sanya.Solodkov@cern.ch
 *           S. Karpov      Sergey.Karpov@cern.ch
 * CREATED:  Jan 15, 2013
 *
 * PURPOSE:  To create Ntuple file from Tile CaloCalibrationHitContainer-s
 *
 *  Input: CaloCalibHitCnt (Calibration hit containers)
 *  Output: NTuple 
 *  Parameters: stream name, ntuple name, max tree size
 *   
 ********************************************************************/

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

// Calo includes
#include "CaloIdentifier/CaloDM_ID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"

#include <string>
#include <vector>

#include <inttypes.h>
#include "Gaudi/Property.h"

class CaloCell_ID;
class CaloDM_ID;

/**
@class TileCalibHitCntNtup
@brief This class creates ntuple with calibration hits container info
*/
class TileCalibHitCntNtup: public AthAlgTool {

public:
  TileCalibHitCntNtup(const std::string& type, const std::string& name, const IInterface* parent); //!< Constructor

  virtual ~TileCalibHitCntNtup(); //!< Destructor

  virtual StatusCode initialize();  //!< intialize method
  virtual StatusCode StoreCNT( const CaloCalibrationHitContainer* ActiveHitCnt,  //!< execute method
                               const CaloCalibrationHitContainer* InactiveHitCnt, 
                               const CaloCalibrationHitContainer* dmHitCnt );
  virtual StatusCode finalize();    //!< finalize method

  static const InterfaceID& interfaceID( ) ; //!< AlgTool InterfaceID

private:

  // handle to THistSvc
  ServiceHandle<ITHistSvc> m_thistSvc{this, "THistSvc", "THistSvc"};

  // NTuple parameters
  Gaudi::Property<std::string> m_streamName{this, "StreamName", "AANT"};
  Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "TileCalibHitCnt"};
  Gaudi::Property<int64_t> m_treeSize{this, "TreeSize", 16000000000LL};

  // The ntuple
  TTree* m_ntuplePtr{nullptr};

  // ntuple variables
  long m_act_nchan{0};
  long m_inact_nchan{0};
  long m_dm_nchan{0};

  float m_act_totE{0};
  float m_act_visE{0};
  float m_inact_totE{0};
  float m_inact_visE{0};
  float m_DM_totE{0};
  float m_DM_visE{0};

  float m_barr_totE{0};
  float m_barr_Em{0};
  float m_barr_NonEm{0};
  float m_barr_Invisible{0};
  float m_barr_Escaped{0};

  float m_inact_barr_totE{0};
  float m_inact_barr_Em{0};
  float m_inact_barr_NonEm{0};
  float m_inact_barr_Invisible{0};
  float m_inact_barr_Escaped{0};

  float m_ext_totE{0};
  float m_ext_Em{0};
  float m_ext_NonEm{0};
  float m_ext_Invisible{0};
  float m_ext_Escaped{0};

  float m_inact_ext_totE{0};
  float m_inact_ext_Em{0};
  float m_inact_ext_NonEm{0};
  float m_inact_ext_Invisible{0};
  float m_inact_ext_Escaped{0};

  float m_itc_totE{0};
  float m_itc_Em{0};
  float m_itc_NonEm{0};
  float m_itc_Invisible{0};
  float m_itc_Escaped{0};

  float m_inact_itc_totE{0};
  float m_inact_itc_Em{0};
  float m_inact_itc_NonEm{0};
  float m_inact_itc_Invisible{0};
  float m_inact_itc_Escaped{0};

  float m_gscin_totE{0};
  float m_gscin_Em{0};
  float m_gscin_NonEm{0};
  float m_gscin_Invisible{0};
  float m_gscin_Escaped{0};

  float m_inact_gscin_totE{0};
  float m_inact_gscin_Em{0};
  float m_inact_gscin_NonEm{0};
  float m_inact_gscin_Invisible{0};
  float m_inact_gscin_Escaped{0};

  std::vector<float>* m_Total{nullptr};
  std::vector<float>* m_Em{nullptr};
  std::vector<float>* m_NonEm{nullptr};
  std::vector<float>* m_Invisible{nullptr};
  std::vector<float>* m_Escaped{nullptr};
  std::vector<long>* m_subCalo{nullptr};
  std::vector<long>* m_section{nullptr};
  std::vector<long>* m_side{nullptr};
  std::vector<long>* m_module{nullptr};
  std::vector<long>* m_tower{nullptr};
  std::vector<long>* m_sample{nullptr};

  std::vector<float>* m_inact_Total{nullptr};
  std::vector<float>* m_inact_Em{nullptr};
  std::vector<float>* m_inact_NonEm{nullptr};
  std::vector<float>* m_inact_Invisible{nullptr};
  std::vector<float>* m_inact_Escaped{nullptr};
  std::vector<long>* m_inact_subCalo{nullptr};
  std::vector<long>* m_inact_section{nullptr};
  std::vector<long>* m_inact_side{nullptr};
  std::vector<long>* m_inact_module{nullptr};
  std::vector<long>* m_inact_tower{nullptr};
  std::vector<long>* m_inact_sample{nullptr};

  std::vector<float>* m_dm_ene{nullptr};
  std::vector<float>* m_dm_vis{nullptr};
  std::vector<long>* m_dm_subDet{nullptr};
  std::vector<long>* m_dm_type{nullptr};
  std::vector<long>* m_dm_sampling{nullptr};
  std::vector<long>* m_dm_region{nullptr};
  std::vector<long>* m_dm_eta{nullptr};
  std::vector<long>* m_dm_phi{nullptr};

  Gaudi::Property<std::string> m_ActiveHitContainer{this, "ActiveHitCnt", "TileCalibHitActiveCell"};
  Gaudi::Property<std::string> m_InactiveHitContainer{this, "InactiveHitCnt", "TileCalibHitInactiveCell"};
  Gaudi::Property<std::string> m_dmHitContainer{this, "DMCalibHitCnt", "TileCalibHitDeadMaterial"};

  const CaloCell_ID* m_caloCell_ID{nullptr};
  const CaloDM_ID* m_caloDM_ID{nullptr};

  int m_max_chan{4999};

};

#endif
