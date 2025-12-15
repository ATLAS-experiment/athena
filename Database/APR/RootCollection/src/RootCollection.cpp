/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "CollectionCommon.h"
#include "RootCollectionCursor.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "RootCollection/AttributeListLayout.h"

#include "PersistentDataModel/Token.h"
#include "RootUtils/APRDefaults.h"

#include "CollectionSvc/ICollectionColumn.h"
#include "CollectionSvc/CollectionNames.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IService.h"

#include "TTree.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TSystem.h"

#include <exception>
#include <map>
#include <deque>
#include <ctype.h>

using namespace std;

namespace pool {

  namespace RootCollection { 

    const char* const RootCollection::c_tokenBranchName = "Token";
    const char* const RootCollection::c_attributeListLayoutName = "Schema"; 

     RootCollection::RootCollection(
            const pool::ICollectionDescription* description,
            pool::ICollection::OpenMode mode,
            pool::ISession* )
        : APRMessaging( "RootCollection"),
         m_description( *description ),
         m_name( description->name() ),
         m_fileName( description->name() + ".root" ),
         m_mode( mode ),
         m_tree( 0 ),
         m_file( 0 ),
         m_session( 0 ),
         m_open( false ),
         m_readOnly( mode == ICollection::READ ? true : false ),
         m_schemaWritten( true )
    {
       RootCollection::open();
    }

     
     RootCollection::~RootCollection()
     {
        if( m_open ) try {
           RootCollection::close();
        } catch( std::exception& exception ) {
           ATH_MSG_ERROR( exception.what() );
           cleanup();
        }
        else cleanup();
     }


     void RootCollection::addTreeBranch( const std::string& name, const std::string& type_name )
     {
        static const std::map< std::string, char > typeDict = {
           // primitive types supported in ROOT (4.00.08) TTrees 
           //  - C : a character string terminated by the 0 character
           //  - B : an 8 bit signed integer (Char_t)
           //  - b : an 8 bit unsigned integer (UChar_t)
           //  - S : a 16 bit signed integer (Short_t)
           //  - s : a 16 bit unsigned integer (UShort_t)
           //  - I : a 32 bit signed integer (Int_t)
           //  - i : a 32 bit unsigned integer (UInt_t)
           //  - F : a 32 bit floating point (Float_t)
           //  - D : a 64 bit floating point (Double_t)
           //  - L : a 64 bit signed integer (Long64_t)
           //  - l : a 64 bit unsigned integer (ULong64_t)
           { "double", 'D' },
           { "long double", 'D' },        // only 64 bit doubles are supported 
           { "float", 'F' },
           { "int", 'I' },
           { "long", 'I' },
           { "unsigned int", 'i' },
           { "unsigned long", 'i' },
           { "long long", 'L' },
           { "unsigned long long", 'l' },
           { "short", 'S' },
           { "unsigned short", 's' },
           { "char", 'B' },
           { "unsigned char", 'b' },
           { "bool", 'B' },
           { "string", 'C' },
           { "Token", 'C' },
        };

        std::string type = "/?";
        auto it = typeDict.find (type_name);
        if (it != typeDict.end()) {
          type[1] = it->second;
        }
        std::string leaflist = name + type;
        m_tree->Branch( name.c_str(), 0, leaflist.c_str() );
     
        m_schemaWritten = false;
        ATH_MSG_DEBUG( "Created Branch " <<  name << ", Type=" <<  type_name );
     }

     void  RootCollection::delayedFileOpen( const std::string& method )
     {
        if( m_open && !m_file && m_session && m_mode != ICollection::READ ) {
           m_file = TFile::Open(m_fileName.c_str(), pool::RootCollection::poolOptToRootOpt[m_mode] );
           if(!m_file || m_file->IsZombie()) {
              throw std::runtime_error( string("ROOT cannot \"") + pool::RootCollection::poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RootCollection::" + method + " \" from \" RootCollection \")" );
           }
           ATH_MSG_INFO( "File " << m_fileName << " opened in " << method );
 
           m_tree->SetDirectory(m_file);
           if( !m_schemaWritten ) {
              m_tree->GetCurrentFile()->cd();
              AttributeListLayout all( m_description );
              ATH_MSG_DEBUG( "###### Writing schema...." );
              all.Write( RootCollection::c_attributeListLayoutName, TObject::kOverwrite );
              m_schemaWritten = true;
           }
        }
     }


