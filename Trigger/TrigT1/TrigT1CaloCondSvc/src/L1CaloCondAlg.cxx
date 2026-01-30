// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "TrigT1CaloCondSvc/L1CaloCondAlg.h"


#include <iomanip>
#include "TrigT1CaloCalibConditions/IL1CaloPersistenceCapable.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/WriteCondHandle.h"


L1CaloCondAlg :: L1CaloCondAlg ( const std::string &name,ISvcLocator *pSvcLocator): AthCondAlgorithm( name, pSvcLocator )    
										    
										    

{
 
}





StatusCode  L1CaloCondAlg:: initialize ()
{
  
  ATH_MSG_DEBUG( "initialize " << name() );                
  
 

  // Disabled Towers 
  ATH_CHECK( m_disabledTowers.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_disabledTowersContainer.initialize() );


  // Derived Run Pars
  ATH_CHECK(m_derivedRunPars.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_derivedRunParsContainer.initialize(SG::AllowEmpty) );

  // Ppr Channel Calib 
  ATH_CHECK(m_pprChanCalibContainer.initialize() );

  // Cond Container
  ATH_CHECK(m_pprConditionsContainer.initialize(SG::AllowEmpty) );


  // Ppr Defaults Calib
  ATH_CHECK(m_pprChanDefaults.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_pprChanDefaultsContainer.initialize() );

  // Ppr FineTime Refs
  ATH_CHECK(m_ppmFineTimeRefs.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_ppmFineTimeRefsContainer.initialize(SG::AllowEmpty) );
  
  // Run Parameters
  ATH_CHECK(m_runParameters.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_runParametersContainer.initialize(SG::AllowEmpty) );

  // PprChanStrategy
  ATH_CHECK(m_pprChanStrategy.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_pprChanStrategyContainer.initialize(SG::AllowEmpty) );


  // Ppm Dead Channels 
  ATH_CHECK(m_ppmDeadChannels.initialize(SG::AllowEmpty) );
  ATH_CHECK(m_ppmDeadChannelsContainer.initialize() );


  // Disabled Channel
  ATH_CHECK(m_pprDisabledChannelContainer.initialize() );

  // Readout Config
  ATH_CHECK( m_readoutConfig.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_readoutConfigContainer.initialize(SG::AllowEmpty) );
  
  // Readout Config JSON (Run 3)
  ATH_CHECK( m_readoutConfigJSON.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_readoutConfigContainerJSON.initialize(SG::AllowEmpty) );

  ATH_CHECK ( m_physicsKeys.initialize (m_usePhysicsRegime) );
  ATH_CHECK( m_calib1Keys.initialize (m_useCalib1Regime) );
  ATH_CHECK( m_calib2Keys.initialize (m_useCalib2Regime) );

  return StatusCode::SUCCESS;

}

template <typename T> StatusCode L1CaloCondAlg::updateCond(const EventContext& ctx, const SG::WriteCondHandleKey<T>& wkey, const std::vector<std::reference_wrapper<const SG::ReadCondHandleKey<CondAttrListCollection>>>& rkeys, std::unique_ptr<T> obj) const {
    if(wkey.empty()) return StatusCode::SUCCESS; // no creation to do
    SG::WriteCondHandle<T> wh{wkey, ctx};
    if(wh.isValid()) return StatusCode::SUCCESS; // condition already valid, no update needed
    std::map<std::string, const CondAttrListCollection *> listMap;
    for(auto rkey : rkeys) { // note: rkey is a reference_wrapper round an actual sg key, hence the use of .get() below
        if(rkey.get().empty()) continue;
        SG::ReadCondHandle <CondAttrListCollection> rh(rkey.get(), ctx);
        CHECK(rh.isValid());
        ATH_MSG_DEBUG("Size of CondAttrListCollection " << rh.fullKey() << " = " << rh->size());
        wh.addDependency(rh); // will become invalid when read handle validity ends
        listMap[rkey.get().key()] = *rh;
    }
    if (listMap.empty()) return StatusCode::SUCCESS;
    if(!obj) obj = std::make_unique<T>();
    obj->makeTransient(listMap);
    CHECK(wh.record(std::move(obj)));
    ATH_MSG_DEBUG("recorded new " <<  wkey.key()  << " with range " << wh.getRange() << " into Conditions Store");
    return StatusCode::SUCCESS;
}

