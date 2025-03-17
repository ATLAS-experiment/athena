/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERSISTENTDATAMODELTPCNV_DATAHEADERCNV_P6_H
#define PERSISTENTDATAMODELTPCNV_DATAHEADERCNV_P6_H

/**
 *  @file DataHeaderCnv_p6.h
 *  @brief This file contains the class definition for DataHeaderCnv_p6 
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

class DataHeader;
class DataHeader_p6;
class DataHeaderForm_p6;
class Token;

/** @class DataHeaderCnv_p6
 *  @brief This class provides the converter to customize the saving of DataHeader_p6.
 **/
class DataHeaderCnv_p6 {
public:
   DataHeaderCnv_p6() {}

   DataHeader* createTransient(const DataHeader_p6* persObj, const DataHeaderForm_p6& form, const Token* dhToken) const;
   DataHeader_p6* createPersistent(const DataHeader* transObj, DataHeaderForm_p6& form) const;

   /// convert single DH element to persistent represenation
   void elemToPers(const DataHeaderElement* trans, DataHeader_p6* pers, DataHeaderForm_p6& form) const;

   /// restore single DH element from persistent represenation
   /// return false if p_idx is out of bounds for the Form
   bool persToElem(const DataHeader_p6* pers, unsigned p_idx, DataHeaderElement* trans,
                   const DataHeaderForm_p6& form ) const;

  /// insert DH self reference as one of the DHElements. Contains the stream Key
  void insertDHRef( DataHeader_p6* pers_dh, const std::string& key, const std::string& dh_tokstr,
                    DataHeaderForm_p6& dh_form ) const;

   void setSGAliasFiltering( bool doFiltering ) { m_SGAliasFiltering = doFiltering; }

   bool m_SGAliasFiltering { true };
};

#endif
