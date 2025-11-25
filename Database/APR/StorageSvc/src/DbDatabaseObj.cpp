/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDatabaseObj object implementation
//--------------------------------------------------------------------
//
//  Package    : System (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

#include "StorageSvc/DbString.h"
#include "DbDatabaseObj.h"
#include "DbContainerObj.h"

// Public POOL include files
#include "StorageSvc/DbToken.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/IDbDatabase.h"
#include "StorageSvc/IDbContainer.h"

#include <memory>
#include <cstdio>

using namespace pool;

std::ostream& operator << (std::ostream& os, const Token::OID_t oid ) {
   os << "("<<oid.first<<","<<oid.second<<")";
   return os;
}

static const Guid s_localDb("00000000-0000-0000-0000-000000000000");


// Standard Constructor
DbDatabaseObj::DbDatabaseObj( DbDomain&       dom, 
                              const std::string&   pfn, 
                              const std::string&   fid, 
                              DbAccessMode    mod) 
: Base(fid, mod, dom.type(), dom.db()), 
  APRMessaging( pfn ),
  m_dom(dom), m_info(0), m_string_t(0), m_fileAge(0)
{
  m_logon = pfn;
  std::unique_ptr<DbToken> tok(new DbToken());
  tok->setTechnology(dom.type().type());
  tok->setClassID(Guid::null());
  tok->setDb(fid);
  tok->oid().first  = INVALID;
  tok->oid().second = INVALID;
  tok->setKey(DbToken::TOKEN_FULL_KEY);
  tok->setKey(DbToken::TOKEN_CONT_KEY);
  m_token = tok.release();
  if ( 0 == db() )    {
    ATH_MSG_ERROR("->  Access   DbDatabase   " << accessMode(mode())
                  << " [" << type().storageName() << "] " << name() 
                  << " impossible." << endmsg
                  << "                          " << logon());
    type().missingDriver(msg());
    return;
  }
  if( !m_dom.add( name(), this ).isSuccess() ) {
    ATH_MSG_ERROR("->  Access   DbDatabase   " << accessMode(mode())
                  << " [" << type().storageName() << "] " << name()
                  << " impossible." << endmsg
                  << "                          " << logon()
                  << " Error inserting DbDatabaseObj into domain!");
    return;
  }
  ATH_MSG_INFO("->  Access   DbDatabase   " << accessMode(mode())
               << " [" << type().storageName() << "] " << name());
  DbString s;
  DbTypeInfo::Columns c;
  c.push_back(new DbColumn("db_string",DbColumn::STRING,size_t(static_cast<std::string*>(&s))-size_t(&s),0,1,0));
  m_string_t = DbTypeInfo::create(std::string("pool::DbString"), c);
  if ( m_string_t ) m_string_t->addRef();
}

// Standard Destructor
DbDatabaseObj::~DbDatabaseObj()  {
  clearEntries();
  cleanup();
  if( m_string_t ) {
     m_string_t->deleteRef();
     m_string_t = 0;
  }
  m_dom.remove(this);
  m_token->release();
}

/// Access the size of the database: May be undefined for some technologies
long long int DbDatabaseObj::size() {
  if ( 0 == m_info )    {  // Re-open the database if it was retired
     open();
  }
  return 0==m_info ? -1 : m_info->size();
}

// Perform cleanup of internal structures.
DbStatus DbDatabaseObj::cleanup()  {
  for(LinkVector::iterator i=m_linkVec.begin(); i != m_linkVec.end(); ++i) {
    delete (*i);
  }
  for(ShapeMap::iterator j=m_shapeMap.begin(); j != m_shapeMap.end(); ++j) {
    ((*j).second)->deleteRef();
  }
  m_linkMap.clear();
  m_linkVec.clear();
  m_indexMap.clear();
  m_shapeMap.clear();
  m_paramMap.clear();
  m_classMap.clear();
  if ( m_info )   {
    deletePtr( m_info );
    ATH_MSG_INFO("->  Deaccess DbDatabase   " << accessMode(mode())
                  << " [" << type().storageName() << "] " << name());
  }
  return Success;
}