     TTree*  RootCollection::getCollectionTree()
     {
        TTree *tree( NULL );
        if( m_file ) {
           tree = dynamic_cast<TTree*>(m_file->Get(APRDefaults::TTreeNames::EventTag));
           if( tree )
              ATH_MSG_DEBUG( "Retrieved Collection TTree  '" << tree->GetName() << "' from file " << m_fileName );
        }
        return tree;
     }
        
        
     
     void RootCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
     {
        if( m_mode == pool::ICollection::READ ) {
           throw std::runtime_error( "Cannot modify the data of a collection in READ open mode. (APR: \" RootCollection::insertRow \" from \" RootCollection \")" );
        }
        std::map< std::string, TBranch* > branchByName;
        const TObjArray* branches = m_tree->GetListOfBranches();
        Int_t nbranches = branches->GetEntriesFast();
        for(int i = 0; i < nbranches; ++i) {
           TBranch* branch = (TBranch*)branches->UncheckedAt(i);
           branchByName[ branch->GetName() ] = branch;
        }
        std::deque<std::string> stringBuffer;
        for( pool::TokenList::const_iterator iToken = inputRowBuffer.tokenList().begin();
              iToken != inputRowBuffer.tokenList().end(); ++iToken )  {
           stringBuffer.push_back( iToken->toString() );
           branchByName[ iToken.tokenName() ]->SetAddress( stringBuffer.back().data() );
        }
        coral::AttributeList attribs_nc = inputRowBuffer.attributeList();
        for( coral::Attribute& att : attribs_nc ) {
           if( att.specification().type() == typeid(std::string) ) {
              std::string&       str = att.data<std::string>();
              branchByName[ att.specification().name() ]->SetAddress( str.data() );
           } else {
              branchByName[ att.specification().name() ]->SetAddress( att.addressOfData() );
           }
        }
	if( m_tree->Fill() <= 0 ) throw std::runtime_error( "TTree::Fill() failed. (APR: \" RootCollection::insertRow \" from \" RootCollection \")" );
     }
        
        
     
