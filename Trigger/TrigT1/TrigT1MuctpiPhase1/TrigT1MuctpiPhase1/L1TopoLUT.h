// This file is really -*- C++ -*-.

/*                                                                                                                      
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1MUCTPIPHASE1_L1TOPOLUT_H
#define TRIGT1MUCTPIPHASE1_L1TOPOLUT_H

#include <boost/property_tree/ptree.hpp>

#include <unordered_map>
#include <map>
#include <set>
#include <utility>
#include <sstream>

namespace LVL1MUCTPIPHASE1
{
  namespace pt = boost::property_tree;

  struct L1TopoCoordinates
  {
    bool operator==(const L1TopoCoordinates& rhs) const
    {
      return (eta == rhs.eta && phi == rhs.phi && 
	      eta_min == rhs.eta_min && phi_min == rhs.phi_min &&
	      ieta == rhs.ieta && iphi == rhs.iphi);
    }
    double eta=0;
    double phi=0;
    double eta_min=0;
    double eta_max=0;
    double phi_min=0;
    double phi_max=0;
    unsigned short ieta=0;
    unsigned short iphi=0;
  };

  class L1TopoLUT //: public extends<AthService IL1TopoLUT>
  {
  public:
    bool initializeLUT(const std::string& barrelFileName, 
		       const std::string& ecfFileName,
		       const std::string& side0LUTFileName,
		       const std::string& side1LUTFileName);

    L1TopoCoordinates getCoordinates(const unsigned short& side,
				     const unsigned short& subsystem,
				     const unsigned short& sectorID,
				     const unsigned short& roi) const;
    
    unsigned short getBarrelROI(unsigned short side, unsigned short sector, unsigned short ieta, unsigned short iphi) const;
    
    const std::vector<std::string>& getErrors() const {return m_errors;}
    
    float getCompactedValue_eta(unsigned short subsystem, unsigned short side, unsigned short sectorID, unsigned short roi);
    float getCompactedValue_phi(unsigned short subsystem, unsigned short side, unsigned short sectorID, unsigned short roi);
    
  protected:
    
    //for coordinate LUTs (they fit in the same data types, but meaning of indices is non-trivial; 
    // maps used where indices are not guaranteed to be continuous and/or relevant json files do not use arrays)    
    // phi row  = roi & 0x3 , eta col = roi >> 2, sector low bit = sector & 0x1
    //                                                 barrel eta/phi:    /* sector  */,        /* eta/phi LUT code */ -> float eta/phi value
    //                                                 endcap eta    :    /* eta col */,        /* sector low bit  */  -> float eta value
    //                                                 endcap phi    :    /* sector */,         /* phi row */          -> float phi value
    //                                                 forward eta   :    /* sector low bit */, /* eta col */          -> float eta value
    //                                                 forward phi   :    /* phi row  */,       /* sector */           -> float phi value
    void fillFromPtree(const boost::property_tree::ptree& node, std::map<unsigned short,std::vector<float>>& theLut) const;
    
    //for barrel "encoding" LUT                                           /*sectorID*/ ,          /* roi */    ,       /* eta LUT code */, /* phi LUT code */
    void fillFromPtree(const boost::property_tree::ptree& node, std::map<unsigned short,std::map<unsigned short, std::pair<unsigned short, unsigned short>>>& theLut) const;
    
    bool initializeCompactedLUTs(const std::string& side0LUTFileName,
			     const std::string& side1LUTFileName);    
    bool initializeLUT(const std::string& inFileName, const bool& isBarrel);
    bool initializeJSON(const std::string& inFileName, bool side);
    bool initializeJSONForSubsystem(pt::ptree& root,
				    const std::string& nodeName, 
				    bool side,
				    unsigned short subsystem);
    
    struct L1TopoLUTKey
    {
      unsigned short side;
      unsigned short subsystem;
      unsigned short sectorID;
      unsigned short roi;

      std::string info()
      {
	std::stringstream str;
	str << "side, subsystem, sectorID, roi = " 
	    << side << ", " << subsystem << ", " << sectorID << ", " << roi;
	return str.str();
      }

      //implement == operator for hashing within unordered_map
      bool operator==(const L1TopoLUTKey& rhs) const
      {
	return (side == rhs.side &&
		subsystem == rhs.subsystem &&
		sectorID == rhs.sectorID &&
		roi == rhs.roi);
      }
    };

    struct L1TopoLUTKeyHasher
    {
      unsigned long operator()(const L1TopoLUTKey& key) const
      {
	return key.side | (key.subsystem << 8) | (key.sectorID << 16) | (key.roi << 24);
      }
    };
    std::map<unsigned short,std::vector<float>> m_barrel_eta_lookup0;
    std::map<unsigned short,std::vector<float>> m_barrel_eta_lookup1;
    std::map<unsigned short,std::vector<float>> m_barrel_phi_lookup0;
    std::map<unsigned short,std::vector<float>> m_barrel_phi_lookup1;
    std::map<unsigned short,std::vector<float>> m_endcap_eta_lookup0;
    std::map<unsigned short,std::vector<float>> m_endcap_eta_lookup1;
    std::map<unsigned short,std::vector<float>> m_endcap_phi_lookup0;
    std::map<unsigned short,std::vector<float>> m_endcap_phi_lookup1;
    std::map<unsigned short,std::vector<float>> m_forward_eta_lookup0;
    std::map<unsigned short,std::vector<float>> m_forward_eta_lookup1;
    std::map<unsigned short,std::vector<float>> m_forward_phi_lookup0;
    std::map<unsigned short,std::vector<float>> m_forward_phi_lookup1;
    
    std::map<unsigned short,std::map<unsigned short, std::pair<unsigned short, unsigned short>>> m_barrel_encoding0;
    std::map<unsigned short,std::map<unsigned short, std::pair<unsigned short, unsigned short>>> m_barrel_encoding1;
    
    //reverse map: eta/phi subword(16b) as sent to Topo -> ROI     
    std::map<unsigned short, unsigned short> m_barrel_reverse_encoding0;
    std::map<unsigned short, unsigned short> m_barrel_reverse_encoding1;
    
    std::unordered_map<L1TopoLUTKey, L1TopoCoordinates, L1TopoLUTKeyHasher> m_encoding;
    std::vector<std::string> m_errors;
  };
}

#endif
