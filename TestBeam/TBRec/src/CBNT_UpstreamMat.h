/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_CBNT_UPSTREAMMAT_H
#define TBREC_CBNT_UPSTREAMMAT_H

#include "CBNT_TBRecBase.h"
#include <string>


class CBNT_UpstreamMat : public CBNT_TBRecBase {
public:
	CBNT_UpstreamMat(const std::string &name, ISvcLocator *pSvcLocator);
	virtual ~CBNT_UpstreamMat();
	virtual StatusCode CBNT_initialize() override;
	virtual StatusCode CBNT_execute() override;
	virtual StatusCode CBNT_finalize() override;
	virtual StatusCode CBNT_clear() override;

private:
	Gaudi::Property<std::string> m_key{this, "CollectionName", "", "SG key of TrackRecordCollection"};
	bool m_notFound{false};
   //Ntuple variables:

	long     m_nElec{0};
	std::vector<float>    *m_ElecEne{nullptr};
	std::vector<float>    *m_ElecAng{nullptr};
	long     m_nPhot{0};
	std::vector<float>    *m_PhotEne{nullptr};
	std::vector<float>    *m_PhotAng{nullptr};
};

#endif // CBNT_EVENTINFO_H