     void RootCollection::commit( bool )
     {
        delayedFileOpen("commit");
        if( m_open ) {

	  if (m_tree->GetCurrentFile() == 0) {
        ATH_MSG_DEBUG( "setting TFile for " << m_tree->GetName() << " to " << m_file->GetName() );
	     m_tree->SetDirectory(m_file);
	  }

           ATH_MSG_DEBUG( "Commit: saving collection TTree to file: " << m_tree->GetCurrentFile()->GetName() );

           Long64_t bytes = m_tree->AutoSave();

           ATH_MSG_DEBUG( "   bytes written to TTree " << (size_t)bytes );
        }
     }

     
    void RootCollection::close()
    {
       ATH_MSG_INFO( "Closing " << (m_open? "open":"not open") << " collection '" << m_fileName << "'" );
       if(m_open) {
          delayedFileOpen("close");
              
          if( m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE ) {
             const TObject* tree = getCollectionTree();
             if( tree )
                m_mode = ICollection::UPDATE;
          }
          if( m_mode != ICollection::READ ) {
             if( !m_schemaWritten ) {
                m_tree->GetCurrentFile()->cd();
                AttributeListLayout all( m_description );
                ATH_MSG_DEBUG( "###### Writing schema...." );
                all.Write( RootCollection::c_attributeListLayoutName, TObject::kOverwrite );
                m_schemaWritten = true;
             }
             // m_tree->Print();
             // m_file->Write( "0", TObject::kOverwrite );
          }
          cleanup();
       }
    }

     
    void RootCollection::cleanup()
    {
       if( m_file ) {
          int n = 0;
          if( m_fileMgr ) {
             n = m_fileMgr->close(m_file, "RootCollection");
          } else {
             m_file->Close();
          }
          if( n==0 ) delete m_file; 
          m_file = 0;
       }
       m_tree = 0;
       m_open = false;
    }       
       
     
    void RootCollection::open()  try
    {
      const string myFileType = "RootCollectionFile";

      if( m_open ) close();

      if( m_fileName.starts_with ( "PFN:") && m_description.connection().empty() )
      {
        // special case with no catalog and PFN specified
        // create the collection with exactly PFN file name
        m_fileName = m_description.name().substr(4);   // remove the PFN: prefix
      }

      TDirectory::TContext dirctxt;
      if( m_session == 0 || m_mode == ICollection::READ || m_mode == ICollection::UPDATE ) {
         // first step: Try to open the file
         ATH_MSG_INFO( "Opening Collection File " << m_fileName << " in mode: " << pool::RootCollection::poolOptToRootOpt[m_mode] );
         bool fileExists = !gSystem->AccessPathName( m_fileName.c_str() );
         ATH_MSG_DEBUG( "File " << m_fileName << (fileExists? " exists." : " does not exist." ) );
         // open the file if it exists, or create if requested
         if( !fileExists && m_mode != ICollection::CREATE && m_mode != ICollection::CREATE_AND_OVERWRITE )
            m_file = 0;
         else {
            const char* root_mode = pool::RootCollection::poolOptToRootOpt[m_mode];
            Io::IoFlags io_mode = pool::RootCollection::poolOptToFileMgrOpt[m_mode];

            if( fileExists && (m_mode == ICollection::CREATE
                               || m_mode == ICollection::CREATE_AND_OVERWRITE ) ) {
               // creating collection in an existing file
               root_mode = "UPDATE";
	            io_mode = (Io::WRITE | Io::APPEND);
            }
            if( !m_fileMgr ) {
               m_fileMgr = Gaudi::svcLocator()->service("FileMgr");
               if ( !m_fileMgr ) {
                  ATH_MSG_ERROR( "unable to get the FileMgr, will not manage TFiles" );
               }
	    }
	    // FIXME: hack to avoid issue with setting up RecExCommon links
	    if (m_fileMgr &&
		   m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
          ATH_MSG_INFO( "Unable to locate ROOT file handler via FileMgr. Will use default TFile::Open" );
	      m_fileMgr.reset();
	    }

	    if (!m_fileMgr) {
	      m_file = TFile::Open(m_fileName.c_str(), root_mode);
	    } else {
	      void *vf(0);
	      // open in shared mode only for writing
	      bool SHARED(false);
	      if (io_mode.isWrite()) {
		SHARED = true;
	      }	       
	      int r = m_fileMgr->open(Io::ROOT,"RootCollection",m_fileName,io_mode,vf,"TAG",SHARED);
	      if (r < 0) {
         ATH_MSG_ERROR( "unable to open '" << m_fileName << "' for " << root_mode );
	      } else {      
		m_file = (TFile*)vf;
	      }
	    }
         }
         if(!m_file || m_file->IsZombie()) {
             throw std::runtime_error( string("ROOT cannot \"") + pool::RootCollection::poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RootCollection::open \" from \" RootCollection \")" );
         }
         ATH_MSG_INFO( "File " << m_fileName << " opened" );
      }

      if( m_mode == ICollection::READ || m_mode == ICollection::UPDATE ) {
         // retrieve the TTree from file 
         m_tree = getCollectionTree();
         if( !m_tree ) {
	   // m_file->Write();
	   
	   int n(0);
	   if (!m_fileMgr) {
	     m_file->Close();
	   } else {
	     n = m_fileMgr->close(m_file,"RootCollection");
	   }

	   if (n == 0) delete m_file; 
	   m_file=0;
           throw std::runtime_error( string("POOL Collection TTree not found in file ") + m_fileName + " (APR: \" RootCollection::open \" from \" RootCollection \")" );
         }

         AttributeListLayout* all = dynamic_cast<AttributeListLayout*>( m_tree->GetCurrentFile()->Get(RootCollection::c_attributeListLayoutName) );
         CollectionDescription desc( m_description.name(), m_description.type(), m_description.connection() );
         // clear the description
         m_description = std::move(desc);
         if( all ) {
            // Copy the specification to collection description
            all->fillDescription( m_description );
            delete all;
         } else {
            ATH_MSG_INFO( " Collection Description not found in file, reconstructing " );
            bool      foundToken = false;
            for( int i = 0; i < m_tree->GetNbranches(); i++ ) {
               TBranch* branch = (TBranch*)m_tree->GetListOfBranches()->UncheckedAt(i);
               std::string column_name = branch->GetName();
               std::string column_type = branch->GetTitle();
               ATH_MSG_DEBUG( "  + adding column: " << column_name );
               ATH_MSG_DEBUG( "      column type: " << column_type );
               if( column_type.substr(0,5) != "Token" ) {
                  m_description.insertColumn( column_name, column_type.substr(0, column_type.size() -2) );
               } else {
                  if( !foundToken ) {
                     foundToken = true;
                     m_description.setEventReferenceColumnName( column_name );
                  } else {
                     throw std::runtime_error( "Can't reconstruct Description if more than one Token column. (APR: \" RootCollection::readAttributeListSpecification \" from \" RootCollection \")" );
                  }
               }
            }
            if( !foundToken ) {
               m_description.setEventReferenceColumnName( "DummyRef" );
            }
         }
      }

      if( m_mode == ICollection::CREATE || m_mode == ICollection::CREATE_AND_OVERWRITE ) {
        // create a new TTree

        m_tree = new TTree(APRDefaults::TTreeNames::EventTag, m_name.c_str());
        ATH_MSG_DEBUG( "Created Collection TTree. Collection file will be " << m_fileName );
        m_schemaWritten = false;
        for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
             std::string columnName = m_description.tokenColumn(col_id).name();
             addTreeBranch( columnName, CollectionNames::tokenTypeName );
        }
        for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
             const ICollectionColumn& column = m_description.attributeColumn(col_id);
             addTreeBranch( column.name(), column.type() );
        }
      }

