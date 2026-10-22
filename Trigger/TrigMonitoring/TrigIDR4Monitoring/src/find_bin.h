/** emacs: this is -*- c++ -*- 
 **
 **   @file    find_bin.h        
 **                   
 **   @author  sutt
 **   @date    Tue 30 Jun 2026 21:32:29 BST
 **
 **   $Id: find_bin.h, v0.0   Tue 30 Jun 2026 21:32:29 BST sutt $
 **
 **   Copyright (C) 2002-2076 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef IDTPM2_BINS_FIND_BIN
#define IDTPM2_BINS_FIND_BIN

#include <vector>
#include <stdexcept>


namespace IDTPM2::bins {

template<typename T,typename U=T> 
size_t find_bin( const std::vector<T>& v, const U& x ) {
  if ( v.size()<2) throw std::runtime_error("cannot define a bin range with fewer than two limits");		      
  if ( x>=v.back() ) return v.size()-2;
  if ( x<v.front() ) return 0;
  typename std::vector<T>::const_iterator itr = std::lower_bound( v.begin(), v.end(), x );
  return (itr-v.begin())-1;
}

}

#endif // IDTPM2_BINS_FIND_BIN

