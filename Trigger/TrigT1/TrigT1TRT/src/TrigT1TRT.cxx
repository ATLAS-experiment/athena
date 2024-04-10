/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <stdint.h>
#include <fstream>

#include "TrigT1TRT.h"

#include "PathResolver/PathResolver.h"
#include "nlohmann/json.hpp"

namespace LVL1 {
  
  //--------------------------------
  // Constructors and destructors
  //--------------------------------

  TrigT1TRT::TrigT1TRT(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator),
    m_ConditionsSummary("TRT_StrawStatusSummaryTool", this),
    m_TRTStrawNeighbourSvc("TRT_StrawNeighbourSvc", name)
    {
      declareProperty("ConditionsSummaryTool",m_ConditionsSummary);
    }

  //---------------------------------
  // initialise()
  //---------------------------------

  StatusCode TrigT1TRT::initialize() {

    ATH_CHECK( m_trtCTPLocation.initialize() );

    ATH_CHECK( m_trtRDOKey.initialize() );

    ATH_CHECK( detStore()->retrieve(m_mgr, "TRT") );
    ATH_MSG_DEBUG( "Connected to TRT DetectorManager"  );

    ATH_CHECK( detStore()->retrieve(m_pTRTHelper, "TRT_ID") );
    ATH_MSG_DEBUG( "Connected to TRT Helper"  );

    ATH_CHECK( m_ConditionsSummary.retrieve() );

    ATH_CHECK( m_TRTStrawNeighbourSvc.retrieve() );

    ATH_MSG_INFO("Setting Fast-OR trigger multiplicity" << m_TTCMultiplicity );

    // Find the full path to filename:
    std::string file = PathResolver::find_file(m_maskedChipsFile, "DATAPATH");
    ATH_MSG_INFO("Reading file " << file);
    std::ifstream fin(file.c_str());
    if(!fin){
      ATH_MSG_ERROR("Can not read file: " << file);
      return StatusCode::FAILURE;
    }
    nlohmann::json data = nlohmann::json::parse(fin);

    m_maskedChipsBarrel = data["Barrel"].template get<std::array<std::array<std::vector<int>, 32>, 2>>();
    m_maskedChipsEc = data["Endcap"].template get<std::array<std::array<std::vector<int>, 32>, 2>>();

    // initialize numberOfStraws array for use in strawNumber function.
    for(int j=0; j<75; j++) {
      m_numberOfStraws[j]=0;
    }

    // numberofstraws in a phi module layer. m_numberOfStraws[<layer number>]
    m_numberOfStraws[1]=15;
    m_numberOfStraws[2]=m_numberOfStraws[3]=m_numberOfStraws[4]=m_numberOfStraws[5]=16;
    m_numberOfStraws[6]=m_numberOfStraws[7]=m_numberOfStraws[8]=m_numberOfStraws[9]=m_numberOfStraws[10]=17;
    m_numberOfStraws[11]=m_numberOfStraws[12]=m_numberOfStraws[13]=m_numberOfStraws[14]=m_numberOfStraws[15]=18;
    m_numberOfStraws[16]=m_numberOfStraws[17]=m_numberOfStraws[18]=19;
    m_numberOfStraws[19]=18;
    m_numberOfStraws[20]=19;
    m_numberOfStraws[21]=m_numberOfStraws[22]=m_numberOfStraws[23]=m_numberOfStraws[24]=m_numberOfStraws[25]=20;
    m_numberOfStraws[26]=m_numberOfStraws[27]=m_numberOfStraws[28]=m_numberOfStraws[29]=m_numberOfStraws[30]=21;
    m_numberOfStraws[31]=m_numberOfStraws[32]=m_numberOfStraws[33]=m_numberOfStraws[34]=m_numberOfStraws[35]=22;
    m_numberOfStraws[36]=m_numberOfStraws[37]=m_numberOfStraws[38]=m_numberOfStraws[39]=m_numberOfStraws[40]=23;
    m_numberOfStraws[41]=m_numberOfStraws[42]=24;
    m_numberOfStraws[43]=23;
    m_numberOfStraws[44]=23;
    m_numberOfStraws[45]=m_numberOfStraws[46]=m_numberOfStraws[47]=m_numberOfStraws[48]=24;
    m_numberOfStraws[49]=m_numberOfStraws[50]=m_numberOfStraws[51]=m_numberOfStraws[52]=m_numberOfStraws[53]=25;
    m_numberOfStraws[54]=m_numberOfStraws[55]=m_numberOfStraws[56]=m_numberOfStraws[57]=m_numberOfStraws[58]=26;
    m_numberOfStraws[59]=m_numberOfStraws[60]=m_numberOfStraws[61]=m_numberOfStraws[62]=m_numberOfStraws[63]=27;
    m_numberOfStraws[64]=m_numberOfStraws[65]=m_numberOfStraws[66]=m_numberOfStraws[67]=m_numberOfStraws[68]=28;
    m_numberOfStraws[69]=m_numberOfStraws[70]=m_numberOfStraws[71]=m_numberOfStraws[72]=29;
    m_numberOfStraws[73]=28;

    // loop over straw hash index to create straw number mapping for TRTViewer
    unsigned int maxHash = m_pTRTHelper->straw_layer_hash_max();
    for (unsigned int index = 0; index < maxHash; index++) {
      IdentifierHash idHash = index;
      Identifier id = m_pTRTHelper->layer_id(idHash);

      int idBarrelEndcap = m_pTRTHelper->barrel_ec(id);
      int idLayerWheel = m_pTRTHelper->layer_or_wheel(id);
      int idPhiModule = m_pTRTHelper->phi_module(id);
      int idStrawLayer = m_pTRTHelper->straw_layer(id);

      const InDetDD::TRT_BaseElement * element= nullptr;

      // BARREL
      if (m_pTRTHelper->is_barrel(id)) {
        int idSide = idBarrelEndcap?1:-1;
        if(m_pTRTHelper->barrel_ec(id)==-1) {
          element = m_mgr->getBarrelElement(idSide, idLayerWheel, idPhiModule, idStrawLayer);
          if (element == nullptr) continue;

          for(unsigned int istraw = 0; istraw < element->nStraws(); istraw++) {
            if(istraw>element->nStraws()) continue;

            Identifier strawID = m_pTRTHelper->straw_id(id, int(istraw));
            int i_chip;

            int tempStrawNumber = BarrelStrawNumber(istraw, idStrawLayer, idLayerWheel);

            m_TRTStrawNeighbourSvc->getChip(strawID,i_chip);

            //21 chips in mod layer 0
            //33 chips in mod layer 1
            //50 chips in mod layer 2
            if (idLayerWheel == 1) i_chip+=21;
            if (idLayerWheel == 2) i_chip+=54;

            m_mat_chip_barrel[idPhiModule][tempStrawNumber]=i_chip;
            m_mat_chip_barrel[idPhiModule+32][tempStrawNumber]=i_chip;
          }
        }
      }
      // ENDCAP
      else if (m_pTRTHelper->barrel_ec(id)!=1) {
        int idSide = idBarrelEndcap?2:-2;
        if(((m_pTRTHelper->barrel_ec(id)==-2) || (m_pTRTHelper->barrel_ec(id)==2))) {

          if (m_pTRTHelper->barrel_ec(id)==-2) idSide =0;
          else idSide=1;

          element = m_mgr->getEndcapElement(idSide, idLayerWheel, idStrawLayer, idPhiModule);
          if (element == nullptr) continue;

          for(unsigned int istraw = 0; istraw < element->nStraws(); istraw++) {
            if(istraw>element->nStraws()) continue;

            int tempStrawNumber = EndcapStrawNumber(istraw, idStrawLayer, idLayerWheel, idPhiModule, idSide);

            Identifier strawID = m_pTRTHelper->straw_id(id, int(istraw));

            int i_chip = 0;

            m_TRTStrawNeighbourSvc->getChip(strawID,i_chip);
            i_chip -= 103;

            m_mat_chip_endcap[idPhiModule][tempStrawNumber]=i_chip;
            m_mat_chip_endcap[idPhiModule+32][tempStrawNumber]=i_chip;
          }
        }
      }
    }

    ATH_MSG_DEBUG("TrigT1TRT initilized");
    return StatusCode::SUCCESS;
  }

