/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JfexInputMonitorAlgorithm.h"

JfexInputMonitorAlgorithm::JfexInputMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{
}

StatusCode JfexInputMonitorAlgorithm::initialize() {

    ATH_MSG_DEBUG("Initializing JfexInputMonitorAlgorithm algorithm with name: "<< name());
    ATH_MSG_DEBUG("JfexInputMonitorAlgorith::initialize");
    ATH_MSG_DEBUG("Package Name "<< m_Grouphist);
    ATH_MSG_DEBUG("jFexDataTowerKey: "<< m_jFexDataTowerKey);
    ATH_MSG_DEBUG("jFexEmulatedTowerKey: "<< m_jFexEmulatedTowerKey);
    //ATH_CHECK(m_monTool.retrieve());
    //ATH_MSG_DEBUG("Logging errors to " << m_monTool.name() << " monitoring tool");


    // we initialise all the containers that we need
    ATH_CHECK( m_jFexDataTowerKey.initialize() );
    ATH_CHECK( m_jFexEmulatedTowerKey.initialize() );
    
    // decor keys
    ATH_CHECK( m_jtowerEtMeVdecorKey.initialize() );
    ATH_CHECK( m_SCellEtMeVdecorKey.initialize()  );
    ATH_CHECK( m_TileEtMeVdecorKey.initialize()   );
    ATH_CHECK( m_jTowerEtdecorKey.initialize()    );

    ATH_CHECK( m_bcContKey.initialize() );

    // error counter
    m_nJFexWarnVar = 0;

    return AthMonitorAlgorithm::initialize();
}

