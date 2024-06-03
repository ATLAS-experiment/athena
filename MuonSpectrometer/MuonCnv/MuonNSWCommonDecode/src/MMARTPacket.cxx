/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>
#include <algorithm>
#include <tuple>

#include "MuonNSWCommonDecode/MMARTPacket.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"


Muon::nsw::MMARTPacket::MMARTPacket (std::vector<uint32_t>& payload) {

  std::size_t readPointer{0};
  std::span<const std::uint32_t> data{payload.data(), 3}; 
  
  if (payload.size()!=3) {
    throw std::runtime_error( Muon::nsw::format( "ART Packet size not as expected: expected exactly 3 uint32_t, got {}", payload.size() ));
  }

  //decoding
  m_art_BCID =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMART::size_art_BCID);
  m_art_pipeID =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMART::size_art_pipeID);
  m_art_fiberID = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMART::size_art_fiberID);
  m_art_VMMmap =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMART::size_art_VMMmap);
  //remember ART 7 is first in the bit stream, ART 0 is last!
  for (uint8_t i = 0; i < 8; i++){
    m_art_ARTs.push_back( Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMART::size_art_ARTs) );
  }
  std::reverse(m_art_ARTs.begin(), m_art_ARTs.end()); //so that ART0 is element0 and no confusion is made
  
  //populating layer,channel pair
  std::vector<std::tuple<uint8_t,uint8_t,uint8_t>> artHitInfo = VMMmapToHits();

  for (uint8_t i = 0; i < artHitInfo.size(); i++){
      m_channels.push_back( std::pair<uint8_t,uint16_t>(getLayer(), getVMMChannelPosition( getBoardPosition(std::get<0>(artHitInfo[i])) , std::get<1>(artHitInfo[i]) , std::get<2>(artHitInfo[i]))) );
  }

}


std::vector<std::tuple<uint8_t,uint8_t,uint8_t>> Muon::nsw::MMARTPacket::VMMmapToHits (){
  std::vector<std::tuple<uint8_t,uint8_t>> hitmapBoardVMM;
  std::vector<std::tuple<uint8_t,uint8_t,uint8_t>> artHitInfo;

  for (uint8_t i = 0; i<32; i++){
    if ((m_art_VMMmap >> i) & 0b1) {
      hitmapBoardVMM.push_back( std::make_tuple( (int)(i/8), i%8 ) ); //board, vmm
    }
  }

  if (hitmapBoardVMM.size()>8) {
    throw std::runtime_error("ART Packet cannot contain more than 8 ART hits!");
  }

  for (uint8_t i = 0; i<hitmapBoardVMM.size(); i++){
    artHitInfo.push_back( std::make_tuple( std::get<0>(hitmapBoardVMM[i]), std::get<1>(hitmapBoardVMM[i]), m_art_ARTs[i] ) );
  }

  return artHitInfo;
}


int Muon::nsw::MMARTPacket::getBoardPosition ( int board ){
  int rightLeft = (int)(m_art_fiberID/2.) % 2;
  int layer = getLayer();

  int flipBoardOrder = 0;
  int flipFiberOrder = 0;

  if (rightLeft==0){
    if ( layer==1 || layer==2 || layer==5 || layer==6 ) {flipFiberOrder = 1;}
    else {flipBoardOrder = 1;}
  } else {
    if ( layer==1 || layer==2 || layer==5 || layer==6 ) {flipBoardOrder = 1;}
    else {flipFiberOrder = 1;}
  }
  if (flipBoardOrder) board = 3-board;

  int pcb = 0;
  if (!flipFiberOrder){
      if (m_art_fiberID % 2 == 0) {pcb = board;}
      else {pcb = board+4;}
  } else {
      if (m_art_fiberID % 2 == 1) {pcb = board;}
      else {pcb = board+4;}
  }
  pcb+=1;

  int boardPosition = -1;
  if (layer%2==1){
      if (rightLeft==1) {boardPosition = (pcb-1) * 2;}
      else {boardPosition = (pcb-1) * 2 + 1;}
  } else {
      if (rightLeft==0) {boardPosition = (pcb-1) * 2;}
      else {boardPosition = (pcb-1) * 2 + 1;}
  }

  return boardPosition;

}

int Muon::nsw::MMARTPacket::getVMMChannelPosition (int boardPosition, int vmm, int ch){
  int vmmPosition;
  if (boardPosition%2==0){vmmPosition = 7-vmm;}
  else {vmmPosition = vmm;}

  vmmPosition += boardPosition*8;
  int chPosition = 64*vmmPosition;

  if (boardPosition%2==0){chPosition += (63 - ch);}
  else {chPosition += ch;}

  return chPosition;

}
