/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1MuctpiPhase1/L1TopoLUT.h"

#define BOOST_BIND_GLOBAL_PLACEHOLDERS // silence Boost pragma message (fixed in Boost 1.76)
#include <boost/property_tree/json_parser.hpp>

#include <fstream>
#include <map>
#include <iostream>
#include <sstream>

namespace LVL1MUCTPIPHASE1
{
  void L1TopoLUT::fillFromPtree(const boost::property_tree::ptree& node, std::map<unsigned short,std::vector<float>>& theLut) const
  {
    theLut.clear();
    size_t outerIndex = 0;
    for (const boost::property_tree::ptree::value_type& outer_elem : node){
      
      std::vector<float> buff(outer_elem.second.size()); 
      size_t index = 0;
      for (const boost::property_tree::ptree::value_type& inner_elem : outer_elem.second) {
        buff[index] = inner_elem.second.get_value<float>(); 
        ++index; //count index manually to avoid some stoi
      }
      //cannot get around stoi as outer layer may be non-continuous (some indices/keys are not present)
      if (outer_elem.first.length() > 0) { //string type key
        theLut[std::stoi(outer_elem.first)] = std::move(buff);
      } else {
        theLut[outerIndex] = std::move(buff);
        ++outerIndex; //ok to only count in this case, mixed keys/indices at same depth are not valid 
      }
    }
    return;
  }
  
  void L1TopoLUT::fillFromPtree(const boost::property_tree::ptree& node, std::map<unsigned short,std::map<unsigned short, std::pair<unsigned short, unsigned short>>>& theLut) const
  {
    theLut.clear();
    for (const boost::property_tree::ptree::value_type& outer_elem : node){
      
      std::map<unsigned short,std::pair<unsigned short, unsigned short>> buff;
      for (const boost::property_tree::ptree::value_type& inner_elem : outer_elem.second) {
        std::vector<unsigned short> codeBuff;
        for (const boost::property_tree::ptree::value_type& code_elem : inner_elem.second) {
          codeBuff.push_back( code_elem.second.get_value<unsigned short>() );
        }
        if ( codeBuff.size() != 2 ) {
          //stricter error checking/handling comes later
          continue;
        }
        buff[std::stoi(inner_elem.first)] = { codeBuff[0], codeBuff[1] } ; 
      }
      //cannot get around stoi as outer layer may be non-continuous (some indices/keys are not present)
      theLut[std::stoi(outer_elem.first)] = std::move(buff);
    }
    return;
  }
  
  
  bool L1TopoLUT::initializeCompactedLUTs(const std::string& side0LUTFileName,
				      const std::string& side1LUTFileName) {    
    bool success = true;
    boost::property_tree::ptree root0;
    boost::property_tree::read_json(side0LUTFileName.c_str(), root0);
    boost::property_tree::ptree root1;
    boost::property_tree::read_json(side1LUTFileName.c_str(), root1);

    //Barrel encoding
    fillFromPtree(root0.get_child("encode_lookup_barrel") , m_barrel_encoding0); 
    fillFromPtree(root1.get_child("encode_lookup_barrel") , m_barrel_encoding1);
    
    //Barrel, A-side
    fillFromPtree(root0.get_child("lookup_barrel_eta") , m_barrel_eta_lookup0);
    fillFromPtree(root0.get_child("lookup_barrel_phi") , m_barrel_phi_lookup0);
    //Barrel, C-side
    fillFromPtree(root1.get_child("lookup_barrel_eta") , m_barrel_eta_lookup1);
    fillFromPtree(root1.get_child("lookup_barrel_phi") , m_barrel_phi_lookup1);
    //Endcap, A-side
    fillFromPtree(root0.get_child("lookup_endcap_eta") , m_endcap_eta_lookup0);
    fillFromPtree(root0.get_child("lookup_endcap_phi") , m_endcap_phi_lookup0);
    //Endcap, C-side
    fillFromPtree(root1.get_child("lookup_endcap_eta") , m_endcap_eta_lookup1);
    fillFromPtree(root1.get_child("lookup_endcap_phi") , m_endcap_phi_lookup1);
    //Forward, A-side
    fillFromPtree(root0.get_child("lookup_forward_eta") , m_forward_eta_lookup0);
    fillFromPtree(root0.get_child("lookup_forward_phi") , m_forward_phi_lookup0);
    //Forward, C-side
    fillFromPtree(root1.get_child("lookup_forward_eta") , m_forward_eta_lookup1);
    fillFromPtree(root1.get_child("lookup_forward_phi") , m_forward_phi_lookup1);
    
    
    if (m_barrel_encoding1.size() && m_barrel_encoding1.size() &&
        m_barrel_phi_lookup1.size() &&  m_barrel_phi_lookup0.size() && 
        m_endcap_phi_lookup1.size() &&  m_endcap_phi_lookup0.size() &&
        m_forward_phi_lookup1.size() &&  m_forward_phi_lookup0.size() ) {
      return success;
    } 
    return false;
  }