      ATH_MSG_INFO( "Root collection opened, size = " << m_tree->GetEntries() );

      if( m_session && m_mode == ICollection::UPDATE ) {
        m_tree->SetDirectory(0);

	int n(0);
	if (!m_fileMgr) {
	  m_file->Close();
	} else {
	  n = m_fileMgr->close(m_file,"RootCollection");
	}

        if (n == 0) delete m_file; 
	m_file =0;
      }
      m_open = true;
    }
    catch( std::exception &e ) {
       ATH_MSG_DEBUG( "Open() failed with exception: " << e.what() );
       cleanup();
       throw;
    }

     
    bool RootCollection::isOpen() const{
      return m_open;
    }

     
    ICollection::OpenMode RootCollection::openMode() const
    {
      return m_mode;
    }

     
    const ICollectionDescription& RootCollection::description() const
    {
      return m_description;
    }

    ICollectionCursor& RootCollection::cursor()
    {
       if( !isOpen() ) {
          throw std::runtime_error( "Attempt to get cursor for a closed collection. (APR: \" RootCollection::cursor \" from \" RootCollection \")" );
       }

       pool::TokenList outputTokenList;
       coral::AttributeList outputAttributeList;
       for( int j = 0; j < m_description.numberOfAttributeColumns(); j++ )    {
          outputAttributeList.extend( m_description.attributeColumn( j ).name() , m_description.attributeColumn( j ).type() );
       }
       for( int j = 0; j < m_description.numberOfTokenColumns(); j++ )    {
          outputTokenList.extend( m_description.tokenColumn( j ).name() );
       }

       TEventList* eventList = 0;

       // Create collection row buffer
       pool::CollectionRowBuffer collectionRowBuffer( outputTokenList, outputAttributeList );

       ICollectionCursor* cursor = new RootCollectionCursor( m_description, collectionRowBuffer, m_tree, eventList );
       return *cursor;
    }
  }
}