// Add association entry
DbStatus DbDatabaseObj::makeLink(Token* pTok, Token::OID_t& refLnk) {
  if ( pTok )   {
    int   is_dbTok  = (typeid(*pTok) == typeid(DbToken));
    LinkMap::iterator i;
    if ( is_dbTok )   {
      DbToken* pdbTok = static_cast<DbToken*>(pTok);
      pdbTok->setKey(DbToken::TOKEN_CONT_KEY);
      i = m_linkMap.find(pdbTok->contKey());
    }
    else  {
      Guid tmp_key;
      DbToken::makeKey(pTok, DbToken::TOKEN_CONT_KEY, tmp_key);
      i = m_linkMap.find(tmp_key);
    }
    if ( i != m_linkMap.end() )   {
      DbToken* t = (*i).second;
      refLnk.first  = t->oid().first;
      refLnk.second = pTok->oid().second;
      return Success;
    }
    else if ( mode() != pool::READ ) {
      const Guid& dbn = pTok->dbID();
      std::unique_ptr<DbToken> link(new DbToken());
      link->fromString(pTok->toString());
      link->oid().first = m_linkVec.size();
      link->setKey(DbToken::TOKEN_FULL_KEY);
      link->setKey(DbToken::TOKEN_CONT_KEY);
      // Add the persistent entry to the links container
      if ( 0 != m_string_t )   {
        ATH_MSG_DEBUG("--->Adding Assoc :" << link->dbID() 
                      << "/" << link->contID() << " [" << std::hex << link->technology() << "] "
                      << " (" << link->oid().first << " , " << link->oid().second << ")" << std::dec << endmsg
                      << "---->ClassID:" << link->classID().toString() );
        refLnk.first  = link->oid().first;
        refLnk.second = pTok->oid().second;
        if ( dbn == name() )  {
          link->setDb(s_localDb);
	        link->setLocal(true);
        }
        // Update link to use persistent oid
        link->oid().first = m_links->info()->nextRecordId() + 2; // Taking into account unsaved ##Container links
        DbString link_string(link->toString());
        if ( !m_links.store(&link_string, m_string_t).isSuccess() )    {
          return Error;
        }
        link->setDb(dbn);
        // Update the transient list of links
	      m_linkMap.insert( LinkMap::value_type(link->contKey(), link.get()));
        m_indexMap.insert( IndexMap::value_type(link->oid().first, m_linkVec.size()));
        m_linkVec.push_back( link.release() );
        return Success;
      }
    }
  }
  return Error;
}

// Retrieve shape information for a specified object by shape ID
const DbTypeInfo* DbDatabaseObj::objectShape(const Guid& id)  {
  if ( 0 == m_info ) open();
  ShapeMap::const_iterator i = m_shapeMap.find(id);
  if( i != m_shapeMap.end() ) return (*i).second;
  if( id == m_string_t->shapeID() ) return m_string_t;
  return nullptr;
}

// Retrieve shape information for a specified object by reflection handle
const DbTypeInfo* DbDatabaseObj::objectShape(const TypeH& id)  {
  if ( 0 == m_info ) open();
  std::map<TypeH, const DbTypeInfo*>::const_iterator i = m_classMap.find(id);
  if( i != m_classMap.end() ) return i->second;
  if( id == m_string_t->clazz() or id.Name() == "string" ) {
     // hack to enable reading DbStrings from KeyContainer::fetch()
     return m_string_t;
  }
  return nullptr;
}

// Retrieve shape information for a specified object by container name
const DbTypeInfo* DbDatabaseObj::contShape(const std::string& nam) {
  if ( 0 == m_info )    {
    open();
  }
  LinkVector::const_iterator j=m_linkVec.begin();
  for(; j != m_linkVec.end(); ++j ) {
    DbToken* t = (*j);
    if ( !t->typeInfo() )    {
      t->setTypeInfo(objectShape(t->classID()));
    }
    if ( t->typeInfo() )    {
      if ( t->dbID() == name() && t->contID() == nam )  { // in ##Links
        return t->typeInfo();
      }
    }
  }
  return 0;
}

