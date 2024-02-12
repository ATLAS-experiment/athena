/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//    APR Database Container implementation for ROOT/RNTuple
//--------------------------------------------------------------------
//    Author     : M.Nowak
//====================================================================

// Framework include files
#include "POOLCore/DbPrint.h"
#include "RootAuxDynIO/RootAuxDynIO.h"
#include "StorageSvc/DbArray.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/Transaction.h"

// Local implementation files
#include "RNTupleContainer.h"
#include "RootDataPtr.h"
#include "RootDatabase.h"

// Root include files
#include <ROOT/RNTuple.hxx>
#include <ROOT/RField.hxx>

#include "TError.h"
// for version checks
#include <TROOT.h>

#include <algorithm>

using std::string;
using namespace pool;

static UCharDbArrayAthena s_char_Blob ATLAS_THREAD_SAFE;

namespace {
/* Temporarily install ROOT error handler to filter out warnings about RNTuple
   being in development Remove in production
 */
static struct ErrorHandlerInit {
  static ErrorHandlerFunc_t m_oldHandler ATLAS_THREAD_SAFE;
  ErrorHandlerInit() { m_oldHandler = SetErrorHandler(RNTErrorHandler); }
  static void RNTErrorHandler(int level, Bool_t abort, const char* location,
                              const char* msg) {
    // filter out RNTuple warnings, print all other messages
    if (strstr(msg, "The RNTuple file format will change") == NULL and
        strstr(msg, "Pre-release format version") == NULL and m_oldHandler) {
      m_oldHandler(level, abort, location, msg);
    }
  }
} EHI;
ErrorHandlerFunc_t ErrorHandlerInit::m_oldHandler ATLAS_THREAD_SAFE;

}  // namespace


/// Required here for unique_ptr compilation
RNTupleContainer::FieldDesc::FieldDesc(const DbColumn& c) : DbColumn(c) {}


// Get the type name
const std::string RNTupleContainer::FieldDesc::typeName() {
  auto tid = typeID();
  switch (tid) {
    case DbColumn::STRING:
    case DbColumn::LONG_STRING:
    case DbColumn::TOKEN:
      return "std::string";
      break;
    case BLOB:
      return "UCharDbArrayAthena";
      break;
    default:
      break;
  }
  return DbColumn::typeName();
}

/// Standard constructor
RNTupleContainer::RNTupleContainer()
   : m_type(nullptr),
     m_dbH(POOL_StorageType), m_rootDb(nullptr),
     m_ioBytes(0), m_isDirty(false),
     m_index(0), m_indexSize(0), m_indexBump(0), m_indexMulti( getpid() )
{ }


/// Standard destructor
RNTupleContainer::~RNTupleContainer() { close(); }

/// Ask if a given shape is supported
DbStatus RNTupleContainer::isShapeSupported(const DbTypeInfo* typ) const {
  return typ == m_type;
}

uint64_t RNTupleContainer::size() {
  auto s = DbContainerImp::size();
  if( m_pageSource )   s += m_pageSource->GetNEntries();
  if( m_ntupleWriter ) s += m_ntupleWriter->size();
  return s;
}

