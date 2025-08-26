/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DATAHEADERCNV_H
#define DATAHEADERCNV_H

/**
 *  @file DataHeaderCnv.h
 *  @brief This file contains the class definition for the DataHeaderCnv class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModelTPCnv/DataHeader_p6.h"
#include "PersistentDataModelTPCnv/DataHeaderCnv_p5.h"
#include "PersistentDataModelTPCnv/DataHeaderCnv_p6.h"

#include "GaudiKernel/IIncidentListener.h"

#include <map>

class  DataHeader_p5;
class  DataHeaderForm_p6;

/** @class DataHeaderCnv
 *  @brief This class provides the converter to customize the saving of DataHeader.
 **/
typedef  T_AthenaPoolCustomCnv<DataHeader, DataHeader_p6> DataHeaderCnvBase;
class DataHeaderCnv : public DataHeaderCnvBase, virtual public IIncidentListener
{
   friend class CnvFactory<DataHeaderCnv>;
public:
   DataHeaderCnv(ISvcLocator* svcloc);
   ~DataHeaderCnv();
   virtual StatusCode initialize() override;

   /// Extend base-class conversion methods
   virtual StatusCode updateRep(IOpaqueAddress* pAddress, DataObject* pObject) override;

   virtual StatusCode updateRepRefs(IOpaqueAddress* pAddress, DataObject* pObject) override;

   virtual StatusCode DataObjectToPool(IOpaqueAddress* pAddr, DataObject* pObj) override;

   std::unique_ptr<DataHeader_p5> poolReadObject_p5();
   std::unique_ptr<DataHeader_p6> poolReadObject_p6();

   DataHeader_p6* createPersistent(DataHeader* transObj, DataHeaderForm_p6*) ;
   virtual DataHeader* createTransient() override;

   [[deprecated("this converter uses createPersistent() with 2 arguments")]]
   virtual DataHeader_p6* createPersistent(DataHeader*) override { return nullptr; }

   /// Incident service handle listening for EndInputFile.
   virtual void handle(const Incident& incident) override;

   /// Delete cached DHForms for a given input file GUID
   void clearInputDHFormCache( const std::string& dbGuid );

   /// Remove DataHeaderElements with incomplete (dbID="") refs
   void         removeBadElements(DataHeader* dh);
  
protected:
   DataHeaderCnv_p6     m_tpOutConverter;
   DataHeaderCnv_p6     m_tpInConverter;
  
   /// cached shape of the DataHeaderForm_pN
   RootType             m_dhFormType;
   /// if true write only one DataHeaderForm at the end (stop, finalize, or WriteDataHeaderForms incident)
   /// the value is read from AthenaPoolCnvSvc OneDataHeaderForm property
   bool                 m_oneDHForm = true;
   /// Cache for new DHForms created when writing, indexed by ref or placement.
   struct placementComp{ bool operator() (const std::string& lhs, const std::string& rhs) const; };
   std::map<std::string,  std::unique_ptr<DataHeaderForm_p6>, placementComp >  m_persFormMap;
   /// DHForm cache indexed by the DHForm reference (from DataHeader) that was used to read it
   std::map<std::string,  std::unique_ptr<DataHeaderForm_p6> >  m_inputDHForms;

   /// How many DHForms for an input file are in the cache
   std::map<std::string,  unsigned>                             m_inDHFormCount;
   /// Max DHForms to cache per input file
   unsigned                                                     m_inDHFMapMaxsize{};

   std::map< std::string, std::string>  m_lastGoodDHFRef;

   /// for use when reading DataHeader_p5
   DataHeaderCnv_p5                     m_tpInConverter_p5;  
   std::unique_ptr<DataHeaderForm_p5>   m_dhInForm5;
   std::string                          m_dhFormMdx;

   /// cached values for use with SharedWriter server
   DataHeader_p6*                       m_sharedWriterCachedDH = nullptr;  // no ownership
   /// map of cached DHForms for DataHeader ID
   std::map< std::string, std::unique_ptr<DataHeaderForm_p6> >  m_sharedWriterCachedDHForm;
   std::string                          m_sharedWriterCachedDHKey;
   std::string                          m_sharedWriterCachedDHToken;
};

#endif