  //----------------------------------------------
  // execute() method called once per event
  //----------------------------------------------

  StatusCode TrigT1TRT::execute(const EventContext &ctx) const {

    // initialise and empty board score table 
    int barrel_trigger_board[2][32][9] = {{{0}}};
    int endcap_trigger_board[2][32][20] = {{{0}}};

    // initialise and empty empty ttc score table 
    int barrel_trigger_ttc[2][8] = {{0}};
    int endcap_trigger_ttc[2][16] = {{0}};

    // access TRT RDO hits container
    SG::ReadHandle<TRT_RDO_Container> trtRDOs(m_trtRDOKey, ctx);

    ATH_CHECK( trtRDOs.isValid() );

    for (const auto trtRDO : *trtRDOs) {
      const InDetRawDataCollection<TRT_RDORawData>* TRT_Collection(trtRDO);

      if(!TRT_Collection) {
        ATH_MSG_WARNING("InDetRawDataCollection<TRT_RDORawData> is empty");
        continue;
      }
      else {
        // loop over TRT RDOs
        for (const auto p_rdo : *TRT_Collection) {// p_rdo is pointer to trt rdo data vector
          if(!p_rdo)
            ATH_MSG_WARNING("pointer to TRT_RDORawData is nullptr");
          else {
            Identifier TRT_Identifier = p_rdo->identify();

            int barrel_ec = m_pTRTHelper->barrel_ec(TRT_Identifier);

            const TRT_LoLumRawData* p_lolum = dynamic_cast<const TRT_LoLumRawData*>(p_rdo);
            if(!p_lolum) continue;

            // get TRT Identifier (need to know phi module, module layer, straw layer, and straw # with in the layer, to get proper straw numbering.
            TRT_Identifier = p_lolum->identify();
            int phi_module = m_pTRTHelper->phi_module(TRT_Identifier);
            int layer_or_wheel = m_pTRTHelper->layer_or_wheel(TRT_Identifier);
            int straw_layer = m_pTRTHelper->straw_layer(TRT_Identifier);
            int straw = m_pTRTHelper->straw(TRT_Identifier);
            int strawNumber = 0;
            int chip = 0;
            int board = 0;

            // Require good straw status
            if (m_ConditionsSummary->getStatus(TRT_Identifier,ctx) != TRTCond::StrawStatus::Good)
              continue;
            
            if (barrel_ec == 1 || barrel_ec == -1) {
              int side = barrel_ec>0?1:0;             
              strawNumber = BarrelStrawNumber(straw, straw_layer, layer_or_wheel);
              chip = m_mat_chip_barrel[phi_module][strawNumber];
              board = BarrelChipToBoard(chip);
              if (board < 0) {
                ATH_MSG_FATAL( "Failure in BarrelChipToBoard" );
                return StatusCode::FAILURE;
              }

              // check if chip is masked
              std::vector<int> maskedChips = m_maskedChipsBarrel.at(side).at(phi_module);
              if(std::find(maskedChips.begin(), maskedChips.end(), chip) != maskedChips.end())
                continue;

              if ( (p_lolum)->highLevel() ) {
                barrel_trigger_board[side][phi_module][board]++;
              }

            }
            else if (barrel_ec == 2 || barrel_ec == -2) {

              int side = barrel_ec>0?1:0;
              strawNumber = EndcapStrawNumber(straw, straw_layer, layer_or_wheel, phi_module, barrel_ec);
              chip = m_mat_chip_endcap[phi_module][strawNumber];
              board = EndcapChipToBoard(chip);

              // check if chip is masked
              std::vector<int> maskedChips = m_maskedChipsEc.at(side).at(phi_module);
              if(std::find(maskedChips.begin(), maskedChips.end(), chip) != maskedChips.end())
                continue;

              if ( (p_lolum)->highLevel() ) {
                endcap_trigger_board[side][phi_module][board]++;
              }
            }
          }
        }
      }
    }

    // analyse board score table - fill ttc score table
    for (int i=0; i<2; i++) {
      for (int j=0; j<32; j++) {
        for (int k=0; k<9; k++) {
          if (barrel_trigger_board[i][j][k]) {
            barrel_trigger_ttc[i][j%4]++;
          }
        }
        for (int k=0; k<20; k++) {
          if (endcap_trigger_board[i][j][k]) {
            endcap_trigger_ttc[i][j%2]++;
          }
        }
      }
    }

    unsigned int cableWord0 = 0;

    // analyse ttc score table - set cable word
    for (int i=0; i<2; i++) {
      for (int j=0; j<8; j++) {
        if (barrel_trigger_ttc[i][j] >= m_TTCMultiplicity) {
          cableWord0 |= (uint64_t(0x1) << 21); // use of hard coded cable start
        }
      }
      for (int j=0; j<16; j++) {
        if (endcap_trigger_ttc[i][j] >= m_TTCMultiplicity) {
          cableWord0 |= (uint64_t(0x1) << 21); // use of hard coded cable start
        }
      }
    }

    ATH_MSG_DEBUG( " cableWord: " << cableWord0 );

    // form CTP obejct
    SG::WriteHandle<TrtCTP> trtCTP = SG::makeHandle(m_trtCTPLocation, ctx);

    // record CTP object
    ATH_CHECK(trtCTP.record(std::make_unique<TrtCTP>(cableWord0)));
    ATH_MSG_DEBUG("Stored TRT CTP object with bit " << std::dec << cableWord0);

    return StatusCode::SUCCESS;
  }


