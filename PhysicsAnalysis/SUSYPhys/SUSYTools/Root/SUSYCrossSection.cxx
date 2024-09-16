/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Class header
#include "SUSYTools/SUSYCrossSection.h"

// Find the xsec files in datapath
#include "PathResolver/PathResolver.h"

// For directory commands
#include <dirent.h>

// Error messages
#include <iostream>

// Text file i/o
#include <fstream>
#include <array>

namespace{
   enum Sparticles{
    UNKNOWN = -1,
    ngluino,
    nsquark, // (up and down type without bottom/top)
    nantisquark, // (up and down type without bottom/top)
  
    nsbottom,
    nstop,
    nsbottom2,
    nstop2,
    nantisbottom,
    nantistop,
    nantisbottom2,
    nantistop2,
  
    nchi01,
    nchi02,
    nchi03,
    nchi04,
    nch1plus,
    nch2plus,
    nch1minus,
    nch2minus,
  
   //sleptons
    nsmuonRplus,
    nsmuonRminus,
    nselecRplus,
    nselecRminus,
  
    nsmuonLplus,
    nsmuonLminus,
    nselecLplus,
    nselecLminus,
  
    nstau1plus,
    nstau1minus,
    nstau2plus,
    nstau2minus,
  
    //snutrinos
    nselnuL,
    nsmunuL,
    nstaunuL,
    NUM_SPARTICLES
  };
  
  Sparticles
  classifyOne(int pdgId){
    if      (abs(pdgId) == 1000022) return nchi01;
    else if (abs(pdgId) == 1000023) return nchi02;
    else if (abs(pdgId) == 1000025) return nchi03;
    else if (abs(pdgId) == 1000035) return nchi04;
    else if (    pdgId == 1000024) return nch1plus;
    else if (    pdgId == -1000024) return nch1minus;
    else if (    pdgId == 1000037) return nch2plus;
    else if (    pdgId == -1000037) return nch2minus;
    else if (    pdgId == 1000021) return ngluino;
    else if ((abs(pdgId) > 1000000 && abs(pdgId) <= 1000004) || (abs(pdgId) > 2000000 && abs(pdgId) <= 2000004)) {
      if (pdgId > 0) return nsquark;
      else return nantisquark;
    }
    else if (pdgId == 1000005) return nsbottom;
    else if (pdgId == 1000006) return nstop;
    else if (pdgId == 2000005) return nsbottom2;
    else if (pdgId == 2000006) return nstop2;
    else if (pdgId == -1000005) return nantisbottom;
    else if (pdgId == -1000006) return nantistop;
    else if (pdgId == -2000005) return nantisbottom2;
    else if (pdgId == -2000006) return nantistop2;
    else if (pdgId == 2000011) return nselecRminus;
    else if (pdgId == -2000011) return nselecRplus;
    else if (pdgId == 1000011) return nselecLminus;
    else if (pdgId == -1000011) return nselecLplus;
    else if (abs(pdgId) == 1000012) return nselnuL;
    else if (pdgId == 2000013) return nsmuonRminus;
    else if (pdgId == -2000013) return nsmuonRplus;
    else if (pdgId == 1000013) return nsmuonLminus;
    else if (pdgId == -1000013) return nsmuonLplus;
    else if (abs(pdgId) == 1000014) return nsmunuL;
    else if (pdgId == 1000015) return nstau1minus;
    else if (pdgId == -1000015) return nstau1plus;
    else if (pdgId == 2000015) return nstau2minus;
    else if (pdgId == -2000015) return nstau2plus;
    else if (abs(pdgId) == 1000016) return nstaunuL;
    return UNKNOWN;
  }
  

}

SUSY::CrossSectionDB::CrossSectionDB(const std::string& txtfilename, bool usePathResolver, bool isExtended, bool usePMGTool)
  : m_pmgxs("")
{
  setExtended(isExtended);
  setUsePMGTool(usePMGTool);

  // configuring the PMG tool... 
  m_pmgxs.setTypeAndName("PMGTools::PMGCrossSectionTool/PMGCrossSectionTool");
  m_pmgxs.retrieve().ignore(); // Ignore the status code
  std::vector<std::string> inFiles={};
  std::string fullPath = usePathResolver ? PathResolverFindCalibFile(txtfilename) : txtfilename;
  std::ifstream in(fullPath.c_str());
  if (!in) return;
  inFiles.push_back(fullPath);
  m_pmgxs->readInfosFromFiles( inFiles );

}

void SUSY::CrossSectionDB::loadFile(const std::string& txtfilename){

  std::string line;
  
  std::ifstream in(txtfilename.c_str());
  if (!in) return;
  while ( getline(in, line) ){
    // skip leading blanks (in case there are some in front of a comment)
    if ( !line.empty() ){
      while ( line[0] == ' ' ) line.erase(0, 1);
    }
    // skip lines that do not start with a number, they are comments
    if ( !line.empty() && isdigit(line[0]) ){
      std::stringstream is(line);
      int id;
      std::string name;
      float xsect, kfactor, efficiency, relunc;
      float sumweight = -1, stat = -1;
      is >> id >> name >> xsect >> kfactor >> efficiency >> relunc;
      if (m_extended == true){
          is >> sumweight >> stat;
      }
      m_xsectDB[Key(id, name)] = Process(id, name, xsect, kfactor, efficiency, relunc, sumweight, stat);
    }
  }
}