// Add persistent shape to the Database
DbStatus DbDatabaseObj::addShape (const DbTypeInfo* pShape) {
  if ( pShape )    {
    const Guid& id = pShape->shapeID();
    ShapeMap::iterator i = m_shapeMap.find(id);
    if ( i != m_shapeMap.end() )   {
      return Success;
    }
    else if ( m_string_t and (pShape == m_string_t) )  {
      return Success;
    }
    else if ( m_string_t and (id == m_string_t->shapeID()) )  {
      return Success;
    }
    else if ( mode() != pool::READ )  {
      const std::string& dsc = pShape->toString();
      // Add the persistent entry to the links container
      if ( 0 != m_string_t )   {
        // Update the transient list of links
        // This must be done BEFORE the entry 
        // is inserted into the container!
        // Otherwise save will add the type 
        // again and again ending in an 
        // infinite recursion
        const DbTypeInfo *pShape2 = DbTypeInfo::fromString(dsc);
        const DbTypeInfo::Columns& cols = pShape2->columns();
        ATH_MSG_DEBUG("--->Adding Shape[" << m_shapeMap.size() << " , "
                       << pShape2->shapeID().toString() << "]: "
                       << " [" << cols.size() << " Column(s)]" );
        ATH_MSG_DEBUG("---->Class:" << (pShape2->clazz() ? DbReflex::fullTypeName(pShape2->clazz()) : "<not available>"));
        for (size_t ic=0; ic < cols.size();++ic)  {
          const DbColumn* c = cols[ic];
          ATH_MSG_DEBUG("---->[" << ic << "]:" << c->name()
              << " Typ:" << c->typeName() << " ["<< c->typeID() << ']'
              << " Size:" << c->size()
              << " Offset:" << c->offset()
              << " #Elements:" << c->nElement());
        }
        bool inserted = m_shapeMap.insert( ShapeMap::value_type(id, pShape2) ).second;
        if ( pShape2 == m_string_t || id == m_string_t->shapeID() )   {
          return Success;
        }
        DbString shape_string(dsc);
        if ( !m_shapes.store(&shape_string, m_string_t).isSuccess() )  {
          i = m_shapeMap.find(id);
          m_shapeMap.erase(i);
          return Error;
        }
        if ( inserted ) pShape2->addRef();
        if ( pShape2->clazz() )  {
          m_classMap.insert(std::make_pair(pShape2->clazz(), pShape2));
        }
        return Success;
      }
    }
  }
  return Error;
}

