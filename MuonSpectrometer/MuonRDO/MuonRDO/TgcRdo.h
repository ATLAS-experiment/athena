/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_TGCRDO_H
#define MUONRDO_TGCRDO_H 1

#include <inttypes.h>
#include "MuonRDO/TgcRawData.h"
#include "MuonRDO/TgcStatusStructs.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "Identifier/IdentifierHash.h"

/*
  TGC collection class used for bare RDO ByteStream Conversion.
  This class holds one ROD information

  @author Tadashi Maeno
      based on the previous one by H.Kurashige
*/
class TgcRdo : public DataVector<TgcRawData>
{
public:
    // typedef required by container
    typedef uint16_t ID;
    typedef TgcRawData DIGIT;
    using Errors = MuonRDO::Errors;
    using RodStatus = MuonRDO::RodStatus;
    using LocalStatus = MuonRDO::LocalStatus;

    /**Default constructor*/
    TgcRdo();

    /** Creates an empty container ready for writing */
    TgcRdo(uint16_t id, IdentifierHash idHash);

    // P1
    TgcRdo(uint16_t subDetectorId,
           uint16_t rodId,
           uint16_t bcId,
           uint16_t l1Id);

    // Destructor
    virtual ~TgcRdo()
    {
    }

    // Identifier
    uint16_t identify() const
    {
        return m_id;
    }

        // Identifier
    IdentifierHash identifyHash() const
    {
        return m_idHash;
    }
    
    // set methods
    void setL1Id(uint32_t v)
    {
        m_l1Id = v;
    }
    void setBcId(uint16_t v)
    {
        m_bcId = v;
    }
    void setTriggerType(uint16_t v)
    {
        m_triggerType = v;
    }
    void setOnlineId (uint16_t subDetectorId, uint16_t rodId);

    // get methods
    uint16_t subDetectorId() const
    {
        return m_subDetectorId;
    }
    uint16_t rodId() const
    {
        return m_rodId;
    }
    uint16_t triggerType() const
    {
        return m_triggerType;
    }
    uint16_t bcId() const
    {
        return m_bcId;
    }
    uint16_t l1Id() const
    {
        return m_l1Id;
    }

    // class method for RawData identification
    static uint16_t identifyRawData (const TgcRawData &rawData);

    const Errors& errors() const
    {
        return m_errors;
    }
    void setErrors(uint16_t data)
    {
        m_errors = MuonRDO::setErrors(data);
    }

    const RodStatus& rodStatus() const
    {
        return m_rodStatus;
    }
    void setRodStatus(uint32_t data)
    {
        m_rodStatus = MuonRDO::setRodStatus(data);
    }

    const LocalStatus& localStatus() const
    {
        return m_localStatus;
    }
    void setLocalStatus(uint32_t data)
    {
        m_localStatus = MuonRDO::setLocalStatus(data);
    }

    uint32_t orbit() const
    {
        return m_orbit;
    }
    void setOrbit(uint32_t orbit)
    {
        m_orbit = orbit;
    }

    uint16_t version() const
    {
        return m_version;
    }

    void setVersion(uint16_t version)
    {
        m_version = version;
    }

    // ??? This overrides DataVector::clear() with something that
    //     does something different.  Should probably be called
    //     something else.
    void clear();

    // online ID calculator
    static uint16_t calculateOnlineId (uint16_t subDetectorId, uint16_t rodId);

private:
    // Returns offset, MAX_N_ROD
    static std::pair<int, int> initOnlineId();

    uint16_t m_version = 0U; // starting August 2006 version = 300. Before that, version = 0

    /** ID of this instance*/
    uint16_t m_id = 0U;

    /** OFFLINE hash of this collection*/
    IdentifierHash m_idHash; 
    
    // online IDs
    uint16_t m_subDetectorId = 0U;
    uint16_t m_rodId = 0U;

    // Trigger Type
    uint16_t m_triggerType = 0U;

    // BCID and L1ID on ROD
    uint16_t m_bcId = 0U;
    uint16_t m_l1Id = 0U;

    MuonRDO::Errors      m_errors{};
    MuonRDO::RodStatus   m_rodStatus{};
    MuonRDO::LocalStatus m_localStatus{};
    uint32_t m_orbit = 0U;
};

/**Overload of << operator for std::ostream for debug output*/
std::ostream& operator<<(std::ostream& sl, const TgcRdo& coll);

CLASS_DEF(TgcRdo,4185,0)

// Class needed only for persistency
typedef DataVector<TgcRdo> TGC_RDO_vector;
CLASS_DEF(TGC_RDO_vector, 4182, 1)

#endif
