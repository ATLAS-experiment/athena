/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCellDumper.h"
#include <cmath>


namespace {

bool isEqual (double x1, double x2, double thresh = 1e-6)
{
  double den = std::abs(x1+x2);
  if (den < thresh) return true;
  double diff = std::abs (x1-x2) / den;
  if (diff < thresh)
    return true;
  return false;
}

} // anonymous namespace

CaloCellDumper::CaloCellDumper(const std::string & name, ISvcLocator * pSvcLocator) :
  AthAlgorithm(name,pSvcLocator) {}


StatusCode CaloCellDumper::initialize() {
  ATH_CHECK(m_key.initialize());
  m_outfile.open(m_fileName,std::ios::out);

  if (!m_outfile.good()) {
    ATH_MSG_ERROR("Failed to open output text file " << m_fileName);
    return StatusCode::FAILURE;
  }

  if (!m_refName.value().empty()) {
    m_reffile.open(m_refName, std::ios::in);
    if (!m_reffile.good()) {
      ATH_MSG_ERROR("Failed to open reference file " << m_refName);
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_INFO("Using reference file " << m_refName.value());
    }
  }

  ATH_MSG_INFO("Cell energy cut=" << m_eCut.value());
  return StatusCode::SUCCESS;
}


StatusCode CaloCellDumper::finalize() {
  m_outfile.close();
  if (m_reffile.is_open()) {
    m_reffile.close();
  }
 return StatusCode::SUCCESS;
}



StatusCode CaloCellDumper::execute() {

  bool badCompare = false;
  const EventContext& ctx = getContext();
  
  SG::ReadHandle<CaloCellContainer> cells{m_key,ctx};
  
  const unsigned evt=ctx.eventID().event_number();
  m_outfile << "Event " << evt << " contains " << cells->size() << " CaloCells" << std::endl;
  m_outfile << "ID\tEnergy\tTime\tQual\tprov" << std::endl;

  if (m_reffile.is_open()) {
    std::string Event, contains, CaloCells;
    size_t evt_in, cells_size_in;
    m_reffile >> Event >> evt_in >> contains >> cells_size_in >> CaloCells;
    if (!m_reffile.good() || evt_in != evt || cells_size_in != cells->size())
    {
      ATH_MSG_ERROR ("Reference file miscompare.  Read: " <<
                     Event << " " << evt_in << " " << contains << " "
                     << cells_size_in << " " << CaloCells <<
                     "; expected event number " << evt << " and cell count " <<
                     cells->size());
      badCompare = true;
    }

    std::string ID, Energy, Time, Qual, prov;
    m_reffile >> ID >> Energy  >> Time >> Qual >> prov;
    if (!m_reffile.good()) {
      ATH_MSG_ERROR ("Bad read of reference file header: " <<
                     ID  << " " << Energy << " " << Time << " " <<
                     Qual << " " << prov);
      badCompare = true;
    }
  }

  double remainingEne=0;
  unsigned nRemaining=0;

  for (const auto *cell : *cells) {
    if (cell->e()>m_eCut.value()) {
      std::stringstream id;
      if (!m_compact.value()) {
	const CaloDetDescrElement* dde=cell->caloDDE();
	if (dde->is_lar_em_barrel()) {
	  id << "LAr Bar";
	}
	else if (dde->is_lar_em_endcap_inner()){
	  id << "LAR ECI";
	}
	else if (dde->is_lar_em_endcap_outer()){
	  id << "LAR ECO";
	}
	else if (dde->is_lar_hec()) {
	  id << "LAr_HEC";
	}
	else if (dde->is_lar_fcal()) {
	  id << "LAr_FCAL";
	}
	else if (dde->is_tile()) {
	  id << "TILE    ";
	}
	else {
	  id << "UNKNOWN";
	}
	id << ", samp=" << dde->getSampling() << ", ";
      }
     
      m_outfile << id.str() << "0x" << std::hex << cell->ID().get_identifier32().get_compact() << std::dec 
      //m_outfile << 
		<< "\t" << cell->e() << "\t" << cell->time() << "\t" << cell->gain() 
		<< "\t" << cell->quality() << "\t0x" << std::hex << cell->provenance() << std::dec << std::endl;
      if (m_reffile.is_open()) {
        unsigned id_in;
        float energy_in, time_in;
        int quality_in, provenance_in, gain_in;
        if (!m_compact) {
          std::string id, samp;
          m_reffile >> id >> samp;
        }
        m_reffile >> std::hex >> id_in >> std::dec >> energy_in >> time_in >>
          gain_in >> quality_in >> std::hex >> provenance_in >> std::dec;
        if (!m_reffile.good() ||
            !isEqual (energy_in, cell->e(), 1e-5) ||
            !isEqual (time_in, cell->time(), 1e-5) ||
            id_in != cell->ID().get_identifier32().get_compact() ||
            gain_in != cell->gain() ||
            quality_in != cell->quality() ||
            provenance_in != cell->provenance())
        {
          ATH_MSG_ERROR ("Bad read of reference file: " <<
                         std::hex << "0x" << id_in << " " << std::dec <<
                         energy_in << " "
                         << time_in << " " <<
                         gain_in << " " << quality_in << " " <<
                         std::hex << "0x" << provenance_in << std::dec);
          ATH_MSG_ERROR ("Expected " << id.str() << "0x" << std::hex <<
                         cell->ID().get_identifier32().get_compact() << std::dec << " " <<
                         cell->e() << " " << cell->time() << " "
                         << cell->gain() << " " <<
                         cell->quality() << " 0x" << std::hex << cell->provenance() << std::dec);
          badCompare = true;
        }
      }
    }
    else {//not bigger that m_eCut
      ++nRemaining;
      remainingEne+=cell->e();
    }
  }
  if (nRemaining) {
    m_outfile << "Sum of " << nRemaining << " cell energies: " << remainingEne << std::endl;
    if (m_reffile.is_open()) {
      std::string Sum, Of, Cell, Energies;
      unsigned nRemaining_in;
      float remainingEne_in;
      m_reffile >> Sum >> Of >> nRemaining_in >> Cell >> Energies >>
        remainingEne_in;
      if (!m_reffile.good() ||
          nRemaining_in != nRemaining ||
          !isEqual (remainingEne_in, remainingEne))
      {
        ATH_MSG_ERROR ("Bad read of reference file: " <<
                       Sum << Of << nRemaining_in << Cell << Energies <<
                       remainingEne_in);
        ATH_MSG_ERROR ("Expected: " <<
                       "Sum of " << nRemaining << " cell energies: " << remainingEne);
        badCompare = true;
      }
    }
  }
  if (badCompare) {
    ATH_MSG_ERROR ("Reference file miscompare");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