// Open Database object
DbStatus DbDatabaseObj::open()   {
  if ( !m_info && m_dom.isValid() && db() )    {
    m_info = db()->createDatabase();
    if ( m_info->open(m_dom, m_logon, mode()).isSuccess() )    {
      // Age open databases. Aging is only effective
      // for read-only databases. Otherwise no aging
      // is applied, because it is assumed, that objects
      // with pending connections may still be written.
      setAge(0);
      if ( 0==(mode()&pool::CREATE) && 0==(mode()&pool::UPDATE) )  {
        m_dom.ageOpenDbs();
        setAge(0);
        m_dom.closeAgedDbs();
      }
      if ( 0 != m_string_t )   {
        DbDatabase dbH(this);
        const Guid& guid = m_string_t->shapeID();

        // If we're reading, try to deduce the correct type of the POOL internal containers
        auto containerType = type();
        if (mode() == pool::READ) {
          DbContainer testCont;
          int majorTypeValue = containerType.majorType() >> 8;
          for(int minorTypeValue = 0; minorTypeValue < pool::DbType::MINOR_MASK; ++minorTypeValue) {
            DbType testType = pool::makeTechnology(majorTypeValue, minorTypeValue);
            if(testCont.checkAccess(dbH,"##Shapes",testType).isSuccess()) {
              containerType = testType;
              break;
            }
          }
        }

        // Add link to "##Shapes" container
        std::unique_ptr<DbToken> l1(new DbToken());
        l1->setDb(name());
        l1->setCont("##Shapes");
        l1->setTechnology(containerType.type());
        l1->setClassID(guid);
        l1->oid().first  = m_linkVec.size();
        l1->oid().second = INVALID;
        l1->setKey(DbToken::TOKEN_FULL_KEY);
        l1->setKey(DbToken::TOKEN_CONT_KEY);
        // Update the transient list of links
        m_linkMap.insert( LinkMap::value_type(l1->contKey(), l1.get()));
        m_indexMap.insert( IndexMap::value_type(l1->oid().first, m_linkVec.size()));
        m_linkVec.push_back( l1.release() );

        // Add link to "##Links" container
        std::unique_ptr<DbToken> l2(new DbToken());
        l2->setDb(name());
        l2->setCont("##Links");
        l2->setTechnology(type().type());
        l2->setClassID(guid);
        l2->oid().first  = m_linkVec.size();
        l2->oid().second = INVALID;
        l2->setKey(DbToken::TOKEN_FULL_KEY);
        l2->setKey(DbToken::TOKEN_CONT_KEY);
        // Update the transient list of links
        m_linkMap.insert( LinkMap::value_type(l2->contKey(), l2.get()));
        m_indexMap.insert( IndexMap::value_type(l2->oid().first, m_linkVec.size()));
        m_linkVec.push_back( l2.release() );

        // Internal containers use stack buffers with load() to avoid DbHeap overhead
        if ( m_shapes.open(dbH,"##Shapes",m_string_t,containerType,mode()).isSuccess() )    {
          Token::OID_t oid(0, 0);
          DbString shape_str;
          DbObject* ptr = &shape_str;

          while (static_cast<uint64_t>(oid.second) <= m_shapes.size()) {
            auto result = m_shapes.ptr()->load(&ptr, m_string_t, oid, oid, true);
            if (!result.isSuccess() || !ptr) break;
            const DbTypeInfo* pShape = DbTypeInfo::fromString(shape_str);
            const DbTypeInfo::Columns& cols = pShape->columns();
            ATH_MSG_DEBUG("--->Reading Shape[" << m_shapeMap.size() << " , "
                          << pShape->shapeID().toString() << "]: "
                          << "[" << cols.size() << " Column(s)]" );
            for (size_t ic=0; ic < cols.size();++ic)  {
              const DbColumn* c = cols[ic];
              ATH_MSG_DEBUG("---->[" << ic << "]:" << c->name()
                  << " Typ:" << c->typeName() << " ["<< c->typeID() << ']'
                  << " Size:" << c->size()
                  << " Offset:" << c->offset()
                  << " #Elements:" << c->nElement());
            }
            // Update the transient list of links
            if( m_shapeMap.insert(ShapeMap::value_type(pShape->shapeID(), pShape)).second )
               pShape->addRef();
            const bool noIdScan = true;
            if ( pShape->clazz(noIdScan) )  {
               m_classMap.insert(std::make_pair(pShape->clazz(), pShape));
            }
            ++oid.second;
          }
        }

        if ( m_links.open(dbH,"##Links",m_string_t,containerType,mode()).isSuccess() )  {
          Token::OID_t oid(0, 0);
          DbString link_str;
          DbObject* ptr = &link_str;

          while (static_cast<uint64_t>(oid.second) <= m_links.size()) {
            auto result = m_links.ptr()->load(&ptr, m_string_t, oid, oid, true);
            if (!result.isSuccess() || !ptr) break;
            std::unique_ptr<DbToken> link(new DbToken());
            link->fromString(link_str);
            // Update the transient list of links
            if ( s_localDb == link->dbID() ) {
              link->setDb(name());
	      link->setLocal(true);
            }
            ATH_MSG_DEBUG("--->Reading Assoc:" << link->dbID()
                << "/" << link->contID()
                << " [" << std::hex << link->technology() << "] "
                << " (" << link->oid().first << " , " << link->oid().second << ")" << std::dec );
            ATH_MSG_DEBUG("---->ClassID:" << link->classID().toString());
            link->setKey(DbToken::TOKEN_FULL_KEY);
            link->setKey(DbToken::TOKEN_CONT_KEY);
	    if ( m_linkMap.find(link->contKey()) == m_linkMap.end() )  {
	      m_linkMap.insert( LinkMap::value_type(link->contKey(), link.get()));
	    }
            m_indexMap.insert( IndexMap::value_type(link->oid().first, m_linkVec.size()));
	    m_linkVec.push_back(link.release());
            ++oid.second;
          }
        }
        
        if ( m_params.open(dbH,"##Params",m_string_t,containerType,mode()).isSuccess() )    {
	  std::vector<std::string> fids;
          Token::OID_t oid(0, 0);
          DbString param_str;
          DbObject* ptr = &param_str;

          while (static_cast<uint64_t>(oid.second) <= m_params.size()) {
            auto result = m_params.ptr()->load(&ptr, m_string_t, oid, oid, true);
            if (!result.isSuccess() || !ptr) break;
            std::string dsc = param_str;
            size_t id1 = dsc.find("[NAME=");
            size_t id2 = dsc.find("[VALUE=");
            if ( id1 != std::string::npos && id2 != std::string::npos )  {
              size_t id11 = dsc.find(']', id1+6);
              size_t id22 = dsc.find(']', id2+7);
              if ( id11 != std::string::npos && id22 != std::string::npos )  {
                std::string n = dsc.substr(id1+6, id11-id1-6);
                std::string v = dsc.substr(id2+7, id22-id2-7);
                // ParamMap::value_type val(n, v);
                ATH_MSG_DEBUG("--->Reading Param:" << n << "=[" << v << ']');
                m_paramMap[n] = v;
                if (n == "FID") fids.emplace_back(std::move(v));
              }
            }
            ++oid.second;
          }
	  // We assume that the last FID is the true FID of the file...
	  ParamMap::const_iterator fidIt = m_paramMap.find("FID");
	  if ( fidIt != m_paramMap.end() ) {
	    const std::string& fid = (*fidIt).second;
	    for(size_t i=0; fids.size()>0 && i<fids.size()-1;++i)  {	    
	      char num[32];
	      ::sprintf(num, "FID.%d", static_cast<int>(i+1));
	      ATH_MSG_DEBUG("--->Redirect FID[" << i << "]: " << fids[i] << " to " << fid);
	      m_paramMap[num] = fid;
	    }
	  }
	}
        if ( mode()&pool::CREATE || mode()&pool::UPDATE)  {
          std::string par_val;
          if ( !param("FID", par_val).isSuccess() )  {
            if ( !addParam("FID", name()).isSuccess() )  {
              ATH_MSG_ERROR("Failed to write parameter FID=" << name());
            }
          }
          if ( !param("PFN", par_val).isSuccess() )  {
            if ( !addParam("PFN", m_logon).isSuccess() )  {
              ATH_MSG_ERROR("Failed to write parameter PFN=" << m_logon);
            }
          }
          if ( !param("POOL_VSN", par_val).isSuccess() )  {
            if ( !addParam("POOL_VSN", "1.1").isSuccess() )  {
              ATH_MSG_ERROR("Failed to write parameter POOL_VSN.");
            }
          }
        }
        DbDatabase dbd (this);
        return m_info->onOpen(dbd, mode());
      }
    }
    deletePtr(m_info);
    return Error;
  }
  return Success;
}

