/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*
 * Algorithm to dump the R4-style Mdt cabling maps into a JSON file
*/

#ifndef MUONCONDDUMP_MDTTOYTWINCABLINGDUMPALG_H
#define MUONCONDDUMP_MDTTOYTWINCABLINGDUMPALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

/** @brief: Simple algorithm to generate a toy cabling map for the twin tubes
 * 
 */


class MdtToyTwinCablingDumpAlg : public AthAlgorithm {
public:
    MdtToyTwinCablingDumpAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtToyTwinCablingDumpAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    virtual unsigned int cardinality() const override final{return 1;}

private:
    bool equipREwithTwins(const Identifier& detElId) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

    Gaudi::Property<std::string> m_cablingJSON{this, "outCablingJSON", "MdtTwinMapping.json", "Cabling JSON"};
    Gaudi::Property<std::vector<std::string>> m_stationsToTwin{this, "stationsToTwin", {}, "List of station names that should have twinned tubes"};

    Gaudi::Property<std::vector<std::string>> m_detElIdToTwin{this, "detElIdToTwin", {"BOL4A13M1", "BOL4C13M1" }};
    Gaudi::Property<int> m_spacing{this, "IndentSpace", -1, "How many spaces shall be used for indentation"};    


};

#endif