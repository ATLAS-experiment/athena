/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloNoiseCompCondAlg.h"
#include "CaloEvent/CaloCell.h"
#include "CaloIdentifier/CaloIdManager.h"
// For Gaudi
#include "GaudiKernel/MsgStream.h"
#include "TileIdentifier/TileRawChannelUnit.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "AthenaKernel/IOVInfiniteRange.h"

// Exceptions
#include "LArElecCalib/LArConditionsException.h"

using Gaudi::Units::GeV;
 
//////////////////////////////////////////////////

CaloNoiseCompCondAlg::CaloNoiseCompCondAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm( name, pSvcLocator),
    m_atlas_id(nullptr),
    m_calo_id_man(nullptr),
    m_lar_em_id(nullptr),
    m_lar_hec_id(nullptr),
    m_lar_fcal_id(nullptr),
    m_calocell_id(nullptr),
    m_Adc2MeVFactor(0),
    m_RMSpedestal(0),
    m_nsamples(0),
    m_SigmaNoise(0.),
    m_fSampl(0),
    m_AdcPerMev(5.*GeV),
    m_MinBiasRMS(0)

{
}
 
//////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::initialize() {

   ATH_CHECK( detStore()->retrieve( m_calo_id_man ) );
   m_lar_em_id   = m_calo_id_man->getEM_ID();
   m_lar_hec_id  = m_calo_id_man->getHEC_ID();
   m_lar_fcal_id = m_calo_id_man->getFCAL_ID();
   m_calosupercell_id = m_calo_id_man->getCaloCell_SuperCell_ID();

   ATH_CHECK(m_LArOFCObjKey.initialize());
   ATH_CHECK(m_shapeKey.initialize());
   ATH_CHECK(m_fSamplKey.initialize());
   ATH_CHECK(m_LArMinBiasObjKey.initialize());
   ATH_CHECK(m_cablingKey.initialize());
   ATH_CHECK(m_adc2mevKey.initialize());
   ATH_CHECK(m_pedestalKey.initialize());
   ATH_CHECK(m_acorrKey.initialize());

   //diagnostic
   m_diagnostic[CaloGain::LARHIGHGAIN]  =m_DiagnosticHG;
   m_diagnostic[CaloGain::LARMEDIUMGAIN]=m_DiagnosticMG;
   m_diagnostic[CaloGain::LARLOWGAIN]   =m_DiagnosticLG;  

   //set calohash maximums
   m_LArHashMax =0;
   m_LArHashMax  =   m_lar_em_id->channel_hash_max()
                     + m_lar_hec_id->channel_hash_max()
                     + m_lar_fcal_id->channel_hash_max();
   m_CaloHashMax = m_LArHashMax;
 
   //set calohash minimum
   m_CaloHashMin = 0;
 
   ATH_MSG_DEBUG( " => CaloHashMin= " <<m_CaloHashMin << " CaloHashMax= " <<m_CaloHashMax ); 
   //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
   //gain-thresholds 
   m_LowGainThresh[CaloCell_ID::LAREM]    = 3900;//ADC counts in MediumGain 
   m_HighGainThresh[CaloCell_ID::LAREM]   = 1300;//ADC counts in MediumGain
   m_LowGainThresh[CaloCell_ID::LARHEC]   = 2500;//ADC counts in MediumGain 
   m_HighGainThresh[CaloCell_ID::LARHEC]  = 0;//-> high-gain never used for HEC

   m_LowGainThresh[CaloCell_ID::LARFCAL]  = 2000.;//ADC counts
   m_HighGainThresh[CaloCell_ID::LARFCAL] = 1100.;//ADC counts

   m_LowGainThresh[CaloCell_ID::TILE]  = 0.;// unit ?
   m_HighGainThresh[CaloCell_ID::TILE] = 0.;//
   //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
   m_highestGain[CaloCell_ID::LAREM]   = CaloGain::LARHIGHGAIN;
   m_highestGain[CaloCell_ID::LARHEC]  = CaloGain::LARMEDIUMGAIN;
   m_highestGain[CaloCell_ID::LARFCAL] = CaloGain::LARHIGHGAIN;
   m_highestGain[CaloCell_ID::TILE]    = CaloGain::TILEHIGHHIGH;

   //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

   ATH_CHECK(m_outputElecKey.initialize());
   ATH_CHECK(m_outputPileupKey.initialize());

   ATH_CHECK( m_caloMgrKey.initialize() );

   return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::execute() {
   
   const EventContext& ctx = Gaudi::Hive::currentContext();

   SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey};
   if(!caloMgrHandle.isValid()) {
      ATH_MSG_ERROR( "Do not have CaloDetDescrMgr");
      return StatusCode::FAILURE;
   }

   m_calo_dd_man  = *caloMgrHandle;
   m_calocell_id = m_calo_dd_man->getCaloCell_ID();


   //Set up write handles
   SG::WriteCondHandle<CaloNoise> writeElecHandle{m_outputElecKey,ctx};
   SG::WriteCondHandle<CaloNoise> writePileupHandle{m_outputPileupKey,ctx};

   if (writeElecHandle.isValid() && writePileupHandle.isValid()) {
     ATH_MSG_DEBUG("Found valid write handles");
     return StatusCode::SUCCESS;
   }

   //Start with infinite range and narrow it down
   const EventIDRange fullRange=IOVInfiniteRange::infiniteMixed();
   writeElecHandle.addDependency (fullRange);
   writePileupHandle.addDependency (fullRange);

   // Read input conditions
   auto cablingHdl = SG::ReadCondHandle<LArOnOffIdMapping>(m_cablingKey, ctx);
   if(!cablingHdl.isValid()) {
      ATH_MSG_ERROR( "Do not have cabling");
      return StatusCode::FAILURE;
   }
   m_cabling=*cablingHdl;
   writeElecHandle.addDependency (cablingHdl);
   writePileupHandle.addDependency (cablingHdl);

   SG::ReadCondHandle<LArADC2MeV> adc2mevHdl (m_adc2mevKey, ctx);
   const LArADC2MeV* adc2mev{*adc2mevHdl};
   if(!adc2mev) {
      ATH_MSG_ERROR( "Do not have adc2mev");
      return StatusCode::FAILURE;
   }
   writeElecHandle.addDependency (adc2mevHdl);
   writePileupHandle.addDependency (adc2mevHdl);

   auto pedHdl=SG::ReadCondHandle<ILArPedestal>(m_pedestalKey, ctx);
   if(!pedHdl.isValid()){
      ATH_MSG_ERROR( "Do not have pedestals");
      return StatusCode::FAILURE;
   }
   m_ped=*pedHdl;
   writeElecHandle.addDependency (pedHdl);

   auto acorrHdl=SG::ReadCondHandle<ILArAutoCorr>(m_acorrKey, ctx);
   if(!acorrHdl.isValid()){
      ATH_MSG_ERROR( "Do not have autocorr");
      return StatusCode::FAILURE;
   }
   m_acorr=*acorrHdl;
   writeElecHandle.addDependency (acorrHdl);

   auto ofcHdl=SG::ReadCondHandle<ILArOFC>(m_LArOFCObjKey, ctx);
   if(!ofcHdl.isValid()){
      ATH_MSG_ERROR( "Do not have ofc");
      return StatusCode::FAILURE;
   }
   m_ofccond=*ofcHdl;
   writeElecHandle.addDependency (ofcHdl);
   writePileupHandle.addDependency (ofcHdl);

   auto shapeHdl=SG::ReadCondHandle<ILArShape>(m_shapeKey, ctx);
   if(!shapeHdl.isValid()){
      ATH_MSG_ERROR( "Do not have shape");
      return StatusCode::FAILURE;
   }
   m_shapecond=*shapeHdl;
   writeElecHandle.addDependency (shapeHdl);
   writePileupHandle.addDependency (shapeHdl);

   auto fsamplHdl=SG::ReadCondHandle<ILArfSampl>(m_fSamplKey, ctx);
   if(!fsamplHdl.isValid()){
      ATH_MSG_ERROR( "Do not have fSampl");
      return StatusCode::FAILURE;
   }
   m_fsamplcond=*fsamplHdl;
   writePileupHandle.addDependency (fsamplHdl);


   auto minbiasHdl=SG::ReadCondHandle<ILArMinBias>(m_LArMinBiasObjKey, ctx);
   if(!minbiasHdl.isValid()){
      ATH_MSG_ERROR( "Do not have minbias");
      return StatusCode::FAILURE;
   }
   m_minbias=*minbiasHdl;
   writePileupHandle.addDependency (minbiasHdl);

   ATH_CHECK(this->initData(adc2mev));

   //Create the CaloNoise CDO:
   std::unique_ptr<CaloNoise> elecNoiseObj=std::make_unique<CaloNoise>(m_LArHashMax,m_nGains, 0,4,
                                       m_calocell_id,CaloNoise::ELEC);
   std::unique_ptr<CaloNoise> pileupNoiseObj=std::make_unique<CaloNoise>(m_LArHashMax,m_nGains, 0,4,
                                       m_calocell_id,CaloNoise::PILEUP);
   //Get writeable access to underlying storage (boost::multi_array)
   auto& elecnoise =  elecNoiseObj->larStorage();
   auto& pileupnoise =  pileupNoiseObj->larStorage();

   for(unsigned ihash=m_CaloHashMin; ihash < m_CaloHashMax; ++ihash) {

      Identifier id=m_calocell_id->cell_id(ihash);
      auto dde=m_calo_dd_man->get_element(id);
      auto elec3gains = elecNoiseRMS3gains(dde);
      auto pns = pileupNoiseRMS(dde,m_Nminbias);

      for (unsigned igain=0;igain<m_nGains;++igain) {
        elecnoise[igain][ihash] = elec3gains[igain]; 
        pileupnoise[igain][ihash] = pns;
      }
   }

   // store output
   ATH_CHECK(writeElecHandle.record(std::move(elecNoiseObj)));
   ATH_MSG_INFO("recorded new CaloNoise object with key " << writeElecHandle.key() << " and range " << writeElecHandle.getRange());

   ATH_CHECK(writePileupHandle.record(std::move(pileupNoiseObj)));
   ATH_MSG_INFO("recorded new CaloNoise object with key " << writePileupHandle.key() << " and range " << writePileupHandle.getRange());

   return StatusCode::SUCCESS;
}