/// Re-open database with changing access permissions
DbStatus DbDatabaseObj::reopen(DbAccessMode mod) {
  if (mod == pool::READ || mod == pool::UPDATE )  {
    if ( mode() != mod )   {
      setMode(mod);
      DbStatus sc = (0==m_info) ? open() : m_info->reopen(mod);
      if ( sc.isSuccess() )   {
        for (const_iterator i=begin(); i != end(); ++i )  {
          (*i).second->cancelTransaction();
          (*i).second->setMode(mod);
        }
        return sc;
      }
      ATH_MSG_ERROR("Failed to reopen the database " << name()
          << " in mode " << accessMode(mod));
      return sc;
    }
    ATH_MSG_DEBUG("Database already open in the requested access mode.");
    for (const_iterator i=begin(); i != end(); ++i )  {
      (*i).second->setMode(mod);
    }
    return Success;
  }
  ATH_MSG_ERROR("A database can only be re-opened in UPDATE or READ mode!");
  return Error;
}

/// Close Database object
DbStatus DbDatabaseObj::close()  {
  DbStatus sc = retire();
  std::vector<DbContainerObj*> conts;
  for (const_iterator j=begin(); j != end(); ++j )  {
    DbContainerObj* curr = (*j).second;
    conts.push_back(curr);
  }
  for(std::vector<DbContainerObj*>::iterator i=conts.begin(); i != conts.end(); ++i)  {
    DbContainerObj* curr = (*i);
    if ( curr->isOpen() ) curr->close();
    this->remove(curr);
  }
  clearEntries();
  m_dom.remove(this);
  return sc;
}

/// Close Database object
DbStatus DbDatabaseObj::retire()  {
  ATH_MSG_INFO("Database being retired...");

  if (m_links.isValid()) m_links.close();
  if (m_shapes.isValid()) m_shapes.close();
  if (m_params.isValid()) m_params.close();
  for (const_iterator j=begin(); j != end(); ++j )  {
    DbContainerObj* curr = (*j).second;
    curr->retire();
  }
  DbStatus ret = Success;
  if ( m_info )    {
    ret = m_info->close(mode());
  }
  cleanup();
  m_fileAge = 0;
  return ret;
}

