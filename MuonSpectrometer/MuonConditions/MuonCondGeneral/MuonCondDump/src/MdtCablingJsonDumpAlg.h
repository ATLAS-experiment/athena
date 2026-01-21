/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Algorithm to dump the Mdt cabling maps into a JSON file
*/

#ifndef MUONMDT_CABLING_MDTCABLINGMEZZALG_H
#define MUONMDT_CABLING_MDTCABLINGMEZZALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonRecToolInterfacesR4/IRootVisualizationService.h"



class MdtCablingJsonDumpAlg : public AthAlgorithm {
public:
    using AthAlgorithm::AthAlgorithm;
    virtual ~MdtCablingJsonDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}

    using CablingData = MuonMDT_CablingMap::CablingData;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    // MuonDetectorManager from the conditions store
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_DetectorManagerKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                                "Key of input MuonDetectorManager condition data"};

    SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of input MDT cabling map"};

    Gaudi::Property<std::string> m_summaryTxt{this, "SummaryFile", "SummaryFile.txt", "Summary of the extracted mapping"};
    Gaudi::Property<std::string> m_mezzJSON{this, "OutMezzanineJSON", "MezzMapping.json", "Mezzanine JSON"};
    Gaudi::Property<std::string> m_cablingJSON{this, "OutCablingJSON", "MdtCabling.json", "Cabling JSON"};
    Gaudi::Property<bool> m_insertBISCabling{this, "insertBISCabling", false, "override the BIS cabling in the JSON file"};
    
};

#endif