/// Open the container for object access
DbStatus RNTupleContainer::open( DbDatabase& dbH, const std::string& nam,
                                 const DbTypeInfo* info, DbAccessMode mode)
{
   m_name = nam;
   DbPrint log(m_name);
   m_fieldDescs.clear();
   m_rootDb = dynamic_cast<RootDatabase*>(dbH.info());
   if( !dbH.isValid() or !info or !m_rootDb ) {
      log << DbPrintLvl::Error << "Cannot open container '" << m_name << "', invalid Database handle."
          << DbPrint::endmsg;
      return Error;
   }
   m_indexBump = m_rootDb->currentIndexMasterID();

   log << DbPrintLvl::Debug << "Opening, mode=" << accessMode(mode)
       << DbPrint::endmsg;
   std::string ntupleName(m_name);
   std::replace(ntupleName.begin(), ntupleName.end(), '/', '_');
   std::string fieldName;

   try {
      const DbTypeInfo::Columns& cols = info->columns();
      log << DbPrintLvl::Debug << "   attributes# = " << cols.size()
          << DbPrint::endmsg;
      if (cols.size() == 1) {
         // extract ntuple and field name for grouped containers, notation:
         // "ntuple(column)"
         std::string::size_type inx = nam.find('(');
         if (inx != std::string::npos) {
            std::string::size_type inx2 = nam.find(')');
            if (inx2 == std::string::npos or inx2 != nam.size() - 1) {
               log << DbPrintLvl::Error << "Misplaced closing ')' in " << m_name
                   << DbPrint::endmsg;
               return Error;
            }
            fieldName = ntupleName.substr(inx + 1, inx2 - inx - 1);
            ntupleName.resize(inx);
            log << DbPrintLvl::Debug << "Grouped Container '" << ntupleName << "/"
                << fieldName << "'" << DbPrint::endmsg;
         }
      }
      // prepare descriptions for all object data members (aka columns)
      m_fieldDescs.reserve(cols.size());
      for (const auto& col : cols) {
         m_fieldDescs.emplace_back(*col);
         FieldDesc& dsc = m_fieldDescs.back();
         dsc.fieldname = fieldName.empty() ? col->name() : fieldName;
         dsc.sgkey = dsc.fieldname;  // remember the original name (usually coming
         // from SG Key)
         for (auto& c : dsc.fieldname)
            if (!std::isalnum(c)) c = '_';
         if (dsc.typeID() == DbColumn::BLOB or dsc.typeID() == DbColumn::ANY or
             dsc.typeID() == DbColumn::POINTER) {
            if (initObjectFieldDesc(dsc) != Success) return Error;
         }
      }

      if( mode & pool::CREATE ) {
         m_ntupleWriter = m_rootDb->getNTupleWriter(ntupleName, true);
         if( m_ntupleWriter ) {
            log << DbPrintLvl::Debug << "Created container " << m_name
                << " of type " << ROOTRNTUPLE_StorageType.storageName()
                << DbPrint::endmsg;
         } else {
            log << DbPrintLvl::Error << "Could not create container " << m_name
                << " of type " << ROOTRNTUPLE_StorageType.storageName()
                << DbPrint::endmsg;
            return Error;
         }
         // Prepare Field descriptions
         for( auto& dsc : m_fieldDescs ) {
            m_ntupleWriter->addField( dsc.fieldname, dsc.typeName() );
         }
      }
      else if( mode & (pool::READ | pool::UPDATE) ) {
         // create (and keep in the descriptin object) the rntuple field for reading
         m_pageSource = m_rootDb->getNTupleReader(ntupleName);
         auto descGuard = m_pageSource->GetSharedDescriptorGuard();
         for( auto& dsc : m_fieldDescs ) {
            auto fieldId = descGuard->FindFieldId(dsc.fieldname);
            if( fieldId == ROOT::Experimental::kInvalidDescriptorId ) {
               log << DbPrintLvl::Error << "Failed to find RNTuple column " << dsc.fieldname
                   << " when opening container " << m_name
                   << DbPrint::endmsg;
               return Error;
            }
            dsc.field = descGuard->GetFieldDescriptor(fieldId).CreateField( descGuard.GetRef() );
            if( dsc.hasAuxStore() ) {
               // atach RNTuple Reader (owned by the DB)
               dsc.auxdyn_reader = RootAuxDynIO::getNTupleAuxDynReader( dsc.field.get(), m_pageSource );
               // If we set up a reader, then disable aging
               // for this file.  That will prevent POOL from
               // deleting the file while we still have
               // references to its branches.
               dbH.setAge(-10);
            }
         }
      }

      log << DbPrintLvl::Debug << "Opened container " << m_name << " of type "
          << ROOTRNTUPLE_StorageType.storageName()
          << DbPrint::endmsg;
      m_dbH = dbH;
      m_type = info;
      return Success;

   }
   catch( const std::exception& e )    {
      debugBreak(nam, "Cannot open ROOT container(Tree/Branch)", e, false);
      return Error;
   }
   catch (...)   {
      DbPrint err( m_name);
      err << DbPrintLvl::Fatal << "Unknown exception occurred. Cannot give more details."
          << DbPrint::endmsg;
      debugBreak(nam, "Cannot open ROOT container(Tree/Branch)");
      return Error;
   }
}


