// emacs: this is -*- c++ -*-
//
//   @file   IRegSelTool.h        
// 
//           Generic lookup table interface class for all teh different types 
//           of Region Selector lookup tables
//           NB: it would make sense for the IRegSelTool interface to inherit 
//               from this also, so that we guarantee the same interface 
//               for the tables and corresponding tools
//           
//   @author Mark Sutton 
//
// Copyright (C) 2012-2020 CERN for the benefit of the ATLAS collaboration


#ifndef IREGIONSELECTOR_IREGSELLUT_H
#define IREGIONSELECTOR_IREGSELLUT_H

#include "Identifier/IdentifierHash.h"
#include <vector>
#include <stdint.h>
#include <algorithm>

#include "IRegionSelector/IRoiDescriptor.h"

class IRegSelLUT {

public: 

  /// IdentifierHash methods

    
  //! HashIDList interface declaration. %return  list of unique IdentifierHash

  /*!
    \param IRoiDescriptor \c \b roi, the IRoiDescriptor for the roi, all enabled elements in the roi are found.
    \return std::vector<IdentifierHash> which is a list of non-repeated  %Identifier %Hash numbers.
  */
  virtual void HashIDList( const IRoiDescriptor& roi, std::vector<IdentifierHash>& idlist ) const = 0;
  

  //! HashIDList interface declaration. %return list of non-repeated IdentifierHash
  /*!
    \param long           \c \b layer, long int to decide which layer within the detector.
    \param IRoiDescriptor \c \b roi, the IRoiDescriptor for the roi, all enabled elements in the roi are found.
    \return std::vector<IdentifierHash> which is a list of non-repeated Offline %Identifier %Hash numbers.
  */
  virtual void HashIDList( long layer, const IRoiDescriptor& roi, std::vector<IdentifierHash>& idlist ) const = 0; 
   
   
  /// Rob identifier methods methods

  //! ROBIDList interface declaration. This interface can be used by the ID subdetectors. %A list of non-repeated ROBIDs (uint_32_t) is returned by a reference.
  /*!
    \param IRoiDescriptor \c \b the IRoiDescriptor for the roi, all enabled elements in the roi are found. 
    \return std::vector<uint32_t> which is a list of non-repeated ROBID numbers.
  */

  virtual void ROBIDList( const IRoiDescriptor& roi, std::vector<uint32_t>& roblist ) const = 0; 


  //! ROBIDList interface declaration. This interface can be used by the ID subdetectors. %A list of non-repeated ROBIDs (uint_32_t) is returned by a reference.
  /*!
    \param long   \c \b layer, long int to decide which layer within the detector.
    \param IRoiDescriptor \c \b the IRoiDescriptor for the roi, all enabled elements in the roi are found. 
    \return std::vector<uint32_t> which is a list of non-repeated ROBID numbers.
  */
  
  virtual void ROBIDList( long layer, const IRoiDescriptor& roi, std::vector<uint32_t>& roblist ) const = 0;   
   
  virtual ~IRegSelLUT(){};


protected:

  /// useful for removing duplicates if required ...
  template<typename T>
  static void removeDuplicates(std::vector<T>& vec) {
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
  }

  
  template<typename C, typename T> 
  using handler = void (C::*)(const IRoiDescriptor& , std::vector<T>&  ) const; 

  template<typename C, typename T>
  void IDList( const IRoiDescriptor& roi, std::vector<T>& idlist, handler<C,T> lister )  const {
        
    if ( roi.composite() ) {
      idlist.clear();
      for ( unsigned iroi=roi.size() ; iroi-- ;  )  IDList<C>( *(roi.at(iroi)), idlist, lister );
      if ( roi.size()>1 ) IRegSelLUT::removeDuplicates( idlist );
      return;
    }
    
    (dynamic_cast<const C*>(this)->*lister)( roi, idlist ); 
    
  }


  template<typename C, typename T> 
  using handler_layer = void (C::*)(long layer, const IRoiDescriptor& , std::vector<T>&  ) const; 

  template<typename C, typename T>
  void IDList_layer( long layer, const IRoiDescriptor& roi, std::vector<T>& idlist, handler_layer<C,T> lister )  const {
    
    if ( roi.composite() ) {
      idlist.clear();
      for ( unsigned iroi=roi.size() ; iroi-- ;  )  IDList_layer<C>( layer, *(roi.at(iroi)), idlist, lister );
      if ( roi.size()>1 ) IRegSelLUT::removeDuplicates( idlist );
      return;
    }
    
    (dynamic_cast<const C*>(this)->*lister)( layer, roi, idlist ); 
    
  }


  
};



#endif //  IREGIONSELECTOR_IREGSELLUT_H
