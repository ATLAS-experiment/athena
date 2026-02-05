/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RootCollection.h"
#include "TTreeCollectionCursor.h"
#include "RNTupleCollectionCursor.h"

#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeList.h"

#include "PersistentDataModel/Token.h"
#include "RootUtils/APRDefaults.h"

#include "CollectionSvc/CollectionColumn.h"
#include "CollectionSvc/CollectionNames.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/IService.h"

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "ROOT/RNTupleWriteOptions.hxx"

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

     RootCollection::RootCollection(
            const pool::CollectionDescription* description,
            pool::ICollection::OpenMode mode,
            pool::ISession* )
        : APRMessaging( "RootCollection"),
         m_description( *description ),
         m_name( description->name() ),
         m_fileName( description->connection() ),
         m_mode( mode ),
         m_file( 0 ),
         m_tree( 0 ),
         m_session( 0 ),
         m_open( false ) {
        RootCollection::open();
     }


     RootCollection::~RootCollection() {
        RootCollection::close();
     }


     void RootCollection::addTreeBranch( const std::string& name, const std::string& type_name ) {
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

        ATH_MSG_DEBUG( "Created Branch " <<  name << ", Type=" <<  type_name );
     }

     void RootCollection::addField(ROOT::RNTupleModel* model, const std::string& field_name, const std::string& field_type)
     {
        ATH_MSG_DEBUG( "Adding new column: name=" << field_name << " of type " << field_type );
        const std::string actual_type = (field_type == CollectionNames::tokenTypeName? "std::string" : field_type);
        auto field = ROOT::RFieldBase::Create(field_name, actual_type).Unwrap();
        model->AddField( std::move(field) );
     }

     void RootCollection::insertRow( const pool::CollectionRowBuffer& inputRowBuffer )
     {
        if( m_mode == pool::ICollection::READ ) {
           throw std::runtime_error( "Cannot modify the data of a collection in READ open mode. (APR: \" RootCollection::insertRow \" from \" RootCollection \")" );
        }
        if (m_tree) {
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
        } else if (m_rntupleWriter) {
           // MN: TODO: migrate to a const REntry API once ROOT delivers it.
           auto entry = m_rntupleWriter->GetModel().CreateBareEntry();
           std::deque<std::string> stringBuffer;
           for( pool::TokenList::const_iterator iToken = inputRowBuffer.tokenList().begin();
              iToken != inputRowBuffer.tokenList().end(); ++iToken )  {
              stringBuffer.push_back( iToken->toString() );
              entry->BindRawPtr( iToken.tokenName(), &stringBuffer.back() );
           }
           for( const coral::Attribute& att : inputRowBuffer.attributeList() ) {
              void* ptr ATLAS_THREAD_SAFE = const_cast<void*>(att.addressOfData());
              entry->BindRawPtr( att.specification().name(), ptr );
           }
           auto wbytes = m_rntupleWriter->Fill(*entry);
           if( wbytes <= 0 ) throw std::runtime_error( "RNTuple::Fill() failed. (APR: \" RootCollection::insertRow \" from \" RootCollection \")");
        }
     }


     void RootCollection::commit( bool )
     {
        if( m_open ) {
           if( m_tree ) {
              if (m_tree->GetCurrentFile() == 0) {
                 ATH_MSG_DEBUG( "setting TFile for " << m_tree->GetName() << " to " << m_file->GetName() );
                 m_tree->SetDirectory(m_file);
              }
              ATH_MSG_DEBUG( "Commit: saving collection TTree to file: " << m_tree->GetCurrentFile()->GetName() );
              Long64_t bytes = m_tree->AutoSave();
              ATH_MSG_DEBUG( "   bytes written to TTree " << (size_t)bytes );
           }
        }
     }


    void RootCollection::close()
    {
       ATH_MSG_INFO( "Closing " << (m_open? "open":"not open") << " collection '" << m_fileName << "'" );
       if(m_open) {
          cleanup();
       }
    }


    void RootCollection::cleanup()
    {
       m_rntupleWriter.reset();
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
      if( m_fileName.starts_with ( "PFN:") ) {
        m_fileName = m_fileName.substr(4);
      }

      TDirectory::TContext dirctxt;
      if( m_session == 0 || m_mode == ICollection::READ ) {
         // first step: Try to open the file
         ATH_MSG_INFO( "Opening RootCollection File " << m_fileName << " in mode: " << pool::RootCollection::poolOptToRootOpt[m_mode] );
         bool fileExists = !gSystem->AccessPathName( m_fileName.c_str() );
         ATH_MSG_DEBUG( "File " << m_fileName << (fileExists? " exists." : " does not exist." ) );
         // open the file if it exists, or create if requested
         if( !fileExists && m_mode != ICollection::CREATE_AND_OVERWRITE )
            m_file = 0;
         else {
         ATH_MSG_INFO( "Opening Collection File " << m_fileName << " in mode: " << pool::RootCollection::poolOptToRootOpt[m_mode] );
            const char* root_mode = pool::RootCollection::poolOptToRootOpt[m_mode];
            Io::IoFlags io_mode = pool::RootCollection::poolOptToFileMgrOpt[m_mode];
            if( !m_fileMgr ) {
               m_fileMgr = Gaudi::svcLocator()->service("FileMgr");
               if ( !m_fileMgr ) {
                  ATH_MSG_ERROR( "unable to get the FileMgr, will not manage TFiles" );
               }
            }
            // FIXME: needed hack to avoid issue with setting up RecExCommon links
            if (m_fileMgr && m_fileMgr->hasHandler(Io::ROOT).isFailure()) {
               ATH_MSG_INFO( "Unable to locate ROOT file handler via FileMgr. Will use default TFile::Open" );
               m_fileMgr.reset();
            }
            if (!m_fileMgr) {
              m_file = TFile::Open(m_fileName.c_str(), root_mode);
            } else {
              void *vf(0);
              int r = m_fileMgr->open(Io::ROOT,"RootCollection",m_fileName,io_mode,vf,"TAG",false);
              if (r < 0) {
                ATH_MSG_ERROR( "unable to open '" << m_fileName << "' for " << root_mode );
              } else {
                m_file = (TFile*)vf;
              }
            }
         }
         ATH_MSG_INFO( "Opening Collection File " << m_fileName << " in mode: " << pool::RootCollection::poolOptToRootOpt[m_mode] );
         if(!m_file || m_file->IsZombie()) {
             throw std::runtime_error( string("ROOT cannot \"") + pool::RootCollection::poolOptToRootOpt[m_mode] + "\" file " + m_fileName + " (APR: \" RootCollection::open \" from \" RootCollection \")" );
         }
         ATH_MSG_INFO( "File " << m_fileName << " opened" );
      }

      if( m_mode == ICollection::READ ) {
         // retrieve the TTree from file
         if ( m_description.type() == pool::ROOT_StorageType.type() ) {
           m_tree = dynamic_cast<TTree*>(m_file->Get(APRDefaults::TTreeNames::EventTag));
         }
         if ( (!m_tree && m_description.type() == pool::ROOT_StorageType.type()) || m_description.type().exactMatch(pool::ROOTRNTUPLE_StorageType.type()) ) {
           m_reader = ROOT::RNTupleReader::Open( APRDefaults::RNTupleNames::EventTag, m_fileName );
         }
         if( !m_tree && !m_reader ) {
           int n(0);
           if (!m_fileMgr) {
             m_file->Close();
           } else {
             n = m_fileMgr->close(m_file,"RootCollection");
           }
           if (n == 0) delete m_file;
           m_file=0;
           throw std::runtime_error( string("POOL Collection TTree/RNTuple not found in file ") + m_fileName + " (APR: \" RootCollection::open \" from \" RootCollection \")" );
         }

         CollectionDescription desc( m_description.name(), m_description.type(), m_description.connection() );
         // clear the description
         m_description = std::move(desc);
         if ( m_tree ) {
            for( int i = 0; i < m_tree->GetNbranches(); i++ ) {
               TBranch* branch = (TBranch*)m_tree->GetListOfBranches()->UncheckedAt(i);
               std::string column_name = branch->GetName();
               std::string column_type = branch->GetTitle();
               ATH_MSG_DEBUG( "  + adding column: " << column_name );
               ATH_MSG_DEBUG( "      column type: " << column_type );
               if (column_type.find('/') != std::string::npos) column_type = column_type.substr(column_type.find('/'));
               static const std::map< std::string, std::string > typenameConv = {
                  { "/C", "string" },
                  { "/l", "unsigned long long" },
                  { "/i", "unsigned int" },
                  { "/s", "unsigned short" },
                  { "/L", "long long" },
                  { "/I", "int" },
                  { "/S", "short" },
                  { "/D", "double" },
                  { "/F", "float" },
                  { "/O", "bool" },
                  { "/B", "bool" } };
               auto it = typenameConv.find( column_type );
               if( it != typenameConv.end() ) {
                  ATH_MSG_DEBUG( "Replaced type  " << column_type << " with " << it->second );
                  column_type = it->second;
               }
               if( column_name != m_description.eventReferenceColumnName() ) {
                  m_description.insertColumn( column_name, column_type );
               }
            }
         } else if ( m_reader ) {
            const auto& rntdesc = m_reader->GetDescriptor();
            for( const auto &f : rntdesc.GetTopLevelFields() ) {
               const std::string column_name = f.GetFieldName();
               // ignore the index column, it's not a user data
               if( column_name == APRDefaults::IndexColName ) continue;
               std::string column_type = f.GetTypeName();
               ATH_MSG_DEBUG( "  + adding column: " << column_name );
               ATH_MSG_DEBUG( "      column type: " << column_type );
               static const std::map< std::string, std::string > typenameConv = {
                  { "std::string", "string" },
                  { "std::uint64_t", "unsigned long long" },
                  { "std::uint32_t", "unsigned int" },
                  { "std::uint16_t", "unsigned short" },
                  { "std::int64_t", "long long" },
                  { "std::int32_t", "int" },
                  { "std::int16_t", "short" } };
               auto it = typenameConv.find( column_type );
               if( it != typenameConv.end() ) {
                  ATH_MSG_DEBUG( "Replaced type  " << column_type << " with " << it->second );
                  column_type = it->second;
               }
               if( column_name != m_description.eventReferenceColumnName() ) {
                  m_description.insertColumn( column_name, column_type );
               }
            }
         }
      }

      if( m_mode == ICollection::CREATE_AND_OVERWRITE ) {
        if ( m_description.type().exactMatch(pool::ROOTTREE_StorageType.type()) ) {
          // create a new TTree
          m_tree = new TTree(APRDefaults::TTreeNames::EventTag, m_name.c_str());
          for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
            std::string columnName = m_description.tokenColumn(col_id).name();
            addTreeBranch( columnName, CollectionNames::tokenTypeName );
          }
          for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
            const CollectionColumn& column = m_description.attributeColumn(col_id);
            addTreeBranch( column.name(), column.type() );
          }
        } else {
          // create a new RNTuple
          std::string rntupleName = std::string(APRDefaults::RNTupleNames::EventTag);
          m_file->Delete( (rntupleName+";*").c_str() );
          auto model { ROOT::RNTupleModel::Create() };
          model->SetDescription( rntupleName );
          for( int col_id = 0; col_id < m_description.numberOfTokenColumns(); col_id++ ) {
            std::string columnName = m_description.tokenColumn(col_id).name();
            addField( model.get(), columnName, CollectionNames::tokenTypeName );
          }
          for( int col_id = 0; col_id < m_description.numberOfAttributeColumns(); col_id++ ) {
            const CollectionColumn& column = m_description.attributeColumn(col_id);
            addField( model.get(), column.name(), column.type() );
          }

          ROOT::RNTupleWriteOptions opts;
          opts.SetCompression( m_file->GetCompressionSettings() );
          opts.SetUseBufferedWrite( true );
          m_rntupleWriter = ROOT::RNTupleWriter::Append(std::move(model), rntupleName, *m_file, opts);
        }
      }
      m_open = true;
    }
    catch( std::exception &e ) {
       ATH_MSG_DEBUG( "Open() failed with exception: " << e.what() );
       cleanup();
       throw;
    }

    const CollectionDescription& RootCollection::description() const {
       return m_description;
    }

    ICollectionCursor& RootCollection::cursor() {
       if( !m_open ) {
          throw std::runtime_error( "Attempt to get cursor for a closed collection. (APR: \" RootCollection::cursor \" from \" RootCollection \")" );
       }

       // Create collection row buffer
       pool::CollectionRowBuffer collectionRowBuffer;
       this->initNewRow(collectionRowBuffer);
       ICollectionCursor* cursor = nullptr;
       if ( m_tree ) {
          cursor = new TTreeCollectionCursor( m_description, collectionRowBuffer, m_tree );
       } else if ( m_reader ) {
          cursor = new RNTupleCollectionCursor( m_description, collectionRowBuffer, m_reader.get() );
       }
       return *cursor;
    }
  }
}
