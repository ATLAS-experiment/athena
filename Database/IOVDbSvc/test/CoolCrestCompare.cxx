/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/* CoolCrestCompare.cxx
     Author Evgeny Alexandrov
*/
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>
#include <sstream>
#include <boost/program_options.hpp>

#include <chai/Database.h>
#include <chai/GlobalTag.h>

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IOpaqueAddress.h"
//
#include "../src/FolderTypes.h"
#include "../src/IOVDbParser.h"
#include "../src/IOVDbConn.h"
#include "../src/IOVDbFolder.h"
#include "../src/IOVDbCrestTag.h"
#include "../src/IOVDbStringFunctions.h"
#include "../src/IOVDbJsonStringFunctions.h"
//
#include "GaudiKernelFixtureBase.h"
#include "TestFolderFixture.h"
//
#include "CxxUtils/checker_macros.h"

#include "RelationalAccess/ConnectionService.h"
#include "CoolApplication/Application.h"
#include "CoralBase/AttributeListException.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
// coverity[+UNNECESSARY_STRING_COPY]

namespace {
// Stands in for a real dump when loadAt() fails. Inalid JSON on purpose.
const std::string LOAD_FAILED_SENTINEL{"__COOL_CREST_COMPARE_LOAD_FAILED__"};

// Write one folder/vkey's dumpChannelsAsJson() output to <dirName>/<folder>.<vkey>.json
void writeDump(const std::string& dirName, const std::string& folder, uint64_t vkey, const std::string& json){
  std::filesystem::create_directory(dirName);
  const std::string path = dirName+"/"+IOVDbNamespace::sanitiseFilename(folder)+"."+std::to_string(vkey)+".json";
  //ignore return code; if the file does not exist, we don't care
  //coverity[CHECKED_RETURN]
  std::remove(path.c_str());
  std::ofstream f(path, std::ios::out);
  if (!f.is_open()){
    std::cerr<<"File creation for "<<path<<" failed."<<std::endl;
    exit(1);
  }
  f<<json;
}

std::string readWholeFile(std::ifstream& f){
  std::ostringstream ss;
  ss << f.rdbuf();
  return ss.str();
}
}