/// Retrieve the number of user parameters
int DbDatabaseObj::nParam() {
  if ( 0 == m_info )    {  // Re-open the database if it was retired
    open();
  }
  return 0 == m_info ? -1 : int(m_paramMap.size());
}

/// Add a persistent parameter to the file
DbStatus DbDatabaseObj::addParam(const std::string& nam, const std::string& val) {
  if ( !nam.empty() && !val.empty() ) {
    if ( 0 == m_info ) open();
    if ( m_info )  {
      ParamMap::const_iterator i = m_paramMap.find(nam);
      if ( i == m_paramMap.end() )  {
        DbString param_string("[NAME=" + nam + "][VALUE=" + val + ']');
        if ( !m_params.store(&param_string, m_string_t).isSuccess() )  {
          return Error;
        }
        m_paramMap.insert(ParamMap::value_type(nam, val));
        return Success;
      }
      return (*i).second == val ? Success : Error;
    }
  }
  return Error;
}

/// Retrieve existing parameter by name
DbStatus DbDatabaseObj::param(const std::string& nam, std::string& val)  {
  if ( 0 == m_info ) open();
  if ( m_info ) {
    ParamMap::const_iterator i = m_paramMap.find(nam);
    if ( i == m_paramMap.end() )  {
      return Error;
    }
    val = (*i).second;
    return Success;
  }
  return Error;
}

/// Retrieve all parameters
DbStatus DbDatabaseObj::params(Parameters& vals)   {
  vals.clear();
  if ( 0 == m_info ) open();
  if ( m_info ) {
    ParamMap::const_iterator i = m_paramMap.begin();
    for ( ; i != m_paramMap.end(); ++i )  {
      vals.push_back(*i);
    }
    return Success;
  }
  return Error;
}

/// Expand OID into a full Token, based on the Links table.
DbStatus DbDatabaseObj::getLink(const Token::OID_t& oid, Token* pTok)
{
   if ( 0 == m_info ) open();
   if ( 0 != m_info && 0 != pTok && oid.first >= 0 ) {
      pTok->oid() = oid;
      if( !(pTok->type() & DbToken::TOKEN_FULL_KEY) )  {
         if( typeid(*pTok) == typeid(DbToken) )  {
            DbToken* pdbTok = static_cast<DbToken*>(pTok);
	    pdbTok->setKey(DbToken::TOKEN_FULL_KEY);
         }
      }
      m_linkVec[ oid.first ]->set(pTok);
      return Success;
   }
   return Error;
}


std::string DbDatabaseObj::cntName(Token& token) {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    int lnk = m_indexMap[token.oid().first]; // Map link to index
    if ( lnk >= 0 )  {
      if ( lnk < int(m_linkVec.size()) )   {
	DbToken* link = m_linkVec[lnk];
        if ( link != 0 ) {
          if ( token.contID().empty() ) {
            token.setCont(link->contID());
          }
          return link->contID(); // in ##Links
        }
      }
    }
  }
  return "";
}

DbStatus DbDatabaseObj::read(const Token& token, ShapeH shape, void** object) 
{
   if( 0 == m_info ) open();
   if( 0 != m_info ) {
      Token::OID_t oid = token.oid();
      std::string containerName = token.contID();
      if( token.dbID() == name() ) {
         // Regular read operation, make sure we know the container name
         if( containerName.empty() ) {
            auto iter = m_indexMap.find(oid.first);
            if( iter != m_indexMap.end() ) {
               containerName = m_linkVec[ iter->second ]->contID();
            } else {
               if( unsigned(oid.first) < m_indexMap.size() ) {
                  // try a direct link table access
                  containerName = m_linkVec[ oid.first ]->contID();
               } else {
                  ATH_MSG_ERROR("OID1 not found in the index redirection map. Token=" << token.toString());
                  return Error;
               }
            }
         }
      }
      else {
         return Error;
      }

      DbContainer cntH( type() );
      const DbTypeInfo* typ_info = objectShape( token.classID() );
      DbDatabase dbd (this);

      if( cntH.open( dbd, containerName, typ_info, token.technology(), mode() ).isSuccess() )  {
         if ( typ_info && typ_info == shape ) {
            return cntH.load(object, shape, oid);
         }
         ATH_MSG_ERROR("Token ClassID " << token.classID().toString() 
              << " is different from requested Shape " << shape->shapeID().toString());
      }
   }
   return Error;
}


