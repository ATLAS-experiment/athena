/** emacs: this is -*- c++ -*- 
 **
 **   @file    datetimestamp.h
 **            
 **            get the bnuild time of the requested object / function 
 **            using stat on the installed module that it is in
 **
 **   @author  sutt
 **   @date    Sat Oct 24 06:12:45 PM CEST 2026
 **
 **   $Id: Utils.h, v0.0   Sat Oct 24 06:12:45 PM CEST 2026 sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef DATETIMESTAMP_H
#define DATETIMESTAMP_H

#include <dlfcn.h>
#include <sys/stat.h>
#include <time.h>
#include <string>

inline std::string datetimestamp_addr(const void* address)
{
    Dl_info info;

    struct stat st;

    if ( dladdr( address, &info )==0 || info.dli_fname==0 || stat( info.dli_fname, &st )!=0 ) return "???";
    
    std::string stime( 25, '\0' );
    
    ctime_r( &st.st_mtime, &stime[0] );
    stime.pop_back();

    return stime;
}

template<typename T, typename... Args>
std::string datetimestamp(T (*func)(Args...)) { return datetimestamp_addr((void*)(func)); }

template<typename T>
struct timestamp { static void stamper() {} };

template<typename T>
std::string datetimestamp(const T* /* t */ ) { return datetimestamp_addr((void*)(&timestamp<T>::stamper)); }


#endif // DATETIMESTAMP_H
 
