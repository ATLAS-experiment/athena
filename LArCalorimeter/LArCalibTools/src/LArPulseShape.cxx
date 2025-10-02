/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTools/LArPulseShape.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArCOOLConditions/LArDSPConfig.h"
#include "CaloIdentifier/CaloGain.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "TFile.h"
#include "TKey.h"
#include "TTree.h"
#include "TSystem.h"

LArPulseShape::LArPulseShape(const std::string& name, ISvcLocator* pSvcLocator): 
  AthAlgorithm(name, pSvcLocator), m_initialized(false),  m_calo_id(nullptr),  
  m_nt(nullptr) 
{
  declareProperty("NtupleTitle",m_ntTitle="Pulse shape");
  m_ntpath="/NTUPLES/PULSE/"+m_ntname; 
}

LArPulseShape::~LArPulseShape() = default;

StatusCode LArPulseShape::initialize() {
  const CaloIdManager* caloIdMgr{nullptr};
  ATH_CHECK( detStore()->retrieve( caloIdMgr ) );
  m_calo_id      = caloIdMgr->getCaloCell_ID();

  ATH_CHECK( m_lumiDataKey.initialize() );
  
  ATH_CHECK( m_cablingKey.initialize() ); 

  ATH_CHECK( m_BCKey.initialize() );

  ATH_CHECK( m_trigDec.retrieve() );

  if (m_ntpath.empty() || m_ntTitle.empty()) {
    ATH_MSG_ERROR( "Need to set variable 'm_ntpath' and 'm_ntTitle' in constructor of deriving class!" );
    return StatusCode::FAILURE;
  }

  size_t i=m_ntpath.rfind('/');
  if (i==std::string::npos) {
    ATH_MSG_ERROR( "Expected at least on '/' in path " << m_ntpath );
    return StatusCode::FAILURE;
  }
  std::string basepath(m_ntpath.begin(),m_ntpath.begin()+i);

  NTupleFilePtr PULSE(ntupleSvc(),basepath);
  if (!PULSE){
    ATH_MSG_ERROR( "Could not get NTupleFilePtr with path " << basepath << " failed" );
    return StatusCode::FAILURE;
  }
  NTuplePtr nt(ntupleSvc(),m_ntpath);
  if (!nt) {
    nt=ntupleSvc()->book(m_ntpath,CLID_ColumnWiseTuple,m_ntTitle);
  }
  if (!nt){
    ATH_MSG_ERROR( "Booking of NTuple at "<< m_ntpath << " and name " << m_ntTitle << " failed" );
    return StatusCode::FAILURE; 
  }
  
  m_nt=nt;

  int ncell=m_calo_id->calo_cell_hash_max();
 
  for (int i=0;i<ncell;i++) { // loop over cells to book profiles
  
    IdentifierHash idHash=i;
    Identifier id=m_calo_id->cell_id(idHash);

    const int sampl = m_calo_id->sampling( id );
    m_OffId = (int)(id.get_identifier32().get_compact());
    std::string idoff_str = std::to_string(m_OffId);
    const char* id_char = (idoff_str).c_str();
   
    if (m_calo_id->is_em(id)) {    // EM calo
      if (m_calo_id->is_em_barrel(id)) { //EMB
        if (m_calo_id->pos_neg(id) > 0 ) { //A-side
	
	  SmartDataPtr<NTuple::Directory> dir_EMBA(ntupleSvc(),"/NTUPLES/PULSE/EMBA");
   	  if ( !dir_EMBA ) dir_EMBA = ntupleSvc()->createDirectory(PULSE,"EMBA");
          if ( !dir_EMBA ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
                 
          if (sampl==0){ // if it's EMBA Presampler
            SmartDataPtr<NTuple::Directory> dir_EMBA_0(ntupleSvc(),"/NTUPLES/PULSE/EMBA/Presampler");
  	    if ( !dir_EMBA_0 ) dir_EMBA_0 = ntupleSvc()->createDirectory(PULSE,"EMBA/Presampler");
            if ( !dir_EMBA_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==1){ // if it's EMBA Sampling1
  	    SmartDataPtr<NTuple::Directory> dir_EMBA_1(ntupleSvc(),"/NTUPLES/PULSE/EMBA/Sampling1");
  	    if ( !dir_EMBA_1 ) dir_EMBA_1 = ntupleSvc()->createDirectory(PULSE,"EMBA/Sampling1");
  	    if ( !dir_EMBA_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }	     
	  
	  if (sampl==2){ // if it's EMBA Sampling2
  	    SmartDataPtr<NTuple::Directory> dir_EMBA_2(ntupleSvc(),"/NTUPLES/PULSE/EMBA/Sampling2");
  	    if ( !dir_EMBA_2 ) dir_EMBA_2 = ntupleSvc()->createDirectory(PULSE,"EMBA/Sampling2");
  	    if ( !dir_EMBA_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==3){ // if it's EMBA Sampling3
  	    SmartDataPtr<NTuple::Directory> dir_EMBA_3(ntupleSvc(),"/NTUPLES/PULSE/EMBA/Sampling3");
  	    if ( !dir_EMBA_3 ) dir_EMBA_3 = ntupleSvc()->createDirectory(PULSE,"EMBA/Sampling3");
  	    if ( !dir_EMBA_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  		  
        } else {  // C-side
	
	  SmartDataPtr<NTuple::Directory> dir_EMBC(ntupleSvc(),"/NTUPLES/PULSE/EMBC");
   	  if ( !dir_EMBC ) dir_EMBC = ntupleSvc()->createDirectory(PULSE,"EMBC");
   	  if ( !dir_EMBC ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          if (sampl==0){ // if it's EMBC Presampler
            SmartDataPtr<NTuple::Directory> dir_EMBC_0(ntupleSvc(),"/NTUPLES/PULSE/EMBC/Presampler");
  	    if ( !dir_EMBC_0 ) dir_EMBC_0 = ntupleSvc()->createDirectory(PULSE,"EMBC/Presampler");
            if ( !dir_EMBC_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
            m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==1){ // if it's EMBC Sampling1
  	    SmartDataPtr<NTuple::Directory> dir_EMBC_1(ntupleSvc(),"/NTUPLES/PULSE/EMBC/Sampling1");
  	    if ( !dir_EMBC_1 ) dir_EMBC_1 = ntupleSvc()->createDirectory(PULSE,"EMBC/Sampling1");
  	    if ( !dir_EMBC_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }	     
	  
	  if (sampl==2){ // if it's EMBC Sampling2
  	    SmartDataPtr<NTuple::Directory> dir_EMBC_2(ntupleSvc(),"/NTUPLES/PULSE/EMBC/Sampling2");
  	    if ( !dir_EMBC_2 ) dir_EMBC_2 = ntupleSvc()->createDirectory(PULSE,"EMBC/Sampling2");
  	    if ( !dir_EMBC_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==3){ // if it's EMBC Sampling3
  	    SmartDataPtr<NTuple::Directory> dir_EMBC_3(ntupleSvc(),"/NTUPLES/PULSE/EMBC/Sampling3");
  	    if ( !dir_EMBC_3 ) dir_EMBC_3 = ntupleSvc()->createDirectory(PULSE,"EMBC/Sampling3");
  	    if ( !dir_EMBC_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  		     
	} 
	
      } // end EMB
      
      if( m_calo_id->is_em_endcap(id) ){ //EMEC
      
        if (m_calo_id->pos_neg(id) > 0 ) { //A-side
	
	  SmartDataPtr<NTuple::Directory> dir_EMECA(ntupleSvc(),"/NTUPLES/PULSE/EMECA");
   	  if ( !dir_EMECA ) dir_EMECA = ntupleSvc()->createDirectory(PULSE,"EMECA");
          if ( !dir_EMECA ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
                 
          if (sampl==0){ // if it's EMECA Presampler
            SmartDataPtr<NTuple::Directory> dir_EMECA_0(ntupleSvc(),"/NTUPLES/PULSE/EMECA/Presampler");
  	    if ( !dir_EMECA_0 ) dir_EMECA_0 = ntupleSvc()->createDirectory(PULSE,"EMECA/Presampler");
            if ( !dir_EMECA_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
   
            //stopper++;
	    //if (stopper<=10) 
	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==1){ // if it's EMECA Sampling1
  	    SmartDataPtr<NTuple::Directory> dir_EMECA_1(ntupleSvc(),"/NTUPLES/PULSE/EMECA/Sampling1");
  	    if ( !dir_EMECA_1 ) dir_EMECA_1 = ntupleSvc()->createDirectory(PULSE,"EMECA/Sampling1");
  	    if ( !dir_EMECA_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }	     
	  
	  if (sampl==2){ // if it's EMECA Sampling2
  	    SmartDataPtr<NTuple::Directory> dir_EMECA_2(ntupleSvc(),"/NTUPLES/PULSE/EMECA/Sampling2");
  	    if ( !dir_EMECA_2 ) dir_EMECA_2 = ntupleSvc()->createDirectory(PULSE,"EMECA/Sampling2");
  	    if ( !dir_EMECA_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==3){ // if it's EMECA Sampling3
  	    SmartDataPtr<NTuple::Directory> dir_EMECA_3(ntupleSvc(),"/NTUPLES/PULSE/EMECA/Sampling3");
  	    if ( !dir_EMECA_3 ) dir_EMECA_3 = ntupleSvc()->createDirectory(PULSE,"EMECA/Sampling3");
  	    if ( !dir_EMECA_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  		  
        } else {  // C-side
	
	  SmartDataPtr<NTuple::Directory> dir_EMECC(ntupleSvc(),"/NTUPLES/PULSE/EMECC");
   	  if ( !dir_EMECC ) dir_EMECC = ntupleSvc()->createDirectory(PULSE,"EMECC");
   	  if ( !dir_EMECC ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          if (sampl==0){ // if it's EMECC Presampler
            SmartDataPtr<NTuple::Directory> dir_EMECC_0(ntupleSvc(),"/NTUPLES/PULSE/EMECC/Presampler");
  	    if ( !dir_EMECC_0 ) dir_EMECC_0 = ntupleSvc()->createDirectory(PULSE,"EMECC/Presampler");
            if ( !dir_EMECC_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
            m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==1){ // if it's EMECC Sampling1
  	    SmartDataPtr<NTuple::Directory> dir_EMECC_1(ntupleSvc(),"/NTUPLES/PULSE/EMECC/Sampling1");
  	    if ( !dir_EMECC_1 ) dir_EMECC_1 = ntupleSvc()->createDirectory(PULSE,"EMECC/Sampling1");
  	    if ( !dir_EMECC_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }	     
	  
	  if (sampl==2){ // if it's EMECC Sampling2
  	    SmartDataPtr<NTuple::Directory> dir_EMECC_2(ntupleSvc(),"/NTUPLES/PULSE/EMECC/Sampling2");
  	    if ( !dir_EMECC_2 ) dir_EMECC_2 = ntupleSvc()->createDirectory(PULSE,"EMECC/Sampling2");
  	    if ( !dir_EMECC_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  
	  if (sampl==3){ // if it's EMECC Sampling3
  	    SmartDataPtr<NTuple::Directory> dir_EMECC_3(ntupleSvc(),"/NTUPLES/PULSE/EMECC/Sampling3");
  	    if ( !dir_EMECC_3 ) dir_EMECC_3 = ntupleSvc()->createDirectory(PULSE,"EMECC/Sampling3");
  	    if ( !dir_EMECC_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
  	    m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
  	  }
	  		     
	}       
      } // end EMEC
      
	  
	        
    } // end EM calo
    
    if( m_calo_id->is_hec(id) ){ //HEC
    
      if (m_calo_id->pos_neg(id) > 0 ) { //A-side

        SmartDataPtr<NTuple::Directory> dir_HECA(ntupleSvc(),"/NTUPLES/PULSE/HECA");
        if ( !dir_HECA ) dir_HECA = ntupleSvc()->createDirectory(PULSE,"HECA");
    	if ( !dir_HECA ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
    	       
    	if (sampl==0){
    	  SmartDataPtr<NTuple::Directory> dir_HECA_0(ntupleSvc(),"/NTUPLES/PULSE/HECA/Layer1");
          if ( !dir_HECA_0 ) dir_HECA_0 = ntupleSvc()->createDirectory(PULSE,"HECA/Layer1");
    	  if ( !dir_HECA_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==1){
          SmartDataPtr<NTuple::Directory> dir_HECA_1(ntupleSvc(),"/NTUPLES/PULSE/HECA/Layer2");
          if ( !dir_HECA_1 ) dir_HECA_1 = ntupleSvc()->createDirectory(PULSE,"HECA/Layer2");
          if ( !dir_HECA_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }	   
        
        if (sampl==2){
          SmartDataPtr<NTuple::Directory> dir_HECA_2(ntupleSvc(),"/NTUPLES/PULSE/HECA/Layer3");
          if ( !dir_HECA_2 ) dir_HECA_2 = ntupleSvc()->createDirectory(PULSE,"HECA/Layer3");
          if ( !dir_HECA_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==3){
          SmartDataPtr<NTuple::Directory> dir_HECA_3(ntupleSvc(),"/NTUPLES/PULSE/HECA/Layer4");
          if ( !dir_HECA_3 ) dir_HECA_3 = ntupleSvc()->createDirectory(PULSE,"HECA/Layer4");
          if ( !dir_HECA_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        		
      } else {  // C-side

        SmartDataPtr<NTuple::Directory> dir_HECC(ntupleSvc(),"/NTUPLES/PULSE/HECC");
        if ( !dir_HECC ) dir_HECC = ntupleSvc()->createDirectory(PULSE,"HECC");
        if ( !dir_HECC ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
    	if (sampl==0){ 
    	  SmartDataPtr<NTuple::Directory> dir_HECC_0(ntupleSvc(),"/NTUPLES/PULSE/HECC/Layer1");
          if ( !dir_HECC_0 ) dir_HECC_0 = ntupleSvc()->createDirectory(PULSE,"HECC/Layer1");
    	  if ( !dir_HECC_0 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
    	  m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==1){
          SmartDataPtr<NTuple::Directory> dir_HECC_1(ntupleSvc(),"/NTUPLES/PULSE/HECC/Layer2");
          if ( !dir_HECC_1 ) dir_HECC_1 = ntupleSvc()->createDirectory(PULSE,"HECC/Layer2");
          if ( !dir_HECC_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }	   
        
        if (sampl==2){ 
          SmartDataPtr<NTuple::Directory> dir_HECC_2(ntupleSvc(),"/NTUPLES/PULSE/HECC/Layer3");
          if ( !dir_HECC_2 ) dir_HECC_2 = ntupleSvc()->createDirectory(PULSE,"HECC/Layer3");
          if ( !dir_HECC_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==3){ 
          SmartDataPtr<NTuple::Directory> dir_HECC_3(ntupleSvc(),"/NTUPLES/PULSE/HECC/Layer4");
          if ( !dir_HECC_3 ) dir_HECC_3 = ntupleSvc()->createDirectory(PULSE,"HECC/Layer4");
          if ( !dir_HECC_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        		   
      }       
    } // end HEC      
    if( m_calo_id->is_fcal(id) ){ //FCAL
    
      if (m_calo_id->pos_neg(id) > 0 ) { //A-side

        SmartDataPtr<NTuple::Directory> dir_FCALA(ntupleSvc(),"/NTUPLES/PULSE/FCALA");
        if ( !dir_FCALA ) dir_FCALA = ntupleSvc()->createDirectory(PULSE,"FCALA");
    	if ( !dir_FCALA ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
        
        if (sampl==1){ // if it's FCALA Sampling1
          SmartDataPtr<NTuple::Directory> dir_FCALA_1(ntupleSvc(),"/NTUPLES/PULSE/FCALA/Sampling1");
          if ( !dir_FCALA_1 ) dir_FCALA_1 = ntupleSvc()->createDirectory(PULSE,"FCALA/Sampling1");
          if ( !dir_FCALA_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }	   
        
        if (sampl==2){ // if it's FCALA Sampling2
          SmartDataPtr<NTuple::Directory> dir_FCALA_2(ntupleSvc(),"/NTUPLES/PULSE/FCALA/Sampling2");
          if ( !dir_FCALA_2 ) dir_FCALA_2 = ntupleSvc()->createDirectory(PULSE,"FCALA/Sampling2");
          if ( !dir_FCALA_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==3){ // if it's FCALA Sampling3
          SmartDataPtr<NTuple::Directory> dir_FCALA_3(ntupleSvc(),"/NTUPLES/PULSE/FCALA/Sampling3");
          if ( !dir_FCALA_3 ) dir_FCALA_3 = ntupleSvc()->createDirectory(PULSE,"FCALA/Sampling3");
          if ( !dir_FCALA_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        	
      } else {  // C-side

        SmartDataPtr<NTuple::Directory> dir_FCALC(ntupleSvc(),"/NTUPLES/PULSE/FCALC");
        if ( !dir_FCALC ) dir_FCALC = ntupleSvc()->createDirectory(PULSE,"FCALC");
        if ( !dir_FCALC ) ATH_MSG_ERROR ( " failed to get ntuple directory" );

        if (sampl==1){ // if it's FCALC Sampling1
          SmartDataPtr<NTuple::Directory> dir_FCALC_1(ntupleSvc(),"/NTUPLES/PULSE/FCALC/Sampling1");
          if ( !dir_FCALC_1 ) dir_FCALC_1 = ntupleSvc()->createDirectory(PULSE,"FCALC/Sampling1");
          if ( !dir_FCALC_1 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }	   
        
        if (sampl==2){ // if it's FCALC Sampling2
          SmartDataPtr<NTuple::Directory> dir_FCALC_2(ntupleSvc(),"/NTUPLES/PULSE/FCALC/Sampling2");
          if ( !dir_FCALC_2 ) dir_FCALC_2 = ntupleSvc()->createDirectory(PULSE,"FCALC/Sampling2");
          if ( !dir_FCALC_2 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
        
        if (sampl==3){ // if it's FCALC Sampling3
          SmartDataPtr<NTuple::Directory> dir_FCALC_3(ntupleSvc(),"/NTUPLES/PULSE/FCALC/Sampling3");
          if ( !dir_FCALC_3 ) dir_FCALC_3 = ntupleSvc()->createDirectory(PULSE,"FCALC/Sampling3");
          if ( !dir_FCALC_3 ) ATH_MSG_ERROR ( " failed to get ntuple directory" );
          m_cellHistos.insert(std::make_pair(m_OffId, new TProfile(id_char, id_char, 40, -4 , 36, "s" )));
        }
      }     
    } // end FCAL     
    
  } // loop over cells

  SmartDataPtr<NTuple::Directory> dir_FCALC_3(ntupleSvc(),"/NTUPLES/PULSE/Check");
  if ( !dir_FCALC_3 ) dir_FCALC_3 = ntupleSvc()->createDirectory(PULSE,"Check");   
   
  m_TProfpulse_diff = new TProfile("test_diff", "test_diff",500, 0, 500, "s");  
   
  m_BCID = new TH1D("BCID", "BCID",3564, 0, 3564);    
  m_diffBCID = new TH1D("diffBCID", "BCID",3564, 0, 3564);     
  m_actualInt = new TH1D("ActualInt", "ActInt", 60, 0, 20); 
  
  addHistogram(m_TProfpulse_diff);
  
  addHistogram(m_BCID);
  addHistogram(m_diffBCID);  
  addHistogram(m_actualInt);   
  m_initialized=true;
  return StatusCode::SUCCESS;
}
/////////////////////////////////////////////////////////////////////
StatusCode LArPulseShape::execute() {


  const EventContext& ctx = Gaudi::Hive::currentContext();
  short int bunchStr[8] = {1, 101, 201, 301, 1786, 1886, 1986, 2086}; //move to JO
  
  std::vector<std::string> chains = m_trigDec->getListOfTriggers();
  std::vector<std::string> myChains;
  
  SG::ReadCondHandle<LuminosityCondData> lumi (m_lumiDataKey);

  if (m_trigDec->isPassed("L1_RD1_BGRP10")) {   

    int bunchId   = ctx.eventID().bunch_crossing_id();
    
    m_mindist = 3564;
    m_closestBC = 0;
    for (int i = 0; i < 8; i++) {
      if ( (fabs(bunchId - bunchStr[i]) < m_mindist) || (fabs(bunchId - (bunchStr[i]+3564)) < m_mindist)) {
   	m_mindist = std::min(fabs(bunchId - bunchStr[i]),(fabs(bunchId - (bunchStr[i]+3564))));
   	m_closestBC = bunchStr[i];
      }
    }
    if (fabs(bunchId - m_closestBC)  < (fabs(bunchId - (m_closestBC+3564)))) {
      m_mindist = bunchId - m_closestBC; 
    } else {
      m_mindist = bunchId - (m_closestBC+3564);
    }  

    //std::cout << "bunchID = " << bunchId<< ", m_mindist = " << m_mindist << ", m_closestBC = " << m_closestBC << std::endl;
    m_BCID->Fill(bunchId);
    m_diffBCID->Fill(m_mindist);  

    m_actualInt->Fill(lumi->lbLuminosityPerBCIDVector().at(m_closestBC));
    
    const LArDigitContainer* larDigitContainer;
    ATH_CHECK( evtStore()->retrieve(larDigitContainer, "FREE") );
    ATH_CHECK( detStore()->retrieve(m_larPedestal) );
    if (larDigitContainer->empty()) {
      ATH_MSG_WARNING ( "LArDigitContainer with key= is empty!" );

      return StatusCode::SUCCESS;
    }

    SG::ReadCondHandle<LArOnOffIdMapping> larCablingHdl(m_cablingKey);
    const LArOnOffIdMapping* cabling=*larCablingHdl;
    if(!cabling) {
       ATH_MSG_ERROR("Could not get LArOnOffIdMapping !!");
       return StatusCode::FAILURE;
    }
    for (LArDigitContainer::const_iterator digit = larDigitContainer->begin(); digit != larDigitContainer->end(); ++digit) {
    
      HWIdentifier channelID = (*digit)->hardwareID();  
      
      if (cabling->isOnlineConnected(channelID)) {
     
   	CaloGain::CaloGain gain=(*digit)->gain();

   	float pedestal = m_larPedestal->pedestal(channelID, gain); 

   	m_OffId_conv = (int)(cabling->cnvToIdentifier(channelID).get_identifier32().get_compact());
      
   	const std::vector<short>& samples = (*digit)->samples();
   	
   	m_isample=0;
        for (short sample : samples) {
          m_isample++; 
   	  if (lumi->lbLuminosityPerBCIDVector().at(m_closestBC)!=0){
   	    m_TProfpulse_diff->Fill(m_mindist+m_isample, (sample-pedestal)/lumi->lbLuminosityPerBCIDVector().at(m_closestBC));//+0,1,2,3 
   	    m_cellHistos[m_OffId_conv]->Fill(m_mindist+m_isample, (sample-pedestal)/lumi->lbLuminosityPerBCIDVector().at(m_closestBC));    
          }
   	} //loop over samples
      }
    }  
  
  }//trigger
  return StatusCode::SUCCESS;  
}

/////////////////////////////////////////////////////////////////////
StatusCode LArPulseShape::stop() {

  ATH_MSG_INFO( "LArPulseShape has finished." );
  return StatusCode::SUCCESS;
   
}// end finalize-method.
   
