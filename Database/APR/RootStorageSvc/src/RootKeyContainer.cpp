/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//        Root Database container implementation
//--------------------------------------------------------------------
//
//        Package    : pool (The POOL project)
//
//        Author     : M.Frank
//====================================================================
// Framework include files

#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbReflex.h"

// Local implementation files
#include "RootDatabase.h"
#include "RootKeyContainer.h"
#include "RootKeyIOHandler.h"

// Root include files
#include "TROOT.h"
#include "TFile.h"
#include "TClass.h"
#include "TKey.h"

#include <algorithm>

using namespace pool;

RootKeyContainer::RootKeyContainer(const std::string& name) :
  DbContainerImp(name),
  m_dir(0),
  m_dbH(POOL_StorageType),
  m_rootDb(0),
  m_ioHandler(new RootKeyIOHandler),
  m_policy(TObject::kOverwrite),    // On update write new versions
  m_ioBytes(-1)
{
}

/// Standard destructor
RootKeyContainer::~RootKeyContainer()   {
  releasePtr(m_ioHandler);
  RootKeyContainer::close();
}

uint64_t RootKeyContainer::nextRecordId()    {
  if ( m_dir )  {
    TList* list = m_dir->GetListOfKeys();
    if ( list ) {
      unsigned s1 = 0;
      TKey* k = (TKey*)list->Last();
      if ( k ) {
        ::sscanf(k->GetName(), "_pool_valid_%08u", &s1);
        ++s1;
      }
      auto s2 = DbContainerImp::size(); // Number of objects on commit stack
      return s1+s2;
    }
  }
  return -1;
}

uint64_t RootKeyContainer::size()    {
  if ( m_dir )  {
    TList* list = m_dir->GetListOfKeys();
    if ( list ) {
      // Number of committed objects
      auto s1 = list->GetSize();
      // Number of objects on commit write stack
      auto s2 = DbContainerImp::size();
      return s1+s2;
    }
  }
  return -1;
}

/// Execute transaction action
DbStatus RootKeyContainer::transAct(Transaction::Action action) 
{
   // execure action on the base class first
   DbStatus status = DbContainerImp::transAct(action);
   if( !status.isSuccess() ) return status;

   if( action != Transaction::TRANSACT_FLUSH ) return Success;
   if( !m_dir ) return Error;
   m_dir->SaveSelf();
   return Success;
}
   
// Fetch next object address to set token
DbStatus RootKeyContainer::next(Token::OID_t& linkH) {
  char txt[64];
  const long long int stk_size = DbContainerImp::size();
  const long long int cnt_size = nextRecordId()-stk_size;
  for(int j=linkH.second; j < cnt_size; ++j) {
    ++linkH.second;
    ::sprintf(txt, "_pool_valid_%08d", static_cast<int>(linkH.second));
    const TKey* key = (TKey*)m_dir->GetListOfKeys()->FindObject(txt);
    if ( key )    {
      const char* class_name = key->GetClassName();
      const DbTypeInfo* typ = m_dbH.objectShape( DbReflex::forTypeName(class_name) );
      if ( typ )  {
        return Success;
      }
      ATH_MSG_ERROR("Failed to find the correct shape identifier for class:" << class_name);
      return Error;
    }
    else {
      // Here we are if key names have holes due to deletes
      // Try to get the next one.
    }
  }
  return Error;
} 

// Interface Implementation: Find entry in container
DbStatus RootKeyContainer::load( void** ptr, ShapeH shape,
                                 const Token::OID_t& linkH,
                                 Token::OID_t& oid,
                                 bool          any_next)
{
  DbStatus sc = Error;
  oid.second = linkH.second;
  for(long long int cnt = oid.second,last=nextRecordId(); cnt <= last; ++cnt) {
    char txt[64];
    ::sprintf(txt, "_pool_valid_%08d", static_cast<int>(oid.second));
    const TKey* key = (TKey*)m_dir->GetListOfKeys()->FindObject(txt);
    if ( key )    {
       sc = loadObject(ptr, shape, oid);
       return sc;
    }
    if ( !any_next )  {
      return Error;
    }
    oid.second++;
  }
  if ( linkH.second < 0 || (uint64_t)linkH.second <= size() ) {
    ATH_MSG_DEBUG("No objects passing selection criteria..." 
                  << " Container has " << size() << " Entries in total.");
  }
  return sc;
}

DbStatus RootKeyContainer::loadObject( void** ptr, ShapeH shape,
                                       Token::OID_t&   oid )
{
   char txt[64];
   ::sprintf(txt, "_pool_valid_%08d", static_cast<int>(oid.second));
   TDirectory::TContext dirCtxt(m_dir->GetFile());
   TKey* key = (TKey*)m_dir->GetListOfKeys()->FindObject(txt);
   if ( key && ptr )    {
      const char* class_name = key->GetClassName();
      const DbTypeInfo* typ = dynamic_cast<const DbTypeInfo*>(shape);
      if( typ )  {
         TClass* cl = TClass::GetClass(class_name);
         if( 0 != cl ) {
            if ( typ->columns().size() == 1) {
               *ptr = cl->New();
               int nbyte = m_ioHandler->read( key, ptr );
               if ( nbyte > 1 ) {
                  /// Update statistics
                  m_ioBytes = nbyte;
                  m_rootDb->addByteCount(RootDatabase::READ_COUNTER, nbyte);
                  return Success;
               }
            }
            else  {
               ATH_MSG_ERROR("I/O for types with more than 1 data member is not currently supported");
               ATH_MSG_ERROR("Type: " << typ->toString());
               return Error;
            }
         }
      }
   }
  m_ioBytes = -1;
  ATH_MSG_ERROR("Could not read object \"" << txt 
                << "\" from directory \"" << m_dir->GetName() << "\"");
  return Error;
}

