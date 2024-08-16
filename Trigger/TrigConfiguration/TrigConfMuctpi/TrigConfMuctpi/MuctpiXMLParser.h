/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//--------------------------------
// This is the MuctpiXMLParser class. It containts all the methods required to
// parse the configuration XML. The parsed information is stored in a container  
// class MuctpiConfigAlg. This container is sent to the TopoSteering and its contents 
// are unloaded and used to configurate the decision algorithm.
//-------------------------------

#ifndef MUCTPIXMLPARSER_HPP
#define MUCTPIXMLPARSER_HPP



#include "TrigConfBase/TrigConfMessaging.h"//inheritance


// boost libraries
#include <boost/property_tree/ptree.hpp>

#include "TrigConfMuctpi/MuCTPiGeometry.h"//member
#include "TrigConfMuctpi/MuctpiXMLHelper.h"//member
#include <string>

class MioctGeometry;
class MioctSectorGeometry;
class MioctROIGeometry;
class MioctTopoCellGeometry;
class L1MuonPtEncoding;

class MuctpiXMLParser : TrigConf::TrigConfMessaging
{
  
public:
  
  //Constructor
  MuctpiXMLParser();
  
  //Destructor
  virtual ~MuctpiXMLParser() override = default;
  
  // print the parsed configuration
  void printConfiguration() const;
  
  // set verbosity (level out output to screen)
  // something like 3=high, 2=med, 1=low
  void setVerbosity(int v) { m_verbosity=v; }
  
  // method to get a single algorithm from the parsed configuration file
  const MuCTPiGeometry& getMuCTPiGeometry() const {return m_muctpi; }
  
  // read input configuration and check for errors
  void readConfiguration(const std::string & inputfile);
  
  // read input configuration and check for errors
  void setConfiguration(boost::property_tree::ptree ptree) { m_muctpiPT = ptree; }
  
  // parse configuration and fill algorithm containers
  void parseConfiguration();
  
  
private:
  
  // methods to set parameters for menu objects
  void setOctant(  boost::property_tree::ptree menuElement, MioctGeometry &octant);
  void setSector(  boost::property_tree::ptree menuElement, MioctSectorGeometry &sector);
  void setROI(     boost::property_tree::ptree menuElement, MioctROIGeometry &roi);
  void setTopoCell(boost::property_tree::ptree menuElement, MioctTopoCellGeometry &topocell);
  void setPtEncoding(boost::property_tree::ptree menuElement, L1MuonPtEncoding & ptEncoding);
  
  
  // input validation variables
  int  m_verbosity {0};
  
  // configuration variables
  boost::property_tree::ptree m_muctpiPT;
  MuCTPiGeometry m_muctpi;
  MuctpiXMLHelper m_xmlHelper;
};
  


#endif
