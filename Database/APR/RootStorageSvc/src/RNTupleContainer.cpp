/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//    APR Database Container implementation for ROOT/RNTuple
//--------------------------------------------------------------------
//    Author     : M.Nowak
//====================================================================

// Framework include files
#include "RootAuxDynIO/IRootAuxDynIO.h"
#include "StorageSvc/DbArray.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/Transaction.h"

// Local implementation files
#include "RNTupleContainer.h"
#include "RootDataPtr.h"
#include "RootDatabase.h"
#include "RNTupleWriterHelper.h"

#include "Gaudi/PluginService.h"

// Root include files
#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "TFile.h"
#include "TError.h"

#include <algorithm>

using std::string;
using namespace pool;

static UCharDbArrayAthena s_char_Blob ATLAS_THREAD_SAFE;


/// Required here for unique_ptr compilation
RNTupleContainer::FieldDesc::FieldDesc(const DbColumn& c) : DbColumn(c) {}


// Get the type name
const std::string RNTupleContainer::FieldDesc::typeName() {
  auto tid = typeID();
  switch (tid) {
    case DbColumn::STRING:
    case DbColumn::LONG_STRING:
    case DbColumn::NTCHAR:
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
RNTupleContainer::RNTupleContainer(const std::string& name) :
   DbContainerImp(name),
   m_type(nullptr),
   m_dbH(POOL_StorageType), m_rootDb(nullptr),
   m_ioBytes(0), m_isDirty(false),
   m_index(0), m_indexSize(0), m_indexBump(0), m_indexMulti( getpid() )
{ }


/// Standard destructor
RNTupleContainer::~RNTupleContainer() { close(); }

uint64_t RNTupleContainer::size() {
  auto s = DbContainerImp::size();
  if( m_ntupleReader ) s += m_ntupleReader->GetNEntries();
  if( m_ntupleWriter ) s += m_ntupleWriter->size();
  return s;
}

/// Open the container for object access
DbStatus RNTupleContainer::open( DbDatabase& dbH, const std::string& nam,
                                 const DbTypeInfo* info, DbAccessMode mode)
{
   m_name = nam;
   m_fieldDescs.clear();
   m_rootDb = dynamic_cast<RootDatabase*>(dbH.info());
   if( !dbH.isValid() or !info or !m_rootDb ) {
      ATH_MSG_ERROR("Cannot open container '" << m_name << "', invalid Database handle.");
      return pool::Error;
   }
   m_indexBump = m_rootDb->currentIndexMasterID();

   ATH_MSG_DEBUG("Opening, mode=" << accessMode(mode));
   std::string ntupleName(m_name);
   std::replace(ntupleName.begin(), ntupleName.end(), '/', '_');
   std::string fieldName;

   m_auxDynTool = Gaudi::PluginService::Factory< RootAuxDynIO::IFactoryTool*() >::create("RootAuxDynIO::FactoryTool");
   if( !m_auxDynTool ) {
      ATH_MSG_WARNING("Could NOT load RootAuxDynIO::FactoryTool. Dynamic attributes support disabled");
   }
   const DbTypeInfo::Columns& cols = info->columns();
   ATH_MSG_DEBUG("   attributes# = " << cols.size());
   if (cols.size() == 1) {
      // extract ntuple and field name for grouped containers, notation:
      // "ntuple(column)"
      std::string::size_type inx = nam.find('(');
      if (inx != std::string::npos) {
         std::string::size_type inx2 = nam.find(')');
         if (inx2 == std::string::npos or inx2 != nam.size() - 1) {
            ATH_MSG_ERROR("Misplaced closing ')' in " << m_name);
            return pool::Error;
         }
         fieldName = ntupleName.substr(inx + 1, inx2 - inx - 1);
         ntupleName.resize(inx);
         ATH_MSG_DEBUG("Grouped Container '" << ntupleName << "/" << fieldName << "'");
      }
   }
   // prepare descriptions for all object data members (aka columns)
   m_fieldDescs.reserve(cols.size());
   for (const auto& col : cols) {
      m_fieldDescs.emplace_back(*col);
      FieldDesc& dsc = m_fieldDescs.back();
      dsc.fieldname = fieldName.empty() ? col->name() : fieldName;
      dsc.sgkey = dsc.fieldname;  // remember the original name (usually coming from SG Key)
      for (auto& c : dsc.fieldname)
         if (!std::isalnum(c)) c = '_';
      if (dsc.typeID() == DbColumn::BLOB or dsc.typeID() == DbColumn::ANY or
          dsc.typeID() == DbColumn::POINTER) {
         if (initObjectFieldDesc(dsc) != Success) return pool::Error;
      }
   }

   if( mode & pool::CREATE ) {
      m_ntupleWriter = m_rootDb->getNTupleWriter(ntupleName, true);
      if( m_ntupleWriter ) {
         ATH_MSG_DEBUG("Created container " << m_name
             << " of type " << ROOTRNTUPLE_StorageType.storageName());
      } else {
         ATH_MSG_ERROR("Could not create container " << m_name
             << " of type " << ROOTRNTUPLE_StorageType.storageName());
         return pool::Error;
      }
      // Prepare Field descriptions
      for( auto& dsc : m_fieldDescs ) {
         ATH_MSG_DEBUG("Adding new RNTuple Field: name=" << dsc.fieldname 
             << "  typename=" << dsc.typeName());
         m_ntupleWriter->addField( dsc.fieldname, dsc.typeName() );
      }
   }
   else if( mode & (pool::READ | pool::UPDATE) ) {
      // create (and keep in the description object) the rntuple field for reading
      m_ntupleReader = m_rootDb->getNTupleReader(ntupleName);
      if( m_ntupleReader ) {
         ATH_MSG_DEBUG("Created container " << m_name << " for RNTuple reading");
      } else {
         ATH_MSG_ERROR("Could not create container " << m_name << " for RNTuple reading");
         return pool::Error;
      }
      for( auto& dsc : m_fieldDescs ) {
         if( info->clazz().Name()=="pool::DbString" ) {
            dsc.view = m_ntupleReader->GetView(dsc.fieldname, nullptr, typeid(std::string));
         } else {
            // Can't use type_info because of default template argument in DataVectors ATEAM-1087
            dsc.view = m_ntupleReader->GetView(dsc.fieldname, nullptr, info->clazz().Name());
         }
         if( dsc.auxdyn_writer ) {
            // Attach RNTuple Reader (owned by the DB)
            const std::string type_name = dsc.view->GetField().GetTypeName();
            dsc.auxdyn_reader = m_auxDynTool->getNTupleAuxDynReader( dsc.fieldname, type_name, m_ntupleReader );
            // If we set up a reader, then disable aging
            // for this file.  That will prevent POOL from
            // deleting the file while we still have
            // references to its branches.
            dbH.setAge(-10);
         }
      }
   }

   ATH_MSG_DEBUG("Opened container " << m_name << " of type "
       << ROOTRNTUPLE_StorageType.storageName());
   m_dbH = dbH;
   m_type = info;
   return Success;
}


/// This is a specialized method that checks if we can access the underlying RNTuple
DbStatus RNTupleContainer::checkAccess(DbDatabase& dbH,
                                       const std::string& nam) const
{
   if ( dbH.isValid() )    {
      IDbDatabase* idb = dbH.info();
      auto rootDb = dynamic_cast<RootDatabase*>(idb);
      if (rootDb && rootDb->file()->Get<ROOT::RNTuple>(nam.c_str())) {
         return Success;
      }
   }
   ATH_MSG_DEBUG("Cannot access container '" << nam 
      << "', invalid Database handle or container is not of type RNTuple.");
   return pool::Error;
}


/// Init a field description for an object (i.e. find TClass etc.)
DbStatus RNTupleContainer::initObjectFieldDesc( FieldDesc& dsc )
{
   dsc.clazz = TClass::GetClass( dsc.typeName().c_str() );
   if( dsc.clazz )  {
      if( dsc.clazz->GetStreamerInfo() and dsc.clazz->HasDictionary() )  {
         // AUX STORE specifics
         // get rid of the AUX_POSTFIX dot at the end (converted to _ earlier)
         if (dsc.fieldname.ends_with("Aux_")) dsc.fieldname.back() = ':';
         if( m_auxDynTool and m_auxDynTool->hasAuxStoreIO(dsc.clazz) ) {
            dsc.auxdyn_writer = m_auxDynTool->getNTupleAuxDynWriter(*dsc.clazz);
            if( !dsc.auxdyn_writer ) {
               ATH_MSG_ERROR("Cannot get AuxDyn writer for " << dsc.fieldname);
               return pool::Error;
            }
         }
         return Success;
      } else {
         ATH_MSG_ERROR("Failed to open the container " << m_name
             << " of type " << ROOTRNTUPLE_StorageType.storageName() << " Class "
             << dsc.clazz->GetName() << " is unknown.");
      }
   } else {
      ATH_MSG_ERROR("Failed to open the container " << m_name
          << " of type " << ROOTRNTUPLE_StorageType.storageName() << ". Type "
          << dsc.typeName() << " is unknown.");
   }
   return pool::Error;
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
      ATH_MSG_ERROR("Attempt to write to an RNTuple Container twice in the same transaction! ");
      m_ioBytes = -1;
      return pool::Error;
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
             if( dsc.auxdyn_writer ) {
                auto attrList = dsc.auxdyn_writer->collectAuxAttributes( dsc.fieldname, dsc.object );
                for(const auto& itr : attrList) {
                   m_ntupleWriter->addAttribute( itr );
                }
             }
          } catch(const std::exception& exc) {
             ATH_MSG_ERROR("Dynamic attributes writing error: " << exc.what());
             p.ptr = nullptr;  // signal an error condition
             break;
          }
          dsc.rows_written++;
          break;
       case DbColumn::BLOB:
          // MN: BLOBs not really tested
          s_char_Blob.m_size    = p.blobSize();
          s_char_Blob.m_buffer  = (unsigned char*)p.blobData();
          dsc.object            = &s_char_Blob;
          p.ptr                 = dsc.object;
          break;
       case DbColumn::STRING:
       case DbColumn::LONG_STRING:
          dsc.str.clear();  // just to be on the safe side
          // p.ptr is pointing to std::string already
          break;
       case DbColumn::NTCHAR:
       case DbColumn::TOKEN:
          // copy char* to the string buffer dsc.str and make p.ptr point to it
          dsc.str = p.c_str;
          p.ptr = &dsc.str;
          break;
       default:
          // native types are simply passed in p.ptr
          break;
      }
      if( !p.ptr ) {
         ATH_MSG_ERROR("[RNTupleContainer] Could not write an object of type " << dsc.typeName());
         throw std::runtime_error(std::string("[RNTupleContainer] Could not write an object of type  ") + dsc.typeName());
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
      evt_id = m_rootDb->indexLookup(m_ntupleReader, evt_id);
   }
   // lock access to this DB for MT safety
   std::lock_guard<std::recursive_mutex>     lock( m_rootDb->ioMutex() );
   int numBytes = 0;
   for( auto& dsc : m_fieldDescs ) {
      // read the object
      RootDataPtr p(*obj_p);
      switch( dsc.typeID() ) {
       case DbColumn::BLOB:
          {
             // MN: not sure about this one, implement if needed ever
             ATH_MSG_FATAL("[RNTupleContainer] - BLOB reading not implemented yet");
             return pool::Error;
          }
       case DbColumn::ANY:
       case DbColumn::POINTER:
          // MN: should not need any special action here
          break;
       default:
          p.c_str += dsc.offset();
          break;
      }
      if( !p.ptr ) {
         // create the object for the user and pass ownership to them
         p.ptr = dsc.view->GetField().CreateObject<void>().release();
         *obj_p = p.ptr;
      }
      dsc.view->BindRawPtr( p.ptr );
      // read into the object
      (*dsc.view)(evt_id);
      numBytes += 1;

      if (dsc.auxdyn_reader) {
         dsc.auxdyn_reader->addReaderToObject(*obj_p, evt_id, &m_rootDb->ioMutex());
      }
   }
   /// Update statistics
   m_ioBytes = numBytes;
   m_rootDb->addByteCount(RootDatabase::READ_COUNTER, numBytes);
   return Success;
}