DbStatus RootKeyContainer::writeObject(ActionList::value_type& action) {
   if ( m_dir )  {
      char knam[64];
      ::sprintf(knam, "_pool_valid_%08d", static_cast<int>(action.link.second));
      auto typ = static_cast<const DbTypeInfo*>(action.shape);
      if ( 0 == typ )   {
         ATH_MSG_ERROR("No type information present when writing an object!");
         return Error;
      }
      else {
         TDirectory::TContext dirCtxt(m_dir);
         if( typ->columns().size() == 1 ) {
            const DbColumn* col = *(typ->columns().begin());
            const std::string& typ_nam = col->typeName();
            TClass*  cl  = TClass::GetClass(typ_nam.c_str());
            if( !cl ) {
               ATH_MSG_ERROR("GetClass() failed for type " << typ_nam);
               return Error;
            }
            const void* p = action.dataAtOffset( col->offset() );
            int nbyte = m_ioHandler->write(cl, knam, p, m_policy);
            if ( nbyte > 1) {
               m_ioBytes = nbyte;
               m_rootDb->addByteCount(RootDatabase::WRITE_COUNTER, nbyte);
               return Success;
            } else {
               ATH_MSG_ERROR("[RootKeyContainer] Could not write an object");
            }
         } else {
            ATH_MSG_ERROR("I/O for types with more than 1 data member is not currently supported");
            ATH_MSG_ERROR("Type: " << typ->toString());
         }
      }
   }
   else {
      ATH_MSG_ERROR("[RootKeyContainer] Not a valid directory or callback when writing an object");
   }
   m_ioBytes = -1;
   return Error;
}

DbStatus RootKeyContainer::close()   {
  m_dbH = DbDatabase(POOL_StorageType);
  m_rootDb = 0;
  m_dir = 0;
  return DbContainerImp::close();
}

DbStatus RootKeyContainer::open(DbDatabase&           dbH, 
                                const std::string&    dir_nam, 
                                const DbTypeInfo*  /* info */, 
                                DbAccessMode          mode)  
{
  m_name = dir_nam;

  // Sanitise the name by replacing '/' with '_' (excluding the slash separating
  // the container name from the object name)
  std::string sanitisedName(dir_nam);
  std::size_t beg = sanitisedName.find_first_of('(');
  std::size_t end = sanitisedName.find_first_of(')');
  std::string sanitisedObjName = sanitisedName.substr(beg + 1, end - beg - 1);

  if (sanitisedObjName.starts_with("/")) {
      std::replace(sanitisedObjName.begin(), sanitisedObjName.end(), '/', '_');
  }
  if (sanitisedObjName.find("//") != std::string::npos) {
      std::string from = "//";
      std::string to = "/_";
      size_t start_pos = sanitisedObjName.find(from);
      sanitisedObjName.replace(start_pos, from.length(), to);
      std::replace(sanitisedObjName.begin() + start_pos + 1,
                   sanitisedObjName.end(), '/', '_');
  }
  sanitisedName.replace(beg + 1, sanitisedObjName.length(), sanitisedObjName);

  ATH_MSG_DEBUG("Opening RootKeyContainer, mode=" << accessMode(mode));

  if ( dbH.isValid() && dir_nam.length() > 0 )    {
    std::string nam = sanitisedName.starts_with('/') ? sanitisedName.substr(1)
                                                     : std::move(sanitisedName);
    size_t idx1     = 0, idx2 = nam.find('/',1);
    TDirectory::TContext dirCtxt(0);
    IDbDatabase* idb = dbH.info();
    m_rootDb = dynamic_cast<RootDatabase*>(idb);
    if (!m_rootDb) {
      m_dir = 0;
      return Error;
    }
    m_dir  = m_rootDb->file();
    do  {
      std::string s = nam.substr(idx1, idx2-idx1); 
      m_dir->cd();
      TDirectory* dir = (TDirectory*)m_dir->Get(s.c_str());
      if ( 0==dir && mode&pool::CREATE && !s.empty() ) {
        dir = m_dir->mkdir(s.c_str());
      }
      else if ( 0==dir ) {
        m_dir = 0;
        return Error;
      }
      m_dir = dir;
      if ( m_dir )    {
        TClass* cl = m_dir->IsA();
        if ( !cl->InheritsFrom(TDirectory::Class()) )    {
          ATH_MSG_ERROR("Cannot open container. Object with name found, but of the wrong type. " << endmsg
                        << "True type is :" << cl->GetName() << " rather than TDirectory.");
          return Error;
        }
        if (idx2 == std::string::npos) break;
        idx1  = idx2+1;
        idx2  = nam.find('/', idx1);
      }
    } while ( m_dir );
    if (m_dir)
      m_dir->cd();
    DbOption opt1("DEFAULT_WRITEPOLICY","");
    dbH.getOption(opt1);
    opt1._getValue(m_policy);
    /// Parent Database handle
    m_dbH = dbH;
    ATH_MSG_DEBUG("Opened container " << m_name << " of type "
                  << ROOTKEY_StorageType.storageName() << " with policy:" << m_policy);
    return Success;
  }
  ATH_MSG_ERROR("Cannot open container, invalid Database handle.");
  return Error;
}