// Convenient accessor for finding based on *only* a process ID
SUSY::CrossSectionDB::xsDB_t::iterator SUSY::CrossSectionDB::my_find( const int proc ) {
  for (SUSY::CrossSectionDB::xsDB_t::iterator it = m_xsectDB.begin(); it!=m_xsectDB.end();++it){
    if (it->second.ID()==proc) return it;
  }
  return m_xsectDB.end();
}

// Extend the record based on information from a second file
void SUSY::CrossSectionDB::extend(const std::string& txtfilename){
  // Just like the above function, but with more functionality
  std::string line;

  std::ifstream in(txtfilename.c_str());
  if (!in) return;
  while ( getline(in, line) )
    {
      // skip leading blanks (in case there are some in front of a comment)
      if ( !line.empty() ){
          while ( line[0] == ' ' ) line.erase(0, 1);
      }
      // skip lines that do not start with a number, they are comments
      if ( !line.empty() && isdigit(line[0]) ){
          std::stringstream is(line);
          int id;
          std::string name;
          float xsect, kfactor, efficiency, relunc;
          float sumweight = -1, stat = -1;
          is >> id;
          auto my_it = my_find( id );
          if (my_it==m_xsectDB.end()){
            is >> name >> xsect >> kfactor >> efficiency >> relunc;
            if (m_extended == true){
              is >> sumweight >> stat;
            }
            m_xsectDB[Key(id, name)] = Process(id, name, xsect, kfactor, efficiency, relunc, sumweight, stat);
          } else {
            // Now we have extended records
            if (!m_extended) m_extended=true;
            is >> sumweight >> stat;
            my_it->second.sumweight(sumweight);
            my_it->second.stat(stat);
          }
        }
    }

}

SUSY::CrossSectionDB::Process SUSY::CrossSectionDB::process(int id, int proc) const
{
  // for background x-sections, use the PMG tool
  if(proc==0 && m_usePMGTool) {
    return Process( id, m_pmgxs->getSampleName(id), m_pmgxs->getAMIXsection(id), m_pmgxs->getKfactor(id), m_pmgxs->getFilterEff(id), m_pmgxs->getXsectionUncertainty(id), -1, -1 );  
  } else {
    const Key k(id, proc);
    xsDB_t::const_iterator pos = m_cache.find(k);
    if (pos != m_cache.end()) {
      return pos->second;
    } else {
      pos = m_xsectDB.find(k);
      if (pos != m_xsectDB.end()) {
	return pos->second;
      }
    }
  }
  return Process();
}