// Initiate reading with a selection
DbStatus  RNTupleContainer::select(DbSelect& sel)
{
   sel.link().second = -1;
   return Success;
}


// Fetch next object address of the selection to set token
DbStatus RNTupleContainer::fetch(DbSelect& sel)
{
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
          return opt._setValue(int(m_ntupleReader->GetNEntries()));
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
  return pool::Error;
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
  return pool::Error;
}

/// Execute transaction action
DbStatus RNTupleContainer::transAct(Transaction::Action action) {
  // Execute action on the base class first
  DbStatus status = DbContainerImp::transAct(action);
  if (!status.isSuccess()) return status;

  for (auto& desc : m_fieldDescs) {
    desc.rows_written = 0;
    if (desc.typeID() == DbColumn::BLOB) {
      s_char_Blob.release(false);
    }
  }
  clearDirty();

  return Success;
}

/// Add single entry to container
DbStatus RNTupleContainer::save(DbObjectHandle<DbObject>& objH) {
  // Execute action on the base class first
  DbStatus status = DbContainerImp::save(objH);
  // ASM: Do we need to reset rows_written as well?
  clearDirty();
  return status;
}

/// Close the container and deallocate resources
DbStatus RNTupleContainer::close() {
  m_dbH = DbDatabase(POOL_StorageType);
  m_fieldDescs.clear();
  m_rootDb = nullptr;
  return DbContainerImp::close();
}
