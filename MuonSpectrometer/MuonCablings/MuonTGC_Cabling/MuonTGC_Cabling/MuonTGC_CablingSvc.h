/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
    MuonTGC_CablingSvc.h
    Description : online-offline ID mapper for TGC
***************************************************************************/

#ifndef MUONTGC_CABLING_MUONTGC_CABLINGSVC_H
#define MUONTGC_CABLING_MUONTGC_CABLINGSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "MuonCondInterface/ITGCCablingDbTool.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "GaudiKernel/Service.h"
#include "MuonTGC_Cabling/TGCCabling.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

class Identifier;

class MuonTGC_CablingSvc : public AthService
{
 public:
  MuonTGC_CablingSvc(const std::string& name, ISvcLocator* svc);
  virtual ~MuonTGC_CablingSvc() = default;

  virtual StatusCode initialize(void) override;

  const MuonTGC_Cabling::TGCCabling* getTGCCabling() const;

  // give max value of the ROD ID
  int getMaxRodId() { return MuonTGC_Cabling::TGCCabling::MAXRODID; }

  // give max value of ReadoutID parameters
  void getReadoutIDRanges(int& maxRodId,
                          int& maxSRodId,
			  int& maxSswId,
			  int& maxSbloc,
			  int& minChannelId,
			  int& maxChannelId) const;

  // give phi-range which a ROD covers
  bool getCoveragefromRodID(const int rodID,
			    double & startPhi,
			    double & endPhi) const;

  bool getCoveragefromRodID(const int rodID,
			    int & startEndcapSector,
			    int & coverageOfEndcapSector,
			    int & startForwardSector,
			    int & coverageOfForwardSector) const;

  // give phi-range which a SROD covers
  bool getCoveragefromSRodID(const int srodID,
			    double & startPhi,
			    double & endPhi) const;

  bool getCoveragefromSRodID(const int srodID,
                             int & startEndcapSector,
                             int & coverageOfEndcapSector,
                             int & startForwardSector,
                             int & coverageOfForwardSector) const;

  // Readout ID is ored
  bool isOredChannel(const int subDetectorID,
		     const int rodID,
		     const int sswID,
		     const int sbLoc,
		     const int channelID) const;


  // Offline ID has adjacent Readout ID
  bool hasAdjacentChannel(const Identifier & offlineID) const;


  // Online ID has adjacent Readout ID
  bool hasAdjacentChannel(const int subsystemNumber,
			  const int octantNumber,
			  const int moduleNumber,
			  const int layerNumber,
			  const int rNumber,
			  const int wireOrStrip,
			  const int channelNumber) const;


  // readout IDs -> offline IDs
  bool getOfflineIDfromReadoutID(Identifier & offlineID,
				 const int subDetectorID,
				 const int rodID,
				 const int sswID,
				 const int sbLoc,
				 const int channelID,
				 bool orChannel=false) const;

  
  // offline IDs -> readout IDs
  bool getReadoutIDfromOfflineID(const Identifier & offlineID,
				 int & subDetectorID,
				 int & rodID,
				 int & sswID,
				 int & sbLoc,
				 int & channelID,
				 bool adChannel=false) const;

  // offline ID -> online IDs
  bool getOnlineIDfromOfflineID(const Identifier & offlineID,
				int & subsystemNumber,
				int & octantNumber,
				int & moduleNumber,
				int & layerNumber,
				int & rNumber,
				int & wireOrStrip,
				int & channelNumber) const;

  // online IDs -> offline ID
  bool getOfflineIDfromOnlineID(Identifier & offlineID,
				const int subsystemNumber,
				const int octantNumber,
				const int moduleNumber,
				const int layerNumber,
				const int rNumber,
				const int wireOrStrip,
				const int channelNumber) const;

  // readout IDs -> online IDs
  bool getOnlineIDfromReadoutID(const int subDetectorID,
				const int rodID,
				const int sswID,
				const int sbLoc,
				const int channelID,
				int & subsystemNumber,
				int & octantNumber,
				int & moduleNumber,
				int & layerNumber,
				int & rNumber,
				int & wireOrStrip,
				int & channelNumber,
				bool orChannel=false) const;

  // online IDs -> readout IDs
  bool getReadoutIDfromOnlineID(int & subDetectorID,
				int & rodID,
				int & sswID,
				int & sbLoc,
				int & channelID,
				const int subsystemNumber,
				const int octantNumber,
				const int moduleNumber,
				const int layerNumber,
				const int rNumber,
				const int wireOrStrip,
				const int channelNumber,
				bool adChannel=false) const;
  
  // element ID -> readout IDs
  bool getReadoutIDfromElementID(const Identifier & elementID,
				 int & subdetectorID,
				 int & rodID) const;
  
  // readout IDs -> element ID
  bool getElementIDfromReadoutID(Identifier & elementID,
				 const int subDetectorID,
				 const int rodID,
				 const int sswID,
				 const int sbLoc,
				 const int channelID,
				 bool orChannel=false) const;

  // HPT ID -> readout ID
  bool getReadoutIDfromHPTID(const int phi,
			     const bool isAside,
			     const bool isEndcap,
			     const bool isStrip,
			     const int id,
			     int & subsectorID,
			     int & rodID,
			     int & sswID,
			     int & sbLoc) const;

  // readout ID -> SLB ID
  bool getSLBIDfromReadoutID(int &phi,
			     bool & isAside,
			     bool & isEndcap,
			     int & moduleType,
			     int & id,
			     const int subsectorID,
			     const int rodID,
			     const int sswID,
			     const int sbLoc) const;

