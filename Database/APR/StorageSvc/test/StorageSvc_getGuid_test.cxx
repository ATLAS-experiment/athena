/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "StorageSvc/DbReflex.h"
#include "POOLCore/DbPrint.h"

using namespace pool;
using namespace std;


bool testGuid( const string& guid, bool shouldwork, DbPrint& mylog )
{
   
   TypeH typ = DbReflex::forGuid( Guid(guid) );
   if( shouldwork ) {
      if( typ ) mylog << MSG::INFO  << "GetGuid (" << guid << ") worked as expected" << endmsg;
      else      mylog << MSG::ERROR << "GetGuid did NOT work as expected" << endmsg;
   } else {
      if( !typ ) mylog << MSG::INFO << "GetGuid failed as expected" << endmsg;
      else       mylog << MSG::ERROR << "GetGuid(" << guid << ") did NOT fail as expected" << endmsg;
   }
   return not (shouldwork xor typ);
}

//coverity[root_function]
int main()
{
   DbPrint mylog("APR Guid TEST");
   mylog.setLevel( MSG::VERBOSE );
   bool res = true;
   res = testGuid( "AAAAAAAA-AAAA-AAAA-AAAA-AAAAAAAAAAAF", false, mylog ) and res;
   res = testGuid( "F41DF744-242D-11E6-B472-02163E010CEC", true, mylog ) and res;
   
   mylog << MSG::INFO << "Test " << (res?"SUCCESS":"FAILURE") << endmsg;
   return res? 0 : -1;
}