/// Init a field description for an object (i.e. find TClass etc.)
DbStatus RNTupleContainer::initObjectFieldDesc( FieldDesc& dsc )
{
   dsc.clazz = TClass::GetClass( dsc.typeName().c_str() );
   if( dsc.clazz )  {
      if( dsc.clazz->GetStreamerInfo() and dsc.clazz->HasDictionary() )  {
         // AUX STORE specifics
         if( RootAuxDynIO::hasAuxStore(dsc.sgkey, dsc.clazz) ) {
            TClass *storeTClass = dsc.clazz->GetBaseClass("SG::IAuxStoreIO");
            if( storeTClass ) {
               // This is a class implementing SG::IAuxStoreIO
               // Provide writers for its dynamic attibutes
               dsc.aux_iostore_IFoffset = dsc.clazz->GetBaseClassOffset( storeTClass );
               // get rid of the AUX_POSTFIX dot at the end (converter to _ earlier)
               auto last = dsc.fieldname.end() - 1;
               if( *last == '_' )  *last = ':';
            }
         }
         return Success;
      } else {
         DbPrint log(m_name);
         log << DbPrintLvl::Error << "Failed to open the container " << m_name
             << " of type " << ROOTRNTUPLE_StorageType.storageName() << " Class "
             << dsc.clazz->GetName() << " is unknown." << DbPrint::endmsg;
      }
   } else {
      DbPrint log(m_name);
      log << DbPrintLvl::Error << "Failed to open the container " << m_name
          << " of type " << ROOTRNTUPLE_StorageType.storageName() << ". Type "
          << dsc.typeName() << " is unknown." << DbPrint::endmsg;
   }
   return Error;
}


uint64_t RNTupleContainer::nextRecordId()
{
   uint64_t  s = m_indexMulti;
   s = s << 32;
   s += m_indexSize + DbContainerImp::size();
   return s + m_indexBump;
}

void RNTupleContainer::useNextRecordId(uint64_t nextID)
{
   // Find out how this TTree index is behind the master index in the DB
   m_indexBump = nextID - m_indexSize;
   if( m_indexBump < 0 ) {
      // Seems this index is ahead of the master, cannot sync
      m_indexBump = 0;
   }
}


DbStatus RNTupleContainer::writeObject( ActionList::value_type& action )
{
   if( m_isDirty ) {
      DbPrint log(m_name);
      log << DbPrintLvl::Error << "Attempt to write to an RNTuple Container twice in the same transaction! "
          << DbPrint::endmsg;
      m_ioBytes = -1;
      return Error;
   }
   m_isDirty = true;
   int num_bytes = 0;
   for( auto& dsc : m_fieldDescs ) {
      RootDataPtr p( action.dataAtOffset( dsc.offset() ) );
      switch( dsc.typeID() ) {
       case DbColumn::ANY:
       case DbColumn::POINTER:
          dsc.object            = p.ptr;
          try {
             if( dsc.hasAuxStore() ) {
                num_bytes += m_ntupleWriter->writeAuxAttributes( dsc.fieldname, dsc.getIOStorePtr(), dsc.rows_written );
             }
          } catch(const std::exception& exc) {
             DbPrint err(m_name);
             err << DbPrintLvl::Error << "Dynamic attributes writing error: " << exc.what()
                 << DbPrint::endmsg;
             p.ptr = nullptr;  // signal an error condition
             break;
          }
          dsc.rows_written++;
          break;
/*
  MN: Following types not ported to RNTuple (yet?)
       case DbColumn::BLOB:
          s_char_Blob.m_size    = p.blobSize();
          s_char_Blob.m_buffer  = (unsigned char*)p.blobData();
          dsc.object            = &s_char_Blob;
          p.ptr                 = &dsc.object;
          break;
       case DbColumn::STRING:
       case DbColumn::LONG_STRING:
          break;
       case DbColumn::NTCHAR:
       case DbColumn::LONG_NTCHAR:
       case DbColumn::TOKEN:
       {
          void *d = p.deref();
          p.ptr   = d;
       }
       break;
*/
       default:
          break;
      }
      if( !p.ptr ) {
         DbPrint err( m_name);
         err << DbPrintLvl::Error
             << "[RNTupleContainer] Could not write an object of type " << dsc.typeName()
             << DbPrint::endmsg;
         return Error;
      }

      m_ntupleWriter->addFieldValue( dsc.fieldname, p.ptr );
      // fill the index field
      m_index = action.link.second;
      m_ntupleWriter->addFieldValue( "index_ref", &m_index );
      m_indexSize++;
   }

   if( !m_ntupleWriter->isGrouped() and m_ntupleWriter->needsCommit() ) {
      num_bytes += m_ntupleWriter->commit();
   }

   if ( num_bytes > 0 )  {
      m_ioBytes = num_bytes;
      m_rootDb->addByteCount(RootDatabase::WRITE_COUNTER, num_bytes);
   }
   return Success;
}


