/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexTowerSummer  -  description
//                              -------------------
//        Builds gFexDataTowers50 and gFexDataTowers200 from gFexDataTowers
//
//     begin                : 25 06 2025
//     email                : jared.little@cern.ch
//***************************************************************************/

#include "gFexTowerSummer.h"
#include "L1CaloFEXSim/gFEXCompression.h"


#include <vector>
#include <memory>
#include <cmath> //M_PI

namespace LVL1 {

gFexTowerSummer::gFexTowerSummer(const std::string& name, ISvcLocator* svc)
    : AthReentrantAlgorithm(name, svc) {}

StatusCode gFexTowerSummer::initialize() {

  ATH_MSG_INFO(
      "Initializing L1CaloFEXAlgos/gFexEmulatedTowers algorithm with name: "
      << name());
  ATH_MSG_INFO("Writing into SG key: " << m_gTowersWriteKey);

  // initialise keys
  ATH_CHECK( m_gFexFiberTowersReadKey.initialize());
  ATH_CHECK( m_gTowersWriteKey.initialize() );
  ATH_CHECK( m_gTowers50WriteKey.initialize() );  

  ATH_CHECK( m_gTowersEMWriteKey.initialize(SG::AllowEmpty));
  ATH_CHECK( m_gTowersHADWriteKey.initialize(SG::AllowEmpty));  
  
  return StatusCode::SUCCESS;
}

StatusCode gFexTowerSummer::execute(const EventContext& ctx) const {

  // WriteHandle for gFEX Input Towers
  SG::WriteHandle<xAOD::gFexTowerContainer> gTowersContainer(m_gTowersWriteKey, ctx);
  ATH_CHECK( gTowersContainer.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
  ATH_MSG_DEBUG("Recorded gFexEmulatedTower 200 MeV container with key "<< gTowersContainer.key());

  SG::WriteHandle<xAOD::gFexTowerContainer> gTowers50Container(m_gTowers50WriteKey, ctx);
  ATH_CHECK( gTowers50Container.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
  ATH_MSG_DEBUG("Recorded gFexEmulatedTower 50 MeV container with key "<< gTowers50Container.key());

  xAOD::gFexTowerContainer*  gTowersEMContainerPtr = nullptr;
  xAOD::gFexTowerContainer*  gTowersHADContainerPtr = nullptr;
  SG::WriteHandle<xAOD::gFexTowerContainer> gTowersEMContainer = (!m_gTowersEMWriteKey.empty()) ? SG::WriteHandle<xAOD::gFexTowerContainer>(m_gTowersEMWriteKey, ctx) : SG::WriteHandle<xAOD::gFexTowerContainer>();
  SG::WriteHandle<xAOD::gFexTowerContainer> gTowersHADContainer = (!m_gTowersHADWriteKey.empty()) ? SG::WriteHandle<xAOD::gFexTowerContainer>(m_gTowersHADWriteKey, ctx) : SG::WriteHandle<xAOD::gFexTowerContainer>();

  if (!m_gTowersEMWriteKey.empty()) {
    ATH_CHECK( gTowersEMContainer.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexEmulatedTower 200 MeV EM container with key "<< m_gTowersEMWriteKey.key());
    gTowersEMContainerPtr = &*gTowersEMContainer;
  }
  if (!m_gTowersHADWriteKey.empty()) {
    ATH_CHECK( gTowersHADContainer.record(std::make_unique<xAOD::gFexTowerContainer>(), std::make_unique<xAOD::gFexTowerAuxContainer>()));
    ATH_MSG_DEBUG("Recorded gFexEmulatedTower 200 MeV HAD container with key "<< m_gTowersHADWriteKey.key());
    gTowersHADContainerPtr = &*gTowersHADContainer;
  }

  // Atwr, Btwr, Ctwr will contain gTowers towers for each FPGA
  gtFPGA Atwr{};
  gtFPGA Btwr{};
  gtFPGA Ctwr{};
  
  gtFPGA AtwrF{};
  gtFPGA BtwrF{};
  gtFPGA CtwrF{};
  
  gtFPGA Asatur{};
  gtFPGA Bsatur{};
  gtFPGA Csatur{};


  // reconstruct the gTowers/saturation
  ATH_CHECK( gtReconstructABC(ctx, 0, AtwrF, Atwr, Asatur));
  ATH_CHECK( gtReconstructABC(ctx, 1, BtwrF, Btwr, Bsatur));
  ATH_CHECK( gtReconstructABC(ctx, 2, CtwrF, Ctwr, Csatur));  
  

  // Write the towers
  int iEta = 0;
  int iPhi = 0;
  float Eta = 0;
  float Phi = 0;
  int Et  = 0;
  int EtF  = 0;
  int Fpga = 0;
  char IsSaturated = 0;
  int towerID = 0;

  // Assign ID based on FPGA (FPGA-A 0->0; FPGA-B 1->10000, FPGA-C 2->20000) and gTower number assigned as per firmware convention

  int twr_rows = Atwr.size(); // 32
  int twr_cols = Atwr[0].size(); // 12

  Fpga = 0;

  // Save towers from FPGA A in gTower EDM
  for (int irow = 0; irow < twr_rows; irow++){
    for (int icol = 0; icol < twr_cols; icol++){
      iEta = icol + 8;
      iPhi = irow;
      Et = Atwr[irow][icol];
      EtF = AtwrF[irow][icol];
      IsSaturated = Asatur[irow][icol];
      getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
      gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      gTowers50Container->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowers50Container->back()->initialize(iEta, iPhi, Eta, Phi, EtF, Fpga, IsSaturated, towerID);
      if (gTowersEMContainerPtr) {
        gTowersEMContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersEMContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      if (gTowersHADContainerPtr) {
        gTowersHADContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersHADContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      towerID += 1;
  
    }
  }
    
  // Save towers from FPGA B in gTower EDM
  Fpga = 1;
  towerID = 10000;
  // Save towers from FPGA B in gTower EDM             
  for (int irow = 0; irow < twr_rows; irow++){
    for (int icol = 0; icol < twr_cols; icol++){
      iEta = icol + 20;
      iPhi = irow;
      Et = Btwr[irow][icol];
      EtF = BtwrF[irow][icol];
      IsSaturated = Bsatur[irow][icol];
      getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
      gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID); 
      gTowers50Container->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowers50Container->back()->initialize(iEta, iPhi, Eta, Phi, EtF, Fpga, IsSaturated, towerID);
      if (gTowersEMContainerPtr) {
        gTowersEMContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersEMContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      if (gTowersHADContainerPtr) {
        gTowersHADContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersHADContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      towerID += 1;    
    }
  }
  
  // Save towers from FPGA C in gTower EDM
  Fpga = 2;
  towerID = 20000;
  for (int irow = 0; irow < twr_rows; irow++){
    for (int icol = 0; icol < twr_cols/2; icol++){                
      iEta = icol + 2;
      iPhi = irow;
      Et = Ctwr[irow][icol];
      EtF = CtwrF[irow][icol];
      IsSaturated = Csatur[irow][icol];
      getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
      gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      gTowers50Container->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowers50Container->back()->initialize(iEta, iPhi, Eta, Phi, EtF, Fpga, IsSaturated, towerID);
      if (gTowersEMContainerPtr) {
        gTowersEMContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersEMContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      if (gTowersHADContainerPtr) {
        gTowersHADContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersHADContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      towerID += 1;   
    }
    for (int icol = twr_cols/2; icol < twr_cols; icol++){                
      iEta = icol + 26;
      iPhi = irow;
      Et = Ctwr[irow][icol];
      EtF = CtwrF[irow][icol];
      IsSaturated = Csatur[irow][icol];
      getEtaPhi(Eta, Phi, iEta, iPhi, towerID);
      gTowersContainer->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowersContainer->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      gTowers50Container->push_back( std::make_unique<xAOD::gFexTower>() );
      gTowers50Container->back()->initialize(iEta, iPhi, Eta, Phi, EtF, Fpga, IsSaturated, towerID);
      if (gTowersEMContainerPtr) {
        gTowersEMContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersEMContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      if (gTowersHADContainerPtr) {
        gTowersHADContainerPtr->push_back( std::make_unique<xAOD::gFexTower>() );
        gTowersHADContainerPtr->back()->initialize(iEta, iPhi, Eta, Phi, Et, Fpga, IsSaturated, towerID);
      }
      towerID += 1;      
    }
  }

  return StatusCode::SUCCESS;
}


StatusCode gFexTowerSummer::gtReconstructABC(const EventContext& ctx,
					      unsigned int XFPGA, 
					      gtFPGA &XgtF, gtFPGA &Xgt,
					      gtFPGA &Xsaturation) const{

  // Loop over the Fiber Towers and fill gTower arrays as per original byte stream decoder
  
  // Reading the Fiber Tower container
  SG::ReadHandle<xAOD::gFexTowerContainer> gFexFiberTowerContainer(m_gFexFiberTowersReadKey, ctx);
  if (!gFexFiberTowerContainer.isValid()) {
    ATH_MSG_ERROR("Could not retrieve Fiber tower collection "
                  << gFexFiberTowerContainer.key());
    return StatusCode::FAILURE;
  }

  if (gFexFiberTowerContainer->empty()) {
    ATH_MSG_WARNING("Cannot fill gTowers here, fiber container is empty. "
		    << gFexFiberTowerContainer->size());
    return StatusCode::SUCCESS;
  }
  
  // Zero the input gTower sums/saturation
  for(int irow=0; irow<LVL1::gFEXPos::ABC_ROWS; irow++){
    for(int icolumn=0; icolumn<LVL1::gFEXPos::AB_COLUMNS; icolumn++){
      Xgt[irow][icolumn] = 0;
      XgtF[irow][icolumn] = 0;
      Xsaturation[irow][icolumn] = 0;
    }
  }

  // Energy arrays for the EM and hadronic in standard and extra eta regions
  // 200 MeV towers
  std::array<int, LVL1::gFEXPos::AB_TOWERS> etowerData{}; // 384
  std::array<int, LVL1::gFEXPos::AB_TOWERS> htowerData{};
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  xetowerData{}; // 32
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  xhtowerData{};
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  ohtowerData{};
  
  // 50 MeV towers
  std::array<int, LVL1::gFEXPos::AB_TOWERS> etowerDataF{};
  std::array<int, LVL1::gFEXPos::AB_TOWERS> htowerDataF{};
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  xetowerDataF{};
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  xhtowerDataF{};
  std::array<int, LVL1::gFEXPos::ABC_ROWS>  ohtowerDataF{};

  // saturation
  std::array<bool, LVL1::gFEXPos::AB_TOWERS> saturationData{}; // 384

  // loop over the Fiber towers, and fill the tower energy arrays 
  for(const xAOD::gFexTower* gfexFiberTower : *gFexFiberTowerContainer){
    // first match the FPGA
    unsigned int fiberTowerFpga = gfexFiberTower->fpga();
    if (fiberTowerFpga != XFPGA) continue;

    // working with "local" fiber number, iFiber
    unsigned int fiberTowerId = gfexFiberTower->gFEXtowerID();
    unsigned int offset = (XFPGA == 2) ? 20000 : (XFPGA == 1) ? 10000 : 0;
    unsigned int iFiber = (fiberTowerId - offset)/16;

    // Do not exceed maximum number of fibers for FPGA
    unsigned int maxFiberN =  (XFPGA == 2) ? LVL1::gFEXPos::C_FIBERS : LVL1::gFEXPos::AB_FIBERS;
    if (iFiber >= maxFiberN) continue;
    
    ATH_MSG_DEBUG(" accessing " << fiberTowerId << " " << XFPGA << " " << offset << " " << iFiber);
    
    unsigned int iDatum = fiberTowerId%16;
   
    int fiber_type  = (XFPGA == 0) ? LVL1::gFEXPos::AMPD_NFI[iFiber] :
      (XFPGA == 1) ? LVL1::gFEXPos::BMPD_NFI[iFiber] : LVL1::gFEXPos::CMPD_NFI[iFiber];

    // tells where the data is coming from
    // -  0 - EMB, EMB/EMEC -> EM contribution
    // -  1 - TREX,HEC - Had contribution     
    // -  2 - extended region ( EMEC)         
    // -  3 - extended region ( HEC)
    // -  6 - 200MeV region only ( HEC)
    // - 11 - HEC - Had contribution

    int dataType = (XFPGA == 0) ? LVL1::gFEXPos::AMPD_DTYP_ARR[fiber_type][iDatum] :
      (XFPGA == 1) ? LVL1::gFEXPos::BMPD_DTYP_ARR[fiber_type][iDatum] :
      LVL1::gFEXPos::CMPD_DTYP_ARR[fiber_type][iDatum];

    // tower number 0 - 383
    int ntower = (XFPGA == 0) ? LVL1::gFEXPos::AMPD_GTRN_ARR[iFiber][iDatum] :
      (XFPGA == 1) ? LVL1::gFEXPos::BMPD_GTRN_ARR[iFiber][iDatum] :
      LVL1::gFEXPos::CMPD_GTRN_ARR[iFiber][iDatum];
    
    // calo type
    // FPGA 0/1 0,1,2
    // FPGA 2   3
    int caloType = (XFPGA == 0) ? LVL1::gFEXPos::ACALO_TYPE[iFiber] :
      (XFPGA == 1) ? LVL1::gFEXPos::BCALO_TYPE[iFiber] : LVL1::gFEXPos::CCALO_TYPE[iFiber]; 

    // saturation
    bool fiberSaturation = (bool)(gfexFiberTower->isSaturated());
    if (fiberSaturation) {
      saturationData[ntower] = fiberSaturation;
    }

    // Get the MLE from the EDM (stored as float)
    unsigned int Toweret_mle = (unsigned int)(gfexFiberTower->towerEt());

    // FPGA 0/1
    if(caloType < 3) {
      switch(dataType){
      case 0:
  etowerData[ntower] = Toweret_mle;
  undoMLE( etowerData[ntower] );
  etowerDataF[ntower] = etowerData[ntower];
  break;
      case 1:
  htowerData[ntower] = Toweret_mle;
  
  // mulitply by 20 to make 50 MeV LSB
  // include this here before mulitplication by 20 
  htowerData[ntower]  = 20*htowerData[ntower];
  htowerDataF[ntower] =  htowerData[ntower];
  break;

      case 2:
  xetowerData[ntower] = Toweret_mle;
  undoMLE( xetowerData[ntower] );
  xetowerDataF[ntower]       = xetowerData[ntower];
  break;
    
      case 3:
  xhtowerData[ntower] = Toweret_mle;
  undoMLE( xhtowerData[ntower] );
  xhtowerDataF[ntower]       = xhtowerData[ntower];
	break;
    
      case 6:
	ohtowerData[ntower] = Toweret_mle; 
	undoMLE( ohtowerData[ntower] );
	ohtowerDataF[ntower]       =  ohtowerData[ntower];
	break;

      case 11:
	htowerData[ntower] = Toweret_mle; 
	undoMLE( htowerData[ntower] );
	htowerDataF[ntower] = htowerData[ntower];
	break;
      }
    } else {

      // this is FPGA C
      // only types 2 and 3 exist in FPGA C    
      switch(dataType){ 
      case 2:
	// 
	etowerData[ntower] = Toweret_mle;
	undoMLE( etowerData[ntower] );
	etowerDataF[ntower] = etowerData[ntower]; 

	break;
        
      case 3:
	htowerData[ntower] = Toweret_mle;	
	undoMLE( htowerData[ntower] );
	htowerDataF[ntower] = htowerData[ntower];
	break;

      case 15:
	break;

      case 99:
	break; 
        
      default:
	ATH_MSG_ERROR("Tower with unknown datatype "
		      << dataType);
	return StatusCode::FAILURE;

      }			
    } // end of case statement for FPGAC 

    ATH_MSG_DEBUG(" end of loop: " << XFPGA << " tower: " << ntower << " e " << etowerDataF[ntower] << " h " << htowerData[ntower] << " fibertower " << fiberTowerId);
    
  } // end of loop over fiber towers    

  // Sum the energy arrays into the output gTowers
  if( XFPGA == 0 ) {
    for(int itower=0;itower<384;itower++){
      int icolumn = itower%12;
      int irow    =  itower/12;

      // saturation
      Xsaturation[irow][icolumn] = saturationData[itower];

      
      // 50 MeV towers 
      int xF = etowerDataF[itower] + htowerDataF[itower];
      // 200 MeV towers 
      int x   = ( (etowerData[itower]>>2) + (htowerData[itower]>>2) );
      
      ATH_MSG_DEBUG("sss1 " << icolumn << " " << irow << " " << xF << " " << x << " " <<  etowerDataF[itower] <<"  " << htowerDataF[itower]);

      signExtend(&xF,18);
      signExtend(&x,18);

      ATH_MSG_DEBUG("sss2 " << icolumn << " " << irow << " " << xF << " " << x);
      	
      Xgt[irow][icolumn]  = x;
      XgtF[irow][icolumn] = xF;

      ATH_MSG_DEBUG("sss3 " << icolumn << " " << irow << " " << XgtF[irow][icolumn] << " " <<  Xgt[irow][icolumn]);
	
      // eta  region in FPGA A  (eta ~ -2.5)
      if ( icolumn == 0) {
        int xx =  ( (xetowerData[irow]>>2) + (xhtowerData[irow]>>2) );
        signExtend(&xx,18);
        Xgt[irow][icolumn]  = Xgt[irow][icolumn]  + xx;
        ATH_MSG_DEBUG("sss4 " << icolumn << " " << irow << " " << XgtF[irow][icolumn] << " " <<  Xgt[irow][icolumn]);
      }

      if ( icolumn == 4) {
        // 200 MeV towers
        int ox =  (ohtowerData[irow] >> 2 ) ;
        signExtend(&ox,18);
        Xgt[irow][icolumn]  = Xgt[irow][icolumn]   + ox ;
        ATH_MSG_DEBUG("sss5 " << icolumn << " " << irow << " " << XgtF[irow][icolumn] << " " <<  Xgt[irow][icolumn]);
      }
      
      ATH_MSG_DEBUG("sss filling standard " << Xgt[irow][icolumn]  << " fiber " << XgtF[irow][icolumn]);
    }
  }
  else if ( XFPGA == 1 ) {
    for(int itower=0;itower<384;itower++){
      int icolumn = itower%12;
      int irow    =  itower/12;

      // saturation
      Xsaturation[irow][icolumn] = saturationData[itower];
      
      // 50 MeV towers
      int xF =  etowerDataF[itower]  +  htowerDataF[itower] ;
      // 200 MeV towers 
      int x  =  ( (etowerData[itower]>>2) +  (htowerData[itower] >> 2) );
      
      signExtend(&xF,18);
      signExtend(&x,18);
      
      Xgt[irow][icolumn]  = x;
      XgtF[irow][icolumn] = xF;
      
      // extra region FPGA B (eta ~ 2.5) 
      if ( icolumn == 11) {
        // 200 MeV towers 
        int xx = ( (xetowerData[irow]>>2) + (xhtowerData[irow]>>2) );
        signExtend(&xx,18);
        Xgt[irow][icolumn]  = Xgt[irow][icolumn]  + xx;
      }
      if ( icolumn == 7 ) {
        // 200 MeV towers
        int xo =  ohtowerData[irow]>>2;
        signExtend(&xo,18);
        Xgt[irow][icolumn]  = Xgt[irow][icolumn]   + xo;
      }
    }
  }
  else if ( XFPGA == 2 ) {
    for(int itower=0;itower<384;itower++){
      int icolumn = itower%12;
      int irow    =  itower/12;

      // saturation
      Xsaturation[irow][icolumn] = saturationData[itower];
      
      // 50 MeV towers 
      int xF =   etowerDataF[itower] + htowerDataF[itower] ;
      // 200 MeV towers 
      int x =  ( (etowerData[itower]>>2 ) + (htowerData[itower]>>2));
      signExtend(&xF,18);
      signExtend(&x,18);
	
      Xgt[irow][icolumn] = x;
      XgtF[irow][icolumn] = xF;
    }
  } 
  
  return StatusCode::SUCCESS;
}
  
void gFexTowerSummer::undoMLE(int &datumPtr ) const{

  // limit input to 12 bits to avoid accidental sign extension
  int din = (0x00000FFF &  datumPtr );
  // map all special cases to zero for now
  // limit negative values
  if( (din > 0) && ( din < 962 )  ) din =  962;
  //zeroZero
  if( din == 0) din = 0x4EE;

  int dout = 0; 
  
  int FPGA_CONVLIN_TH1 = 5; 
  int FPGA_CONVLIN_TH2 = 749;
  int FPGA_CONVLIN_TH3 = 1773;
  int FPGA_CONVLIN_TH4 = 2541;
  int FPGA_CONVLIN_TH5 = 4029;
  int FPGA_CONVLIN_TH6 = 4062;
  
  // These two variables are unused
  //int FPGA_CONVLIN_OF0 = -5072;
  //int FPGA_CONVLIN_OF1 = -2012;
  int FPGA_CONVLIN_OF2 = -1262;
  int FPGA_CONVLIN_OF3 = -3036;
  int FPGA_CONVLIN_OF4 = -8120;
  int FPGA_CONVLIN_OF5 = -4118720;
  
  int oth0 = 0;
  int oth1 = 0;
  int oth2 = 0;
  int oth3 = 0;
  int oth4 = 0;
  int oth5 = 0;
  int oth6 = 0;
  
  // These two variables are unused
  //int r1shv = 0;
  //int r2shv = 0;
  int r3shv = 0;
  int r4shv = 0;
  int r5shv = 0;
  int r6shv = 0;
  // int trxv = 0;
  
  
  int r3conv = 0;
  int r4conv = 0;
  int r5conv = 0;
  int r6conv = 0;
  // int r3offs = 0;

  //r1shv = ((din & 0x0000007F) << 9 )  & 0x0000FE00 ;
  //r2shv = ((din & 0x00000FFF) << 1 )  & 0x00001FFE ;
  r3shv = (din &  0x00000FFF) ;
  r4shv = ((din & 0x00000FFF) << 1 )  & 0x00001FFE ;
  r5shv = ((din & 0x00000FFF) << 2 )  & 0x00003FFC ;
  r6shv = ((din & 0x00000FFF) << 10 ) & 0x003FFC00 ;
  
  // These two variables are unused
  //r1conv =  r1shv + FPGA_CONVLIN_OF0;
  //r2conv =  r2shv + FPGA_CONVLIN_OF1;
  r3conv =  r3shv + FPGA_CONVLIN_OF2;
  r4conv =  r4shv + FPGA_CONVLIN_OF3;
  r5conv =  r5shv + FPGA_CONVLIN_OF4;
  r6conv =  r6shv + FPGA_CONVLIN_OF5;

  if( din > 0 ) {
    oth0 = 1;
  }
  else{
    oth0 = 0; 
  }
  if ( din > FPGA_CONVLIN_TH1 ){
    oth1 = 1;
  }
  else{
    oth1 = 0; 
  }
  if ( din > FPGA_CONVLIN_TH2 ){
    oth2 = 1;
  }else{
    oth2 = 0; 
  }
  if ( din > FPGA_CONVLIN_TH3 ){
    oth3 = 1;
  }else{
    oth3 = 0; 
  }
  if ( din > FPGA_CONVLIN_TH4 ){
    oth4 = 1;
  }else{
    oth4 = 0; 
  }
  if ( din > FPGA_CONVLIN_TH5 ){
    oth5 = 1;
  }
  else{
    oth5 = 0; 
    }
  if ( din > FPGA_CONVLIN_TH6 ){
    oth6 = 1;
  }
  else{
    oth6 = 0; 
  }


  // divide by 2 to 50 MeV LSB
  
  if( (! oth0) & (! oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 ) & (! oth6 )   ) {
    dout = 0;
  } 
  /** The following cases cannot be reached, given the preceding conditions.
  
  else if( ( oth0) & (! oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 ) & (! oth6 )  ) {
    dout =  r1conv >>1;
  } 
  else if( ( oth0) & (  oth1 ) & (! oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 ) & (! oth6 )  ) {
    dout = r2conv >>1;
  } 
  **/
  else if( ( oth0) & (  oth1 ) & ( oth2 ) & (! oth3 ) &  (! oth4 ) & (! oth5 ) & (! oth6 )  ) {
    dout = r3conv >>1;
  }  
  else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (! oth4 ) & (! oth5 ) & (! oth6 )  ) {
    dout = r4conv >>1;
  }  
  else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (  oth4 ) & (! oth5 ) & (! oth6 ) ) {
    dout = r5conv >>1;
  } 
  else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (  oth4 ) & ( oth5 ) & (! oth6 ) ) {
    dout = r6conv >>1;
  }  
  else if( ( oth0) & (  oth1 ) & (  oth2 ) & ( oth3 ) &  (  oth4 ) & (  oth5 ) & ( oth6 )  ) {
    dout = 0;
  } 
  /* This code cannot be reached, given the preceding conditions.
  else {
    dout = 0; 
  }
  */

  signExtend(&dout,15);
  
  datumPtr = dout;
}


void gFexTowerSummer::signExtend(int *xptr, int upto) const{

  // sign extend x to 32 bits assuming a hardware word length upto+1 bits (e.g. for 16 bit word upto should be 15 as in firmware) 
  // xptr pointer to input datum
  // word length in hardware 
  int x = *xptr; 
  //printf("before %x \n", x);
  //printf("masks %x %x  \n", (0x00000001<<upto) , (0xFFFFFFFF<<(upto+1))  );
  if( x & (0x00000001<<upto) ) {
    x = ( x | (0xFFFFFFFF<<(upto+1)) );
  } else {
    // for now assume 17 bits -- but could be up to 18 bits 
    x = ( x & 0x000FFFF); 
  }
  *xptr = x; 
  
}  
  
void gFexTowerSummer::getEtaPhi(float& Eta, float& Phi, int iEta, int iPhi,
                                 int gFEXtowerID) const {

  float s_centralPhiWidth =
      (2 * M_PI) / 32;  // In central region, gFex has 32 bins in phi
  float s_forwardPhiWidth =
      (2 * M_PI) / 16;  // In forward region, gFex has 16 bins in phi (before
                        // rearranging bins)

  const std::vector<float> s_EtaCenter = {
      -4.5, -3.8, -3.38, -3.18, -3.15, -3,   -2.8, -2.6, -2.35, -2.1,
      -1.9, -1.7, -1.5,  -1.3,  -1.1,  -0.9, -0.7, -0.5, -0.3,  -0.1,
      0.1,  0.3,  0.5,   0.7,   0.9,   1.1,  1.3,  1.5,  1.7,   1.9,
      2.1,  2.35, 2.6,   2.8,   3.0,   3.15, 3.18, 3.38, 3.8,   4.5};

  // Transform Eta and Phi indices for the most forward towers into the
  // "original" indices, as before rearranging the towers such that the forward
  // region is 12(ieta)x32(iphi). The FPGA-C has now the same format (12*32) as
  // FPGA-A and FPGA-B. This is the result of a transformation in the firmware.
  // Note that for the most forward towers, the Phi index and Eta index have
  // been considered accordingly, so in order to get the correct float values of
  // Phi and Eta we need to retrieve the "original" indices.
  int towerID_base = 20000;
  int iEtaOld = 0, iPhiOld = 0;

  if (iEta == 2) {
    if (iPhi == ((gFEXtowerID - towerID_base) / 24) * 2) {
      iEtaOld = 0;
      iPhiOld = iPhi / 2;
    }
    if (iPhi == (((gFEXtowerID - towerID_base - 12) / 24) * 2) + 1) {
      iEtaOld = 1;
      iPhiOld = (iPhi - 1) / 2;
    }
  }

  else if (iEta == 3) {
    if (iPhi == ((gFEXtowerID - towerID_base - 1) / 24) * 2) {
      iEtaOld = 2;
      iPhiOld = iPhi / 2;
    }
    if (iPhi == (((gFEXtowerID - towerID_base - 13) / 24) * 2) + 1) {
      iEtaOld = 3;
      iPhiOld = (iPhi - 1) / 2;
    }
  }

  else if (iEta == 36) {
    if (iPhi == (((gFEXtowerID - towerID_base - 22) / 24) * 2) + 1) {
      iEtaOld = 36;
      iPhiOld = (iPhi - 1) / 2;
    }
    if (iPhi == ((gFEXtowerID - towerID_base - 10) / 24) * 2) {
      iEtaOld = 37;
      iPhiOld = iPhi / 2;
    }
  }

  else if (iEta == 37) {
    if (iPhi == (((gFEXtowerID - towerID_base - 23) / 24) * 2) + 1) {
      iEtaOld = 38;
      iPhiOld = (iPhi - 1) / 2;
    }
    if (iPhi == ((gFEXtowerID - towerID_base - 11) / 24) * 2) {
      iEtaOld = 39;
      iPhiOld = iPhi / 2;
    }
  }

  else {
    iEtaOld = iEta;
    iPhiOld = iPhi;
  }

  Eta = s_EtaCenter[iEtaOld];

  float Phi_gFex = -99;

  if ((iEtaOld <= 3) || ((iEtaOld >= 36))) {
    Phi_gFex = ((iPhiOld * s_forwardPhiWidth) + s_forwardPhiWidth / 2);
  } else {
    Phi_gFex = ((iPhiOld * s_centralPhiWidth) + s_centralPhiWidth / 2);
  }

  if (Phi_gFex < M_PI) {
    Phi = Phi_gFex;
  } else {
    Phi = (Phi_gFex - 2 * M_PI);
  }
}

}  // namespace LVL1