//////////////////////////////////////////////////
 
StatusCode 
CaloNoiseCompCondAlg::initContainers()
{
  //initialize the maps m_ElecNoiseContainer and m_ScaleContainer 
  //(assuming type of elements of the containers is the same for LAr)

  MsgStream log( msgSvc(), name() );
  ATH_MSG_INFO( "initContainers() begin " );

  // intialise indices
  ATH_CHECK(this->initIndex()); 

  //::::::::::::::::::::::::::::::::::::::     
  m_elecNoiseRAWContainer.resize(m_idSymmCaloHashContainer.size());
  m_elecNoiseCELLContainer.resize(m_idSymmCaloHashContainer.size());
  m_pileupNoiseContainer.resize(m_idSymmCaloHashContainer.size());
  m_adc2mevContainer.resize(m_idSymmCaloHashContainer.size());
  //::::::::::::::::::::::::::::::::::::::
  ATH_MSG_INFO( "initContainers() end : " <<" size of containers = " <<m_idSymmCaloHashContainer.size() );
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::initIndex() {
  //::::::::::::::::::::::::::::::::::::::
  m_indexContainer.clear();
  m_indexContainer.resize(m_CaloHashMax,
                          static_cast<unsigned int> (-1));

  //maybe the other container sould be reset
  m_idSymmCaloHashContainer.clear();
  m_idSymmCaloHashContainer.reserve(5000);

  
  
  for (unsigned int intIdCaloHash=m_CaloHashMin; intIdCaloHash<m_CaloHashMax;
       ++intIdCaloHash)
  {  
    

    IdentifierHash idCaloHash=static_cast<IdentifierHash>(intIdCaloHash);

    //    std::cout << "DRDEBUG in initIndex loop " << intIdCaloHash << std::endl ;

    
    // initialize the vector of indexes (big vector without symmetry)




    // o idCaloHash -> id -> idSymm (symmetry phi->0 and z->|z|)
    // o idSymm -> idSymmCaloHash
    // o idSymmCaloHash stored in m_idSymmCaloHashContainer
    // o an index is associated to an idSymmCaloHash
    // o index stored in m_indexContainer
    
    CaloCell_ID::SUBCALO iCalo = this->caloNum(idCaloHash); 
    Identifier     id    = m_calocell_id->cell_id(idCaloHash);
    Identifier     regId;
    Identifier     idSymm;
    IdentifierHash idSymmCaloHash;

    if(m_UseSymmetry){
      if(iCalo==CaloCell_ID::LAREM) 
      {
        int barrel_ec = m_lar_em_id->barrel_ec(id);
        int sampling  = m_lar_em_id->sampling(id);
        int region    = m_lar_em_id->region(id);
        int eta       = m_lar_em_id->eta(id);
        regId         = m_lar_em_id->region_id(abs(barrel_ec),sampling,region);
        idSymm        = m_lar_em_id->channel_id(regId,
                                                eta,
                                                m_calocell_id->phi_min(regId));
        idSymmCaloHash= m_calocell_id->calo_cell_hash(idSymm);
      }
      else if(iCalo==CaloCell_ID::LARHEC) 
      {
        int pos_neg  = m_lar_hec_id->pos_neg(id);
        int sampling = m_lar_hec_id->sampling(id);
        int region   = m_lar_hec_id->region(id);
        int eta      = m_lar_hec_id->eta(id);
        regId        = m_lar_hec_id->region_id(abs(pos_neg),sampling,region);
        idSymm       = m_lar_hec_id->channel_id(regId,
                                                eta,
                                                m_calocell_id->phi_min(regId));
        idSymmCaloHash= m_calocell_id->calo_cell_hash(idSymm);
      }
      else if(iCalo==CaloCell_ID::LARFCAL) 
      {
        int pos_neg = m_lar_fcal_id->pos_neg(id);
        int module  = m_lar_fcal_id->module(id);
        int eta     = m_lar_fcal_id->eta(id);
        int phi     = m_lar_fcal_id->phi(id);
        if(phi>7) phi = phi-8; //as in LArMCSymTool
        regId       = m_lar_fcal_id->module_id(abs(pos_neg),module);
        idSymm      = m_lar_fcal_id->channel_id(regId,
                                                eta,
                                                phi);
        idSymmCaloHash= m_calocell_id->calo_cell_hash(idSymm);
      }
      else if(iCalo==CaloCell_ID::TILE) 
      {
        idSymm         = id;
        idSymmCaloHash = m_calocell_id->calo_cell_hash(idSymm);
      }
      else
      {
        MsgStream log( msgSvc(), name() );
        ATH_MSG_WARNING("CaloNoiseCompCondAlg::chooseIndex  wrong id ! " << m_lar_em_id->show_to_string(id));
        continue ;
      }

      assert (idSymmCaloHash < m_CaloHashMax);
      if (m_indexContainer[idSymmCaloHash] != static_cast<unsigned int>(-1)) {
        m_indexContainer[idCaloHash] = m_indexContainer[idSymmCaloHash];
        continue;
      }
    }
    else idSymmCaloHash=idCaloHash;// no symmetry


    if(iCalo!=CaloCell_ID::TILE) {
      if(this->checkIfConnected(id)==false) {
        std::cout << "DRDEBUG ... NOT connected " << std::endl ;
        continue; 
      }
      // else { std::cout << "DRDEBUG ... connected " << std::endl ;}
    }

    //  std::cout << "DRDEBUG ... check connected OK " << intIdCaloHash << std::endl ;

    /* cabling eta= 0 -> 0.8  (for private debug)
       int samp  = m_lar_em_id->sampling(id);
       int region= m_lar_em_id->region(id);
       int eta   = m_lar_em_id->eta(id);
       int phi   = m_lar_em_id->phi(id);
       if(samp==1 && eta>=256) return;
       if(samp==1 && eta==0) return;
       if(samp==2 && eta>=32) return;
       if(samp==3 && eta>=16) return;
       if(region>0) return;
    */
  
    // std::cout << "DRDEBUG ... register under  " << m_new_index << std::endl ;

    //we come here if idSymmHash is not yet indexed (and is connected)   
    m_indexContainer[idCaloHash] = 
      m_indexContainer[idSymmCaloHash] =
      m_idSymmCaloHashContainer.size();
  
    m_idSymmCaloHashContainer.push_back(idSymmCaloHash);
  }// loop on all cells

  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

bool 
CaloNoiseCompCondAlg::checkIfConnected(const Identifier &id)
{
  try
  {
    HWIdentifier hwid = m_cabling->createSignalChannelID(id);
    if(!m_cabling->isOnlineConnected(hwid)) 
    {
      //std::cout<<m_lar_em_id->show_to_string(id)
      //	       <<" not connected !!"<<std::endl;
      return false;
    }
  }
  catch(LArID_Exception & except) 
    {return false;}
  //std::cout<<m_lar_em_id->show_to_string(id)
  //	       <<" connected !!"<<std::endl;  
  return true;
}

//////////////////////////////////////////////////

int 
CaloNoiseCompCondAlg::index(const IdentifierHash &idCaloHash)
{
  return m_indexContainer[idCaloHash];
}


//////////////////////////////////////////////////
 
StatusCode 
CaloNoiseCompCondAlg::initData(const LArADC2MeV *adc2mev)
{
  MsgStream log( msgSvc(), name() );

  StatusCode sc ;
  sc = this->initContainers();
  if (sc.isFailure()) {
    ATH_MSG_WARNING( "initContainers failed" ) ;
    return sc;
  }
 
  // reset diagnostics
  for(int igain=0;igain<CaloGain::LARNGAIN;++igain)
  {
    m_nCellsWithProblem[igain]=0;
    for(int i=0;i<5000;++i) m_nReason[i][igain]=0;
    for(int i=0;i<10;++i)   m_itReason[i][igain]=0;
  }

 
  //stores the Adc2MeV factors 
  sc = this->initAdc2MeV(adc2mev);
  if (!sc.isSuccess())  
    ATH_MSG_ERROR( "initData(): error with initAdc2MeV() " ); 

  //calculates and stores the electronic noise
  sc = this->initElecNoise(); 
  if (!sc.isSuccess())  
    ATH_MSG_ERROR( "initData(): error with initElecNoise() " );

  //calculates and stores the pileup noise
  sc = this->initPileUpNoise();
  if (!sc.isSuccess())  
    ATH_MSG_ERROR( "initData(): error with initPileUpNoise() ");
  

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::initAdc2MeV(const LArADC2MeV *adc2mev) 
{
  MsgStream log( msgSvc(), name() );
  ATH_MSG_INFO( "initAdc2MeV() begin " );
  for (unsigned int it=0; it<m_adc2mevContainer.size(); ++it)
  { 
    CaloCell_ID::SUBCALO iCalo = this->caloNum(m_idSymmCaloHashContainer[it]);
    Identifier id=m_calocell_id->cell_id(m_idSymmCaloHashContainer[it]);
    //::::::::::::::::::::::::::::::::::::::
    //::::::::::::::::::::::::::::::::::::::
    if(iCalo!=CaloCell_ID::TILE) 
    {
      std::vector<float>& adc2mevVector = m_adc2mevContainer[it];
      adc2mevVector.reserve (CaloGain::LARNGAIN);
      for(unsigned int igain=0;igain<CaloGain::LARNGAIN;++igain)
      {
        auto polynom_adc2mev = adc2mev->ADC2MEV(id,igain);
        if(polynom_adc2mev.size()==0)
          adc2mevVector.push_back(0.);
        else 
          adc2mevVector.push_back(polynom_adc2mev[1]);
      }
    }
    //::::::::::::::::::::::::::::::::::::::
  } 
  ATH_MSG_INFO( "initAdc2MeV() end " );
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::initElecNoise()
{
  // initialize the parameters (the same for each event for each Identifier) 
  // for the calculation of the electronic noise


  MsgStream log( msgSvc(), name() );
  ATH_MSG_DEBUG( "initElecNoise() begin " );
 
  for (unsigned int it=0; it<m_elecNoiseCELLContainer.size(); ++it)
  {
    //::::::::::::::::::::::::::::::::::::::        
    m_elecNoiseCELLContainer[it] =
	this->calculateElecNoiseForLAR(m_idSymmCaloHashContainer[it]);
    //::::::::::::::::::::::::::::::::::::::
  }

  //print diagnostic
  for(int igain=0;igain<CaloGain::LARNGAIN;++igain)
    if(m_diagnostic[igain])
    {
      ATH_MSG_INFO("===== Diagnostic for  gain "<<igain<<" =====");
      for(int i=0;i<m_nCellsWithProblem[igain];++i)
      {
        Identifier id = m_calocell_id->cell_id(m_idHash[i][igain]);
        log<<MSG::DEBUG<<m_idHash[i][igain]<<" "
           <<m_lar_em_id->show_to_string(id)
           <<" "<<m_nReason[i][igain]<<" : ";
        for(int j=0;j<m_nReason[i][igain];++j)
          log<<MSG::DEBUG<<m_reasonName[m_reason[i][j][igain]]<<" ";
        log << MSG::DEBUG<<endmsg;
      }
      log<<MSG::DEBUG<<endmsg;
      ATH_MSG_INFO("N cells with problem(s) = " <<m_nCellsWithProblem[igain]);
      for(int i=0;i<10;++i)
      if(m_itReason[i][igain]>0) 
        ATH_MSG_INFO( i<<" "<<m_reasonName[i] <<": for "<<m_itReason[i][igain]<<" cells" );
    }

  ATH_MSG_DEBUG( "initElecNoise() end " );
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

StatusCode 
CaloNoiseCompCondAlg::initPileUpNoise()
{
  // initialize the parameters (the same for each event for each Identifier) 
  // for the calculation of the PileUp noise

  MsgStream log( msgSvc(), name() );
  ATH_MSG_DEBUG( "initPileUpNoise() begin " );
  ATH_MSG_INFO( "N events of Minimum Bias per bunch crossing =  " << m_Nminbias);
  //::::::::::::::::::::::::::::::::::::::
  for (unsigned int it=0; it<m_pileupNoiseContainer.size(); ++it)  
    m_pileupNoiseContainer[it]
      =this->calculatePileUpNoise(m_idSymmCaloHashContainer[it],m_Nminbias);
  //::::::::::::::::::::::::::::::::::::::
  ATH_MSG_DEBUG( "initPileUpNoise() end " );
  return StatusCode::SUCCESS;
}

//////////////////////////////////////////////////

std::vector<float> 
CaloNoiseCompCondAlg::calculateElecNoiseForLAR(const IdentifierHash & idCaloHash)
{ 
 /*  

E=SUMi { OFCi * (short[ (PulseShapei*Ehit/Adc2MeV(gain) + Noisei(gain)  
                       + pedestal) ]  
                 - pedestal) * Adc2Mev(gain) ] } 
   with Noisei =SUMj { cij*Rndm } * m_SigmaNoise

   NB:  without short and with cij=identity (no autocorrelation) 
        E=SUMi { NOISEi(gain)*Rndm } 
        with  NOISEi(gain) = Adc2MeV(gain) * OFCi * m_SigmaNoise(gain)        

   => Sigma^2=SUMi{NOISEi(gain)*NOISEj(gain)*cij} + quantification part
             =        NOISE(gain)                 +   REST
      Sigma  = std::sqrt( NOISE(gain) + REST) 


*/

  std::vector<float> sigmaVector (CaloGain::LARNGAIN,BADVALUE);
  float sigma;

  Identifier id = m_calocell_id->cell_id(idCaloHash);

  for(int igain=0;igain<CaloGain::LARNGAIN;++igain) 
  {
    bool noiseOK=true;
    //::::::::::::::::::::::::::::::::::::::::::::::::::
    //==== retrieve the database ====
    //::::::::::::::::::::::::::::::::::::::::::::::::::

    std::vector<bool> retrieve(nDATABASE,false); 
    retrieve[iADC2MEV]=true;
    retrieve[iSIGMANOISE]=true;
    retrieve[iAUTOCORR]=true;
    retrieve[iOFC]=true;
    //retrieve[iSHAPE]=true;
    StatusCode sc=this->retrieveCellDatabase(idCaloHash,id,igain, retrieve);    
    //if(sc.isFailure()) continue;
      //NOTE: if an element of the database is empty, 
      //      leave the iteration (on gains) => value will be BADVALUE
      //the interfaces take care of that !   

    //::::::::::::::::::::::::::::::::::::::::::::::::::
    //==== calculations ====
    //::::::::::::::::::::::::::::::::::::::::::::::::::

    if(sc.isFailure()) {
      sigma=float(BADVALUE_TO_RETURN);
    }
    else    
    {
      float OFC_AC_OFC,OFC_OFC;
      this->commonCalculations(OFC_AC_OFC,OFC_OFC,1);
      //::::::::::::::::::::::::::::::::::::::
      float NOISE= OFC_AC_OFC*m_SigmaNoise*m_SigmaNoise ;
      float REST = OFC_OFC*(1./12.);// 12.=std::sqrt(12)*std::sqrt(12)
      sigma=(NOISE+REST) * m_Adc2MeVFactor*m_Adc2MeVFactor;   
      //::::::::::::::::::::::::::::::::::::::
      if(sigma>0) sigma=std::sqrt(sigma);
      else  
      {
        sigma=-std::sqrt(-sigma);
        //:::::::::::::::::
        //      MsgStream log(msgSvc(), name());
        //      if(igain==0) log << MSG::ERROR 
        //	  <<m_lar_em_id->show_to_string(id)<<" gain "<<igain
        //	  <<" : negative root square => WRONG noise "
        //	  <<"(please check if OFC or AutoCorr are correct for this cell)"
        //	  <<endreq;
      }

      //diagnostic    
      if(m_diagnostic[igain]) 
      {
        if(noiseOK && sigma<0)       
  	  this->updateDiagnostic(9,"sigma<0",igain, noiseOK);         
        if(!noiseOK) 
        {
	  m_idHash[m_nCellsWithProblem[igain]][igain]=idCaloHash;
	  ++m_nCellsWithProblem[igain];
	  m_nReason[m_nCellsWithProblem[igain]][igain]=0;
        }      
      }
      //::::::::::::::::::::::::::::::::::::::
      if(noiseOK==false || sigma<0) sigma=float(BADVALUE_TO_RETURN);
    }
    sigmaVector[igain]=sigma;

  }//loop on gains
  
  return sigmaVector;  
}

//////////////////////////////////////////////////


float 
CaloNoiseCompCondAlg::calculatePileUpNoise(const IdentifierHash & idCaloHash, 
				    const float &Nminbias)
{
  if(Nminbias<=0.000001) return 0.;
    //only on WorkMode==1
  if(this->caloNum(idCaloHash)==CaloCell_ID::TILE) return 0.;
    //no pile-up for tiles, for the moment ...

  /*
    SigmaPileUp^2 = ( SigmaE * std::sqrt(Nmb) )^2 * Ipileup/Tc
    where:  
      - Ipileup = Tc * SUM(k=1->Nb) g(tk)^2 
      - Tc is the time between bunch crossings
      - Nb is the number of bunch crossings 
        (over which the response function is non-zero)
      - g is the shape
      - SigmaE is the RMS of the energy in 1 minimum bias event
      - Nmb is the number of minimum bias events (depending on the luminosity)
   */ 
  
  Identifier id = m_calocell_id->cell_id(idCaloHash);

  //::::::::::::::::::::::::::::::::::::::

  std::vector<bool> retrieve(nDATABASE,false);   
  retrieve[iAUTOCORR]=true;
  retrieve[iOFC]=true;
  retrieve[iSHAPE]=true;
  retrieve[iMINBIASRMS]=true;
  retrieve[iFSAMPL]=true;
  StatusCode sc=this->retrieveCellDatabase(idCaloHash,id,
					   CaloGain::LARHIGHGAIN,retrieve);    
  if(sc.isFailure()) return 0.;

  //::::::::::::::::::::::::::::::::::::::

  //in the database, RMS is at the scale of the Hits, 
  // so we need to scale it at the e.m scale using the sampling fraction ...
  m_MinBiasRMS /= m_fSampl;

  //::::::::::::::::::::::::::::::::::::::

// overall normalization factor
  float  PileUp=m_MinBiasRMS*std::sqrt(Nminbias);
 
  //::::::::::::::::::::::::::::::::::::::

  float OFC_AC_OFC,OFC_OFC; 
  unsigned int firstSample=m_firstSample; 
  // for HEC, always use firstSample=1 when the number of samples is 4 
  if (m_lar_hec_id->is_lar_hec(id) && m_nsamples==4 && m_firstSample==0u) firstSample=1; 
  this->commonCalculations(OFC_AC_OFC,OFC_OFC,2,firstSample); 
 
  //::::::::::::::::::::::::::::::::::::::

  PileUp*=std::sqrt(OFC_AC_OFC);

  //std::cout<<"PILEUP "<<m_lar_em_id->show_to_string(id)<<" "
  //	   <<MinBiasRMS<<" "<<OFC_AC_OFC<<" "<<PileUp<<std::endl;

  return PileUp; 
}

//////////////////////////////////////////////////

void
CaloNoiseCompCondAlg::commonCalculations(float & OFC_AC_OFC,float & OFC_OFC,int icase, unsigned int firstSample) 
{

  // case 1 electronic noise
  if (icase==1) {
     //calculate the matrix of autocorrelation
     for(int i=0;i<m_nsamples;++i) 
       for(int j=0;j<m_nsamples;++j)  
       { 
         if(i==j)               m_c[i][j] = 1.; 
         for(int k=1;k<m_nsamples;++k) 
	   if(i==j-k || i==j+k)	
	     m_c[i][j] = m_AutoCorr[k-1];
       }
  }
// other case: pileup noise
  else {
     for (int i=0;i<m_nsamples;i++) {
      for (int j=0;j<m_nsamples;j++)
      {
        m_c[i][j]=0.;
        int nsize = m_Shape.size();
        for (int k=0;k<nsize;k++) {
           if ((j-i+k)>=0 && (j-i+k)<nsize) {
             int ibunch=0;
             if ((i+firstSample-k)%m_deltaBunch == 0 ) ibunch=1;
             m_c[i][j] += ((double) (ibunch)) * (m_Shape[k]) * (m_Shape[j-i+k]);
           }
        }
      }
     }
  }

  //::::::::::::::::::::::::::::::::::::::
  OFC_AC_OFC=0;
  OFC_OFC=0;
  
  float tmp;
  for(int i=0;i<m_nsamples;++i) 
  {   
    tmp=0.; 
    for(int j=0;j<m_nsamples;++j)  	
      tmp+=m_c[i][j]*m_OFC[j]; 
    tmp*=m_OFC[i]; 
    OFC_AC_OFC+=tmp;  
    OFC_OFC+= m_OFC[i] * m_OFC[i]; 
    //std::cout<<"    "<<i<<" "<<OFC_AC_OFC<<" "<<OFC_OFC<<std::endl;
  }
  //::::::::::::::::::::::::::::::::::::::
}

//////////////////////////////////////////////////

StatusCode
CaloNoiseCompCondAlg::retrieveCellDatabase(const IdentifierHash & idCaloHash,
				    const Identifier & id, 
				    int igain,
                                    std::vector<bool> &retrieve)
{
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //ADC2MEV
  if(retrieve[iADC2MEV])
  {
    {      
      int index=this->index(idCaloHash);
      m_Adc2MeVFactor = (m_adc2mevContainer[index])[igain];       
    } 
    ATH_MSG_VERBOSE("m_Adc2MeVFactor="<<m_Adc2MeVFactor);
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SIGMANOISE
  if(retrieve[iSIGMANOISE])
  {
    m_RMSpedestal = m_ped->pedestalRMS(m_cabling->createSignalChannelID(id),igain);
    if(m_RMSpedestal>(1.0+LArElecCalib::ERRORCODE)) 
      m_SigmaNoise = m_RMSpedestal;
    else
    {     
      m_SigmaNoise = 0.;
    }
    ATH_MSG_VERBOSE("m_SigmaNoise(inADC)="<<m_SigmaNoise<<" m_RMSpedestal="<<m_RMSpedestal);
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //AUTOCORR
  if(retrieve[iAUTOCORR])
  {
    m_AutoCorr = m_acorr->autoCorr(m_cabling->createSignalChannelID(id),igain);
    ////////// 
    ATH_MSG_VERBOSE("AutoCorr= ");
      for(unsigned int i=0;i<m_AutoCorr.size();++i)                
	ATH_MSG_VERBOSE(m_AutoCorr[i]<<" ");
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //OFC
  if(retrieve[iOFC])
  {
    m_OFC = m_ofccond->OFC_a(m_cabling->createSignalChannelID(id), igain) ;
    /////////
    ATH_MSG_VERBOSE("OFC= ");
      for(unsigned int i=0;i<m_OFC.size();++i) 
	ATH_MSG_VERBOSE(m_OFC[i]<<" ");
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SHAPE
  if(retrieve[iSHAPE])
  {
    m_Shape = m_shapecond->Shape(m_cabling->createSignalChannelID(id),0);
    //////////
    ATH_MSG_VERBOSE("Shape= ");
      for(unsigned int i=0;i<m_Shape.size();++i) 
	ATH_MSG_VERBOSE(m_Shape[i]<<" ");
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //MinimumBias RMS 
  if(retrieve[iMINBIASRMS])
  {
    m_MinBiasRMS = m_minbias->minBiasRMS(m_cabling->createSignalChannelID(id));
    ATH_MSG_VERBOSE("MinBiasRMS="<<m_MinBiasRMS);
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SAMPLING FRACTION
  if(retrieve[iFSAMPL])
  {
    m_fSampl = m_fsamplcond->FSAMPL(m_cabling->createSignalChannelID(id));
    ATH_MSG_VERBOSE("fSampl="<<m_fSampl);
  }

  return this->checkCellDatabase(id,igain, retrieve);
}

//////////////////////////////////////////////////

StatusCode
CaloNoiseCompCondAlg::checkCellDatabase(const Identifier & id, int igain, std::vector<bool> &retrieve)
{
  StatusCode StatusDatabase=StatusCode::SUCCESS;

  bool dummy=false;
  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //ADC2MEV
  if(retrieve[iADC2MEV]) {
    if(std::fabs(m_Adc2MeVFactor)<0.000001) {
      StatusDatabase=StatusCode::FAILURE;
      if(m_diagnostic[igain]) 
	this->updateDiagnostic(0,"m_Adc2MeVFactor=0",igain,dummy);
    }
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SIGMANOISE
  if(retrieve[iSIGMANOISE]) {
    if(std::fabs(m_SigmaNoise)<0.000001) {
      StatusDatabase=StatusCode::FAILURE;
      if(m_diagnostic[igain]) 
	this->updateDiagnostic(1,"m_SigmaNoise=0",igain,dummy);	
    }
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //AUTOCORR
  if(retrieve[iAUTOCORR])
  {
    if (!m_AutoCorr.valid()) 
    {
      ATH_MSG_WARNING( " AutoCorr invalid for " <<m_lar_em_id->show_to_string(id)<<" at gain "<<igain);
      StatusDatabase=StatusCode::FAILURE;
    }
    if (m_AutoCorr.size()==0) 
    {
      StatusDatabase=StatusCode::FAILURE;
      if(m_diagnostic[igain]) this->updateDiagnostic(2,"AC empty",igain,dummy);
    }
      // autocorr can be null (and it is for low-gain !), so allow it !
      m_nsamples=m_AutoCorr.size()+1;
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //OFC
  if(retrieve[iOFC])
  {
    if (!m_OFC.valid()) 
    {
      ATH_MSG_WARNING( "  OFC pointer null for " <<m_lar_em_id->show_to_string(id)<<" at gain "<<igain);
      StatusDatabase=StatusCode::FAILURE;
    }
    if (m_OFC.size()==0) 
    {
      StatusDatabase=StatusCode::FAILURE;
      if(m_diagnostic[igain]) this->updateDiagnostic(4,"OFC empty",igain,dummy);
    }
    else
      if(m_diagnostic[igain]) 
      {
	unsigned int n_OFCnull=0;
	for(auto ofc : m_OFC)
	  if(std::fabs(ofc)<0.000001) ++n_OFCnull;
	if(n_OFCnull==m_OFC.size()) this->updateDiagnostic(5,"OFC=0",igain,dummy);
      }
    m_nsamples=m_OFC.size();
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SHAPE
  if(retrieve[iSHAPE])
  {
    if (!m_Shape.valid()) 
    {
      MsgStream log(msgSvc(), name());
      ATH_MSG_WARNING( "  Shape pointer null -> PileUp will be 0 for " <<m_lar_em_id->show_to_string(id) );
      StatusDatabase=StatusCode::FAILURE;
    }
    if (m_Shape.size()==0) 
    {      
      //      MsgStream log(msgSvc(), name());
      //      log<<MSG::WARNING
      //       <<"  Shape vector empty -> PileUp will be 0 for "
      //       <<m_lar_em_id->show_to_string(id)<<endreq;
      StatusDatabase=StatusCode::FAILURE;
      if(m_diagnostic[igain]) this->updateDiagnostic(6,"Shape empty",igain,dummy);
    }
    else
      if(m_diagnostic[igain]) 
      {
	unsigned int n_SHAPEnull=0;
	for(auto shp : m_Shape)
	  if(std::fabs(shp)<0.000001) ++n_SHAPEnull;
	if(n_SHAPEnull==m_Shape.size()) 	  
	  this->updateDiagnostic(7,"Shape=0",igain,dummy);
      }	
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //NSAMPLES
  if(retrieve[iOFC] && retrieve[iAUTOCORR] 
     && m_OFC.size()!=m_AutoCorr.size()+1)
  {
    m_nsamples=std::min(m_OFC.size(),m_AutoCorr.size()+1);
    MsgStream log( msgSvc(), name() );
    ATH_MSG_DEBUG( "AutoCorr and OFC vectors have not the same " <<"number of elements" <<" ("<<m_AutoCorr.size()<<"/"<<m_OFC.size() <<" ) => will take into account only " << m_nsamples << " samples !" );
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  if(retrieve[iOFC] && retrieve[iSHAPE] && m_OFC.size()==m_Shape.size())
  {
    float scalar=0;
    for(unsigned int i=0;i<m_Shape.size();++i)
      scalar+=m_Shape[i]*m_OFC[i];
    if((scalar-1)>0.05)
      this->updateDiagnostic(8,"[Shape].[OFC] not 1",igain,dummy);
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  //SAMPLING FRACTION
  if(retrieve[iFSAMPL])
  {
    if (m_fSampl<0.000001) 
    {
      MsgStream log(msgSvc(), name());
      ATH_MSG_WARNING("  fSampl null -> PileUp will be 0 for " <<m_lar_em_id->show_to_string(id) );
      StatusDatabase=StatusCode::FAILURE;
    }
  }

  //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  return StatusDatabase;
}

//////////////////////////////////////////////////

void
CaloNoiseCompCondAlg::updateDiagnostic(int ireason,const std::string &nameReason,int igain, bool &noiseOK)
{
  int nTmp=m_nCellsWithProblem[igain];
  if (nTmp >= 5000) return;
  int nr = m_nReason[nTmp][igain];
  if (nr >= 10) return;
  m_reason[nTmp][nr][igain]=ireason;
  m_reasonName[ireason]=nameReason;
  ++m_nReason[nTmp][igain];
  ++m_itReason[ireason][igain]; 
  noiseOK=false;
  //std::cout<<nTmp<<" "<<n_reason[nTmp][igain]<<" "
  //         <<it_reason[ireason][igain]<<std::endl;
}

//////////////////////////////////////////////////


//========================  USER INTERFACES ===================================


//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::elecNoiseRMS(const CaloDetDescrElement* caloDDE, 
			    const CaloGain::CaloGain gain)
{  


  float sigma=0.;

  const IdentifierHash idCaloHash = caloDDE->calo_hash();
  //CaloCell_ID::SUBCALO iCalo = this->caloNum(idCaloHash);
  CaloCell_ID::SUBCALO iCalo = caloDDE->getSubCalo();
  int index=this->index(idCaloHash);

  
  int igain=static_cast<int>(gain);//for LAr  
  if(iCalo==CaloCell_ID::TILE)     //for Tile 
  {
    CaloGain::CaloGain convertedGain;
    switch(gain)
    {
  //convert Tile gain into LAr gain (the one used to store the noise in arrays)
    case CaloGain::TILEHIGHHIGH : convertedGain=CaloGain::LARHIGHGAIN;   break;
    case CaloGain::TILEHIGHLOW :  convertedGain=CaloGain::LARMEDIUMGAIN; break;
    case CaloGain::TILELOWHIGH :  convertedGain=CaloGain::LARMEDIUMGAIN; break;
    case CaloGain::TILELOWLOW :   convertedGain=CaloGain::LARLOWGAIN;    break;
    case CaloGain::TILEONEHIGH :  convertedGain=CaloGain::LARHIGHGAIN;   break;
    case CaloGain::TILEONELOW :   convertedGain=CaloGain::LARLOWGAIN;    break;
    default: convertedGain = CaloGain::INVALIDGAIN;
    }
    igain=static_cast<int>(convertedGain);
  } 

  if (gain==CaloGain::INVALIDGAIN || gain==CaloGain::UNKNOWNGAIN) {
    MsgStream log( msgSvc(), name() );
    ATH_MSG_WARNING( " ask noise for invalid/unknown gain, will return noise for high gain " );
    igain=static_cast<int>(CaloGain::LARHIGHGAIN);
  }

  if (iCalo<0 || index<0)
  {
    MsgStream log(msgSvc(), name());
    ATH_MSG_WARNING( "CaloNoiseCompCondAlg::elecNoiseRMS  wrong id ! " << "iCalo="<<iCalo << "index="<<index << "id:" << m_lar_em_id->show_to_string(caloDDE->identify()) );
    return 0.;
  } 
  else 
  {         
    const std::vector<float>* sigmaVector = 0;
    sigmaVector = &m_elecNoiseCELLContainer[index];

    bool retry=true;
    int shift_gain=0;
    int gain_wanted=igain;    
    int gain_shifted=gain_wanted;

    while(retry)
    {      
      //:::::::::::::::::
      retry=false;
      //:::::::::::::::::
      gain_shifted=gain_wanted-shift_gain;
      //:::::::::::::::::
      sigma = (*sigmaVector)[gain_shifted];
      //:::::::::::::::::
      sigma = this->calculateElecNoiseForLAR(idCaloHash) [gain_shifted];
      //:::::::::::::::::
      if(this->isBadValue(sigma)) 
      {
	++shift_gain;
	if(shift_gain<=igain) retry=true;
	MsgStream log(msgSvc(), name());
	ATH_MSG_WARNING( "noise is missing for this cell " << m_lar_em_id->show_to_string(caloDDE->identify()) << " at this gain (" <<gain_wanted<<"), return the noise at next gain (" <<gain_shifted<<")" );
      }
      //:::::::::::::::::
    }
    return sigma;   
  }  
}


//////////////////////////////////////////////////

std::vector<float> 
CaloNoiseCompCondAlg::elecNoiseRMS3gains(const CaloDetDescrElement* caloDDE)
{  
  std::vector<float> sigma;
  sigma.reserve (CaloGain::LARNGAIN);
  for(int igain=0;igain<CaloGain::LARNGAIN;++igain)
    sigma.push_back(this->elecNoiseRMS(caloDDE,
				       static_cast<CaloGain::CaloGain>(igain)));
  for(int igain=0;igain<CaloGain::LARNGAIN;++igain)
    if(this->isBadValue(sigma[igain]) && 
       igain!=CaloGain::LARHIGHGAIN) 
      sigma[igain]=sigma[igain-1];//take the next gain (low->medium->high)
  return sigma;
}


//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::elecNoiseRMS(const CaloCell* theCell) {
  const CaloDetDescrElement* caloDDE = theCell->caloDDE();
  CaloGain::CaloGain igain = theCell->gain(); 
  return this->elecNoiseRMS(caloDDE, igain);
}

float 

CaloNoiseCompCondAlg::pileupNoiseRMS(const CaloCell* theCell,
			      const float Nminbias) {
  const CaloDetDescrElement* caloDDE = theCell->caloDDE();
  return this->pileupNoiseRMS(caloDDE,Nminbias);
}

float 

CaloNoiseCompCondAlg::totalNoiseRMS(const CaloCell* theCell,
			     const float Nminbias) {
  float elecNoiseRMS_tmp   = this->elecNoiseRMS(theCell);
  float pileupNoiseRMS_tmp = this->pileupNoiseRMS(theCell,Nminbias);
  
  if(elecNoiseRMS_tmp>=0)
    return std::sqrt((elecNoiseRMS_tmp*elecNoiseRMS_tmp) + (pileupNoiseRMS_tmp*pileupNoiseRMS_tmp));
  return(-1);
}


//////////////////////////////////////////////////


float 
CaloNoiseCompCondAlg::pileupNoiseRMS(const CaloDetDescrElement* caloDDE, 
			      const float Nminbias)
// << base class >>
{


  const IdentifierHash idCaloHash = caloDDE->calo_hash();
  CaloCell_ID::SUBCALO iCalo = this->caloNum(idCaloHash);
  
  if(iCalo!=CaloCell_ID::TILE)
  {
    int index=this->index(idCaloHash);
    float PileUp;
    // check if noise stored in container was calcualted with this Nminbias
    if ((Nminbias==m_Nminbias) || (Nminbias<=0)) //default 
      PileUp=m_pileupNoiseContainer[index];
    else
      PileUp=this->calculatePileUpNoise(idCaloHash,Nminbias);//slower !!
    return PileUp;
  }
  else//TILE
  {
    return 0.;   
  }
}

//////////////////////////////////////////////////


float 
CaloNoiseCompCondAlg::totalNoiseRMS(const CaloDetDescrElement* caloDDE, 
			     const CaloGain::CaloGain gain, 
			     const float Nminbias)
{
  float elecNoiseRMS_tmp   = this->elecNoiseRMS(caloDDE,gain);
  float pileupNoiseRMS_tmp = this->pileupNoiseRMS(caloDDE,Nminbias);
  
  float totalNoiseRMS = -1;
  
  // checks that elecNoise is valid
  if(elecNoiseRMS_tmp>0) 
    totalNoiseRMS= std::sqrt((elecNoiseRMS_tmp*elecNoiseRMS_tmp) + (pileupNoiseRMS_tmp*pileupNoiseRMS_tmp) );
  
  return totalNoiseRMS;
}

//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::totalNoiseRMSHighestGain(const CaloCell* caloCell, 
					const float Nminbias)
{
  return this->totalNoiseRMSHighestGain(caloCell->caloDDE(),Nminbias);
}

//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::totalNoiseRMSHighestGain(const CaloDetDescrElement* caloDDE, 
					const float Nminbias)
{
  //getSubCalo can return 999999 (CaloCell_ID::SUBCALO::NOT_VALID)
  CaloCell_ID::SUBCALO iCalo = caloDDE->getSubCalo();
  if (iCalo ==  CaloCell_ID::SUBCALO::NOT_VALID){
    throw std::runtime_error("Invalid CaloID in CaloNoiseCompCondAlg::totalNoiseRMSHighestGain");
  }
  CaloGain::CaloGain highestGain=m_highestGain[iCalo];
  return this->totalNoiseRMS(caloDDE,highestGain,Nminbias);
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::adc2mev(const CaloDetDescrElement* caloDDE,
		       const CaloGain::CaloGain gain)
{

  float factor=1.;
  IdentifierHash idCaloHash = caloDDE->calo_hash();
  CaloCell_ID::SUBCALO iCalo = this->caloNum(idCaloHash);
 
  if(iCalo==CaloCell_ID::LAREM || iCalo==CaloCell_ID::LARHEC)
  {      
    int index=this->index(idCaloHash);
    factor=(m_adc2mevContainer[index])[gain];
  }
  else if(iCalo==CaloCell_ID::LARFCAL)
  {
    int index=this->index(idCaloHash);
    factor=(m_adc2mevContainer[index])[gain];
  }  
  else if(iCalo==CaloCell_ID::TILE)
  {
    //TILE_PART
    MsgStream log( msgSvc(), name() );
    ATH_MSG_WARNING("CaloNoiseCompCondAlg::adc2mev(id,gain) : NOT IMPLEMENTED !" <<"for TILE (-> returns 1. for the moment)" );    
    factor=1.; 
  }  
  else
  {
    MsgStream log( msgSvc(), name() );
    ATH_MSG_WARNING("CaloNoiseCompCondAlg::adc2mev(id,gain)  wrong id ! " <<m_lar_em_id->show_to_string(caloDDE->identify()) );
    factor=0.;
  }  
  return factor;
}

//////////////////////////////////////////////////

float 
CaloNoiseCompCondAlg::adc2mev(const Identifier& id,const CaloGain::CaloGain gain)
{
  return adc2mev(m_calo_dd_man->get_element(id),gain);
}


