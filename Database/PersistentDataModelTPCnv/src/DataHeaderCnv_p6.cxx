/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file DataHeaderCnv_p6.cxx
 *  @brief This file contains the implementation for the DataHeaderCnv_p6 class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModelTPCnv/DataHeader_p6.h"
#include "PersistentDataModelTPCnv/DataHeaderCnv_p6.h"
#include <climits>
#include <algorithm>

using FullElement  = DataHeader_p6::FullElement;

//______________________________________________________________________________
bool DataHeaderCnv_p6::persToElem( const DataHeader_p6* pers, unsigned p_idx,
                                    DataHeaderElement* trans, const DataHeaderForm_p6& form,
                                   bool sameForm ) const
{
   int obj_idx = pers->m_shortElements[p_idx];
   if( obj_idx == INT32_MIN ) return true;

   Token& token = trans->m_token;
   unsigned db_idx = 0;
   unsigned long long oid2 = 0;
   if( obj_idx >= 0 ) {
      db_idx = pers->m_commonDbIndex;
      oid2   = pers->m_commonOID2;
   } else {
      const FullElement &full_el = pers->m_fullElements[ -1 - obj_idx ];
      db_idx = full_el.dbIdx;
      obj_idx = full_el.objIdx;
      oid2 = full_el.oid2;
   }

   if( form.sizeDb() > db_idx ) {
      // Append DbGuid
      token.setDb(         form.getDbGuid( db_idx ) );
      token.setTechnology( form.getDbTech( db_idx ) );
   }
   if( form.sizeObj() > (size_t)obj_idx ) {
      token.setOid( Token::OID_t( form.getObjOid1(obj_idx), oid2) );

      if (!sameForm) {
         // If the form hasn't changed, these should all be the same ---
         // so don't need to copy them again.
         token.setCont(       form.getObjContainer( obj_idx ) );
         // Append ClassId
         token.setClassID(    form.getObjClassId(obj_idx) );
         // StoreGate
         trans->m_key = form.getObjKey( obj_idx );
         trans->m_alias = form.getObjAlias( obj_idx );
         trans->m_pClid = form.getObjType( obj_idx );
         trans->m_clids = form.getObjSymLinks( obj_idx );
         trans->m_hashes = form.getObjHashes( obj_idx );

         if (!std::ranges::is_sorted (trans->m_alias)) {
           // Should really be sorted, but just in case...
           std::ranges::sort (trans->m_alias);
           auto ret = std::ranges::unique (trans->m_alias);
           trans->m_alias.erase (ret.begin(), ret.end());
         }
      }
   }
   return form.sizeDb() > db_idx and form.sizeObj() > (size_t)obj_idx;
}

//______________________________________________________________________________
DataHeader* DataHeaderCnv_p6::createTransient( const DataHeader_p6* pers,
                                               const DataHeaderForm_p6& form,
                                               const Token* dhToken ) const
{
   DataHeader* trans = m_dhQueue.get();
   const unsigned int provSize = pers->m_provenanceSize;
   // DataHeaders with a self Reference at the end have the list longer by 1 element
   int selfRefSizeCorrection = (form.version() ==  DataHeaderForm_p6::DHverFormRef? 1 : 0);
   size_t nelts = pers->m_shortElements.size() - provSize;
   bool sameForm = false;
   if (!pers->dhFormToken().empty() &&
       pers->dhFormToken() == trans->dhFormToken() &&
       trans->m_inputDataHeader.size() == provSize &&
       trans->m_dataHeader.size() == nelts)
   {
     sameForm = true;
   }
   else {
     trans->setDhFormToken (pers->dhFormToken());
     trans->m_inputDataHeader.resize(provSize);
     trans->m_dataHeader.resize( nelts );
   }

   // convert all elements - transient vectors need to have the right sizes
   unsigned i = 0;
   for( auto& elem : trans->m_dataHeader ) {
      persToElem( pers, i++, &elem, form, sameForm );
      // Last entry is the self-reference, which is handled below.
      if (i == nelts - selfRefSizeCorrection) break;
   }
   for( auto& elem : trans->m_inputDataHeader ) {
      persToElem( pers, i++, &elem, form, sameForm );
   }
   // Add the self reference
   if (selfRefSizeCorrection > 0) {
     auto& elem = trans->m_dataHeader.back();
     // convert the self ref that was stored at the end of the element list
     persToElem( pers, i++, &elem, form, sameForm );

     if( elem.getToken()->contID().find("DataHeader") == std::string::npos ) {
       // discard wrong element
       trans->m_dataHeader.pop_back();
     }
   }
   trans->setStatus(DataHeader::Input);
   trans->setEvtRefTokenStr( dhToken->toString() );
   return trans;
}


//______________________________________________________________________________
void DataHeaderCnv_p6::elemToPers(const DataHeaderElement* trans,
                                  DataHeader_p6* pers,
                                  DataHeaderForm_p6& form) const
{
   // Translate PoolToken
   const Token *token =  trans->getToken();
   if( !token ) {
      // store marker for NO Token
      pers->m_shortElements.push_back( INT32_MIN );
   } else {
      // Database GUID & Technology
      DataHeaderForm_p6::DbRecord  db_rec( token->dbID(), token->technology() );
      unsigned db_idx = form.insertDb( db_rec );
      // StoreGate Type/Key & persistent Class GUID
      DataHeaderForm_p6::ObjRecord transObj( token->classID(), token->contID(), trans->m_key,
                                             trans->m_pClid, token->oid().first );
      unsigned obj_idx = form.insertObj(transObj, trans->m_alias, m_SGAliasFiltering,
                                        trans->m_clids, trans->m_hashes);
      unsigned long long oid2 = token->oid().second;

      // first element sets the common DB
      if( pers->m_shortElements.empty() ) {
         // first element - set the common DB and OID2 values
         pers->m_commonDbIndex = db_idx;
         pers->m_commonOID2 = oid2;
      }
      if( db_idx == pers->m_commonDbIndex && oid2 == pers->m_commonOID2 ) {
         // Can use short DH element
         pers->m_shortElements.push_back( obj_idx );
      } else {
         // need to use full DH element
         // store the index (as negative) to the full element in the short vector
         pers->m_shortElements.push_back( -1 - pers->m_fullElements.size() );
         pers->m_fullElements.push_back( FullElement(oid2, db_idx, obj_idx) );
      }
   }
}

//______________________________________________________________________________
DataHeader_p6* DataHeaderCnv_p6::createPersistent(const DataHeader* trans, DataHeaderForm_p6& form) const
{
   DataHeader_p6* pers = new DataHeader_p6();
   const unsigned int provSize = trans->m_inputDataHeader.size();
   pers->m_provenanceSize = provSize;

   pers->m_shortElements.reserve( provSize + trans->m_dataHeader.size() );
   form.resize(provSize + trans->m_dataHeader.size() + 1);
   for( const auto& transElem: trans->m_dataHeader ) {
      elemToPers( &transElem, pers, form );
   }
   for( const auto& transElem: trans->m_inputDataHeader ) {
      elemToPers( &transElem, pers, form );
   }
   return pers;
}

//______________________________________________________________________________
void DataHeaderCnv_p6::insertDHRef( DataHeader_p6* pers,
                                    const std::string& key, const std::string& tokstr,
                                    DataHeaderForm_p6& form ) const
{
   Token token;
   token.fromString( tokstr );
   DataHeaderElement tEle(ClassID_traits<DataHeader>::ID(), key, std::move(token));
   elemToPers( &tEle, pers, form );
}
