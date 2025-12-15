/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <cstdio> // For sprintf on gcc45
#include <exception>
#include <iostream>
#include <string>

#include "PersistentDataModel/Token.h"

#include "POOLCore/SystemTools.h"

#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/CollectionService.h"
#include "CxxUtils/checker_macros.h"

// Need this to suppress a spurious ubsan warning.
#include "TInterpreter.h"


ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


using namespace std;
using namespace pool;

//coverity[root_function]
int main(int , char** )
{
  SystemTools::initGaudi();
  
//  const string collConnection = "./";
  const string collConnection = "";
  const string collType = "RootCollection";
  
  Token* token = new Token();
  token->fromString("[DB=AEC1DFE2-010B-D811-9832-000347F31C25]"
                    "[CNT=TestContainer]"
                    "[CLID=12345678-1234-42AF-9CB6-567A94781A78]"
                    "[TECH=00000202]"
                    "[OID=00000003-00000002]");
  
//  AttributeList attList;
  CollectionService service;
  
  try{
    
    cout << "Create collection" << endl;
    
    ICollection* out_collection = 0;
    CollectionDescription out_desc("Collection", collType, collConnection);
    out_collection = service.plugin(out_desc, ICollection::CREATE_AND_OVERWRITE);

    out_collection->commit();
    delete out_collection;
    out_collection = nullptr;
    
    ICollection* in_collection = 0;
    
    cout << "Open collection with physical name .... ";
    CollectionDescription	in_desc("Collection", collType );
    in_collection = service.plugin(in_desc, ICollection::READ);
    cout << (in_collection ? "OK":"KO") << endl;
    delete in_collection;
    in_collection = nullptr;

  }catch( std::exception& e ){
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }
  cout << "releasing token" << endl;
  token->release();
  cout << "exiting" << endl;
  return 0;
}
