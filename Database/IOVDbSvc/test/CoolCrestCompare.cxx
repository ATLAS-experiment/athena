/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/* CoolCrestCompare.cxx
     Author Evgeny Alexandrov
*/
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>

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
#include "../src/IOVDbStringFunctions.h"
#include "../src/CoralCrestManager.h"
//
#include "GaudiKernelFixtureBase.h"
#include "TestFolderFixture.h"
//
#include "CxxUtils/checker_macros.h"

#include "RelationalAccess/ConnectionService.h"
#include "CoolApplication/Application.h"
#include "CoralBase/AttributeListException.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
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
  std::string m_crest_folder_desc;
  std::vector<uint64_t> m_vList;
  bool m_head;
  std::string m_tag;
public:
  CoolCrestCompare(std::string& cool_str,std::string& crest_str,std::string& gTagCrest,std::string& gTagCool, std::string& folder, std::vector<uint64_t>& vList,bool isHead,std::string& tag):m_msgSvc("msgSvc","test"),
  m_cool_con_str(cool_str),
  m_crest_str(crest_str),
  m_gTagCrest(gTagCrest),
  m_gTagCool(gTagCool),
  m_folder(folder),
  m_log(m_msgSvc.get(), "IOVDbFolder_test"),
  m_clidSvc("ClassIDSvc","test"),
  m_crest_tag(""),
  m_crest_folder_desc(""),
  m_vList(vList),
  m_head(isHead),
  m_tag(tag)	
  {
  }
  void compareFiles() {
    const std::string fileSuffix{".json"};
    const std::string delimiter{"."};
    std::string fMainCool("cool_dump");
    std::string fMainCrest("crest_dump");
    for (uint64_t vkey : m_vList) {
      const std::string p1=fMainCool+"/"+IOVDbNamespace::sanitiseFilename(m_folder)+delimiter+std::to_string(vkey)+fileSuffix;
      const std::string p2=fMainCrest+"/"+IOVDbNamespace::sanitiseFilename(m_folder)+delimiter+std::to_string(vkey)+fileSuffix;

      std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
      std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

      if (f1.fail() || f2.fail()) {
        if(f1.fail())
          std::cerr<<"COOL output file problem"<<std::endl;
        else
	  std::cerr<<"CREST output file problem"<<std::endl;
        return;
      }
      bool result=true;
      if (f1.tellg() != f2.tellg()) {
        result=false; //size mismatch
      }
      if(result){
      //seek back to beginning and use std::equal to compare contents
        f1.seekg(0, std::ifstream::beg);
        f2.seekg(0, std::ifstream::beg);
        result = std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(f2.rdbuf()));
      }
      std::cout<<"-----------------------------------------------------------"<<std::endl;
      if(result)
	    std::cout<<"The folder \""<<m_folder<<"\" is the same in COOL and CREST at timestamp: "<< vkey<<std::endl;
      else{
	    std::cout<<"The folder \""<<m_folder<<"\" is different in COOL and CREST at timestamp: "<<vkey<<std::endl;
	    std::cout<<"To check differences use the following command:"<<std::endl;
	    std::cout<<"diff "<<p1<<" "<<p2<<std::endl;
     }
    }
  }
  void startCool(){
    std::cout<<"Start COOL dump:"<<std::endl;
    ServiceHandle<ITagInfoMgr> tagInfoMgr{"TagInfoMgr","TagInfoMgr"};
    IOVDbParser parser(m_folder+m_crest_folder_desc,m_log);
    IOVDbConn connection(m_cool_con_str, true, m_log);
    IOVDbFolder f(&(connection), parser, m_log, m_clidSvc.get(), nullptr, false, false, "COOL_DATABASE","http://unknown","unknown",true);
    f.preLoadFolder(tagInfoMgr.get() , 0, 0);
    for (uint64_t vkey : m_vList) {
    	f.loadCache(vkey, 0,m_gTagCool, true);
    }
  }
  void startCrest(){
    std::cout<<"Start CREST dump:"<<std::endl;
    ServiceHandle<ITagInfoMgr> tagInfoMgr{"TagInfoMgr","TagInfoMgr"};
    std::map<std::string, std::string> cresttagmap;
    cresttagmap.clear();
    cresttagmap = CoralCrestManager::getGlobalTagMap(m_crest_str,m_gTagCrest);
    m_crest_tag = cresttagmap[m_folder];
    if(m_crest_tag.size()==0){
      std::cerr<<"ERROR in Crest. No folder:\""<<m_folder<<"\" in Global tag:\""<<m_gTagCrest<<"\""<<std::endl;
      exit(1);
    }
    CoralCrestManager mg(m_crest_str,m_crest_tag); 
    m_crest_folder_desc=mg.getFolderDescription();
    if(m_head)
      m_crest_folder_desc+="<tag>HEAD</tag>";
    else if(m_crest_tag.ends_with("-HEAD"))
      m_crest_folder_desc+="<tag>HEAD</tag>";
    else if(m_tag.size()>0){
      m_crest_folder_desc+="<tag>"+m_tag+"</tag>";	    
    }    
    IOVDbParser parser(m_folder+m_crest_folder_desc,m_log);
    IOVDbConn connection("", true, m_log);
    IOVDbFolder f(&(connection), parser, m_log, m_clidSvc.get(), nullptr, false, false, "CREST",m_crest_str,m_crest_tag,true);
    f.preLoadFolder(tagInfoMgr.get() , 0, 0);
    for (uint64_t vkey : m_vList) {
      f.loadCache(vkey, 0,m_gTagCrest, true);
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
	( "head,H", boost::program_options::bool_switch()->default_value(false), "Use HEAD tag" );

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
    std::string tag="";
    bool isHead = false; 
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
    if (arguments.count("tag")) {
      tag = arguments["tag"].as<std::string>();
    }

    if (arguments.count("head"))
      isHead=arguments["head"].as<bool>();
    CoolCrestCompare pr(conStr,crestStr,globalTagCrest,globalTagCool,folder,vList,isHead,tag);
    pr.startCrest();
    pr.startCool();
    pr.compareFiles();
}
