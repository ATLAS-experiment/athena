/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCONDUTILS_LARHVPATHOLOGYDBALG_H
#define LARCONDUTILS_LARHVPATHOLOGYDBALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRecConditions/LArHVIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "PersistentDataModel/AthenaAttributeList.h"

#include "LArRecConditions/LArHVPathology.h"
#include "LArRecConditions/LArHVPathologiesDb.h"

#include <optional>
#include <memory>

class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;
class LArOnlineID;
class CaloIdManager;
class Identifier;


/**
 @class LArHVPathologyDBAlg 
 @brief Algorithm to read/write HV pathologies from/to a text file and to fill an sqlite database file
*/
class LArHVPathologyDbAlg : public AthAlgorithm 
{
 public:

  using AthAlgorithm::AthAlgorithm;
  ~LArHVPathologyDbAlg() =default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:
  std::optional<LArHVPathologiesDb> createCondObjects (const EventContext& ctx, const CaloDetDescrManager* calodetdescrmgr) const;
  StatusCode printCondObjects (const EventContext& ctx, const CaloDetDescrManager* calodetdescrmgr, const std::vector<LArHVPathologiesDb::LArHVElectPathologyDb>* path=nullptr) const;

  std::vector<unsigned int> getElectInd(const LArHVIdMapping& hvIdMapping,
                                        const Identifier& id, unsigned int module, unsigned int line,
					const CaloDetDescrManager* calodetdescrmgr) const;

  int getHVline(const LArHVIdMapping& hvIdMapping,
                const Identifier& id, short unsigned int ElectInd,
		const CaloDetDescrManager* calodetdescrmgr) const;


  std::unique_ptr<AthenaAttributeList> hvPathology2AttrList(const LArHVPathologiesDb& pathologyContainer) const;
 
  BooleanProperty           m_writeCondObjs{this,"WriteCondObjs",false};
  StringProperty            m_inpFile{this,"InpFile",{}};
  StringProperty            m_outFile{this,"OutFile",{}};
  StringProperty            m_folder{this,"Folder","/LAR/HVPathologiesOfl/Pathologies"};
  IntegerProperty m_mode{this,"Mode",0,"Mode to read file (0=offlineID/elecID, 1=online ID fields + HV module/line, 2=type is HV value to overwrite)"};

  const CaloIdManager* m_caloIdMgr{nullptr};
  const LArEM_ID*      m_larem_id{nullptr};
  const LArHEC_ID*     m_larhec_id{nullptr};
  const LArFCAL_ID*    m_larfcal_id{nullptr};
  const LArOnlineID*   m_laronline_id{nullptr};

  SG::ReadCondHandleKey<LArHVIdMapping> m_hvCablingKey
    {this, "LArHVIdMapping", "LArHVIdMap", "SG key for HV ID mapping"};
  SG::ReadCondHandleKey<LArOnOffIdMapping>  m_cablingKey
    {this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey 
    {this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };

  SG::ReadCondHandleKey<LArHVPathology> m_hvPathologyKey
    {this, "HVPAthologyKey", "LArHVPathology", "Key for HV pathologies in Cond. store"};
};

#endif