class CoolCrestCompare{
private:
  ServiceHandle<IMessageSvc> m_msgSvc;
  std::string m_cool_con_str;
  std::string m_crest_str;
  std::string m_gTagCrest;
  std::string m_gTagCool;
  std::string m_folder;
  MsgStream m_log;
  ServiceHandle<IClassIDSvc> m_clidSvc;
  std::string m_crest_tag;
  std::string m_crestTagOverride;
  std::vector<uint64_t> m_vList;
  bool m_strictIov;
  bool m_head;
  std::string m_tag;
public:
  CoolCrestCompare(std::string& cool_str,std::string& crest_str,std::string& gTagCrest,
                    std::string& gTagCool, std::string& folder, std::vector<uint64_t>& vList,
                    bool strictIov, bool isHead, const std::string& tag,
                    const std::string& crestTagOverride):m_msgSvc("msgSvc","test"),
  m_cool_con_str(cool_str),
  m_crest_str(crest_str),
  m_gTagCrest(gTagCrest),
  m_gTagCool(gTagCool),
  m_folder(folder),
  m_log(0, "IOVDbFolder_test"),
  m_clidSvc("ClassIDSvc","test"),
  m_crest_tag(""),
  m_crestTagOverride(crestTagOverride),
  m_vList(vList),
  m_strictIov(strictIov),
  m_head(isHead),
  m_tag(tag)
  {
  }
  // Returns false if any timestamp had a file problem, a load failure on either
  // side, or a content mismatch. Continues past a bad timestamp to report every
  // one in m_vList instead of stopping at the first.
  bool compareFiles() {
    const std::string fileSuffix{".json"};
    const std::string delimiter{"."};
    std::string fMainCool("cool_dump");
    std::string fMainCrest("crest_dump");
    bool allOk = true;
    for (uint64_t vkey : m_vList) {
      const std::string p1=fMainCool+"/"+IOVDbNamespace::sanitiseFilename(m_folder)+delimiter+std::to_string(vkey)+fileSuffix;
      const std::string p2=fMainCrest+"/"+IOVDbNamespace::sanitiseFilename(m_folder)+delimiter+std::to_string(vkey)+fileSuffix;

      std::ifstream f1(p1, std::ifstream::binary);
      std::ifstream f2(p2, std::ifstream::binary);

      if (f1.fail() || f2.fail()) {
        if(f1.fail())
          std::cerr<<"COOL output file problem"<<std::endl;
        else
	  std::cerr<<"CREST output file problem"<<std::endl;
        allOk = false;
        continue;
      }

      const std::string content1 = readWholeFile(f1);
      const std::string content2 = readWholeFile(f2);
      const bool coolLoadFailed = (content1 == LOAD_FAILED_SENTINEL);
      const bool crestLoadFailed = (content2 == LOAD_FAILED_SENTINEL);
      if (coolLoadFailed || crestLoadFailed) {
        if (coolLoadFailed) {
          std::cerr<<"COOL load failed for folder \""<<m_folder<<"\" at timestamp "<<vkey<<std::endl;
        }
        if (crestLoadFailed) {
          std::cerr<<"CREST load failed for folder \""<<m_folder<<"\" at timestamp "<<vkey<<std::endl;
        }
        allOk = false;
        continue;
      }

      // Bounds are stripped before comparing by default, leaving only payload content
      const bool result = m_strictIov
        ? (content1 == content2)
        : (IOVDbNamespace::stripIovBounds(content1) == IOVDbNamespace::stripIovBounds(content2));
      std::cout<<"-----------------------------------------------------------"<<std::endl;
      if(result) {
	      std::cout<<"The folder \""<<m_folder<<"\" is the same in COOL and CREST at timestamp: "<< vkey<<std::endl;
      }
      else{
        std::cout<<"The folder \""<<m_folder<<"\" is different in COOL and CREST at timestamp: "<<vkey<<std::endl;
        std::cout<<"To check differences use the following command:"<<std::endl;
        std::cout<<"diff "<<p1<<" "<<p2<<std::endl;
        allOk = false;
     }
    }
    return allOk;
  }
  void startCool(){
    std::cout<<"Start COOL dump:"<<std::endl;
    ServiceHandle<ITagInfoMgr> tagInfoMgr{"TagInfoMgr","TagInfoMgr"};
    // --head and --tag reach the COOL folder as a <tag> modifier.
    // A CREST tag named ...-HEAD implies --head.
    std::string tagModifier;
    if (m_head || m_crest_tag.ends_with("-HEAD")) {
      tagModifier = "<tag>HEAD</tag>";
    } else if (!m_tag.empty()) {
      tagModifier = "<tag>" + m_tag + "</tag>";
    }
    IOVDbParser parser(m_folder + tagModifier,m_log);
    IOVDbConn connection(m_cool_con_str, true, m_log);
    IOVDbFolder f(&(connection), parser, m_log, &(*m_clidSvc), nullptr, false, 0, m_gTagCool, true);
    f.preload(tagInfoMgr.get() , 0, 0);
    for (uint64_t vkey : m_vList) {
    	const bool loaded = f.loadAt(vkey);
    	writeDump("cool_dump", m_folder, vkey, loaded ? f.dumpChannelsAsJson(vkey) : LOAD_FAILED_SENTINEL);
    }
  }
  void startCrest(){
    std::cout<<"Start CREST dump:"<<std::endl;
    ServiceHandle<ITagInfoMgr> tagInfoMgr{"TagInfoMgr","TagInfoMgr"};
    const std::string connectStr = IOVDbNamespace::chaiConnectString(m_crest_str);
    chai::Database db(connectStr);
    // --crest-tag names the CREST tag directly. Otherwise look up in global tag.
    if (!m_crestTagOverride.empty()) {
      m_crest_tag = m_crestTagOverride;
    } else {
      const auto mapping = db.getGlobalTag(m_gTagCrest)->getMapping();
      for (const auto& [mapKey, tagMapping] : mapping) {
        const auto& [label, record] = mapKey;
        if (label == m_folder) {
          m_crest_tag = tagMapping.tagName;
          break;
        }
      }
      if (m_crest_tag.empty()) {
        std::cerr<<"ERROR in Crest. No folder:\""<<m_folder<<"\" in Global tag:\""<<m_gTagCrest<<"\""<<std::endl;
        exit(1);
      }
    }
    IOVDbParser parser(m_folder,m_log);
    IOVDbConn connection("", true, m_log);
    IOVDbCrestTag f(&connection, parser, m_log, &(*m_clidSvc), nullptr, db, m_crest_tag);
    f.preload(tagInfoMgr.get() , 0, 0);
    for (uint64_t vkey : m_vList) {
      const bool loaded = f.loadAt(vkey);
      writeDump("crest_dump", m_folder, vkey, loaded ? f.dumpChannelsAsJson(vkey) : LOAD_FAILED_SENTINEL);
    }
  }
};
//coverity[root_function]
int main(int argc, char ** argv)
{
    boost::program_options::options_description description( "Options" );

    description.add_options()
	( "help,h", "produce help message" )
	( "coolsource,c", boost::program_options::value<std::string>(), "COOL connection string" )
	( "crestsource,C", boost::program_options::value<std::string>(), "CREST URL string" )
        ( "globalTagCrest,g", boost::program_options::value<std::string>(), "Global tag for CREST" )
	( "globalTagCool,G", boost::program_options::value<std::string>(), "Global tag for COOL" )
	( "folder,f", boost::program_options::value<std::string>(), "name of Folder" )
        ( "timestamp,t", boost::program_options::value<std::vector<uint64_t>>()->multitoken(), "Time of data. Support multiple space separated values. Example: -t 1715204691957781740 1725204691957781740" )
	( "tag,T",  boost::program_options::value<std::string>(), "name of Tag")
	( "head,H", boost::program_options::bool_switch()->default_value(false), "Use HEAD tag" )
        ( "crest-tag", boost::program_options::value<std::string>(),
          "CREST tag to read, bypassing the global tag mapping" )
        ( "strict-iov", boost::program_options::bool_switch()->default_value(false),
          "Compare since/until bounds too (stripped by default)" );

    boost::program_options::variables_map arguments;
    try {
	boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description ), arguments );
	boost::program_options::notify( arguments );
    }
    catch ( boost::program_options::error & ex )
	{
	    std::cerr << ex.what() << std::endl;
	    description.print( std::cout );
	    return 1;
	}

    if ( arguments.count("help") ) {
	std::cout << "Test application of the 'dqm' package" << std::endl;
	description.print( std::cout );
	return 0;
    }
    std::string folder;
    std::string globalTagCrest;
    std::string globalTagCool;
    std::string conStr="";
    std::string crestStr="";
    std::vector<uint64_t> vList;
    if (arguments.count("folder")) {
      folder = arguments["folder"].as<std::string>();
    }
    else{
      std::cerr <<"Error do not define folder"<<std::endl;
      return -1;
    }
    if (arguments.count("globalTagCrest")) {
      globalTagCrest = arguments["globalTagCrest"].as<std::string>();
    }
    else{
      std::cerr <<"Error do not define globalTagCrest"<<std::endl;
      return -1;
    }
    if (arguments.count("globalTagCool")) {
      globalTagCool = arguments["globalTagCool"].as<std::string>();
    }
    else{
      std::cerr <<"Error do not define globalTagCool"<<std::endl;
      return -1;
    }
    if (arguments.count("coolsource")) {
      conStr = arguments["coolsource"].as<std::string>();
    }
    else{
      std::cerr <<"Error do not define COOL connection string"<<std::endl;
      return -1;
    }
    if (arguments.count("crestsource")) {
      crestStr = arguments["crestsource"].as<std::string>();
    }
    else{
      std::cerr <<"Error do not define CREST URL string"<<std::endl;
      return -1;
    }
    if (arguments.count("timestamp")) {
      vList = arguments["timestamp"].as<std::vector<uint64_t>>();
    }
    else{
      std::cerr <<"Error do not define timestamp"<<std::endl;
      return -1;
    }

    std::string tag;
    if (arguments.count("tag")) {
      tag = arguments["tag"].as<std::string>();
    }
    std::string crestTagOverride;
    if (arguments.count("crest-tag")) {
      crestTagOverride = arguments["crest-tag"].as<std::string>();
    }

    const bool isHead = arguments["head"].as<bool>();
    const bool strictIov = arguments["strict-iov"].as<bool>();
    CoolCrestCompare pr(conStr,crestStr,globalTagCrest,globalTagCool,folder,vList,
                        strictIov,isHead,tag,crestTagOverride);
    pr.startCrest();
    pr.startCool();
    const bool same = pr.compareFiles();
    return same ? 0 : 1;
}