  //----------------------------------------------
  // trigger logic methods
  //----------------------------------------------

  int TrigT1TRT::BarrelChipToBoard(int chip) const {
    // return logical board index:
    // 0 for Board 1S (has 10 chips)  0 -  9
    // 1 for 1L (11)                 10 - 20
    // 2 for 2S (15)                 21 - 35
    // 3 for 2L, first 9 chips       36 - 44
    // 4 for 2L, second 9 chips      45 - 53
    // 5 for 3S, first 11            54 - 64
    // 6 for 3S, second 12           65 - 76
    // 7 for 3L, first 13            77 - 89 
    // 8 for 3L, second 14           90 - 103

    int list[] = {10, 11, 15, 9, 9, 11, 12, 13, 14};
    int count = 0;
    chip--;

    for (int i=0; i<9; i++) {
      count += list[i];
      if (chip < count) return i+1;
      else if (chip == 104) return 9;
    }

    throw std::runtime_error("Board not found!");
    return -1;
  }

  int TrigT1TRT::EndcapChipToBoard(int chip) const {
    int Board = -1;

    int remainder = (chip-1) % 12;
    Board = int(((chip -1) - remainder) / 12);
    return Board;
  }

  int TrigT1TRT::EndcapStrawNumber(int strawNumber, int strawLayerNumber, int LayerNumber, int phi_stack, int side) const {

    // before perfoming map, corrections need to be perfomed.
    //////// apply special rotations for endcap mappings /////

    // for eca, rotate triplets by 180 for stacks 9-16, and 25-32.
    static const int TripletOrientation[2][32] = { 
      {1,1,1,1,1,1,1,1, 
        0,0,0,0,0,0,0,0, 
        1,1,1,1,1,1,1,1, 
        0,0,0,0,0,0,0,0},
      {1,1,1,1,1,1,1,1, 
        0,0,0,0,0,0,0,0, 
        1,1,1,1,1,1,1,1, 
        0,0,0,0,0,0,0,0}
    };

    int phi1=-1; 
    if(side==2) phi1=phi_stack, side=1; 
    else if (side==-2) phi1=31-phi_stack, side=0; 
    if (phi1>-1){
      if (TripletOrientation[side][phi1]){
        // change straw number from 0-23 in straw layer to 0-192
        if (strawLayerNumber < 8)strawNumber = strawNumber + 24*strawLayerNumber;
        if (strawLayerNumber > 7)strawNumber = strawNumber + 24*(strawLayerNumber -8);
        strawNumber = (192-1)*TripletOrientation[side][phi1]+strawNumber*(1-2*TripletOrientation[side][phi1]);//actual rotation

        // take strawNumber back to 0-23
        if (strawLayerNumber<8) strawLayerNumber = int(strawNumber/24);
        if (strawLayerNumber>7) strawLayerNumber = int(strawNumber/24) + 8;
        strawNumber = strawNumber%24;
      }
    
      // finish rotation
    
      // flip straw in layer.
    
      if (side==0) strawNumber = 23 - strawNumber;
    
      // finish flipping
    }

    // done with corrections

    // start mapping from athena identifiers to TRTViewer maps
    int strawNumberNew=0;

    if(LayerNumber<6 && strawLayerNumber>7) {
      strawNumberNew=strawNumberNew+(384*LayerNumber);
      strawNumberNew=strawNumberNew+192+(strawLayerNumber%8)+(strawNumber*8);
    }
    else if(LayerNumber<6 && strawLayerNumber<8) {
      strawNumberNew=strawNumberNew+(384*LayerNumber);
      strawNumberNew=strawNumberNew + (strawLayerNumber%8) + (strawNumber*8);
    }
    else if(LayerNumber>5 && strawLayerNumber>7) {
      strawNumberNew = strawNumberNew + 2304 + 192*(LayerNumber-6);
      strawNumberNew = strawNumberNew + 192 + (strawLayerNumber%8) + (8*strawNumber);
    } 
    else if(LayerNumber>5 && strawLayerNumber<8) {
      strawNumberNew = strawNumberNew + 2304 + 192*(LayerNumber-6);
      strawNumberNew = strawNumberNew + (strawLayerNumber%8) + (8*strawNumber);
    }

    strawNumber=strawNumberNew;

    return strawNumber;
  }

  int TrigT1TRT::BarrelStrawNumber(int strawNumber, int strawlayerNumber, int LayerNumber) const {
    int addToStrawNumber=0;
    int addToStrawNumberNext=0;
    int i=0;

    do {
      i++;
      addToStrawNumber+=m_numberOfStraws[i-1];
      addToStrawNumberNext = addToStrawNumber+m_numberOfStraws[i];
    }
    while(BarrelStrawLayerNumber(strawlayerNumber,LayerNumber)!=i-1);
    strawNumber = addToStrawNumberNext - strawNumber-1;
    return strawNumber;
  }

  int TrigT1TRT::BarrelStrawLayerNumber(int strawLayerNumber, int LayerNumber) const {
    if(LayerNumber==0) {
      strawLayerNumber+=0;
    } else if(LayerNumber==1) {
      strawLayerNumber+=19;
    } else if(LayerNumber==2) {
      strawLayerNumber+=43;
    }
    return strawLayerNumber;
  }

}
