/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DETDESCRCNVSVC_DETDESCRCNVSVC_H
#define DETDESCRCNVSVC_DETDESCRCNVSVC_H

#include "GaudiKernel/ConversionSvc.h"

// Forward declarations
class IOpaqueAddress;
class StoreGateSvc;

class DetDescrCnvSvc : public ConversionSvc {

   public:
    /// Initialize the service.
    virtual StatusCode initialize();

    /// Add new address to the Detector Store
    virtual StatusCode addToDetStore(const CLID &clid,
                                     const std::string &name) const;

    /// Basic create address
    virtual StatusCode createAddress(long svc_type, const CLID &clid,
                                     const std::string *par,
                                     const unsigned long *ip,
                                     IOpaqueAddress *&refpAddress);

    /// Create address from string form
    virtual StatusCode createAddress(long svc_type, const CLID &clid,
                                     const std::string &refAddress,
                                     IOpaqueAddress *&refpAddress);

    /// Convert address to string form
    virtual StatusCode convertAddress(const IOpaqueAddress *pAddress,
                                      std::string &refAddress);

    /**@name: Object implementation     */
    //@{
    /// Standard Constructor
    DetDescrCnvSvc(const std::string &name, ISvcLocator *svc);

    /// Standard Destructor
    virtual ~DetDescrCnvSvc();

   private:
    StatusCode fillTDSRefs();

    void initTDSItems();

    ServiceHandle<StoreGateSvc> m_detStore;
    StringArrayProperty m_detMgrs{this, "DetectorManagers", {}};
    StringArrayProperty m_detNodes{this, "DetectorNodes", {}};
    BooleanProperty m_decodeIdDict{this, "DecodeIdDict", true};
    BooleanProperty m_idDictFromRDB{this, "IdDictFromRDB", false};
    StringProperty m_idDictName{this, "IdDictName", ""};
    StringProperty m_idDictGlobalTag{this, "IdDictGlobalTag", ""};
    StringProperty m_idDictATLASName{this, "AtlasIDFileName", ""};
    StringProperty m_idDictInDetName{this, "InDetIDFileName", ""};
    StringProperty m_idDictLArName{this, "LArIDFileName", ""};
    StringProperty m_idDictTileName{this, "TileIDFileName", ""};
    StringProperty m_idDictLVL1Name{this, "CaloIDFileName", ""};
    StringProperty m_idDictMuonName{this, "MuonIDFileName", ""};
    StringProperty m_idDictLArHighVoltageName{this, "HighVoltageIDFileName",
                                              ""};
    StringProperty m_idDictLArElectrodeName{this, "LArElectrodeIDFileName", ""};
    StringProperty m_idDictForwardName{this, "ForwardIDFileName", ""};
    StringProperty m_fcal2dNeighborsName{this, "FCAL2DNeighborsFileName", ""};
    StringProperty m_fcal3dNeighborsNextName{this,
                                             "FCAL3DNeighborsNextFileName", ""};
    StringProperty m_fcal3dNeighborsPrevName{this,
                                             "FCAL3DNeighborsPrevFileName", ""};
    StringProperty m_tileNeighborsName{this, "TileNeighborsFileName", ""};
    StringProperty m_fullAtlasNeighborsName{this, "FullAtlasNeighborsFileName",
                                            ""};

    BooleanProperty m_fromRoot{this, "ReadFromROOT", false};
    BooleanProperty m_fromNova{this, "ReadFromNova", false};
    BooleanProperty m_detElemsfromDetNodes{this, "InitDetElemsFromGeoModel",
                                           false};
    BooleanProperty m_compact_ids_only{this, "CompactIDsOnly", false};
    BooleanProperty m_do_checks{this, "DoIdChecks", false};
    BooleanProperty m_do_neighbours{this, "DoInitNeighbours", true};

    /// Switch on/off the TRT
    BooleanProperty m_hasTRT{this, "HasTRT", false};

    /// Switch on/off the muon detectors

    BooleanProperty m_hasCSC{this, "HasCSC", false};
    BooleanProperty m_hasSTGC{this, "HasSTgc", false};
    BooleanProperty m_hasMM{this, "HasMM", false};
    BooleanProperty m_hasMDT{this, "HasMDT", true};
    BooleanProperty m_hasRPC{this, "HasRPC", true};
    BooleanProperty m_hasTGC{this, "HasTGC", true};

    BooleanProperty m_useGeomDB_InDet{this, "useGeomDB_InDet", false};

    inline MsgStream &msg(MSG::Level lvl) const { return msgStream(lvl); }
};
#endif