StatusCode JfexInputMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {

    ATH_MSG_DEBUG("JfexInputMonitorAlgorithm::fillHistograms");

    // Access jFex tower container
    SG::ReadHandle<xAOD::jFexTowerContainer> jFexTowerContainer{m_jFexDataTowerKey, ctx};
    if(!jFexTowerContainer.isValid()) {
        ATH_MSG_WARNING("No jFex Tower container valid in storegate with key: "<< m_jFexDataTowerKey);
        return StatusCode::FAILURE;
    }

    // mismatches can be caused by recent/imminent OTF maskings, so track timings
    auto timeSince = Monitored::Scalar<int>("timeSince", -1);
    auto timeUntil = Monitored::Scalar<int>("timeUntil", -1);
    SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, ctx };
    if(larBadChan.isValid()) {
        timeSince = ctx.eventID().time_stamp() - larBadChan.getRange().start().time_stamp();
        timeUntil = larBadChan.getRange().stop().time_stamp() - ctx.eventID().time_stamp();
    }
    auto EventType = Monitored::Scalar<std::string>("EventType","Normal");
    if((timeSince>=0&&timeSince<10)) EventType="JustAfterMask";
    else if((timeUntil>=0&&timeUntil<10)) EventType="JustBeforeMask";

    auto Decision = Monitored::Scalar<std::string>("Decision","");

    //Run the monitoring only when the input data is filled (it is pre-scaled), otherwise skip
    if(jFexTowerContainer->empty()){
        ATH_MSG_DEBUG("number of jfex towers = "<< jFexTowerContainer->size());
        Decision += "MissingReadout;";
        fill(m_Grouphist,EventType,Decision);
        return StatusCode::SUCCESS;
    }

    SG::ReadHandle<xAOD::jFexTowerContainer> jFexEmulatedTowerContainer{m_jFexEmulatedTowerKey, ctx};
    if(!jFexEmulatedTowerContainer.isValid()) {
        ATH_MSG_ERROR("No jFex Tower container valid in storegate with key: "<< m_jFexEmulatedTowerKey);
        return StatusCode::FAILURE;
    }
    
   
    SG::ReadDecorHandle<xAOD::jFexTowerContainer, int >   jTowerEtMeV     (m_jtowerEtMeVdecorKey, ctx);
    if(!jTowerEtMeV.isAvailable()) {
        ATH_MSG_WARNING("Decorated variable jTowerEtMeV is not available. Is "<< m_jFexDataTowerKey << " decorated?");
        return StatusCode::SUCCESS;
    }
    
    SG::ReadDecorHandle<xAOD::jFexTowerContainer, float > SCellEtMeV      (m_SCellEtMeVdecorKey , ctx);
    if(!SCellEtMeV.isAvailable()) {
        ATH_MSG_WARNING("Decorated variable SCellEtMeV is not available. Is "<< m_jFexDataTowerKey << " decorated?");
        return StatusCode::SUCCESS;
    }
        
    SG::ReadDecorHandle<xAOD::jFexTowerContainer, float > TileEtMeV       (m_TileEtMeVdecorKey  , ctx);
    if(!TileEtMeV.isAvailable()) {
        ATH_MSG_WARNING("Decorated variable TileEtMeV is not available. Is "<< m_jFexDataTowerKey << " decorated?");
        return StatusCode::SUCCESS;
    }
    
    SG::ReadDecorHandle<xAOD::jFexTowerContainer, int >   emulated_jtowerEt (m_jTowerEtdecorKey   , ctx);
    if(!emulated_jtowerEt.isAvailable()) {
        ATH_MSG_WARNING("Decorated variable emulated_jtowerEt is not available. Is "<< m_jFexDataTowerKey << " decorated?");
        return StatusCode::SUCCESS;
    }
    
    
   
    
    
    // monitored variables for histograms 
    auto nJfexTowers = Monitored::Scalar<int>("NJfexTowers",0.0);
    auto Towereta = Monitored::Scalar<float>("TowerEta",0.0);
    auto Towerphi = Monitored::Scalar<float>("TowerPhi",0.0);
    auto ToweretaDeco = Monitored::Scalar<float>("TowerEtaDeco",0.0);
    auto TowerphiDeco = Monitored::Scalar<float>("TowerPhiDeco",0.0);
    auto DataEt = Monitored::Scalar<float>("DataEt",0.0);
    auto EmulatedEt = Monitored::Scalar<float>("EmulatedEt",0.0);
    auto DataEt_Tile = Monitored::Scalar<float>("DataEt_Tile",0.0);
    auto EmulatedEt_Tile = Monitored::Scalar<float>("EmulatedEt_Tile",0.0);
    auto region = Monitored::Scalar<int>("region",0.0);
    auto type = Monitored::Scalar<int>("type",0.0);
    auto frac_SCellSum = Monitored::Scalar<float>("frac_SCellSum",0.0);
    auto ToweretaInvalid = Monitored::Scalar<float>("TowerEtaInvalid",-99.0);
    auto TowerPhiInvalid = Monitored::Scalar<float>("TowerPhiInvalid",-99.0);
    auto ToweretaEmpty = Monitored::Scalar<float>("TowerEtaEmpty",-99.0);
    auto TowerPhiEmpty = Monitored::Scalar<float>("TowerPhiEmpty",-99.0);
    auto Towerglobaleta = Monitored::Scalar<int>("TowerGlobalEta",0);
    auto Towerglobalphi = Monitored::Scalar<uint32_t>("TowerGlobalPhi",0);
    auto Towermodule = Monitored::Scalar<uint8_t>("TowerModule",0);
    auto Towerfpga = Monitored::Scalar<uint8_t>("TowerFpga",0);
    auto Towerchannel = Monitored::Scalar<uint8_t>("TowerChannel",0);
    auto TowerdataID = Monitored::Scalar<uint8_t>("TowerDataID",0);
    auto TowersimulationID = Monitored::Scalar<uint32_t>("TowerSimulationID",0.0);
    auto Towercalosource = Monitored::Scalar<uint8_t>("TowerCalosource",0);

    auto Toweretcount_barrel = Monitored::Scalar<uint16_t>("TowerEtcount_barrel",0);
    auto Toweretcount_tile = Monitored::Scalar<uint16_t>("TowerEtcount_tile",0);
    auto Toweretcount_emec = Monitored::Scalar<uint16_t>("TowerEtcount_emec",0);
    auto Toweretcount_hec = Monitored::Scalar<uint16_t>("TowerEtcount_hec",0);
    auto Toweretcount_fcal1 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal1",0);
    auto Toweretcount_fcal2 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal2",0);
    auto Toweretcount_fcal3 = Monitored::Scalar<uint16_t>("TowerEtcount_fcal3",0);

    auto Towersaturationflag = Monitored::Scalar<char>("TowerSaturationflag",0);

    unsigned int njfexTowers=jFexTowerContainer->size();
    nJfexTowers=njfexTowers;
    fill(m_Grouphist,nJfexTowers);
    
    std::unordered_map< int, const xAOD::jFexTower* >dataTowers_map;

    for(const xAOD::jFexTower* dataTower : *jFexTowerContainer) {
        
        //Adding 1e-5 for plotting style 
        Towereta=dataTower->eta()+1e-5;
        Towerphi=dataTower->phi();
        
        if(dataTower->isCore()){
            const int TTID = dataTower->jFEXtowerID();
            const int source = dataTower->Calosource();
            int code = codedVal(TTID, source);
            
            
            auto it_TTower2SCells = dataTowers_map.find( code );
            if(it_TTower2SCells == dataTowers_map.end()){
                dataTowers_map[ code ] = dataTower;
            }
            else{
                if ( m_nJFexWarnVar++ < m_nJFexWarnMax ) {
                    ATH_MSG_WARNING("DataTower: "<<TTID<< " with Calosource: "<<source<< "is repeated. It shouldn't! ");
                }
            }
            
            DataEt         = jTowerEtMeV(*dataTower);
            EmulatedEt     = SCellEtMeV(*dataTower);

            
            // If Tile (Calosource == 1) then use Tile information
            if(source == 1){
                EmulatedEt      = TileEtMeV(*dataTower);
               
                DataEt_Tile     = DataEt/Gaudi::Units::GeV;
                EmulatedEt_Tile = EmulatedEt/Gaudi::Units::GeV;
                fill(m_Grouphist+"_details_"+std::to_string(source),DataEt_Tile,EmulatedEt_Tile); 
            }
            else{
                fill(m_Grouphist+"_details_"+std::to_string(source),DataEt,EmulatedEt); 
            }          
                                    
            fill(m_Grouphist+"_decorated_all" ,DataEt,EmulatedEt);
            
             
            
            //Looking at decorated variables. Is real time path data Et the same as the simulated one (encoded Et)?
            if( (dataTower->et_count()).at(0) != emulated_jtowerEt(*dataTower)) {
                
                std::string location = "EM layer";
                if(dataTower->Calosource()== 1 || dataTower->Calosource()== 3 || dataTower->Calosource() == 5 ||  dataTower->Calosource() == 6 ){ // if source is Tile, HEC, FCAL2 and FCAL3 then location is HAD
                    location="HAD layer";
                } 
                
                if( (dataTower->et_count()).at(0) != m_InvalidCode ){
                    if ( m_nJFexWarnVar++ < m_nJFexWarnMax ) {
                                ATH_MSG_WARNING("Tower:"<< TTID << " source:"<< +dataTower->Calosource() << " for eventNumber:"<< GetEventInfo(ctx)->eventNumber()<< " and LB:"<<GetEventInfo(ctx)->lumiBlock() << ". DataTower Et:"<< (dataTower->et_count()).at(0) <<"/"<< DataEt<<" MeV vs EmulatedTower Et:" << emulated_jtowerEt(*dataTower)<<"/"<< EmulatedEt<< " MeV");
                    }
                    frac_SCellSum = DataEt != 0 ? (EmulatedEt - DataEt)/DataEt : 0;
                    fill(m_Grouphist+"_decorated",Towereta,Towerphi,DataEt,EmulatedEt,frac_SCellSum);

                    if(source!=1) {
                        if(std::string(Decision).find("LArMismatches")==std::string::npos)  Decision+="LArMismatches;";
                    } else {
                        if(std::string(Decision).find("TileMismatches")==std::string::npos)  Decision+="TileMismatches;";
                    }


                    genError("Input_Mismatch", location);
                }
                else{
                    if(source!=1) {
                        if(std::string(Decision).find("LArInvalid")==std::string::npos)  Decision+="LArInvalid;";
                    } else {
                        if(std::string(Decision).find("TileInvalid")==std::string::npos)  Decision+="TileInvalid;";
                    }
                    genError("Input_Invalids", location);
                }
                
                region = source;
                type = (dataTower->et_count()).at(0) == m_InvalidCode ? 0 : 1;
                
                fill(m_Grouphist,region,type);
                    
            }
        }
        
        fill(m_Grouphist,Towereta,Towerphi);

        Towerglobaleta=dataTower->globalEta();
        Towerglobalphi=dataTower->globalPhi();
        fill(m_Grouphist,Towerglobaleta,Towerglobalphi);

        Towermodule=dataTower->module();
        fill(m_Grouphist,Towermodule);

        Towerfpga=dataTower->fpga();
        fill(m_Grouphist,Towerfpga);

        Towerchannel=dataTower->channel();
        fill(m_Grouphist,Towerchannel);

        TowerdataID=dataTower->jFEXdataID();
        fill(m_Grouphist,TowerdataID);

        TowersimulationID=dataTower->jFEXtowerID();
        fill(m_Grouphist,TowersimulationID);

        Towercalosource=dataTower->Calosource();
        fill(m_Grouphist,Towercalosource);

        std::vector<uint16_t> Toweret_count=dataTower->et_count();
        Toweretcount_barrel=Toweret_count.at(0);
        Toweretcount_tile=Toweret_count.at(0);
        Toweretcount_emec=Toweret_count.at(0);
        Toweretcount_hec=Toweret_count.at(0);
        Toweretcount_fcal1=Toweret_count.at(0);
        Toweretcount_fcal2=Toweret_count.at(0);
        Toweretcount_fcal3=Toweret_count.at(0);
        
        if(dataTower->Calosource() != 1){ // Only for LATOME, not Tile
            if(Toweret_count.at(0) == m_InvalidCode){
                ToweretaInvalid=dataTower->eta();
                TowerPhiInvalid=dataTower->phi();
                fill(m_Grouphist,ToweretaInvalid,TowerPhiInvalid);
            }
            
            if(Toweret_count.at(0) == m_EmptyCode){
                ToweretaEmpty=dataTower->eta();
                TowerPhiEmpty=dataTower->phi();
                fill(m_Grouphist,ToweretaEmpty,TowerPhiEmpty);
            }            
        }


        if(Towercalosource==0) {
            fill(m_Grouphist,Toweretcount_barrel);
        }
        if(Towercalosource==1) {
            fill(m_Grouphist,Toweretcount_tile);
        }
        if(Towercalosource==2) {
            fill(m_Grouphist,Toweretcount_emec);
        }
        if(Towercalosource==3) {
            fill(m_Grouphist,Toweretcount_hec);
        }
        if(Towercalosource==4) {
            fill(m_Grouphist,Toweretcount_fcal1);
        }
        if(Towercalosource==5) {
            fill(m_Grouphist,Toweretcount_fcal2);
        }
        if(Towercalosource==6) {
            fill(m_Grouphist,Toweretcount_fcal3);
        }

        std::vector<char> Tower_saturationflag=dataTower->isjTowerSat();
        Towersaturationflag=Tower_saturationflag.at(0);
        fill(m_Grouphist,Towersaturationflag);
        
        

    }
    
    for(const xAOD::jFexTower* emulTower : *jFexEmulatedTowerContainer) {
        
        

        if(emulTower->isCore()){
            
            auto it_TTower2SCells = dataTowers_map.find( codedVal(emulTower->jFEXtowerID(), emulTower->Calosource() ) );
            
            if(it_TTower2SCells == dataTowers_map.end() ){
                if ( m_nJFexWarnVar++ < m_nJFexWarnMax ) {
                    ATH_MSG_WARNING("DataTower: "<<emulTower->jFEXtowerID()<< " with Calosource: "<<emulTower->Calosource()<< " does not exists. It should! Investigate ");
                }
                continue;
            }
            
            const xAOD::jFexTower* dataTower = it_TTower2SCells->second;       
            
            if(emulTower->et_count().at(0) != dataTower->et_count().at(0) ){
                //Adding 1e-5 for plotting style 
                Towereta=emulTower->eta()+1e-5;
                Towerphi=emulTower->phi();            

                fill(m_Grouphist+"_emulated",Towereta,Towerphi);                
            }   
            
            if(emulTower->et_count().at(0) != emulated_jtowerEt(*dataTower) ){
                //Adding 1e-5 for plotting style 
                ToweretaDeco=emulTower->eta()+1e-5;
                TowerphiDeco=emulTower->phi();            

                fill(m_Grouphist+"_emulated",ToweretaDeco,TowerphiDeco);                
            }   
            
             
 
        }
        
    }

    if(std::string(Decision)=="") Decision = "AllOk";
    fill(m_Grouphist,EventType,Decision);

    return StatusCode::SUCCESS;

}

int JfexInputMonitorAlgorithm::codedVal(const int ID, const int source) const{
    return (ID<<4) + source;
}


void  JfexInputMonitorAlgorithm::genError(const std::string& /*location*/, const std::string& /*title*/) const {
//    Monitored::Group(m_monTool,
//                     Monitored::Scalar("genLocation",location.empty() ? std::string("UNKNOWN") : location),
//                     Monitored::Scalar("genType",title.empty()    ? std::string("UNKNOWN") : title)
//                    );
}
