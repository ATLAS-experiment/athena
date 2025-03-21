/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERSISTENTDATAMODEL_TOKENADDRESS_H
#define PERSISTENTDATAMODEL_TOKENADDRESS_H

/** @file TokenAddress.h
 *  @brief This file contains the class definition for the TokenAddress class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

// Framework include files
#include "GaudiKernel/GenericAddress.h"

#include "PersistentDataModel/Token.h"
#include "CxxUtils/CachedValue.h"
#include <memory>

/** @class TokenAddress
 *  @brief This class provides a Generic Transient Address for POOL tokens.
 **/
class TokenAddress : public GenericAddress {

public:
   /// No copy/assignment allowed
   TokenAddress(const TokenAddress& rhs) = delete;
   TokenAddress& operator=(const TokenAddress& rhs) = delete;

   /// Dummy constructor
   TokenAddress() = default;

   /// Standard Constructor
   TokenAddress(long svc,
	   const CLID& clid,
	   const std::string& p1 = "",
	   const std::string& p2 = "",
	   unsigned long ip = 0,
	   const Token* pt = 0) : GenericAddress(svc, clid, p1, p2, ip), m_token(pt) {
   }
   TokenAddress(long svc,
	   const CLID& clid,
	   const std::string& p1,
	   const std::string& p2,
	   unsigned long ip,
           std::unique_ptr<Token> pt) :
     GenericAddress(svc, clid, p1, p2, ip),
     m_ownedToken (std::move (pt)),
     m_token(m_ownedToken.get()) {
   }
   TokenAddress(const GenericAddress& genAddr, const Token* pt = 0) : GenericAddress(genAddr), m_token(pt) {
   }
   TokenAddress(const GenericAddress& genAddr,
                std::unique_ptr<Token> pt) :
     GenericAddress(genAddr),
     m_ownedToken (std::move(pt)),
     m_token(m_ownedToken.get()) {
   }

   virtual ~TokenAddress() = default;

   Token* getToken();
   const Token* getToken() const { return m_token; }
   void setToken(std::unique_ptr<Token> token);
   virtual const std::string* par() const override;

private:
   struct Pars {
     std::string par[3];
   };
   std::unique_ptr<Token> m_ownedToken;
   const Token* m_token = nullptr;
   /// The parameter array.  We create it lazily in par().
   CxxUtils::CachedValue<Pars> m_par;
};

#endif