  // readout ID -> slbAddr
  bool getSLBAddressfromReadoutID(int & slbAddr,
				  const int subsectorID,
				  const int rodID,
				  const int sswID,
				  const int sbLoc) const;

  // readout ID -> RxID
  bool getRxIDfromReadoutID(int & rxId,
			    const int subsectorID,
			    const int rodID,
			    const int sswID,
			    const int sbLoc) const;
  
  // ROD_ID / SSW_ID / RX_ID -> SLB ID
  bool getSLBIDfromRxID(int &phi,
			bool & isAside,
			bool & isEndcap,
			int & moduleType,
			int & id,
			const int subsectorID,
			const int rodID,
			const int sswID,
			const int rxId) const;

  // SLB ID -> readout ID
  bool getReadoutIDfromSLBID(const int phi,
			     const bool isAside,
			     const bool isEndcap,
			     const int moduleType,
			     const int id,
			     int & subsectorID,
			     int & rodID,
			     int & sswID,
			     int & sbLoc) const;
  
  // readout ID (ROD) -> SL ID 
  bool getSLIDfromReadoutID(int & phi,
			    bool & isAside,
			    bool & isEndcap,
			    const int subsectorID,
			    const int rodID,
			    const int sswID,
			    const int sbLoc) const;

  // readout ID (SROD) -> SL ID 
  bool getSLIDfromSReadoutID(int & phi,
                             bool & isAside,
                             const int subsectorID,
                             const int srodID,
                             const int sector,
                             const bool forward) const;

  // SL ID -> readout ID ( ROD )
  bool getReadoutIDfromSLID(const int phi,
			    const bool isAside,
			    const bool isEndcap,
			    int & subsectorID,
			    int & rodID,
			    int & sswID,
			    int & sbLoc) const;

  // SL ID -> readout ID ( SROD )
  bool getSReadoutIDfromSLID(const int phi,
                             const bool isAside,
                             const bool isEndcap,
                             int & subsectorID,
                             int & srodID,
                             int & sswID,
                             int & sbLoc) const;

  // HighPtID used in Simulation -> HighPtID in RDO
  bool getRDOHighPtIDfromSimHighPtID(const bool isForward,
                                     const bool isStrip,
                                     int & index,
                                     int & chip,
                                     int & hitId) const;
 
  // HighPtID in RDO -> HighPtID used in Simulation
  bool getSimHighPtIDfromRDOHighPtID(const bool isForward,
                                     const bool isStrip,
                                     int & index,
                                     int & chip,
                                     int & hitId) const;


  // high pt coincidence IDs -> offline IDs
  bool getOfflineIDfromHighPtID(Identifier & offlineID,
				const int subDetectorID,
				const int rodID,
				const int sectorInReadout,
				const bool isStrip,
				const bool isForward,
				const int hpb,
				const int chip,
				const int hitID,
				const int pos ) const;

  // offline IDs -> high pt coincidence IDs
  bool getHighPtIDfromOfflineID(const Identifier & offlineID,
				int & subDetectorID,
				int & rodID,
				int & sectorInReadout,
				bool & isStrip,
				bool & isForward,
				int & hpb,
				int & chip,
				int & hitID,
				int & pos) const;

  // HPT HitID -> ROI Number
  bool getROINumberfromHighPtID(int &roi,
				bool isForward,
				int hpb_wire,
				int chip_wire,
				int hitId_wire,
				int sub_wire,
				int chip_strip,
				int hitId_strip,
				int sub_strip) const;

  // HPT HitID -> ROI Number
  bool getHighPtIDfromROINumber(int roi,
				bool isForward,
				bool isStrip,
				int & hpb,
				int & chip,
				int & hitID,
				int & sub) const;
  
  // low pt coincidence IDs -> offline IDs
  bool getOfflineIDfromLowPtCoincidenceID(Identifier & offlineID,
					  const int subDetectorID,
					  const int rodID,
					  const int sswID,
					  const int sbLoc,
					  const int block,
					  const int pos,
					  bool middle=false) const;
  
  // offline IDs -> low pt coincidence IDs
  bool getLowPtCoincidenceIDfromOfflineID(const Identifier & offlineID,
					  int & subDetectorID,
					  int & rodID,
					  int & sswID,
					  int & sbLoc,
					  int & block,
					  int & pos,
					  bool middle=false) const;


  /////////////////////////////////////////////////////////////
  // channel connection
  MuonTGC_Cabling::TGCChannelId* 
    getChannel(const MuonTGC_Cabling::TGCChannelId* channelId,
	       MuonTGC_Cabling::TGCChannelId::ChannelIdType type,
	       bool orChannel=false) const;
  
  // module connection
  MuonTGC_Cabling::TGCModuleMap* 
    getModule(const MuonTGC_Cabling::TGCModuleId* moduleId,
	      MuonTGC_Cabling::TGCModuleId::ModuleIdType type) const;	

  ///////////////////////  

 private:
  std::unique_ptr<MuonTGC_Cabling::TGCCabling> m_cabling;
  ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
  ToolHandle<ITGCCablingDbTool> m_condDataTool{this,"TGCCablingDbTool","TGCCablingDbTool"};

 protected:
  IntegerProperty m_AsideId;
  IntegerProperty m_CsideId;
  IntegerArrayProperty m_rodId;

  StringProperty m_databaseASDToPP;
  StringProperty m_databaseInPP;
  StringProperty m_databasePPToSL;
  StringProperty m_databaseSLBToROD;

  StringProperty m_databaseASDToPPdiff;
};


inline const MuonTGC_Cabling::TGCCabling* MuonTGC_CablingSvc::getTGCCabling() const
{
  return m_cabling.get();
}  

#endif // MUONTGC_CABLING_MUONTGC_CABLINGSVC_H
