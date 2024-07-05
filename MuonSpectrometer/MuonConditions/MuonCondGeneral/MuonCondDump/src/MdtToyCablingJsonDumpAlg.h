/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Algorithm to dump the R4-style Mdt cabling maps into a JSON file
*/

#ifndef MUONCONDDUMP_MDTCABLINGJSONDUMPALG_H
#define MUONCONDDUMP_MDTCABLINGJSONDUMPALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

/** @brief: Simple algorithm to generate a toy cabling map for the Mdt detectors
 * 
 */


class MdtToyCablingJsonDumpAlg : public AthAlgorithm {
public:
    MdtToyCablingJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtToyCablingJsonDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

    Gaudi::Property<std::string> m_cablingJSON{this, "OutCablingJSON", "MdtCabling.json", "Cabling JSON"};

    Gaudi::Property<std::string> m_mezzanineJSON{this, "OutMezzanineJSON", "MdtMezzanine.json", "Cabling JSON"};


};

#endif
