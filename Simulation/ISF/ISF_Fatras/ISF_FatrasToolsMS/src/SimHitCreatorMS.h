/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLSMS_SIMHITCREATORMS_H
#define ISF_FATRASTOOLSMS_SIMHITCREATORMS_H

// Athena & Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaKernel/IAtRndmGenSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "CxxUtils/checker_macros.h"
// ISF includes
#include "ISF_FatrasInterfaces/ISimHitCreator.h"
// Tracking includes
#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/ITimedExtrapolator.h"
// Muon includes
#include "MuonSimEvent/MDTSimHitCollection.h"
#include "MuonSimEvent/RPCSimHitCollection.h"
#include "MuonSimEvent/TGCSimHitCollection.h"
#include "MuonSimEvent/CSCSimHitCollection.h"
#include "MuonSimEvent/MMSimHitCollection.h"
#include "MuonSimEvent/sTGCSimHitCollection.h"
//Muon ReadoutGeometry includes
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGRecTools/IMuonTGMeasTool.h"

// Muon
class MdtHitIdHelper;
class RpcHitIdHelper;
class CscHitIdHelper;
class TgcHitIdHelper;

namespace MuonGM {
  class MuonDetectorManager;
}

namespace Trk {
  class Layer;
}

namespace ISF {
    class ISFParticle;
}

struct MM_SimIdToOfflineId;
struct sTgcSimIdToOfflineId;

namespace iFatras 
{

  /** @class SimHitCreatorMS 

      A dedicated instance for Fatras sim hit creation
      in the Muon System.
      
      @author  Sharka Todorova <Sarka.Todorova@cern.ch>, 
               Andreas Salzburger <Andreas.Salzburger@cern.ch>
  */  

  class ATLAS_NOT_THREAD_SAFE SimHitCreatorMS : public extends<AthAlgTool, ISimHitCreator, IIncidentListener>  // deprecated: ATLASSIM-6020
    {
    public:
      using base_class::base_class;

       /** default destructor */
      virtual ~SimHitCreatorMS ()=default;
      
      virtual StatusCode initialize();

      /** handle for incident service */
      void handle(const Incident& inc);    

       /** Loop over the hits and call the hit creator - also provide the ISF particle to register the truth link */
      void createHits(const ISF::ISFParticle& isp, 
                      const std::vector<Trk::HitInfo>& hits) const;

      void initDeadChannels(const MuonGM::MdtReadoutElement* mydetEl);
         
    private:

      /** Private HitCreate method - returns bool for a successful hit creation */       
      bool createHit(const ISF::ISFParticle& isp, const Trk::Layer* , const Trk::TrackParameters*, Identifier, double, double, bool) const;

      int  offIdToSimId(Identifier id) const;

      /** Incident Service */  
      ServiceHandle<IIncidentSvc>          m_incidentSvc{"IncidentSvc", name()};

      /** Tool using the track creator per event */
      PublicToolHandle<Trk::ITimedExtrapolator> m_extrapolator{this, "Extrapolator", ""};
      
      /** Muon TrackingGeometry Measurement Tool */
      PublicToolHandle<Muon::IMuonTGMeasTool> m_measTool{this, "MeasurementTool", "Muon::MuonTGMeasurementTool/MuonTGMeasurementTool"};

      /* Hit collections and collection helpers */
      MDTSimHitCollection                  *m_mdtSimHitCollection{nullptr};
      RPCSimHitCollection                  *m_rpcSimHitCollection{nullptr};
      TGCSimHitCollection                  *m_tgcSimHitCollection{nullptr};
      CSCSimHitCollection                  *m_cscSimHitCollection{nullptr};
      MMSimHitCollection          	   *m_mmSimHitCollection{nullptr};
      sTGCSimHitCollection          	   *m_stgcSimHitCollection{nullptr};
      StringProperty                       m_mdtCollectionName{this, "MDTCollectionName", "MDT_Hits"};
      StringProperty                       m_rpcCollectionName{this, "RPCCollectionName", "RPC_Hits"};
      StringProperty                       m_tgcCollectionName{this, "TGCCollectionName", "TGC_Hits"};
      StringProperty                       m_cscCollectionName{this, "CSCCollectionName", "CSC_Hits"};
      StringProperty                       m_mmCollectionName{this, "MMCollectionName", "MM_Hits"};
      StringProperty                       m_stgcCollectionName{this, "sTGCCollectionName", "sTGC_Hits"};
      
      /** Pointer to the random number generator service */
      ServiceHandle<IAtRndmGenSvc>         m_randomSvc{this, "RandomNumberService", "AtDSFMTGenSvc", "Random number generator"}; //!< Random Svc  
      StringProperty                       m_randomEngineName{this, "RandomStreamName", "FatrasRnd", "Name of the random number stream"}; //!< Name of the random number stream
      CLHEP::HepRandomEngine*              m_randomEngine{nullptr};    //!< Random Engine 
      const MdtHitIdHelper*                m_mdtHitIdHelper{nullptr};
      const RpcHitIdHelper*                m_rpcHitIdHelper{nullptr};
      const CscHitIdHelper*                m_cscHitIdHelper{nullptr};
      const TgcHitIdHelper*                m_tgcHitIdHelper{nullptr};
      ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
      MM_SimIdToOfflineId*                 m_mmOffToSimId{nullptr};
      sTgcSimIdToOfflineId*                m_stgcOffToSimId{nullptr};
 
      const MuonGM::MuonDetectorManager*   m_muonMgr{nullptr};
      
      DoubleProperty                       m_mdtSigmaDriftRadius{this, "MdtSigmaDriftRadius", 0.08};
      
      int   				   m_BMGid{-1}; //added to protect against dead sensors
      BooleanProperty                      m_createAllMdtHits{this, "CreateAllMdtHits", true};      
      bool   				   m_BMGpresent = false;//added to protect against dead sensors
      std::map<Identifier, std::vector<Identifier> > m_DeadChannels;
    }; 

    
} // end of namespace

#endif 