/// Find object by object identifier and load it into memory
DbStatus RNTupleContainer::loadObject(void** obj_p, ShapeH, Token::OID_t& oid)
{
   int64_t evt_id = oid.second;
   if( (evt_id >> 32) > 0 ) {
      evt_id = m_rootDb->indexLookup(m_pageSource, evt_id);
   }
   // lock access to this DB for MT safety
   std::lock_guard<std::recursive_mutex>     lock( m_rootDb->ioMutex() );
   try {
      int numBytes = 0;
      for( auto& dsc : m_fieldDescs ) {
         // read the object
         RootDataPtr p(*obj_p);
         switch( dsc.typeID() ) {
          case DbColumn::BLOB:
             {
                // MN: not sure about this one, implement if needed ever
                DbPrint err(m_name);
                err << DbPrintLvl::Fatal << "[RNTupleContainer] - BLOB reading not implemented yet" << DbPrint::endmsg;
                return Error;
             }
             break;
          case DbColumn::ANY:
          case DbColumn::POINTER:
             // MN: should not need any special action here
             break;
          default:
             p.c_str += dsc.offset();
             break;
         }
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 31, 0 )
         // connect the field (with subfields) to the pageSource
         if( dsc.field->GetState() != RFieldBase::EState::kConnectedToSource ) {
            ROOT::Experimental::Internal::CallConnectPageSourceOnField(*dsc.field, *m_pageSource);
            for( auto& subfield : *dsc.field ) {
               if( subfield.GetState() != RFieldBase::EState::kConnectedToSource ) {
                  ROOT::Experimental::Internal::CallConnectPageSourceOnField(subfield, *m_pageSource);
               }
            }
         }
         if( p.ptr ) {
            // read into an object given by the user
            auto v = dsc.field->BindValue( std::shared_ptr<void>(p.ptr, [](void *) {}) );
            v.Read( evt_id );
         } else {
            // create the object for the user and pass ownership to them
            auto v = std::make_unique<RFieldBase::RValue>( dsc.field->CreateValue() );
            v->Read( evt_id );
            *obj_p = v->GetPtr<void>().get();
            v.release(); // This leaks the RValue!
         }
#endif
         numBytes += 1;

         // case DbColumn::BLOB:
         //    p.blob->adopt((char*)s_char_Blob.m_buffer,s_char_Blob.m_size);
         //    s_char_Blob.release(false);

         if (dsc.auxdyn_reader) {
            dsc.auxdyn_reader->addReaderToObject(*obj_p, evt_id,
                                                 &m_rootDb->ioMutex());
         }
      }
      /// Update statistics
         m_ioBytes = numBytes;
         m_rootDb->addByteCount(RootDatabase::READ_COUNTER, numBytes);
         return Success;
   } catch (const std::exception& e) {
      DbPrint err(m_name);
      err << DbPrintLvl::Fatal << "[RNTupleContainer] "
          << "STL C++ Exception: " << e.what() << DbPrint::endmsg;
   } catch (...) {
      DbPrint err(m_name);
      err << DbPrintLvl::Fatal << "[RNTupleContainer] "
          << "Unknown exception occurred. Cannot give more details."
          << DbPrint::endmsg;
   }
   DbPrint log(m_name);
   log << DbPrintLvl::Info << "Cannot load entry No." << evt_id << "..."
       << "Container has " << size() << " Entries in total." << DbPrint::endmsg;
   m_ioBytes = -1;
   return Error;
}