  bool L1TopoLUT::initializeLUT(const std::string& barrelFileName,
				const std::string& ecfFileName,
				const std::string& side0LUTFileName,
				const std::string& side1LUTFileName) {
    m_encoding.clear();
    m_errors.clear();
    
    bool success = true;

    // read the compacted LUTs containing the simplified eta/phi values
    // used for encoding/decoding for/at L1Topo
    success = success && initializeCompactedLUTs(side0LUTFileName, side1LUTFileName);

    //read remaining info from the json files 
    //(the input labels are swapped between A-side and C-side)
    //success = success && initializeJSON(side1LUTFileName, 0);
    //success = success && initializeJSON(side0LUTFileName, 1);
    
    //read the barrel file
    success = success && initializeLUT(barrelFileName, true);

    //read the endcap+forward file
    success = success && initializeLUT(ecfFileName, false);

    return success;
  }

  bool L1TopoLUT::initializeLUT(const std::string& inFileName, const bool& isBarrel) {
    //map between L1TopoLUTKey -> map between eta/phi -> index
    std::unordered_map<L1TopoLUTKey, std::map<float, unsigned short>, L1TopoLUTKeyHasher> sector_eta_indices, sector_phi_indices;

    std::ifstream inFile(inFileName.c_str());
    if (!inFile) return false;
    while (!inFile.eof() && inFile.good()) {
      unsigned short side;
      unsigned short subsystem=0;
      unsigned short sectorID;
      unsigned short roi;
      double eta_min, eta_max;
      double phi_min, phi_max;

      inFile >> side;
      if (inFile.eof()) { break; }

      if (!isBarrel) {
	      inFile >> subsystem;      
	      //in the EC+FW file, EC=0 and FW=1, so increment to distinguish from barrel
	      subsystem++;
	      side = !side; //EC+FW file uses inverted side numbers w.r.t. Barrel...
      } 
      //at this point the "side" convention is
      // 0 -> C side
      // 1 -> A side
      // the LUT json file naming, however, if inverse (follows EC+FW file...)
      
      inFile >> sectorID;
      inFile >> roi;
      inFile >> eta_min;
      inFile >> eta_max;
      inFile >> phi_min;
      inFile >> phi_max;

      double eta = getCompactedValue_eta(subsystem, side, sectorID, roi);
      // coverity [tainted_data]
      double phi = getCompactedValue_phi(subsystem, side, sectorID, roi);
      
      unsigned short ieta = 0;
      unsigned short iphi = 0;
      if (subsystem == 0) { //barrel
        //note: interpretation of values 0/1 for side flipped on purpose here (different conventions)!
        ieta = (side ? m_barrel_encoding0 : m_barrel_encoding1)[sectorID][roi].first;
        iphi = (side ? m_barrel_encoding0 : m_barrel_encoding1)[sectorID][roi].second;
      } else if (subsystem == 1) { //endcap
        ieta  = (roi>>2); //eta part of roi
        ieta |= 1 << 6; //detector code
        iphi  = roi & 0x3; //phi part of roi
        iphi |= (sectorID & 0x3F) << 2; 
      } else if (subsystem == 2) { //forward
        ieta  = (roi>>2); //eta part of roi
        ieta |= 1 << 5; //detector code
        iphi  = (roi & 0x3) << 1; //phi part of roi
        iphi |= (sectorID & 0x1F) << 3; 
      } 
      //truncate to 8bit values each:
      ieta &= 0xFF;
      iphi &= 0xFF;
     
      if (subsystem == 0) {
        //populate reverse map for barrel
        unsigned short etaPhiSubWord = ((ieta&0xF) << 9) | ((sectorID&0x1F) << 3) | (iphi&0x7);
        if (side) {
          m_barrel_reverse_encoding0[etaPhiSubWord] = roi;
        } else {
          m_barrel_reverse_encoding1[etaPhiSubWord] = roi;
        }
      }
     
      //reminder in case of debugging needs:
      //compacted values should be close to these averages
      //(albeit some are different enough to cause mismatches in Topo!)
      //float avgEta = (eta_min+eta_max)/2.;
      //float avgPhi = (phi_min+phi_max)/2.;
     
     
      L1TopoLUTKey key = {side, subsystem, sectorID, roi};
      L1TopoCoordinates value = {eta, phi, eta_min, eta_max, phi_min, phi_max, ieta, iphi};
      
      if (m_encoding.find(key) != m_encoding.end())
      {
	      m_errors.push_back("Duplicate key found in L1TopoLUT: "+key.info());
	      return false;
      }
      m_encoding[key] = value;
    }
    return true;
  }


