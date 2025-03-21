/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/** @file DataHeaderCnv.cxx
 *  @brief This file contains the implementation for the DataHeaderCnv class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "DataHeaderCnv.h"

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"

#include "PersistentDataModelTPCnv/DataHeaderCnv_p3.h"
#include "PersistentDataModelTPCnv/DataHeaderCnv_p4.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include <stdexcept>
#include <format>


static const pool::Guid DHForm_p6_Guid("7BE56CEF-C866-4BEE-9348-A5F34B5F1DAD");

// cppcheck-suppress uninitMemberVar
DataHeaderCnv::DataHeaderCnv(ISvcLocator* svcloc) :
      DataHeaderCnvBase(svcloc, "DataHeaderCnv")
{
   m_classDesc = RootType( "DataHeader_p6" );
}

//______________________________________________________________________________
DataHeaderCnv::~DataHeaderCnv()
{
   // Remove itself from the IncidentSvc - if it is still around
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", "DataHeaderCnv");
   if( incSvc.retrieve().isSuccess() ) {
      incSvc->removeListener(this, IncidentType::EndInputFile);
   }
}
//______________________________________________________________________________
StatusCode DataHeaderCnv::initialize()
{
   // Read properties from the ConversionSvc
   m_inDHFMapMaxsize = 100;   // default DHForm cache size
   bool doFilterDHAliases = true;
   SmartIF<IProperty> cnvSvc{service("AthenaPoolCnvSvc")};
   if( cnvSvc ) {
      IntegerProperty sizeProp("maxDHFormCacheSize", m_inDHFMapMaxsize);
      if( cnvSvc->getProperty(&sizeProp).isSuccess() ) {
         m_inDHFMapMaxsize = sizeProp.value();
      }
      BooleanProperty aliasFilterProp("doFilterDHAliases", doFilterDHAliases);
      if( cnvSvc->getProperty(&aliasFilterProp).isSuccess() ) {
         doFilterDHAliases = aliasFilterProp.value();
      }
      BooleanProperty oneDHForm("OneDataHeaderForm", m_oneDHForm);
      if( cnvSvc->getProperty(&oneDHForm).isSuccess() ) {
         m_oneDHForm = oneDHForm.value();
      }
   }
   ATH_MSG_VERBOSE("Using DHForm cache size: " << m_inDHFMapMaxsize);
   if( doFilterDHAliases ) {
      ATH_MSG_VERBOSE("Will filter SG Aux aliases in DataHeader");
   } else {
      ATH_MSG_VERBOSE("Will NOT filter SG Aux aliases in DataHeader");
   }
   m_tpOutConverter.setSGAliasFiltering( doFilterDHAliases );

   // Get IncidentSvc
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", "DataHeaderCnv");
   ATH_CHECK( incSvc.retrieve() );
   // Listen to EndInputFile incidents to clear old DataHeaderForms from the cache
   incSvc->addListener(this, IncidentType::EndInputFile, 0);
   incSvc->addListener(this, "PreFork", 0);
   // Listen to WriteDataHeaderForms incidents to flush DHForms after 
   incSvc->addListener(this, "WriteDataHeaderForms", 0);
   return DataHeaderCnvBase::initialize();
}


void DataHeaderCnv::handle(const Incident& incident)
{
   if( incident.type() == "PreFork" ) {
      m_persFormMap.clear();
   }
   if( incident.type() == IncidentType::EndInputFile ) {
      // remove cached DHForms that came from the file that is now being closed
      const std::string& guid = static_cast<const FileIncident&>(incident).fileGuid();
      clearInputDHFormCache( guid );
   }
   if( incident.type() == "WriteDataHeaderForms" ) {
      // Issued from OutputStream MetaData write() or directly called from AthenaPoolCnvSvc
      const std::string& fileName = static_cast<const FileIncident*>(&incident)->fileName();
      // Write out all the cached DHForms before closing a given output stream
      std::vector<std::string> toWrite;
      auto pos = fileName.find("[OutputCollection=MetaDataHdr]");
      bool metaDataCommit = (pos != std::string::npos);
      std::string justFileName = fileName.substr(0, pos);
      ATH_MSG_DEBUG("Handling WriteDataHeaderForms incident for stream: " << fileName);
      // collect DHForms related to the stream
      for( const auto& elem : m_persFormMap ) {
         const std::string& placementStr = elem.first;
         Placement formPlacement;
         formPlacement.fromString( placementStr );
         ATH_MSG_VERBOSE("DatHeaderForm key in map=" << placementStr);
         if( formPlacement.fileName() == justFileName or fileName == "*" ) {
            // write only the Forms that correspond to the commit type (MetaData or others)
             if( (placementStr.find("[CONT=MetaData") != std::string::npos) == metaDataCommit ) {
               toWrite.push_back( placementStr );
            }
         }
      }
      // write the DHForms one by one (the only mode supported by RNTuple)
      for( std::size_t n = 0; const std::string& placementStr : toWrite ) {
         Placement formPlacement;
         formPlacement.fromString( placementStr );
         auto form_ptr = m_persFormMap[placementStr].get();
         if( form_ptr->isModified() ) {
            static const RootType dhFormType( typeid( *form_ptr ) );
            ATH_MSG_DEBUG("Writing DatHeaderForm " << placementStr);
            Token* form_token = m_athenaPoolCnvSvc->registerForWrite(&formPlacement, form_ptr, dhFormType);
            if( !form_token ) {
               std::string errmsg = std::format("Failed to write {} {}", dhFormType.Name(), placementStr);
               ATH_MSG_FATAL( errmsg );
               throw GaudiException(errmsg, "DataHeaderCnv::WriteDataHeaderForms", StatusCode::FAILURE);
            }
            ATH_MSG_DEBUG("Wrote DatHeaderForm, placeemnt was " << placementStr << "  token=" << form_token->toString());
            form_token->release(); form_token = nullptr;
            bool doCommit = (++n == toWrite.size());
            const std::string connection = ( fileName!="*"? fileName : formPlacement.fileName() );
            if( !m_athenaPoolCnvSvc->commitOutput(connection, doCommit).isSuccess() ) {
               throw GaudiException("WriteDataHeaderForms failed",  "DataHeaderCnv::WriteDataHeaderForms", StatusCode::FAILURE);
            }
         }
         m_persFormMap.erase( placementStr );
      }
   }   
}


void DataHeaderCnv::clearInputDHFormCache( const std::string& dbGuid )
{
   auto iter = m_inputDHForms.begin();
   while( iter != m_inputDHForms.end() ) {
      size_t dbpos = iter->first.find("[DB=");
      if( dbpos != std::string::npos && iter->first.compare(dbpos+4, dbpos+36, dbGuid) == 0 ) {
         iter = m_inputDHForms.erase( iter );
      } else {
         iter++;
      }
   }
   m_inDHFormCount[ dbGuid ] = 0;
}


std::string getValForKey(const std::string& str, const std::string& key)
{
   auto start = str.find(key);
   if( start == std::string::npos )
      return "";
   start += key.size();
   auto end = str.find("]", start);
   return str.substr(start, end-start);
}

inline
std::string getSWNFromStr(const std::string& str) { return getValForKey(str, "[SWN="); }
inline
std::string getFILEFromStr(const std::string& str) { return getValForKey(str, "[FILE="); }

std::string makeKeyValStr(const std::string &key, const std::string & val) {
   return std::format("[{}={}]", key, val);
}


// Sort DHForms according to the SW client number so they can be written out in the correct order
inline
std::string storeSWNInStr(const std::string& str) { return std::format("[SWN={}]", str); }

bool DataHeaderCnv::placementComp::operator() (const std::string& lhs, const std::string& rhs) const
{
   std::string lval = getValForKey(lhs,"[FILE=") + getValForKey(lhs,"[CONT=");
   std::string rval = getValForKey(rhs,"[FILE=") + getValForKey(rhs,"[CONT=");
   if( lval != rval ) return lval < rval;

   lval = getSWNFromStr(lhs);
   rval = getSWNFromStr(rhs);
   if( lval.empty() or rval.empty() ) return false;
   return std::stoul( lval ) < std::stoul( rval );
}

//______________________________________________________________________________
/* For SharedWriter
   Update the DataHeader reference to the DataHeaderForm with new fileName, DataHeader SG Key
   and SharedWriter client number. The client number will be used by DH to find the right DHForm.
   The forms are written out in the order of the client numbers, so the Form object OID 
   corresponds to the client number
     pObject contains pointer to DataHeader_p6
     pAddress: par[0] contains DH token, par[1] contains placement + DH processingTag as KEY
*/
StatusCode DataHeaderCnv::updateRep(IOpaqueAddress* pAddress, DataObject* pObject)
{
   auto dataHeader = reinterpret_cast<DataHeader_p6*>( pObject );
   const std::string dhRef = pAddress->par()[0];
   const std::string dhPlacementStr = pAddress->par()[1];
   const std::string dhFile = getFILEFromStr( dhPlacementStr );
   Placement dhFormPlacement;
   dhFormPlacement.fromString( dataHeader->dhFormToken() );
   dhFormPlacement.setFileName( dhFile );
   const std::string clientN = getSWNFromStr( dhPlacementStr );
   if( !clientN.empty() ) {
      // Client num in the DHForm ref means it's OneDataHeaderForm mode
      std::string dhFormNewRef = dhFormPlacement.toString() + storeSWNInStr( clientN );
      ATH_MSG_DEBUG("Setting DH formRef to: " << dhFormNewRef);
      dataHeader->setDhFormToken( dhFormNewRef );
   } else {
      /* DH is received by the SharedWriter first and needs the DHForm object both for
         reference (Token) and to add DHElem with the ref to itself.
         These updates will be handled by updateRepRefs() - cache the DH info so it can
         by used later by updateRepRefs
      */
       if( m_sharedWriterCachedDH ) {
         ATH_MSG_ERROR( "updateRep called but the previous DataHeader was not yet processed."
                         << " cached DH Key=" << m_sharedWriterCachedDHKey
                         << " cached DH Ref=" << m_sharedWriterCachedDHToken );
         return StatusCode::FAILURE;
      }
      // remember this DH and finish processing in updateRepRefs()
      m_sharedWriterCachedDH = dataHeader;
      m_sharedWriterCachedDHToken = dhRef;
      std::size_t tagBeg = dhPlacementStr.find("[KEY=") + 5;
      std::size_t tagSize = dhPlacementStr.find(']', tagBeg) - tagBeg;
      m_sharedWriterCachedDHKey = dhPlacementStr.substr( tagBeg, tagSize );
   }
   return StatusCode::SUCCESS;
}