unsigned int SUSY::finalState(const int SUSY_Spart1_pdgId, const int SUSY_Spart2_pdgId)
{
  std::array<int, NUM_SPARTICLES> n{};
  //Classification of the event follows (gg, sq...):
  int idx = classifyOne(SUSY_Spart1_pdgId);
  if ( (idx>=0) and (idx<std::ssize(n) )){
    ++n[idx];
  }
  int idx2 = classifyOne(SUSY_Spart2_pdgId);
  if ( (idx2>=0) and (idx2<std::ssize(n) )){
    ++n[idx2];
  }
 
  ///Final classification
  // gluino/squark + X
  auto nanysquark=[&n](int i)->bool {
    return (n[nsquark] == i) or (n[nantisquark] == i);
  };
  if (n[ngluino] == 1 && nanysquark(1)) return 1;
  else if (n[ngluino] == 2) return 2;
  else if (nanysquark(2)) return 3;
  else if (n[nsquark] == 1 && n[nantisquark] == 1) return 4;

  else if (n[nsbottom] == 1 && n[nantisbottom] == 1) return 51;
  else if (n[nsbottom2] == 1 && n[nantisbottom2] == 1) return 52;
  else if (n[nstop] == 1 && n[nantistop] == 1) return 61;
  else if (n[nstop2] == 1 && n[nantistop2] == 1) return 62;

  else if (n[ngluino] == 1 && n[nchi01] == 1) return 71;
  else if (n[ngluino] == 1 && n[nchi02] == 1) return 72;
  else if (n[ngluino] == 1 && n[nchi03] == 1) return 73;
  else if (n[ngluino] == 1 && n[nchi04] == 1) return 74;

  else if (n[ngluino] == 1 && n[nch1plus] == 1) return 75;
  else if (n[ngluino] == 1 && n[nch2plus] == 1) return 76;
  else if (n[ngluino] == 1 && n[nch1minus] == 1) return 77;
  else if (n[ngluino] == 1 && n[nch2minus] == 1) return 78;

  else if (nanysquark(1) && n[nchi01] == 1) return 81;
  else if (nanysquark(1) && n[nchi02] == 1) return 82;
  else if (nanysquark(1) && n[nchi03] == 1) return 83;
  else if (nanysquark(1) && n[nchi04] == 1) return 84;

  else if (nanysquark(1) && n[nch1plus] == 1) return 85;
  else if (nanysquark(1) && n[nch2plus] == 1) return 86;
  else if (nanysquark(1) && n[nch1minus] == 1) return 87;
  else if (nanysquark(1) && n[nch2minus] == 1) return 88;


  // Gaugino pair-production
  // chi^{0}_1 + X
  else if (n[nchi01] == 2) return 111;
  else if (n[nchi01] == 1 && n[nchi02] == 1) return 112;
  else if (n[nchi01] == 1 && n[nchi03] == 1) return 113;
  else if (n[nchi01] == 1 && n[nchi04] == 1) return 114;
  else if (n[nchi01] == 1 && n[nch1plus] == 1) return 115;
  else if (n[nchi01] == 1 && n[nch2plus] == 1) return 116;
  else if (n[nchi01] == 1 && n[nch1minus] == 1) return 117;
  else if (n[nchi01] == 1 && n[nch2minus] == 1) return 118;

  // chi^{0}_2 + X
  else if (n[nchi02] == 2) return 122;
  else if (n[nchi02] == 1 && n[nchi03] == 1) return 123;
  else if (n[nchi02] == 1 && n[nchi04] == 1) return 124;
  else if (n[nchi02] == 1 && n[nch1plus] == 1) return 125;
  else if (n[nchi02] == 1 && n[nch2plus] == 1) return 126;
  else if (n[nchi02] == 1 && n[nch1minus] == 1) return 127;
  else if (n[nchi02] == 1 && n[nch2minus] == 1) return 128;

  // chi^{0}_3 + X
  else if (n[nchi03] == 2) return 133;
  else if (n[nchi03] == 1 && n[nchi04] == 1) return 134;
  else if (n[nchi03] == 1 && n[nch1plus] == 1) return 135;
  else if (n[nchi03] == 1 && n[nch2plus] == 1) return 136;
  else if (n[nchi03] == 1 && n[nch1minus] == 1) return 137;
  else if (n[nchi03] == 1 && n[nch2minus] == 1) return 138;

  // chi^{0}_4 + X
  else if (n[nchi04] == 2) return 144;
  else if (n[nchi04] == 1 && n[nch1plus] == 1) return 145;
  else if (n[nchi04] == 1 && n[nch2plus] == 1) return 146;
  else if (n[nchi04] == 1 && n[nch1minus] == 1) return 147;
  else if (n[nchi04] == 1 && n[nch2minus] == 1) return 148;

  // chi^{+}_1/2 + chi^{-}_1/2
  else if (n[nch1plus] == 1 && n[nch1minus] == 1) return 157;
  else if (n[nch1plus] == 1 && n[nch2minus] == 1) return 158;

  else if (n[nch2plus] == 1 && n[nch1minus] == 1) return 167;
  else if (n[nch2plus] == 1 && n[nch2minus] == 1) return 168;

  // slepton
  else if (n[nselecLplus] == 1 && n[nselecLminus] == 1) return 201; // sElectronLPair
  else if (n[nselecRplus] == 1 && n[nselecRminus] == 1) return 202; // sElectronRPair
  else if (n[nselnuL] == 2) return 203; // sElectron neutrino pair
  else if (n[nselecLplus] == 1 && n[nselnuL] == 1) return 204; // sElectron+ sNutrino
  else if (n[nselecLminus] == 1 && n[nselnuL] == 1) return 205; // sElectron- sNutrino
  else if (n[nstau1plus] == 1 && n[nstau1minus] == 1) return 206;
  else if (n[nstau2plus] == 1 && n[nstau2minus] == 1) return 207;
  else if ((n[nstau1plus] == 1 || n[nstau1minus] == 1) && (n[nstau2plus] == 1 || n[nstau2minus] == 1)) return 208;
  else if (n[nstaunuL] == 2) return 209; // sTau neutrino pair
  else if (n[nstau1plus] == 1 && n[nstaunuL] == 1) return 210;
  else if (n[nstau1minus] == 1 && n[nstaunuL] == 1) return 211;
  else if (n[nstau2plus] == 1 && n[nstaunuL] == 1) return 212;
  else if (n[nstau2minus] == 1 && n[nstaunuL] == 1) return 213;

  else if (n[nsmuonLplus] == 1 && n[nsmuonLminus] == 1) return 216; // sMuonPair
  else if (n[nsmuonRplus] == 1 && n[nsmuonRminus] == 1) return 217; // sMuonPair
  else if (n[nsmunuL] == 2) return 218; // sMuon neutrino pair
  else if (n[nsmuonLplus] == 1 && n[nsmunuL] == 1) return 219; // sMuon+ sNutrino
  else if (n[nsmuonLminus] == 1 && n[nsmunuL] == 1) return 220; // sMuon- sNutrino

  std::cerr << "ERROR. could not determine finalState for:" << std::endl;
  std::cerr << "  SUSY_Spart1_pdgId: " << SUSY_Spart1_pdgId << std::endl;
  std::cerr << "  SUSY_Spart2_pdgId: " << SUSY_Spart2_pdgId << std::endl;
  std::cerr << "Returning 0" << std::endl;

  return 0;
}