  L1TopoCoordinates L1TopoLUT::getCoordinates(const unsigned short& side,
					      const unsigned short& subsystem,
					      const unsigned short& sectorID,
					      const unsigned short& roi) const {
    L1TopoLUTKey key = {side, subsystem, sectorID, roi};
    auto itr = m_encoding.find(key);
    if (itr == m_encoding.end())
    {
      L1TopoCoordinates null;
      return null;
    }
    return itr->second;
  }
  
  unsigned short L1TopoLUT::getBarrelROI(unsigned short side, unsigned short sector, unsigned short ieta, unsigned short iphi) const {
    unsigned short etaPhiSubWord = ((ieta&0xF) << 9) | ((sector&0x1F) << 3) | (iphi&0x7);
    unsigned short retval = 0;
    if (side) {
      retval = m_barrel_reverse_encoding0.at(etaPhiSubWord);
    } else {
      retval = m_barrel_reverse_encoding1.at(etaPhiSubWord);
    }
    return retval;
  }
  
    
  float L1TopoLUT::getCompactedValue_eta(unsigned short subsystem, unsigned short side, unsigned short sectorID, unsigned short roi) {
    //note: scheme used for "side" argument here is
    // 0 -> C side
    // 1 -> A side
    float retval = std::numeric_limits<float>::quiet_NaN();
    try {
      if (subsystem == 0) { //barrel
        //barrel is very irregular, so need to retrieve indices for MUCTPI->L1Topo encoding scheme first
        unsigned short lutCode = (side ? m_barrel_encoding0 : m_barrel_encoding1)[sectorID][roi].first;
        retval = (side ? m_barrel_eta_lookup0 : m_barrel_eta_lookup1)[sectorID][lutCode];
      } else if (subsystem == 1) { //endcap
        retval = (side ? m_endcap_eta_lookup0 : m_endcap_eta_lookup1)[roi >> 2][sectorID & 0x1];
      } else if (subsystem == 2) { //forward
        retval = (side ? m_forward_eta_lookup0 : m_forward_eta_lookup1)[sectorID & 0x1][roi >> 2];
      } else {
        std::stringstream ess;
        ess << "[TrigT1MuctpiPhase1/L1TopoLUT] Unknown muon subsystem value: " << subsystem;
        throw std::out_of_range(ess.str());
      }
    } catch (const std::exception& e) {
      std::stringstream ess;
      ess << "[TrigT1MuctpiPhase1/L1TopoLUT] Failed to look up eta value for subsystem=" << subsystem
          << ", side="<< (side?"A":"C")
          << ", sectorID="<< sectorID
          << ", roi="<< roi
          << "\n  (caught exception: " << e.what() <<")";
      throw std::out_of_range(ess.str());
    }
    return retval;
  }
  
  float L1TopoLUT::getCompactedValue_phi(unsigned short subsystem, unsigned short side, unsigned short sectorID, unsigned short roi) {
    //note: scheme used for "side" argument here is
    // 0 -> C side
    // 1 -> A side
    float retval = std::numeric_limits<float>::quiet_NaN();
    try {
      if (subsystem == 0) { //barrel
        //barrel is very irregular, so need to retrieve indices for MUCTPI->L1Topo encoding scheme first
        unsigned short lutCode = (side ? m_barrel_encoding0 : m_barrel_encoding1)[sectorID][roi].second;
        retval = (side ? m_barrel_phi_lookup0 : m_barrel_phi_lookup1)[sectorID][lutCode];
      } else if (subsystem == 1) { //endcap
        retval = (side ? m_endcap_phi_lookup0 : m_endcap_phi_lookup1)[sectorID][roi & 0x3];
      } else if (subsystem == 2) { //forward
        retval = (side ? m_forward_phi_lookup0 : m_forward_phi_lookup1)[roi & 0x3][sectorID];
      } else {
        std::stringstream ess;
        ess << "[TrigT1MuctpiPhase1/L1TopoLUT] Unknown muon subsystem value: " << subsystem;
        throw std::out_of_range(ess.str());
      }
    } catch (const std::exception& e) {
      std::stringstream ess;
      ess << "[TrigT1MuctpiPhase1/L1TopoLUT] Failed to look up phi value for subsystem=" << subsystem
          << ", side="<< (side?"A":"C")
          << ", sectorID="<< sectorID
          << ", roi="<< roi
          << "\n  (caught exception: " << e.what() <<")";
      throw std::out_of_range(ess.str());
    }
    return retval;
  }
  
  
}