//______________________________________________________________________________
StatusCode DataHeaderCnv::updateRepRefs(IOpaqueAddress* pAddress, DataObject* pObject)
{
   if( m_sharedWriterCachedDH ) {
      /* Attach a DHForm to the previous DataHeader (in SharedWriter server mode)
         Finish writing of the DataHeader by attaching the DHForm to it and by adding
         the self reference. DHForm is passed as pObject and is cached until a new one
         arrives. This method is called for each event after the DH is received.
         pObject is null if there is no new DHForm for this event - in this case the old
         one is used
      */
      std::string dhid = pAddress->par()[1];
      if( pObject ) {
         this->setToken( pAddress->par()[0] );
         if( !compareClassGuid( DHForm_p6_Guid ) ) {
            ATH_MSG_ERROR( "updateRepRefs called without DataHeaderForm" );
            return StatusCode::FAILURE;
         }
         // replace the old DHForm
         // will keep this DHForm  until a new one arrives 
         m_sharedWriterCachedDHForm[dhid].reset( reinterpret_cast<DataHeaderForm_p6*>( pObject ) );
         m_sharedWriterCachedDHForm[dhid]->setToken( pAddress->par()[0] );
      }
      if( m_sharedWriterCachedDHForm.find(dhid) == m_sharedWriterCachedDHForm.end() ) {
         ATH_MSG_ERROR( "updateRepRefs: missing DataHeaderForm for DH ID=" << dhid );
         return StatusCode::FAILURE;
      }
      // update the cached DataHeader (can be done until a commit is called)
      m_sharedWriterCachedDH->setDhFormToken( m_sharedWriterCachedDHForm[dhid]->getToken() );
      m_tpOutConverter.insertDHRef( m_sharedWriterCachedDH, m_sharedWriterCachedDHKey,
                                    m_sharedWriterCachedDHToken, *m_sharedWriterCachedDHForm[dhid] );
      // this DataHeader object is now fully processed, so forget it
      m_sharedWriterCachedDH = nullptr;
   }
   else {
      // In OneDataHeaderForm mode - keep the DHForms from all clients and write them out at the end
      m_persFormMap[ pAddress->par()[1] ].reset( reinterpret_cast<DataHeaderForm_p6*>( pObject ) );
   }
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
StatusCode DataHeaderCnv::DataObjectToPool(IOpaqueAddress* pAddr, DataObject* pObj)
{
   DataHeader* obj = nullptr;
   if (!SG::fromStorable(pObj, obj) || obj == nullptr) {
      ATH_MSG_ERROR( "Failed to cast DataHeader to transient type" );
      return(StatusCode::FAILURE);
   }
   // DHForm placement:
   Placement dhf_placement = setPlacementWithType("DataHeaderForm", pObj->name(), *pAddr->par());
   std::string form_placement_str = dhf_placement.toString();
   // Find or create Form
   std::unique_ptr<DataHeaderForm_p6>& dhForm = m_persFormMap[form_placement_str];
   if (dhForm == nullptr) {
      // create new DHF for this file.  Every new file/container should get its own DHForm
      dhForm = std::make_unique<DataHeaderForm_p6>();
      dhForm->setProcessTag( obj->getProcessTag() );
   }

   // DH placement first:
   Placement dh_placement = setPlacementWithType("DataHeader", pObj->name(), *pAddr->par());
   // remember the connection string, it may get changed in registerForWrite by SharedWriter
   const std::string connection = dh_placement.fileName();
   // Create persistent DH and update Form
   DataHeader_p6* persObj = nullptr;
   try {
      persObj = createPersistent(obj, dhForm.get());
   } catch (std::exception &e) {
      ATH_MSG_FATAL("Failed to convert DataHeader to persistent type: " << e.what());
      return(StatusCode::FAILURE);
   }
   // Queue the DH for write
   std::unique_ptr<Token> dh_token (m_athenaPoolCnvSvc->registerForWrite(&dh_placement, persObj, m_classDesc));
   if (dh_token == nullptr) {
      ATH_MSG_FATAL("Failed to write DataHeader");
      return(StatusCode::FAILURE);
   }
   keepPoolObj(persObj, connection);
   // insert self reference - this updates DH and can modify the Form
   m_tpOutConverter.insertDHRef(persObj, obj->getProcessTag(), dh_token->toString(), *dhForm);

   if( !m_oneDHForm  ) {
      if( dhForm->isModified() ) {
         // Write DHForm if in legacy mode in case it was modified (or new)
         dhForm->setVersion( DataHeaderForm_p6::DHverFormRef );
         static const RootType dhFormType(typeid(*dhForm));
         Token* dhf_token = m_athenaPoolCnvSvc->registerForWrite(&dhf_placement, dhForm.get(), dhFormType);
         if (dhf_token  == nullptr) {
            ATH_MSG_FATAL("Failed to write " << dhFormType.Name());
            return(StatusCode::FAILURE);
         }
         if (dhf_token->technology() != 0) { // Only store DHF token if technology allows it to be appended to DH
            dhForm->setToken(dhf_token->toString());
         } else { // Otherwise keep string empty to cause DHF reading via DH token
            ATH_MSG_DEBUG("Technology does not support setting DHF token for: " << dh_token->toString());
            dhForm->setToken("");
         }
         ATH_MSG_DEBUG("wrote new DHForm with " << dhForm->sizeObj() << " SG object data");
         dhf_token->release(); dhf_token = nullptr;
         dhForm->clearModified();
      }
      // update the Ref to the DHForm 
      persObj->setDhFormToken( dhForm->getToken() );
   }
   else {
      // use the DHForm placement as a Ref, as the DHForm will be written out at the end only
      // but add DB ID to the placement str in case the filename is not unique between jobs
      if( dhForm->getToken().empty() ) {
         form_placement_str += std::format("[DB={}]", dh_token->dbID().toString());
         dhForm->setToken( form_placement_str );
      } else {
         form_placement_str = dhForm->getToken();
      }
      // remove the [FILE= part as the DBID will be enough for reading back
      auto b = form_placement_str.find("[FILE=");
      auto e = form_placement_str.find("]", b);
      form_placement_str.erase(b, e-b+1);
      // store the modified DHForm placement in place of the Ref in the DH
      persObj->setDhFormToken( form_placement_str );
   }

   const coral::AttributeList* list = obj->getAttributeList();
   if (list != nullptr) {
      obj->setEvtRefTokenStr(dh_token->toString());
      Placement attr_placement = this->setPlacementWithType("AttributeList", "Token", *pAddr->par());
      const Token* ref_token = m_athenaPoolCnvSvc->registerForWrite(&attr_placement,
	      obj->getEvtRefTokenStr().c_str(),
	      RootType("Token"));
      delete ref_token; ref_token = nullptr;
      for (coral::AttributeList::const_iterator iter = list->begin(), last = list->end(); iter != last; ++iter) {
         attr_placement = this->setPlacementWithType("AttributeList", (*iter).specification().name(), *pAddr->par());
         const Token* attr_token = m_athenaPoolCnvSvc->registerForWrite(&attr_placement,
	         (*iter).addressOfData(),
                 RootType((*iter).specification().type()) );
         delete attr_token; attr_token = nullptr;
      }
   }
   TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(pAddr);
   if (tokAddr != nullptr) {
      tokAddr->setToken(std::move(dh_token));
   } else {
      return(StatusCode::FAILURE);
   }
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
std::unique_ptr<DataHeader_p5> DataHeaderCnv::poolReadObject_p5()
{
   void* voidPtr1 = nullptr;
   m_athenaPoolCnvSvc->setObjPtr(voidPtr1, m_i_poolToken);
   if (voidPtr1 == nullptr) {
      throw std::runtime_error("Could not get object for token = " + m_i_poolToken->toString());
   }
   std::unique_ptr<DataHeader_p5> header( reinterpret_cast<DataHeader_p5*>(voidPtr1) );

   void* voidPtr2 = nullptr;
   Token mapToken;
   mapToken.fromString( header->dhFormToken() );
   mapToken.setAuxString( m_i_poolToken->auxString() );  // set PersSvc context
   if (mapToken.classID() != Guid::null()) {
      if( header->dhFormMdx() != m_dhFormMdx ) {
         m_athenaPoolCnvSvc->setObjPtr(voidPtr2, &mapToken);
         if (voidPtr2 == nullptr) {
            throw std::runtime_error("Could not get object for token = " + mapToken.toString());
         }
         m_dhInForm5.reset( reinterpret_cast<DataHeaderForm_p5*>(voidPtr2) );
         m_dhFormMdx = header->dhFormMdx();
      }
   }
   return header;
}

//______________________________________________________________________________
// Read the persistent rep of DataHeader
// Also read DataHeaderForm if not yet cached
std::unique_ptr<DataHeader_p6> DataHeaderCnv::poolReadObject_p6()
{
   void* voidPtr1 = nullptr;
   std::string error_message;
   try {
      m_athenaPoolCnvSvc->setObjPtr(voidPtr1, m_i_poolToken);
   } catch(const std::exception& err) {
      voidPtr1 = nullptr;
      error_message = err.what();
   }
   if (voidPtr1 == nullptr) {
      throw std::runtime_error("Could not get object for token = " + m_i_poolToken->toString() + ", " + error_message);
   }
   std::unique_ptr<DataHeader_p6> header( reinterpret_cast<DataHeader_p6*>(voidPtr1) );

   std::string dhFormToken =  header->dhFormToken();
   Token formToken;
   if( !dhFormToken.empty() and dhFormToken.find("[OID=") == std::string::npos) {
      // OneForm format of Form Ref (without OID)
      // we need to reconstruct the real Ref before checking in the Ref cache
      Placement dhf_placement;
      dhf_placement.fromString( dhFormToken );
      formToken.setDb( m_i_poolToken->dbID() );
      formToken.setCont( dhf_placement.containerName() );
      formToken.setTechnology( dhf_placement.technology() );
      formToken.setAuxString( dhf_placement.auxString() );
      formToken.setClassID( DHForm_p6_Guid );
      std::int64_t oid2 =  m_i_poolToken->oid().second;
      oid2 >>= 32; oid2 <<= 32;
      std::string swn = getSWNFromStr( dhFormToken );
      // add the row number from the SHForm Ref
      if( !swn.empty() ) oid2 += std::stoul( swn ) - 1;
      formToken.setOid( {0,oid2} );
      ATH_MSG_DEBUG("Constructed DHForm Ref=" << formToken.toString() << " from " << dhFormToken );
      dhFormToken = formToken.toString();
      header->setDhFormToken( dhFormToken );
   }
   // see if the DataHeaderForm is already cached
   if( dhFormToken.empty() || m_inputDHForms.find(dhFormToken) == m_inputDHForms.end() ) {
      // no cached DHForm
      size_t dbpos = dhFormToken.find("[DB=");
      if( dbpos != std::string::npos ) {
         const std::string dbGuid = dhFormToken.substr(dbpos+4, dbpos+36);
         if( ++m_inDHFormCount[dbGuid] > m_inDHFMapMaxsize ) {
            // prevent the input DHFMap from growing too large
            clearInputDHFormCache( dbGuid );
            m_inDHFormCount[dbGuid] = 1;
         }
      }
      // we need to read a new DHF
      void* voidPtr2 = nullptr;
      if( dhFormToken.empty() ) {
         // Some technologies can't set DHF token, use DH token with new CLID.
         m_i_poolToken->setData(&formToken);
         formToken.setClassID( DHForm_p6_Guid );
      } else {
         formToken.fromString( dhFormToken );
         formToken.setAuxString( m_i_poolToken->auxString() );  // set PersSvc context
      }
      if (formToken.classID() != Guid::null()) {
         try {
            m_athenaPoolCnvSvc->setObjPtr(voidPtr2, &formToken);
         } catch(const std::exception& err) {
            voidPtr2 = nullptr;
            error_message = err.what();
         }
         if (voidPtr2 == nullptr) {
            // if there is no good lastGoodDHFRef then try reading DataHeaderForm Token from the first DataHeader
            if( m_lastGoodDHFRef.find(m_i_poolToken->contID()) == m_lastGoodDHFRef.end() ) {
               Token firstToken;
               m_i_poolToken->setData(&firstToken);
               firstToken.setOid(Token::OID_t(firstToken.oid().first, 0));
               void* firstPtr1 = nullptr;
               try {
                  m_athenaPoolCnvSvc->setObjPtr(firstPtr1, &firstToken);
               } catch(const std::exception& err) {
                  firstPtr1 = nullptr;
                  error_message = err.what();
               }
               if (firstPtr1 == nullptr) throw std::runtime_error("Could not get first DataHeader for token = " + firstToken.toString() + ", " + error_message);

               // Get DataHeaderForm Token from the first DataHeader
               std::unique_ptr<DataHeader_p6> firstHeader( reinterpret_cast<DataHeader_p6*>(firstPtr1) );
               dhFormToken = firstHeader->dhFormToken();

               // Read DataHeaderForm and insert it to the cache
               formToken.fromString( dhFormToken );
               formToken.setAuxString( m_i_poolToken->auxString() );  // set PersSvc context
               try {
                  m_athenaPoolCnvSvc->setObjPtr(voidPtr2, &formToken);
               } catch(const std::exception& err) {
                  voidPtr2 = nullptr;
                  error_message = err.what();
               }
               if (voidPtr2 == nullptr) throw std::runtime_error("Could not get DataHeaderForm for token = " + formToken.toString() + ", " + error_message);
               m_lastGoodDHFRef[m_i_poolToken->contID()] = dhFormToken;
               m_inputDHForms[dhFormToken].reset( reinterpret_cast<DataHeaderForm_p6*>(voidPtr2) );
               ATH_MSG_WARNING("DataHeaderForm read exception: " << error_message << " - reusing the last good DHForm");
            } else {
               // try to reuse the last good DHForm Ref (object is already cached)
               ATH_MSG_WARNING("DataHeaderForm read exception: " << error_message << " - reusing the last good DHForm");
            }
            header->setDhFormToken(m_lastGoodDHFRef[m_i_poolToken->contID()]);
            return header;
         }
         if (voidPtr2 == nullptr) {
            throw std::runtime_error("Could not get object for token = " + formToken.toString());
         }
         m_lastGoodDHFRef[m_i_poolToken->contID()] = dhFormToken;
      }
      m_inputDHForms[dhFormToken].reset( reinterpret_cast<DataHeaderForm_p6*>(voidPtr2) );
   }
   return header;
}

//______________________________________________________________________________
DataHeader_p6* DataHeaderCnv::createPersistent(DataHeader* transObj, DataHeaderForm_p6* dh_form)
{
   return m_tpOutConverter.createPersistent( transObj, *dh_form );
}

//______________________________________________________________________________
void DataHeaderCnv::removeBadElements(DataHeader* dh)
{
   auto iter = dh->m_inputDataHeader.begin();
   while( iter != dh->m_inputDataHeader.end() ) {
      if( iter->getToken()->dbID() == Guid::null() ) {
         ATH_MSG_WARNING("Removed incomplete Input Element");
         iter = dh->m_inputDataHeader.erase(iter);
      } else {
         ++iter;
      }
   }
}

//______________________________________________________________________________
DataHeader* DataHeaderCnv::createTransient() {
   if (this->m_i_poolToken == nullptr) {
      return(nullptr);
   }
   if (this->m_i_poolToken->technology() == 0x00001000) { // Artificial ByteStream DataHeader Token
      DataHeader* dh = new DataHeader();
      std::string bestPfn, fileType;
      m_athenaPoolCnvSvc->getPoolSvc()->lookupBestPfn(this->m_i_poolToken->dbID().toString(), bestPfn, fileType);
      DataHeaderElement dhe(ClassID_traits<DataHeader>::ID(), bestPfn, Token(this->m_i_poolToken));
      dh->insert(dhe);
      return(dh);
   }
   static const pool::Guid p6_guid("4DDBD295-EFCE-472A-9EC8-15CD35A9EB8D");
   static const pool::Guid p5_guid("D82968A1-CF91-4320-B2DD-E0F739CBC7E6");
   static const pool::Guid p4_guid("9630EB7B-CCD7-47D9-A39B-CBBF4133CDF2");
   static const pool::Guid p3_guid("EC1318F0-8E28-45F8-9A2D-2597C1CC87A6");
   try {
      if( compareClassGuid( p6_guid ) ) {
         std::unique_ptr<DataHeader_p6> header( poolReadObject_p6() );
         auto dhForm = m_inputDHForms[ header->dhFormToken() ].get();
         auto dh = m_tpInConverter.createTransient( header.get(), *dhForm, m_i_poolToken );
         removeBadElements(dh);
         // To dump the DataHeader uncomment below
         // std::ostringstream ss;  dh->dump(ss); std::cout << ss.str() << std::endl;
         return dh;
      } else if (this->compareClassGuid( p5_guid )) {
         std::unique_ptr<DataHeader_p5> obj_p5( poolReadObject_p5() );
         return m_tpInConverter_p5.createTransient( *obj_p5, *m_dhInForm5 ).release();
      } else if (this->compareClassGuid( p4_guid )) {
         std::unique_ptr<DataHeader_p4> obj_p4(this->poolReadObject<DataHeader_p4>());
         DataHeaderCnv_p4 tPconverter_p4;
         return(tPconverter_p4.createTransient(obj_p4.get()));
      } else if (this->compareClassGuid( p3_guid )) {
         std::unique_ptr<DataHeader_p3> obj_p3(this->poolReadObject<DataHeader_p3>());
         DataHeaderCnv_p3 tPconverter_p3;
         return(tPconverter_p3.createTransient(obj_p3.get()));
      }
   } catch (std::exception &e) {
      // upgrade all exceptions to runtime_error (who needs that? MN)
      std::string error = e.what();
      throw std::runtime_error(error);
   }
   return(nullptr);
}