// Initiate reading with a selection
DbStatus  RNTupleContainer::select(DbSelect& sel)
{
   if( sel.criteria().length() == 0 || sel.criteria() == "*" )  {
      sel.link().second = -1;
      return Success;
   } else  {
      DbPrint log(m_name);
      log << DbPrintLvl::Warning << "RNTuple selection not implemented, reading everything"
          << DbPrint::endmsg;
      sel.link().second = -1;
      return Success;
   }
   return Error;
}


// Fetch next object address of the selection to set token
DbStatus RNTupleContainer::fetch(DbSelect& sel)
{
   if( sel.criteria().length() > 0 and sel.criteria() != "*" )  {
      DbPrint log(m_name);
      log << DbPrintLvl::Warning << "RNTuple selection not implemented, reading everything"
          << DbPrint::endmsg;
   }
   sel.link().second++;
   return DbContainerImp::fetch(sel.link(), sel.link());
}


/// Access options
DbStatus RNTupleContainer::getOption(DbOption& opt) {
  const char* n = opt.name().c_str();
  if (!strcasecmp(n, "BYTES_IO")) {
    for (auto& desc : m_fieldDescs) {
      if (desc.auxdyn_reader) {
        m_ioBytes += desc.auxdyn_reader->getBytesRead();
        desc.auxdyn_reader->resetBytesRead();
      }
    }
    return opt._setValue((int)m_ioBytes);
  } else if (::toupper(n[0]) == 'T' and opt.name().length() > 9) {
    switch (::toupper(n[8])) {
      case 'E':
        if (!strcasecmp(n + 5, "ENTRIES"))
          return opt._setValue(int(m_pageSource->GetNEntries()));
        break;
      case 'T':
        if (!strcasecmp(n + 5, "TOTAL_BYTES")) {
          // metrics must be enabled
          // MN: need to learn how to use Metrics
          // const Detail::RNTupleMetrics& metr = m_ntupleReader->GetMetrics();
          return opt._setValue((double)0);
        }
        break;
      case 'Z':
        if (!strcasecmp(n + 5, "ZIP_BYTES")) {
          // MN TODO
          return opt._setValue(double(0));
        }
        break;
    }
  }
  return Error;
}

/// Set options
DbStatus RNTupleContainer::setOption(const DbOption& opt) {
  const char* n = opt.name().c_str();
  if (::toupper(n[0]) == 'R' and opt.name().length() > 9) {  // RNTUPLE_
    switch (::toupper(n[8])) {
      case 'S':
        if (!strcasecmp(n + 8, "SOME_RNTUPLE_OPTION")) {
          // so far no real options to set
          int val = 1;
          opt._getValue(val);
          // Use "val" for something
          return Success;
        }
        break;
      default:
        break;
    }
  }
  return Error;
}

/// Execute transaction action
DbStatus RNTupleContainer::transAct(Transaction::Action action) {
  DbPrint log(m_name);
  // Execute action on the base class first
  DbStatus status = DbContainerImp::transAct(action);
  if (!status.isSuccess()) return status;

  for (auto& desc : m_fieldDescs) {
    desc.rows_written = 0;
    if (desc.typeID() == DbColumn::BLOB) {
      s_char_Blob.release(false);
    }
  }
  m_isDirty = false;

  return Success;
}

/// Close the container and deallocate resources
DbStatus RNTupleContainer::close() {
  m_dbH = DbDatabase(POOL_StorageType);
  m_fieldDescs.clear();
  m_rootDb = nullptr;
  return DbContainerImp::close();
}
