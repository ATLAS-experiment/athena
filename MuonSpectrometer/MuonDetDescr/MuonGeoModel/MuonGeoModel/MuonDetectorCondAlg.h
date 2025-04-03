/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONGEOMODEL_MUONDETECTORCONDALG_H
#define MUONGEOMODEL_MUONDETECTORCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonAlignmentData/NswAsBuiltDbData.h"
#include "MuonAlignmentData/sTGCAsBuiltData.h"
#include "MuonGeoModel/MuonDetectorTool.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonAlignmentData/NswPassivationDbData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

class MuonDetectorCondAlg : public AthReentrantAlgorithm {

  public:
    // Standard Constructor
    MuonDetectorCondAlg(const std::string &name, ISvcLocator *pSvcLocator);

    // Standard Destructor
    virtual ~MuonDetectorCondAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual bool isReEntrant() const override final { return false; }

  private:
    StatusCode copyInertMaterial(MuonGM::MuonDetectorManager& detMgr) const;
    Gaudi::Property<bool> m_applyMmPassivation{this, "applyMmPassivation", false};
    
    Gaudi::Property<bool> m_applyNswAsBuilt{this, "applyNswAsBuilt", true, 
                                            "Toggles the application of the Nsw as-built parameters"};
    Gaudi::Property<bool> m_applysTGCAsBuilt{this, "applysTGCAsBuilt", false, 
                                            "Toggles the application of the alternative sTGC as-built parameters"};

    Gaudi::Property<bool> m_applyMdtAsBuilt{this, "applyMdtAsBuilt", true, 
                                            "Toggles the application of the Mdt as-built parameters"};
    /// Apply translations and rotations to align the Muon stations
    Gaudi::Property<bool> m_applyALines{this, "applyALines", true};
    /// Apply the chamber deformation model (Mdts + Nsw)
    Gaudi::Property<bool> m_applyBLines{this, "applyBLines", true};
    /// Apply internal transformations on the CSCs
    Gaudi::Property<bool> m_applyILines{this, "applyILines", true};
    
    ToolHandle<MuonDetectorTool> m_iGeoModelTool{this, "MuonDetectorTool", "MuonDetectorTool", "The MuonDetector tool"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    // Read Handles
    SG::ReadCondHandleKey<ALineContainer> m_readALineKey{this, "ReadALineKey", "ALineContainer", "Key of input muon alignment ALine condition data"};
    SG::ReadCondHandleKey<BLineContainer> m_readBLineKey{this, "ReadBLineKey", "BLineContainer", "Key of input muon alignment BLine condition data"};
    SG::ReadCondHandleKey<ALineContainer> m_readILineKey{this, "ReadILineKey", "CscInternalAlignmentContainer",
                                                         "Key of input muon alignment CSC/ILine condition data"};
    SG::ReadCondHandleKey<MdtAsBuiltContainer> m_readMdtAsBuiltKey{this, "ReadMdtAsBuiltKey", "MdtAsBuiltContainer", "Key of output muon alignment MDT/AsBuilt condition data"};
    SG::ReadCondHandleKey<NswAsBuiltDbData> m_readNswAsBuiltKey{this, "ReadNswAsBuiltKey", "NswAsBuiltDbData", "Key of NswAsBuiltDbData object containing conditions data for NSW as-built params!"};
    SG::ReadCondHandleKey<sTGCAsBuiltData> m_readsTGCAsBuiltKey{this, "ReadsTGCAsBuiltKey", "sTGCAsBuilt", "Key of sTGCAsBuiltData object containing conditions data for the sTGC as-built model!"};
    SG::ReadCondHandleKey<NswPassivationDbData> m_condMmPassivKey {this, "condMmPassivKey", "NswPassivationDbData", "Key of NswPassivationDbData object containing passivation data for MMs"};

    // Write Handle
    SG::WriteCondHandleKey<MuonGM::MuonDetectorManager> m_writeDetectorManagerKey{this, "WriteDetectorManagerKey", "MuonDetectorManager",
                                                                                  "Key of output MuonDetectorManager condition data"};
};

#endif
