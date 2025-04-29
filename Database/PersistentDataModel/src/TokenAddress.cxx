/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "PersistentDataModel/TokenAddress.h"


void TokenAddress::setToken(std::unique_ptr<Token> token) {
  m_ownedToken = std::move (token);
  m_token = m_ownedToken.get();
  m_par.reset();
}

Token* TokenAddress::getToken()
{
  if (!m_ownedToken && m_token) {
    m_ownedToken = std::make_unique<Token> (m_token);
    m_token = m_ownedToken.get();
  }
  return m_ownedToken.get();
}

const std::string* TokenAddress::par() const
{
  if (!m_par.isValid()) {
    // We haven't created the parameter array yet.
    // Do it now.
    Pars pars;
    pars.par[0] = this->GenericAddress::par()[0];
    pars.par[1] = this->GenericAddress::par()[1];
    pars.par[2] = this->GenericAddress::par()[2];
    if (pars.par[0].empty() && m_token) {
      pars.par[0] = m_token->toString();
    }
    m_par.set (std::move (pars));
  }
  return m_par.ptr()->par;
}
