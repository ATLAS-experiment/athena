/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigConf_StorageMgrBase
#define TrigConf_StorageMgrBase

#include <string>
#include <memory>
#include "TrigConfBase/MsgStream.h"
#include "TrigConfStorage/IStorageMgr.h"

namespace TrigConf {

   class IMasterTableLoader;
   class IMenuLoader;                 
   class IMuctpiLoader;               
   class IDeadTimeLoader;             
   class IPrescaledClockLoader;       
   class IRandomLoader;               
   class IThresholdConfigLoader;      
   class ITriggerThresholdLoader;     
   class ITriggerThresholdValueLoader;
   class IThresholdMonitorLoader;     
   class ICTPFilesLoader;             
   class ICaloJetInputLoader;         
   class ICaloSinCosLoader;           
   class ICaloInfoLoader;             
   class ILutCamLoader;               
   class IPrescaleSetLoader;          
   class IPrioritySetLoader;          
   class IBunchGroupLoader;           
   class IBunchGroupSetLoader;        
   class ICTPConfigLoader;            
   class IMuonThresholdSetLoader;     
   class IHLTFrameLoader;             
   class IJobOptionTableLoader;       
   class IL1TopoMenuLoader;

   class StorageMgrBase : virtual public IStorageMgr {
   public:
    
      /** @brief constructor */
      StorageMgrBase();
     
      /** @brief destructor */
      virtual ~StorageMgrBase() override;

      // setting log level of all loaders
      virtual void setLevel(MSGTC::Level lvl) override = 0;

   protected:

      std::unique_ptr<IMasterTableLoader>           m_masterTableLoader;
      std::unique_ptr<IMenuLoader>                  m_menuLoader;
      std::unique_ptr<IMuctpiLoader>                m_muctpiLoader;
      std::unique_ptr<IDeadTimeLoader>              m_deadTimeLoader;
      std::unique_ptr<IPrescaledClockLoader>        m_prescaledClockLoader;
      std::unique_ptr<IRandomLoader>                m_randomLoader;
      std::unique_ptr<IThresholdConfigLoader>       m_thresholdConfigLoader;
      std::unique_ptr<ITriggerThresholdLoader>      m_triggerThresholdLoader;
      std::unique_ptr<ITriggerThresholdValueLoader> m_triggerThresholdValueLoader;
      std::unique_ptr<IThresholdMonitorLoader>      m_thresholdMonitorLoader;
      std::unique_ptr<ICTPFilesLoader>              m_ctpFilesLoader;
      std::unique_ptr<ICaloJetInputLoader>          m_caloJetInputLoader;
      std::unique_ptr<ICaloSinCosLoader>            m_caloSinCosLoader;
      std::unique_ptr<ICaloInfoLoader>              m_caloInfoLoader;
      std::unique_ptr<ILutCamLoader>                m_lutCamLoader;
      std::unique_ptr<IPrescaleSetLoader>           m_prescaleSetLoader;
      std::unique_ptr<IPrioritySetLoader>           m_prioritySetLoader;
      std::unique_ptr<IBunchGroupLoader>            m_bunchGroupLoader;
      std::unique_ptr<IBunchGroupSetLoader>         m_bunchGroupSetLoader;
      std::unique_ptr<ICTPConfigLoader>             m_ctpConfigLoader;
      std::unique_ptr<IMuonThresholdSetLoader>      m_muonThresholdSetLoader;
      std::unique_ptr<IHLTFrameLoader>              m_HLTFrameLoader;
      std::unique_ptr<IJobOptionTableLoader>        m_jobOptionTableLoader;
      std::unique_ptr<IL1TopoMenuLoader>            m_l1topoMenuLoader;

      unsigned int   m_ctpVersion { 0 };
      unsigned int   m_l1Version { 0 };

   };

}

#endif
