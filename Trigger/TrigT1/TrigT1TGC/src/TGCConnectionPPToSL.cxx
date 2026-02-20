/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCConnectionPPToSL.h"
#include "TrigT1TGC/TGCDatabaseManager.h"
#include <fstream>
#include <sstream>
#include <string>

#include "PathResolver/PathResolver.h"

namespace LVL1TGCTrigger {

TGCConnectionPPToSL::TGCConnectionPPToSL():
  m_PPToSB(),
  m_SBToHPB(),
  m_HPBToSL()
{
}

TGCConnectionPPToSL::TGCConnectionPPToSL(const TGCConnectionPPToSL& right):
  m_PPToSB(right.m_PPToSB),
  m_SBToHPB(right.m_SBToHPB),
  m_HPBToSL(right.m_HPBToSL)
{
}


TGCConnectionPPToSL& TGCConnectionPPToSL::operator =(const TGCConnectionPPToSL& right)
{ 
  if (this != &right) {
    m_PPToSB  = right.m_PPToSB;
    m_SBToHPB = right.m_SBToHPB;
    m_HPBToSL = right.m_HPBToSL;
  }
  return *this;
}

bool TGCConnectionPPToSL::readData(TGCRegionType type)
{
  std::string fn = TGCDatabaseManager::getFilename(2);    // PP2SL.db
  std::string fullpath = PathResolver::find_file(fn, "PWD");
  if (fullpath.length() == 0)
    fullpath = PathResolver::find_file(fn, "DATAPATH");
  std::ifstream inputfile(fullpath.c_str(), std::ios::in);

  static constexpr int BufferSize = 512;
  char buf[BufferSize];

  // find entries match in region type
  bool isMatched = false;
  while (inputfile && inputfile.getline(buf,BufferSize)) {
    std::istringstream line(buf);

    std::string region;
    line >> region ;
    isMatched = (region == "Endcap" && type == TGCRegionType::ENDCAP) ||
                (region == "Forward" && type == TGCRegionType::FORWARD);
    if (isMatched) break;
  }
  if (!isMatched) return false;

  // read entries for HighPtBoard
  if (inputfile.getline(buf, BufferSize)) {
    std::istringstream infileStr(buf);
    std::string boardName;
    infileStr >> boardName;

    for (int itype=0; itype < m_HPBToSL.getNumberOfType(); itype++) {
      int board_number{0};
      infileStr >> board_number;
      //coverity[TAINTED_SCALAR]
      m_HPBToSL.setNumber(itype, board_number);
      for (int j=0; j < board_number; j++) {
	inputfile.getline(buf, BufferSize);
	std::istringstream infileStr2(buf);
        int id, port;
	infileStr2 >> id >> port;
	m_HPBToSL.setId(itype, j, id);
	//coverity[TAINTED_SCALAR]
	m_HPBToSL.setSLPortToHPB(itype, id, port);
      }
    }
  }

  // read entries for SlaveBoard
  if (inputfile.getline(buf, BufferSize)) {
    std::istringstream infileStr(buf);
    std::string boardName;
    infileStr >> boardName;

    for (int itype=0; itype < m_SBToHPB.getNumberOfType(); itype++) {
      // No HPT board for Inner
      if (itype == WISB || itype == SISB) continue;
      int board_number{0};
      infileStr >> board_number;
      //coverity[TAINTED_SCALAR]
      m_SBToHPB.setNumber(itype, board_number);
      for (int j=0; j < board_number; j++) {
	inputfile.getline(buf, BufferSize);
	std::istringstream infileStr2(buf);
        int id, idHPB, port;
	infileStr2 >> id >> idHPB >> port;
        if (id < 0 || idHPB < 0 || port < 0) continue;
	m_SBToHPB.setId(itype, j, id);   // BoardType, Number in a type, id
	//coverity[TAINTED_SCALAR]
	m_SBToHPB.setHPBIdToSB(itype, id, idHPB);
	m_SBToHPB.setHPBPortToSB(itype, id, port);
      }
    }
  }

  // read entries for PatchPanel
  if (inputfile.getline(buf, BufferSize)) {
    std::istringstream infileStr(buf);
    std::string boardName;
    infileStr >> boardName;

    for (int itype=0; itype < m_PPToSB.getNumberOfType(); itype++) {
      int board_number{0};
      infileStr >> board_number;
      //coverity[TAINTED_SCALAR]
      m_PPToSB.setNumber(itype, board_number);
      for(int j=0; j<board_number; j++) {
        inputfile.getline(buf, BufferSize);
        std::istringstream infileStr2(buf);
        int id{0};     // PP ID
        int idSB1{0};  // SB ID for Part0
        int idSB2{0};  // SB ID for Part1
        infileStr2 >> id >> idSB1 >> idSB2;
        m_PPToSB.setId(itype, j, id);
        m_PPToSB.setSBIdToPP(itype, 0, j, idSB1);  //!! assume id = index
        m_PPToSB.setSBIdToPP(itype, 1, j, idSB2);  //!! assume id = index 
      }
    }
  }

  return true;
}

}  // end of namespace
