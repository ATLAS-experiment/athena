/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "CollectionSvc/CollectionService.h"
#include "CollectionSvc/CollectionDescription.h"
#include "CollectionSvc/ICollection.h"
#include "CollectionSvc/CollectionRowBuffer.h"
#include "CollectionSvc/TokenList.h"
#include "CollectionSvc/ICollectionColumn.h"
#include "CollectionSvc/ICollectionCursor.h"
#include "RootCollection/AttributeListLayout.h"

#include "PersistentDataModel/Token.h"
#include "CxxUtils/checker_macros.h"
#include "CoralBase/Attribute.h"
#include "AthenaKernel/getMessageSvc.h"

#include "rw_test.h"

#include <iostream>
#include <memory>


using namespace std;
using namespace pool;


TestDriver::TestDriver( const std::string& name,
                        const std::string& type,
                        const std::string& connection )
   : m_name( name ),  m_type( type ), m_connection( connection )
{
   Athena::getMessageSvcQuiet = true;
}


void
TestDriver::write ATLAS_NOT_THREAD_SAFE ()
{
   cout << "Creating CollectionService" << endl;
   unique_ptr<CollectionService> serviceHandle(new CollectionService());
   cout << "Creating Collection Description" << endl;
   pool::CollectionDescription description( m_name, m_type, m_connection );
   description.insertColumn( "attr1", "int" );
   description.insertColumn( "attr2", "string" );
   description.insertColumn( "attr3", "double" );
   description.insertColumn( "attr4", "bool" );
   description.insertColumn( "attr64bit", "unsigned long long" );

   cout << "Creating Collection" << endl;
   pool::ICollection* collection = serviceHandle->create( description, true );
   if ( ! collection )   {
      throw std::runtime_error( "Could not create a relational collection object" );
   }

   cout << "Adding 10 elements to the collection." << endl;
   // Create empty collection and data table row buffers
   pool::CollectionRowBuffer rowBuffer;
   collection->initNewRow( rowBuffer );

   const unsigned long long ntab[] = {
      0x0000000000000000ULL,
      0x0000000000000001ULL,
      0x0000000000010000ULL,
      0x0000000000010001ULL,
      0x7000000000010001ULL,

      0x8000000000000000ULL,
      0x8000000000000001ULL,
      0x8000000000010000ULL,
      0x8000000000010001ULL,
      0xF000000000010001ULL
   };

   for( unsigned int i=0; i<10; i++ )   {
      rowBuffer.attributeList()[ "attr1" ].data<int>() = i ;
      char s[20];
      ::sprintf( s, "%s_%u", "testString", i );
      rowBuffer.attributeList()[ "attr2" ].data<std::string>() = string( s );
      rowBuffer.attributeList()[ "attr3" ].data<double>() = double( i ) * (1./3);
      rowBuffer.attributeList()[ "attr4" ].data<bool>() = ((i%3)!=0);
      rowBuffer.attributeList()[ "attr64bit" ].data<unsigned long long>() = ntab[i];
      cout << ">>  writing uint64: " << ntab[i] << "  " << (double)ntab[i] << "   " << (unsigned long long)(double)ntab[i] << endl;
      collection->insertRow( rowBuffer );
   }

   cout << "Done." << endl;
   cout << "Commiting the collection." << endl;
   collection->commit();
   cout << "Closing the collection." << endl;
   collection->close();
   delete collection;

   cout << "[OVAL] finished hopefully" << endl;
}

   

void
TestDriver::read ATLAS_NOT_THREAD_SAFE ()
{
   cout << endl << "Creating CollectionService" << endl;
   unique_ptr<CollectionService> serviceHandle(new CollectionService());

  cout << "Getting handle to existing collection ( opened for read-only transactions by default )" << endl;
  pool::ICollection* collection = serviceHandle->handle( m_name, m_type , m_connection );

  if( ! collection )   {
     throw std::runtime_error( "Could not create a rootCollection object" );
  }

  pool::ICollectionCursor* cursor1_ptr = &collection->cursor();
  pool::ICollectionCursor& cursor1 = *cursor1_ptr;
  cout << "Iterating over collection..." << endl;
  int counter = 0;
  while( cursor1.next() && counter < 10 )  {
     std::cout << "Token : " << cursor1.eventRef().toString() << std::endl;
     coral::AttributeList attributeList = cursor1.currentRow().attributeList();
     std::cout << "Meta data : ";
     for( coral::AttributeList::const_iterator iAttribute = attributeList.begin();
           iAttribute != attributeList.end(); ++iAttribute )
     {
        if ( iAttribute != attributeList.begin() ) std::cout << ", ";
        std::cout << "[";
        iAttribute->toOutputStream( std::cout );
        std::cout << "]";
     }
     cout << endl;
     counter++;
  }
  std::cout << counter << " records read back" << std::endl;
  delete cursor1_ptr;

  collection->close();
  delete collection;
}


// Force a link dependency on libRootCollection; otherwise, the linker
// will remove the dependency.  And if we don't link against libRootCollection,
// then we can get ubsan errors related to ICollectionQuery because in that
// case, the typeinfo for ICollectionQuery won't get exported from
// the test binary.
void dum()
{
  AttributeListLayout all;
}
