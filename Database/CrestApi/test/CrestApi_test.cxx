
/**
 * @file CrestApi/test/CrestApi_test.cxx
 * @brief Some tests for server methods. 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_CRESTAPI

#include <boost/test/unit_test.hpp>

#include "../CrestApi/CrestApi.h"
#include <string>

using namespace Crest;

BOOST_AUTO_TEST_SUITE(CrestApiTest)

  const std::string crest_server = "http://crest-04.cern.ch";
  const std::string tagname = "test_ctest_tag_01";
  const std::string global_tag = "TEST_GLOBAL_TAG_01";


  BOOST_AUTO_TEST_CASE(tag_test){
  CrestClient myCrestClient = CrestClient(crest_server,false);	  
    std::cout << "\n====== CrestApi Test for the server methods =====\n";
    std::cout << "CREST server:  " << crest_server << "\n\n";
    

    std::cout << std::endl << "1) Tag test" << std::endl;    

    nlohmann::json js =
      {
          {"description", "none"},
          {"endOfValidity", 0},
          {"insertionTime", "2018-12-06T11:18:35.641+0000"},
          {"lastValidatedTime", 0},
          {"modificationTime", "2018-12-06T11:18:35.641+0000"},
          {"name", tagname},
          {"payloadSpec", "Json2Cool"},
          {"synchronization", "none"},
          {"timeType", "time"}
      };

    std::cout << "tag (" << tagname << ") =" << std::endl
              << js.dump(4) << std::endl;
    
    TagDto dto = TagDto();
    dto = dto.from_json(js);

    try
    {
      myCrestClient.createTag(dto);
      std::cout << "Tag " << tagname 
                << " created on the CREST server. " << std::endl;
    } 
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot create the tag " << tagname
                << " on CREST server." << std::endl;
      std::cout << e.what() << std::endl;
    }

    TagDto dto2;

    std::cout << "Reading the file from the CREST server..." << std::endl;
    
    try
    {
      dto2 = myCrestClient.findTag(tagname);
      std::cout << std::endl
                << "tag (" << tagname << ") = " << std::endl
                << dto2.to_json().dump(4) << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot get the tag " << tagname
                << " from the CREST server." << std::endl;
      std::cout << e.what() << std::endl;
    }

    // create the patch
    nlohmann::json js2 = dto2.to_json();
    js.erase("insertionTime");
    js2.erase("insertionTime");
    js.erase("modificationTime");
    js2.erase("modificationTime");
    nlohmann::json patch = nlohmann::json::diff(js, js2);

    bool res = patch.empty();
    if (res) {
      std::cout << "Tag test passed successfully." << std::endl;
    }
    else {
      std::cout << "Tag test failed." << std::endl;
    }

    BOOST_TEST(res);
  }


  BOOST_AUTO_TEST_CASE(tag_meta_test){
    CrestClient myCrestClient = CrestClient(crest_server,false);
    std::cout << std::endl << "2) Tag meta info test" << std::endl;   

    PayloadSpecDto spec;
    spec.add("ModuleId","UInt32");
    spec.add("ErrorCode","UInt32");
    spec.add("json","Blob64k");

    std::string description = "<timeStamp>time</timeStamp><addrHeader><address_header service_type=\"71\" clid=\"1238547719\"/></addrHeader><typeName>CondAttrListCollection</typeName>";
    TagInfoDto taginfo(description);
    taginfo.setPayloadSpec(spec);

    ChannelSetDto chanset;
    chanset.add("583008256","");
    chanset.add("583204864","");

    taginfo.setChannel(chanset);

    std::string tag_description = "{\"dbname\":\"OFLP200\",\"nodeFullpath\":\"/TRIGGER/L1Calo/V1/Calibration/JfexModuleSettings\",\"schemaName\":\"COOLOFL_TRIGGER\"}";
    TagMetaDto dto(tagname,tag_description,taginfo);
    nlohmann::json js = dto.to_json();

    try
    {
      myCrestClient.createTagMeta(dto);
      std::cout << "Tag meta info for tag " << tagname 
                << " created on the CREST server."
		<< std::endl;
    } 
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot create the tag meta info for tag " << tagname
                << " on the CREST server" << std::endl;
      std::cout << e.what() << std::endl;
    }

    TagMetaDto dto2;

    std::cout << "Reading the tag meta from the CREST server..." << std::endl;
    
    try
    {
      dto2 = myCrestClient.findTagMeta(tagname);
      std::cout << std::endl
                << "tag meta (" << tagname << ") = " << std::endl
                << dto2.to_json().dump(4) << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot get the tag meta " << tagname
                << " from the CREST server." << std::endl;
      std::cout << e.what() << std::endl;
    }

    // create the patch
    nlohmann::json js2 = dto2.to_json();
    js.erase("insertionTime");
    js2.erase("insertionTime");
    nlohmann::json patch = nlohmann::json::diff(js, js2);

    bool res = patch.empty();
    if (res) {
      std::cout << "Tag meta info test passed successfully." << std::endl;
    }
    else {
      std::cout << "Tag meta info test failed." << std::endl;
    }

    BOOST_TEST(res);
  }


  BOOST_AUTO_TEST_CASE(global_tag_test){
    CrestClient myCrestClient = CrestClient(crest_server,false);
    std::cout << std::endl << "3) Global Tag test" << std::endl;    

    nlohmann::json js =
    {
      {"name", global_tag},
      {"validity", 0.0},
      {"description", "test"},
      {"release", "1"},
      {"insertionTime", "2018-12-18T11:32:58.081+0000"},
      {"snapshotTime", "2018-12-18T11:32:58+0000"},
      {"scenario", "test"},
      {"workflow", "M"},
      {"type", "t"},
    };

    std::cout << "global tag (" << global_tag << ") =" << std::endl
              << js.dump(4) << std::endl;

    GlobalTagDto dto = GlobalTagDto();
    dto = dto.from_json(js);

    try
    {
      myCrestClient.createGlobalTag(dto);
      std::cout << "Global tag " << global_tag 
                << " created on the CREST server. " << std::endl;
    } 
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot create the global tag " << global_tag
                << " on CREST server." << std::endl;
      std::cout << e.what() << std::endl;
    }

    GlobalTagDto dto2;

    std::cout << "Reading the global tag from the CREST server..." << std::endl;
    
    try
    {
      dto2 = myCrestClient.findGlobalTag(global_tag);
      std::cout << std::endl
                << "global tag (" << global_tag << ") = " << std::endl
                << dto2.to_json().dump(4) << std::endl;
    }
    catch (const std::exception &e)
    {
      std::cout << "Error: Cannot get the global tag " << global_tag
                << " from the CREST server." << std::endl;
      std::cout << e.what() << std::endl;
    }

    // create the patch
    nlohmann::json js2 = dto2.to_json();
    js.erase("insertionTime");
    js2.erase("insertionTime");
    js.erase("modificationTime");
    js2.erase("modificationTime");
    js2.erase("insertionTimeMilli");
    js2.erase("snapshotTimeMilli");
    nlohmann::json patch = nlohmann::json::diff(js, js2);
    std::cout << "global tag patch = " << std:: endl
	      << patch.dump(4) << std::endl;

    bool res = patch.empty();
    if (res) {
      std::cout << "The global tag test passed successfully." << std::endl;
    }
    else {
      std::cout << "The global tag test failed." << std::endl;
    }

    BOOST_TEST(res);
  }


  BOOST_AUTO_TEST_CASE(global_tag_removing_test){
    CrestClient myCrestClient = CrestClient(crest_server,false);
    std::cout << std::endl << "4) Global tag removing test" << std::endl;    
    bool res = true;

    try{
      myCrestClient.removeGlobalTag(global_tag);
      std::cout << "The global tag " << global_tag << " was removed from CREST server." << std::endl;
    }
    catch (const CrestException& e) {
      std::cout << "The global tag " << global_tag << " was not removed from server." << std::endl;
      res = false;
    }

    BOOST_TEST(res);
  }


  BOOST_AUTO_TEST_CASE(tag_removing_test){
    CrestClient myCrestClient = CrestClient(crest_server,false);
    std::cout << std::endl << "5) Tag removing test" << std::endl;    
    bool res = true;

    try{
      myCrestClient.removeTag(tagname);
      std::cout << "The tag " << tagname << " was removed from CREST server." << std::endl;
    }
    catch (const CrestException& e) {
      std::cout << "Tag " << tagname << " was not removed from server." << std::endl;
      res = false;
    }

    BOOST_TEST(res);
  }

BOOST_AUTO_TEST_SUITE_END()

