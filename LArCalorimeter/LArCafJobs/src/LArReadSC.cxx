/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCafJobs/LArReadSC.h"

#include "xAODEventInfo/EventInfo.h"

#include "Identifier/Identifier.h"
#include "Identifier/HWIdentifier.h"

#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloUtils/CaloCellList.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"

#include "LArElecCalib/ILArPedestal.h"



LArReadSC::LArReadSC( const std::string& name, ISvcLocator* pSvcLocator ) : AthAlgorithm( name, pSvcLocator ){
   m_ECell.reserve(35000);
   m_EtaCell.reserve(35000);
   m_PhiCell.reserve(35000);
   m_LayerCell.reserve(35000);
   m_ProvCell.reserve(35000);
   m_ChidCell.reserve(35000);
   m_HwidCell.reserve(35000);
   m_ADC.reserve(35000);
   m_TCell.reserve(35000);
   m_ErecoCell.reserve(35000);
}


LArReadSC::~LArReadSC() {}


StatusCode LArReadSC::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");

  ServiceHandle<ITHistSvc> histSvc("THistSvc",name()); 
  CHECK( histSvc.retrieve() );
  m_tree = new TTree("myTree","myTree");
  std::string out("/"+m_outStream+"/myTree");
  CHECK( histSvc->regTree(out.c_str(),m_tree) );
  ATH_MSG_INFO("Registered tree: " << out);
  m_tree->Branch("RunNumber",&m_runNumber,"RunNumber/I");
  m_tree->Branch("LBNumber",&m_lbNumber,"LBNumber/I");
  m_tree->Branch("EventNumber",&m_eventNumber,"EventNumber/I");
  m_tree->Branch("BCID",&m_bcid,"BCID/I");
  m_tree->Branch("LArError",&m_error,"LArError/I");
  m_tree->Branch("ncells",&m_ncells,"ncells/I");
  m_tree->Branch("ECell",m_ECell.data(),"eCell[ncells]/F");
  m_tree->Branch("EtaCell",m_EtaCell.data(),"etaCell[ncells]/F");
  m_tree->Branch("PhiCell",m_PhiCell.data(),"phiCell[ncells]/F");
  m_tree->Branch("LayerCell",m_LayerCell.data(),"layerCell[ncells]/I");
  m_tree->Branch("ProvCell", m_ProvCell.data(),"provCell[ncells]/I");
  m_tree->Branch("ChidCell", m_ChidCell.data(),"chidCell[ncells]/I");
  m_tree->Branch("HwidCell", m_HwidCell.data(),"hwidCell[ncells]/I");
  if(m_contKey.key().size()) m_tree->Branch("ADC",m_ADC.data(),"ADC[ncells][32]/F");
  if(m_SCRecoKey.key().size()) {
     m_tree->Branch("TCell",m_TCell.data(),"tCell[ncells]/F");
     m_tree->Branch("ErecoCell",m_ErecoCell.data(),"ErecoCell[ncells]/F");
  }

  const CaloIdManager* caloIdMgr = nullptr;
  ATH_CHECK(detStore()->retrieve(caloIdMgr));
  m_calo_id      = caloIdMgr->getCaloCell_SuperCell_ID();
  ATH_CHECK( detStore()->retrieve(m_lar_online_id, "LArOnline_SuperCellID") );

  ATH_CHECK( m_cablingKey.initialize() );
  ATH_CHECK(m_pedestalKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());

  ATH_CHECK(m_contKey.initialize(m_contKey.key().size()));
  ATH_CHECK(m_SCKey.initialize(m_SCKey.key().size()));
  ATH_CHECK(m_SCRecoKey.initialize(m_SCRecoKey.key().size()));

  ATH_MSG_INFO("Energy cut for time computation: " << m_etcut);

  return StatusCode::SUCCESS;
}