/// Allow access to all known containers
DbStatus DbDatabaseObj::containers(std::vector<const Token*>& conts,bool with_internals)  {
  conts.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->oid().second == INVALID )  {
        if ( with_internals || (*j)->contID()[0] != '#' )  { // in ##Links
          conts.push_back(*j);
        }
      }
    }
    return Success;
  }
  return Error;
}

/// Allow access to all known containers
DbStatus DbDatabaseObj::containers(std::vector<IDbContainer*>& conts,bool with_internals)  {
  conts.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
     for (iterator i=begin(); i != end(); ++i )    {
        DbContainerObj* c = (*i).second;
        if( c == m_links.ptr() || c == m_params.ptr() || c == m_shapes.ptr() )
           if( not with_internals) continue;
        if( c->info() ) conts.push_back( c->info() );
     }
     return Success;
  }
  return Error;
}


/// Access local container token (if container exists)
const Token* DbDatabaseObj::cntToken(const std::string& cntName)  {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->contID() == cntName )  { // in ##Links
        return (*j);
      }
    }
  }
  return 0;
}

/// Allow access to all known shapes used by the database
DbStatus DbDatabaseObj::shapes(std::vector<const DbTypeInfo*>& shaps)  {
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    shaps.clear();
    for(ShapeMap::iterator j=m_shapeMap.begin(); j != m_shapeMap.end(); ++j) {
      shaps.push_back((*j).second);
    }
    return Success;
  }
  return Error;
}

/// Allow access to all known associations between containers
DbStatus DbDatabaseObj::associations(std::vector<const Token*>& assocs) {
  assocs.clear();
  if ( 0 == m_info ) open();
  if ( 0 != m_info )    {
    LinkVector::const_iterator j=m_linkVec.begin();
    for(; j != m_linkVec.end(); ++j ) {
      if ( (*j)->oid().second != INVALID )  {
        assocs.push_back(*j);
      }
    }
    return Success;
  }
  return Error;
}

/// Execute Database Transaction action
DbStatus DbDatabaseObj::transAct(Transaction::Action action)  {
  bool upda = (0 != (mode()&pool::CREATE) || 0 != (mode()&pool::UPDATE));
  if ( 0 != m_info )  {
    DbStatus iret, status = Success;
    for (iterator i=begin(); i != end(); ++i )    {
      DbContainerObj* c = (*i).second;
      if( c == m_links.ptr() || c == m_params.ptr() || c == m_shapes.ptr() ) continue;
      iret = c->transAct( action );
      if ( !iret.isSuccess() ) {
         status = iret;
      }
    }
    if ( status.isSuccess() )  {
      status = m_params.transAct(action);
    }
    if ( status.isSuccess() )  {
      status = m_shapes.transAct(action);
    }
      if ( status.isSuccess() )  {
      status = m_links.transAct(action);
    }
    // now execute the action on the DB implementation
    if ( status.isSuccess() )  {
      status = m_info->transAct(action);
    }
    return status;
  }
  else if ( upda )  {
    ATH_MSG_ERROR("The database:" << name() << " was not opened properly. Commit failed.");
    return Error;
  }
  else  {
    // This means, that the database is retired.
    // Only READONLY databases may be retired.
    // Should be safe: Pending updates were checked on Re-open
    for (iterator i=begin(); i != end(); ++i )    {
      (*i).second->cancelTransaction();
    }
    return Success;
  }
}

/// Pass options to the implementation
DbStatus DbDatabaseObj::setOption(const DbOption& refOpt) {
  if ( 0 == m_info ) open();   // Re-open the database if it was retired
  return (0==m_info) ? Error : m_info->setOption(refOpt);
}

/// Pass options to the implementation
DbStatus DbDatabaseObj::getOption(DbOption& refOpt) {
  if ( 0 == m_info ) open();   // Re-open the database if it was retired
  return (0==m_info) ? Error : m_info->getOption(refOpt);
}

/// Update database age
void DbDatabaseObj::setAge(int value) {
  if ( 0 == m_info )  {
    m_fileAge = 0;
  }
  else if (m_fileAge >= 0) {
    switch ( value )  {
    case 0:
      m_fileAge = 0;
      return;
    case 1:
      ++m_fileAge;
      return;
    case -1:
      --m_fileAge;
      return;
    default:
      m_fileAge += value;
      return;
    }
  }
}