/// This is a specialized method that checks if we can access the underlying TDirectory
DbStatus RootKeyContainer::checkAccess(DbDatabase& dbH,
                                       const std::string& dir_nam) const
{
  if ( dbH.isValid() )    {
    IDbDatabase* idb = dbH.info();
    auto rootDb = dynamic_cast<RootDatabase*>(idb);
    if (rootDb && rootDb->file()->Get<TDirectory>(dir_nam.c_str())) {
      return Success;
    }
  }
  ATH_MSG_DEBUG("Cannot access container '" << dir_nam 
    << "', invalid Database handle or container is not of type Directory.");
  return Error;
}

/// Access options
DbStatus RootKeyContainer::getOption(DbOption& opt) {
  if ( m_dir )  {
    const char* n = opt.name().c_str();
    if ( !strcasecmp(n,"BYTES_IO") )  {
      return opt._setValue((int)m_ioBytes);
    }
    else if ( !strcasecmp(n,"DIRECTORY") )  {
      return opt._setValue((void*)m_dir);
    }
    else if ( !strcasecmp(n, "DEFAULT_WRITEPOLICY") ) {
      return opt._setValue(int(m_policy));
    }
    else if ( ::toupper(n[0])=='D' && opt.name().length() > 4 ) {
      switch(::toupper(n[4]))  {
      case 'B':
        if ( !strncasecmp(n+4,"BYTES",5) )
          return opt._setValue((int)m_ioBytes);
        break;
      case 'F':
        if ( !strcasecmp(n+4,"FILE") )  {
          return opt._setValue((void*)m_dir->GetFile());
        }
        break;
      case 'G':
        if ( !strcasecmp(n+4,"GETOBJ") )  {
          return opt._setValue((void*)m_dir->Get(opt.option().c_str()));
        }
        break;
      case 'L':
        if ( !strcasecmp(n+4,"LIST_KEYS") )  {
          return opt._setValue((void*)m_dir->GetListOfKeys());
        }
        else if ( !strcasecmp(n+4,"LS") )  {
          m_dir->ls();
          return opt._setValue(int(1));
        }
        break;
      case 'N':
        if ( !strcasecmp(n+4,"NKEYS") )  {
          return opt._setValue(int(m_dir->GetNkeys()));
        }
        else if ( !strcasecmp(n+4,"NBYTESKEYS") )  {
          return opt._setValue(int(m_dir->GetNbytesKeys()));
        }
        break;
      case 'M':
        if ( !strcasecmp(n+4,"MOTHER") )  {
          return opt._setValue((void*)m_dir->GetMother());
        }
        else if ( !strcasecmp(n+4,"MODIFIED") )  {
          return opt._setValue((int)m_dir->IsModified() ? 1 : 0);
        }
        break;
      case 'P':
        if ( !strcasecmp(n+4,"PRINT") )  {
          m_dir->Print(opt.option().c_str());
          std::cout << std::endl;
          return opt._setValue(1);
        }
        break;
      case 'W':
        if ( !strcasecmp(n+4,"WRITABLE") )  {
          return opt._setValue((int)m_dir->IsWritable() ? 1 : 0);
        }
        break;
      }
    }
  }
  return Error;  
}


/// Set options
DbStatus RootKeyContainer::setOption(const DbOption& opt)  { 
  if ( m_dir )  {
    const char* n = opt.name().c_str();
    if ( !strcasecmp(n, "DEFAULT_WRITEPOLICY") ) {
      opt._getValue(m_policy);
      return Success;
    }
    else if ( ::toupper(n[0]) == 'D' )  {
      switch(::toupper(n[4]))   {
      case 'C':
        if ( !strcasecmp(n+4,"CLOSE") )  {
          m_dir->Close(opt.option().c_str());
          return Success;
        }
        break;
      case 'D':
        if ( !strcasecmp(n+4,"DELETEOBJ") )  {
          m_dir->Delete(opt.option().c_str());
          return Success;
        }
        break;
      case 'P':
        if ( !strcasecmp(n+4,"PRINT") )  {
          m_dir->Print(opt.option().c_str());
          std::cout << std::endl;
          return Success;
        }
        else if ( !strcasecmp(n+4,"PURGE") )  {
          int val=1;
          opt._getValue(val);
          if ( val > 0 )  {
            m_dir->Purge(val);
            return Success;
          }
        }
        break;
      }
    }
  }
  return Error;  
}
