/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IdDict converter package
 -----------------------------------------
 ***************************************************************************/

//<version>	$Name: not supported by cvs2svn $

#ifndef SRC_IDDICTCNVTEST_H
# define SRC_IDDICTCNVTEST_H

#include "AthenaBaseComps/AthAlgorithm.h"

/********************************************************************

Algorithm for testing the loading of the Identifier dictionaries

********************************************************************/

class IdDictCnvTest : public AthAlgorithm
{

public:

    IdDictCnvTest(const std::string& name, ISvcLocator* pSvcLocator);
    ~IdDictCnvTest();

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

private:

    void        tab(size_t level) const;
};

#endif // SRC_IDDICTCNVTEST_H
