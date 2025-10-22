/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Shaun Roe
 * @date Nov 2024
 * @brief Some tests for IdDictMgr
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict


#include "IdDictParser/IdDictParser.h"  
#include <string>
#include <filesystem>

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <algorithm>
namespace utf = boost::unit_test;


//in case we want to catch the debug output
struct cout_redirect {
  cout_redirect( std::streambuf * new_buffer ) 
      : m_old( std::cout.rdbuf( new_buffer ) )
  { }

  ~cout_redirect( ) {
      std::cout.rdbuf( m_old );
  }

private:
    std::streambuf * m_old;
};

static const std::string sctDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};


// Return pairs of system,path pairs for all IdDict xml files
// in DIR.
std::vector<std::pair<std::string, std::string> >
findXMLFiles (const std::string& dir)
{
  using namespace std::filesystem;

  const std::vector<std::string> systems {
    "Calorimeter",
    "ForwardDetectors",
    "InnerDetector",
    "LArCalorimeter",
    "LArHighVoltage",
    "MuonSpectrometer",
    "TileCalorimeter",
  };

  std::vector<std::pair<std::string, std::string> > out;

  using tokenizer = boost::tokenizer<boost::char_separator<char> >;
  boost::char_separator<char> sep(":");
  std::string xmlpath (getenv("XMLPATH"));
  tokenizer tokens(xmlpath, sep);

  for (std::string xmldir : tokens) {
    path d = path(xmldir) / dir;
    if (!std::filesystem::exists (d)) continue;
    for (const directory_entry& dirent : directory_iterator(d)) {
      std::string fn = dirent.path().filename().string();
      if (fn.starts_with ("IdDict")) {
        fn.erase (0, 6);
        std::string::size_type ipos = fn.find ('_');
        if (ipos == std::string::npos) {
          ipos = fn.find ('-');
        }
        if (ipos == std::string::npos) {
          ipos = fn.find ('.');
        }
        if (ipos != std::string::npos) {
          fn.erase (ipos, std::string::npos);
        }
        if (std::ranges::find (systems, fn) != systems.end()) {
          out.emplace_back (fn, dirent.path().string());
        }
      }
    }
  }
  return out;
}

// Try to parse all IdDict XML files in DIR.
bool
parseXMLFiles (const std::string& dir)
{
  for (const auto& [syst, path] : findXMLFiles (dir)) {
    std::cout << syst << " " << path << "\n";
    IdDictParser parser;
    parser.register_external_entity(syst, path);
    BOOST_CHECK_NO_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/ATLAS_IDS.xml"));
  }
  return true;
}

BOOST_AUTO_TEST_SUITE(IdDictMgrTest)
  BOOST_AUTO_TEST_CASE(IdDictMgrConstruction){
    BOOST_CHECK_NO_THROW(IdDictMgr());
  }
  BOOST_AUTO_TEST_CASE(IdDictMgrFromParser){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", sctDictFilename);
    BOOST_CHECK_NO_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/ATLAS_IDS.xml"));
  }
  BOOST_AUTO_TEST_CASE(ParseAllXMLFiles){
    BOOST_CHECK( parseXMLFiles ("IdDictParser") );
    BOOST_CHECK( parseXMLFiles ("InDetIdDictFiles") );
  }
  
BOOST_AUTO_TEST_SUITE_END()