StatusCode  L1CaloCondAlg::execute (const EventContext& ctx) const
{
  
    ATH_MSG_DEBUG( "start execute " << name() );
    ATH_MSG_DEBUG("readConditions() ");

    std::string timingRegime = m_timingRegime;
    std::string strategy = m_strategy;

    if(!m_derivedRunPars.empty()) {
      SG::WriteCondHandle<L1CaloDerivedRunParsContainer> writeHandleDerRunPars{m_derivedRunParsContainer, ctx};
      if(!writeHandleDerRunPars.isValid()) { // condition needs updating
          SG::ReadCondHandle <CondAttrListCollection> readHandleDerRunPars(m_derivedRunPars, ctx);
          CHECK(readHandleDerRunPars.isValid());
          ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandleDerRunPars.fullKey() << " = " <<  readHandleDerRunPars->size());
          writeHandleDerRunPars.addDependency(readHandleDerRunPars); // will become invalid when read handle validity ends
          auto writeCdoDerRunPars = std::make_unique<L1CaloDerivedRunParsContainer>();
          std::map<std::string, const CondAttrListCollection *> listMap;
          listMap[m_derivedRunPars.key()] = *readHandleDerRunPars;
          writeCdoDerRunPars->makeTransient(listMap);
          if (timingRegime.empty()) {
              timingRegime = std::cbegin(*writeCdoDerRunPars)->timingRegime();
          }
          CHECK(writeHandleDerRunPars.record(std::move(writeCdoDerRunPars)));
          ATH_MSG_DEBUG("recorded new " <<  m_derivedRunParsContainer.key()  << " with range " << writeHandleDerRunPars.getRange() << " into Conditions Store");

          ATH_MSG_DEBUG("timing regime --> "<< timingRegime );

      }
    }


    if(!m_pprChanStrategy.empty()) {
        SG::WriteCondHandle<L1CaloPprChanStrategyContainer> writeHandlePprChanStrategy{m_pprChanStrategyContainer, ctx};
        if(!writeHandlePprChanStrategy.isValid()) { // condition needs updating
            SG::ReadCondHandle<CondAttrListCollection> readHandlePprChanStrategy(m_pprChanStrategy, ctx);
            CHECK(readHandlePprChanStrategy.isValid());
            ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandlePprChanStrategy.fullKey() << " = " <<  readHandlePprChanStrategy->size());
            writeHandlePprChanStrategy.addDependency(readHandlePprChanStrategy); // will become invalid when read handle validity ends
            auto writeCdoPprChanStrategy = std::make_unique<L1CaloPprChanStrategyContainer>();
            std::map<std::string, const CondAttrListCollection *> listMap;
            listMap[m_pprChanStrategy.key()] = *readHandlePprChanStrategy;
            writeCdoPprChanStrategy->makeTransient(listMap);
            if (strategy.empty()) {
                for(const auto& it: *writeCdoPprChanStrategy){
                    if (it.timingRegime() == timingRegime){
                        strategy = it.strategy();
                    }
                }
            }
            CHECK(writeHandlePprChanStrategy.record(std::move(writeCdoPprChanStrategy)));
            ATH_MSG_DEBUG("recorded new " <<  m_pprChanStrategyContainer.key()  << " with range " << writeHandlePprChanStrategy.getRange() << " into Conditions Store");

            ATH_MSG_DEBUG("strategy selected --> " << strategy);




        }
    }

    CHECK( updateCond(ctx, m_disabledTowersContainer,{m_disabledTowers}) );
    CHECK( updateCond(ctx, m_pprChanDefaultsContainer,{m_pprChanDefaults}) );
    CHECK( updateCond(ctx, m_ppmFineTimeRefsContainer,{m_ppmFineTimeRefs}) );
    CHECK( updateCond(ctx, m_runParametersContainer,{m_runParameters}) );
    CHECK( updateCond(ctx, m_ppmDeadChannelsContainer,{m_ppmDeadChannels}) );
    CHECK( updateCond(ctx, m_readoutConfigContainer,{m_readoutConfig}) );
    CHECK( updateCond(ctx, m_readoutConfigContainerJSON,{m_readoutConfigJSON}) );

    if (timingRegime == "") timingRegime="Physics"; // default to physics

    const SG::ReadCondHandleKeyArray<CondAttrListCollection>* pprKeys = nullptr;
    if (timingRegime == "Physics") {
        pprKeys = &m_physicsKeys;
    } else if (timingRegime == "Calib1") {
        pprKeys = &m_calib1Keys;
    } else if (timingRegime == "Calib2") {
        pprKeys = &m_calib2Keys;
    } else {
        ATH_MSG_ERROR( "Bad timing regime " << timingRegime << "; must be one of Physics, Calib1, Calib2" );
        return StatusCode::FAILURE;
    }

    CHECK( updateCond(ctx, m_pprChanCalibContainer,{pprKeys->at(PPRCHANCALIB)}) );
    CHECK( updateCond(ctx, m_pprDisabledChannelContainer,{pprKeys->at(PPRCHANCALIB),m_disabledTowers,m_ppmDeadChannels}) );

    // only need this logic if the conditionscontainerrun2 has expired ...
    if(!m_pprConditionsContainer.empty() && !SG::WriteCondHandle<L1CaloPprConditionsContainerRun2>{m_pprConditionsContainer, ctx}.isValid()) {
        std::map <L1CaloPprConditionsContainerRun2::eCoolFolders, std::string> folderKeyMap;
        folderKeyMap[L1CaloPprConditionsContainerRun2::ePprChanDefaults] = m_pprChanDefaults.key();
        if (strategy.empty()) {
            folderKeyMap[L1CaloPprConditionsContainerRun2::ePprChanCalib] = pprKeys->at(PPRCHANCALIB).key();
            auto obj = std::make_unique<L1CaloPprConditionsContainerRun2>(folderKeyMap); // must construct here to pass map to constructor
            CHECK(updateCond(ctx, m_pprConditionsContainer, {m_pprChanDefaults, pprKeys->at(PPRCHANCALIB)}, std::move(obj)));
        } else {
            if (strategy != "HighMu" && strategy != "LowMu") {
                ATH_MSG_ERROR("Invalid strategy: " << strategy << " (must be HighMu or LowMu)");
                return StatusCode::FAILURE;
            }
            folderKeyMap[L1CaloPprConditionsContainerRun2::ePprChanCalibCommon] = pprKeys->at(PPRCHANCOMMON).key();
            folderKeyMap[L1CaloPprConditionsContainerRun2::ePprChanCalibStrategy] = pprKeys->at(
                    strategy == "HighMu" ? PPRCHANHIGHMU : PPRCHANLOWMU).key();

            auto obj = std::make_unique<L1CaloPprConditionsContainerRun2>(folderKeyMap);

            CHECK(updateCond(ctx, m_pprConditionsContainer, {m_pprChanDefaults, pprKeys->at(PPRCHANCOMMON), pprKeys->at(
                    strategy == "HighMu" ? PPRCHANHIGHMU : PPRCHANLOWMU)}, std::move(obj)));
        }
    }

   return StatusCode::SUCCESS;

}
