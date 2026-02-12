/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// SGNTUPProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef SG_NTUP_PROXY_LOADER_H
#define SG_NTUP_PROXY_LOADER_H

#include "ExpressionEvaluation/RelayProxyLoader.h"

#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "SGTools/StlVectorClids.h"
#include "SGTools/BuiltinsClids.h"

namespace ExpressionParsing {
  class SGNTUPProxyLoader : public RelayProxyLoader {
    public:
      typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;

      SGNTUPProxyLoader(StoreGateSvc_t &evtStore) : m_evtStore(evtStore) { }
      virtual ~SGNTUPProxyLoader();

      virtual void reset() override;

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      StoreGateSvc_t m_evtStore;
  };
}

#endif // SG_NTUP_PROXY_LOADER_H
