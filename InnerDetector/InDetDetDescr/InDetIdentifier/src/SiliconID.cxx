/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @file SiliconID.cxx
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 *
 */

#include "InDetIdentifier/SiliconID.h"
#include "Identifier/IdentifierHash.h"
#include "IdDict/IdDictDefs.h"
#include "GaudiKernel/MsgStream.h"
#include <iostream>

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

SiliconID::SiliconID()
  :
  AtlasDetectorID("SiliconID", ""),
  m_wafer_hash_max(0),
  m_pixel_wafer_hash_max(0),
  m_pixel_helper(nullptr),
  m_sct_helper(nullptr)
{}

SiliconID::SiliconID(const PixelID* pixel_helper, const SCT_ID* sct_helper)
  :
  AtlasDetectorID("SiliconID", ""),
  m_wafer_hash_max(0),
  m_pixel_wafer_hash_max(0),
  m_pixel_helper(pixel_helper),
  m_sct_helper(sct_helper)
{}


SiliconID::~SiliconID() = default;



int
SiliconID::initialize_from_dictionary(const IdDictMgr& dict_mgr) {
  MsgStream log(m_msgSvc, "SiliconID");
  log << MSG::INFO << "Initialize from dictionary" << endmsg;
  // Check whether this helper should be reinitialized
  if (!reinitialize(dict_mgr)) {
    log << MSG::INFO << "Request to reinitialize not satisfied - tags have not changed" << endmsg;
    
    return(0);
  } else {
    log << MSG::DEBUG << "(Re)initialize" << endmsg;
  }
  // init base object
  if (AtlasDetectorID::initialize_from_dictionary(dict_mgr)) return(1);
  // Register version of InnerDetector dictionary
  if (register_dict_tag(dict_mgr, "InnerDetector")) return(1);
  // Make sure that pix/sct helpers are initialized
  // CANNOT MODIFY CONST HELPERS
  m_pixel_wafer_hash_max = m_pixel_helper->wafer_hash_max();
  m_wafer_hash_max = m_sct_helper->wafer_hash_max() + m_pixel_wafer_hash_max;
  return 0;
}

// From hash get Identifier
int
SiliconID::get_id(const IdentifierHash& /*hash_id*/,
                  Identifier& id,
                  const IdContext* /*context*/) const {
  int result = 1;
  Identifier newid;

  id = newid;
  return(result);
}

int
SiliconID::get_hash(const Identifier& /*id*/,
                    IdentifierHash& hash_id,
                    const IdContext* /*context*/) const {
  // Get the hash code from either a vec (for wafers) or calculate
  // it (pixels). For the former, we convert to compact and call
  // get_hash again. For the latter, we calculate the hash from the
  // Identifier.

  int result = 1;

  hash_id = 0;
  return(result);
}

int
SiliconID::test_wafer_hashes() const {
  MsgStream log(m_msgSvc, "IdDictTest_SiliconID");

  log << MSG::INFO << "test_wafer_packing: wafer hash max, pix, sct "
                    << wafer_hash_max() << " "
                    << m_pixel_helper->wafer_hash_max() << " "
                    << m_sct_helper->wafer_hash_max() << " "
                    << endmsg;
 

  bool error = false;

  int nids = 0;
  int nblay = 0;
  int nbar = 0;
  int nbars = 0;
  int nbarp = 0;
  int nHashPix = 0;

  for (unsigned int i = 0; i < wafer_hash_max(); ++i, ++nids) {
    Identifier id = wafer_id(i);
    if (i < 10) {
      log << MSG::INFO << "test_wafer_packing: id "
                        << show_to_string(id)
                        << endmsg;
     
    }

    // Check hash
    IdentifierHash hash = wafer_hash(id);
    if (hash != i) {
      log << MSG::ERROR << "test_wafer_packing: wafer_hash not equal to i "
                        << "hash: " << hash << " i: " << i << " "
                        << show_to_string(id)
                        << endmsg;
     
      error = true;
    }

    // Check is_barrel
    if (is_barrel(id)) {
      nbar++;
      if (is_sct(id)) {
        nbars++;
        if (!m_sct_helper->is_barrel(id)) {
          log << MSG::ERROR << "test_wafer_packing: is_barrel fails for sct. "
                            << "hash: " << hash << " i: " << i << " "
                            << show_to_string(id)
                            << endmsg;
        
          error = true;
        }
      } else {
        nbarp++;
        if (!m_pixel_helper->is_barrel(id)) {
          log << MSG::ERROR << "test_wafer_packing: is_barrel fails for pixel. "
                            << "hash: " << hash << " i: " << i << " "
                            << show_to_string(id)
                            << endmsg;
          
          error = true;
        }
      }
    }

    // Check is_blayer
    if (is_blayer(id)) {
      nblay++;
      if (is_sct(id)) {
        log << MSG::ERROR << "test_wafer_packing: is_blayer is sct. "
                          << "hash: " << hash << " i: " << i << " "
                          << show_to_string(id)
                          << endmsg;
        
        error = true;
      } else {
        if (!m_pixel_helper->is_blayer(id)) {
          log << MSG::ERROR << "test_wafer_packing: is_blayer fails for pixel. "
                            << "hash: " << hash << " i: " << i << " "
                            << show_to_string(id)
                            << endmsg;
          
          error = true;
        }
      }
    }

    // Check is_pixel
    if (is_hash_pixel(i) != is_pixel(id)) {
      log << MSG::ERROR << "test_wafer_packing: is_hash_pixel "
                        << "hash: " << hash << " i: " << i << " "
                        << show_to_string(id)
                        << endmsg;
     
      error = true;
    } else {
      nHashPix++;
    }
  }

  
    log << MSG::INFO << "Looped over " << nids << " hashes "
        << endmsg;
    log << MSG::INFO << "Number of is_barrel (pix/sct): " << nbar
        << " " << nbarp << " " << nbars
        << endmsg;
    log << MSG::INFO << "Number of is_blayer: " << nblay
        << endmsg;
    log << MSG::INFO << "Number of matching is_hash_pixel/is_pixel: " << nHashPix
        << endmsg;
  


  if (error) return(1);

  return(0);
}
