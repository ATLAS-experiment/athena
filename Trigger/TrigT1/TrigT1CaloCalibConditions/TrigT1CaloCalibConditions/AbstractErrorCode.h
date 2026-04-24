/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1CALOCALIBCONDITIONS_ABSTRACTERRORCODE_H
#define TRIGT1CALOCALIBCONDITIONS_ABSTRACTERRORCODE_H


/**
 *  AbstractErrorCode abstract base class
 *  for L1Calo error codes
 *
 *  @author Damien Prieur <damien.prieur@cern.ch>
 */

class AbstractErrorCode {

public:
	virtual ~AbstractErrorCode() {};
  AbstractErrorCode& operator=(AbstractErrorCode&&) noexcept = default;
	unsigned int errorCode() const {return m_errorCode;}

protected:
	AbstractErrorCode(unsigned int errorCode) { m_errorCode = errorCode; };

	void errorCode(unsigned int code) {m_errorCode = code;}
  
  
	bool bitIsSet(unsigned int number) const;
	void setBit(unsigned int number, bool value);

private:
	unsigned int m_errorCode{};

};
#endif