StatusCode LArReadSC::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode LArReadSC::execute() {  
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  const EventContext& ctx = Gaudi::Hive::currentContext();

  SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey, ctx};
  const LArOnOffIdMapping* cabling=*cablingHdl;
  if(!cabling) {
     ATH_MSG_ERROR( "Do not have cabling object LArOnOffIdMapping" );
     return StatusCode::FAILURE;
  }

  SG::ReadCondHandle<CaloSuperCellDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx}; 
  const CaloSuperCellDetDescrManager* caloDDMgr = *caloMgrHandle;

  //Get Conditions input
  SG::ReadCondHandle<ILArPedestal> pedHdl{m_pedestalKey, ctx};
  const ILArPedestal* larPedestal=*pedHdl;


  const xAOD::EventInfo* eventInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve( eventInfo) );
  
  m_runNumber   = eventInfo->runNumber();
  m_eventNumber = eventInfo->eventNumber();
  m_lbNumber    = eventInfo->lumiBlock();
  m_bcid        = eventInfo->bcid();
  m_error       = 0;
  if (eventInfo->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error) m_error=1;

  std::vector<const LArDigit*> IndexDigit;
  int nCell = m_calo_id->calo_cell_hash_max();
  IndexDigit.resize(nCell,nullptr);

  const LArDigitContainer* digit_container=nullptr;
  if( m_contKey.key().size() ) { // fill from standard digits
     SG::ReadHandle<LArDigitContainer> hdlDigit(m_contKey, ctx);
     if(!hdlDigit.isValid()) {
        ATH_MSG_WARNING( "Unable to retrieve LArDigitContainer with key " << m_contKey.key() << " from EventStore. " );
     } else {
        digit_container = hdlDigit.cptr();
     }
  }

  if(digit_container) { // build digits index
     LArDigitContainer::const_iterator first_digit = digit_container->begin();
     LArDigitContainer::const_iterator end_digit   = digit_container->end();
     for (; first_digit != end_digit; ++first_digit) {
        HWIdentifier hwid = (*first_digit)->hardwareID();
        Identifier   id = cabling->cnvToIdentifier(hwid);
        int index = (int) (m_calo_id->calo_cell_hash(id));
        if (index>=0 && index<nCell) IndexDigit[index]=(*first_digit);
     }
  }


 SG::ReadHandle<CaloCellContainer> cells{m_SCKey,ctx};
 if(!cells.isValid()){
    ATH_MSG_ERROR("Could not get SC container with key "<<m_SCKey.key());
    return StatusCode::FAILURE;
 }
 const CaloCellContainer* cell_container=cells.cptr();

 const CaloCellContainer* recocell_container=nullptr;
 if(m_SCRecoKey.key().size()){
    SG::ReadHandle<CaloCellContainer> recocells{m_SCRecoKey,ctx};
    if(recocells.isValid()) recocell_container=recocells.cptr();
 }

 std::vector<int> iflag_cell;
 iflag_cell.resize(nCell,0);

 CaloCellList myList(caloDDMgr, cell_container);

 m_ncells=0;
 for (CaloCellContainer::const_iterator cell = cell_container->begin();
                                        cell != cell_container->end(); ++cell) {
     Identifier cellID = (*cell)->ID();
     int index = (int) (m_calo_id->calo_cell_hash(cellID));
     double et    =  (*cell)->et();

     if (et > m_etcut ){
        iflag_cell[index]=1;


        myList.select((*cell)->eta(),(*cell)->phi(),0.10);
        for (const CaloCell* cell : myList) {
          Identifier cellID2 =cell->ID();
          int index2 = (int)(m_calo_id->calo_cell_hash(cellID2));
          iflag_cell[index2]=1;
        }
     }// above first etcut
 }// over cells

 for  (CaloCellContainer::const_iterator cell = cell_container->begin();
                                        cell != cell_container->end(); ++cell) {

     Identifier cellID = (*cell)->ID();
     IdentifierHash hcell=m_calo_id->calo_cell_hash(cellID);
     unsigned int index = hcell.value();

     if (iflag_cell[index]==1 ){
        m_ECell[m_ncells]= (*cell)->energy();
        m_EtaCell[m_ncells]= (*cell)->eta();
        m_PhiCell[m_ncells]= (*cell)->phi();
        m_LayerCell[m_ncells]= m_calo_id->calo_sample(cellID);
        m_ProvCell[m_ncells]=(*cell)->provenance();

        HWIdentifier hwid=cabling->createSignalChannelID(cellID);
        m_ChidCell[m_ncells]=cellID.get_identifier32().get_compact();
        m_HwidCell[m_ncells]=hwid.get_identifier32().get_compact();

        if(m_contKey.key().size()) { // fill samples
           for (int i=0;i<32;i++)  m_ADC[m_ncells][i]=0.;
           float pedestal=0.;
           if (larPedestal) {
            pedestal =  larPedestal->pedestal(hwid,(*cell)->gain());
           }
      
           unsigned int index = (m_calo_id->calo_cell_hash(cellID)).value();
           if (IndexDigit[index]) {
               const std::vector<short>& vSamples=(IndexDigit[index])->samples();
               int nsamples = vSamples.size();
               for (int i=0;i<std::min(32,nsamples);i++) {
                 m_ADC[m_ncells][i]=(float)(vSamples[i])-pedestal;
               }
           }
        } // samples
        m_TCell[m_ncells]=0.;
        m_ErecoCell[m_ncells]=0.;
        if(recocell_container) {
           const CaloCell *rcell= recocell_container->findCell(hcell);
           if(rcell) {
              m_TCell[m_ncells]=rcell->time();
              m_ErecoCell[m_ncells]=rcell->energy();
           }
        }
        m_ncells++;
     }// iflag
 } // over cells

 ATH_MSG_INFO("Number of cells read " << m_ncells );

 m_tree->Fill();

 return StatusCode::SUCCESS;
}

