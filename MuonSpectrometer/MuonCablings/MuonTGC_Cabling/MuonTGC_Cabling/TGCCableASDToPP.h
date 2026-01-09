/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLEASDTOPP_H
#define MUONTGC_CABLING_TGCCABLEASDTOPP_H

#include "MuonTGC_Cabling/TGCCable.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonCondInterface/ITGCCablingDbTool.h"
#include "MuonTGC_Cabling/TGCId.h"
#include "MuonTGC_Cabling/TGCDatabaseASDToPP.h"

#include <string>
#include <vector>

class StatusCode;

namespace MuonTGC_Cabling {

class TGCCableASDToPP : public TGCCable {
 public:
  TGCCableASDToPP(const std::string& filename);
  virtual ~TGCCableASDToPP();

  virtual TGCChannelId* getChannel(const TGCChannelId* channelId, 
				   bool orChannel=false) const;

  StatusCode updateDatabase();

 private:
  TGCCableASDToPP() {}

  void initialize(const std::string& filename);

  virtual TGCChannelId* getChannelIn (const TGCChannelId* ppin,
				      bool orChannel=false) const;
  virtual TGCChannelId* getChannelOut (const TGCChannelId* asdout,
				       bool orChannel=false) const;

  StatusCode getUpdateInfo(const int side,
			   const int sector,
			   const std::string& blockname,
			   std::vector<std::vector<int> >& info);

  TGCDatabaseASDToPP* getDatabase(const int side,
		                  const int region,
	                 	  const int sector,
	                          const int module) const;

  StatusCode updateIndividualDatabase(const int side,
				      const int sector,
				      const std::string& blockname,
				      std::shared_ptr<TGCDatabaseASDToPP>& database);

 private:
  static const int s_stripForward[];

  ToolHandle<ITGCCablingDbTool> m_tgcCablingDbTool; // cannot declare inline, since TGCCableASDToPP is no athena component
  std::vector<std::string> m_ASD2PP_DIFF_12;

 private:
  using ForwardSectorDB = std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>, TGCId::NUM_FORWARD_SECTOR>, TGCId::MaxSideType>;
  using InnerSectorDB   = std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>, TGCId::NUM_INNER_SECTOR>, TGCId::MaxSideType>;
  using EndcapSectorDB  = std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>, TGCId::NUM_ENDCAP_SECTOR>, TGCId::MaxSideType>;

  ForwardSectorDB m_FWDdb{};
  ForwardSectorDB m_FSDdb{};
  ForwardSectorDB m_FWTdb{};
  ForwardSectorDB m_FSTdb{};
  InnerSectorDB   m_FWIdb{};
  InnerSectorDB   m_FSIdb{};

  EndcapSectorDB m_EWDdb{};
  EndcapSectorDB m_ESDdb{};
  EndcapSectorDB m_EWTdb{};
  EndcapSectorDB m_ESTdb{};
  InnerSectorDB  m_EWIdb{};
  InnerSectorDB  m_ESIdb{};

  /** Pointers of common databases are recorded in this array */
  using CommonDB = std::array<std::array<std::shared_ptr<TGCDatabaseASDToPP>, TGCId::MaxModuleType>, TGCId::MaxRegionType>;
  CommonDB m_commonDb{{{nullptr}}};
};
  
} // end of namespace
 
#endif
