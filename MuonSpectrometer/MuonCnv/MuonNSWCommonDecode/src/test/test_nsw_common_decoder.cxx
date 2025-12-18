/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



// TDAQ include files

#include "ers/ers.h"
#include "ers/SampleIssues.h"
#include "EventStorage/pickDataReader.h"
#include "eformat/eformat.h"

#include "MuonNSWCommonDecode/NSWCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWElink.h"
#include "MuonNSWCommonDecode/VMMChannel.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
// STL include files

#include <vector>
#include <string>
#include <chrono>

// Number of sectors - Module ID, to be checked to avoid confusion with NSW TP

static const uint16_t sectors = 16;

// Local issue definition

ERS_DECLARE_ISSUE_BASE (eformat, InconsistentChannelNumber, eformat::Issue, 
                        "Inconsistent channel number: " << nlink << " from link tree " << nflat << " from flat list",
                        ERS_EMPTY, ((size_t) nlink) ((size_t) nflat))

// Error parameters

static const int ERR_NOERR   =  0;
static const int ERR_ARGS    = -1;
static const int ERR_GENERIC = -2;

// Global parameters

struct Params
{
  bool printout {false};
  bool tree_view {true};
  bool flat_view {true};
  bool print_raw {false};
  unsigned int printout_level {0};
  unsigned int max_events {0};
  std::vector <std::string> detectors;
  std::vector <std::string> file_names;
};

struct Statistics
{
  static const unsigned int max_stat {1000000};
  unsigned int nevents {0};
  unsigned int stat_events {0};
  std::vector <unsigned int> nhits; 
  std::vector <float> elapsed_vector;
  std::vector <float> elapsed_vector_event;
};

void test_nsw_common_decoder_help (char *progname)
{
  std::cout << "Usage: " << progname
	    << " [-v] [-r] [-t] [-f] [-d <MMG/STG>] [-n events] [-h] file1, file2, ..." << std::endl;
  std::cout << "\t\t[-n events] maximum number of events to read (default = all)" << std::endl;
  std::cout << "\t\t[-r] print raw fragments" << std::endl;
  std::cout << "\t\t[-t] only shows hits taken from tree view of decoded data" << std::endl;
  std::cout << "\t\t[-f] only shows hits taken from flat view of decoded data" << std::endl;
  std::cout << "\t\t[-d <MMG/STG>] select one of the two subdetectors" << std::endl;
  std::cout << "\t\tMultiple [-v] options increase printout detail level" << std::endl;
}

int test_nsw_common_decoder_opt (int argc, char **argv, Params& params)
{
  int errcode = ERR_NOERR;
  int i;

  for (i=1; i < argc; ++i)
  {
    std::string det;

    if (argv[i][0] == '-')
      switch (argv[i][1])
      {
        case 'v':
	  params.printout = true;
          ++params.printout_level;
	  break;
        case 'd':
	  det = argv[++i];
	  params.detectors.push_back (std::move(det));
	  break;
        case 'r':
	  params.print_raw = true;
	  break;
        case 't':
          params.flat_view = false;
          break;
        case 'f':
          params.tree_view = false;
          break;
        case 'n':
          params.max_events = static_cast <unsigned int> (atoi (argv[++i]));
          break;
        case 'h':
	  test_nsw_common_decoder_help (argv[0]);
	  return ERR_NOERR;
        default:
	  test_nsw_common_decoder_help (argv[0]);
	  return ERR_ARGS;
      }
    else
    {
      std::string data_file_name (argv[i]);
      params.file_names.push_back (std::move(data_file_name));
    }
  }

  if (params.file_names.size () == 0)
  {
    test_nsw_common_decoder_help (argv[0]);
    return (ERR_ARGS);
  }

  return errcode;
}

int test_nsw_common_decoder_init ()
{
  int errcode = ERR_NOERR;
  return errcode;
}

int test_nsw_common_decoder_end (const Statistics &statistics)
{
  int errcode = ERR_NOERR;
  double nhits_average = 0, elapsed_average = 0, elapsed_average_event = 0;

  for (auto n : statistics.nhits)
    nhits_average += n;

  for (auto f : statistics.elapsed_vector)
    elapsed_average += f;

  for (auto f : statistics.elapsed_vector_event)
    elapsed_average_event += f;

  nhits_average /= statistics.nhits.size () ? statistics.nhits.size () : 1;
  elapsed_average /= statistics.elapsed_vector.size () ? statistics.elapsed_vector.size () : 1;
  elapsed_average_event /= statistics.elapsed_vector_event.size () ? statistics.elapsed_vector_event.size () : 1;

  std::cout << "Total event number                = " << statistics.nevents << std::endl;
  std::cout << "Total event number for statistics = " << statistics.stat_events << std::endl;
  std::cout << "Fragments                         = " << statistics.elapsed_vector.size () << std::endl;
  std::cout << "Events with fragments             = " << statistics.elapsed_vector_event.size () << std::endl;
  std::cout << "Average elapsed time per fragment = " << elapsed_average << " ms" << std::endl;
  std::cout << "Average elapsed time per event    = " << elapsed_average_event << " ms" << std::endl;
  std::cout << "Average hits per event            = " << nhits_average << std::endl;

  return errcode;
}

int test_nsw_common_decoder_event (const eformat::read::FullEventFragment &f, const Params &params, Statistics &statistics)
{
  int errcode = ERR_NOERR;
  std::vector <eformat::read::ROBFragment> robs;

  unsigned int time_elapsed_event = 0;
  unsigned int nchan_event = 0;

  if (params.printout_level > 2)
    std::cout << "Entering fragment analysis" << std::endl;

  f.robs (robs);

  for (auto r = robs.begin (); r != robs.end (); ++r)
  {
    bool is_nsw = false, is_mmg = false, is_stg = false;

    // check fragment for errors

    try
    {
      r->check ();
    }
    catch (eformat::Issue &ex)
    {
      ers::warning (ex);
      continue;
    }

    uint32_t sid = r->rob_source_id ();
    eformat::helper::SourceIdentifier source_id (sid);
    eformat::SubDetector s = source_id.subdetector_id ();
    uint16_t m = source_id.module_id ();

    if (s == 0)    // NSW data written before end of March 2021 have wrong source Id
      s = static_cast <eformat::SubDetector> ((sid >> 24) & 0xff);

    if (s == eformat::MUON_MMEGA_ENDCAP_A_SIDE || s == eformat::MUON_MMEGA_ENDCAP_C_SIDE)
      is_nsw = is_mmg = true;
    else if (s == eformat::MUON_STGC_ENDCAP_A_SIDE  || s == eformat::MUON_STGC_ENDCAP_C_SIDE)
      is_nsw = is_stg = true;

    // Sector id is in bits 0:3. In order to be sure these are L1A detector data
    // we need to mask the bits 8 and 9 used to identify multiple ROBs for the same sector, in case
    // that configuration is used (I hope never :))
    // All the others should be 0.

    if (is_nsw && ((m & ~0x0300) < sectors))
    {
      if (params.detectors.size () == 0 ||
	  ((std::find (params.detectors.begin (), params.detectors.end (), "MMG") != params.detectors.end () && is_mmg) ||
	   (std::find (params.detectors.begin (), params.detectors.end (), "STG") != params.detectors.end () && is_stg)))
      {
	if (params.printout_level > 2)
	  std::cout << "NSW fragment found: detector ID = 0x" << std::hex << s << std::dec << " length " << r->rod_ndata () << std::endl;

	// Print out raw fragment

	if (params.print_raw)
	{
	  std::cout << "ROD fragment size in words: Total: " << r->rod_fragment_size_word ()
		    << " Header: " << r->rod_header_size_word () << " Trailer: " << r->rod_trailer_size_word () << std::endl;
	  std::cout << "ROD source ID: 0x" << std::hex << r->rod_source_id ()
		    << " ROD L1 ID: " << r->rod_lvl1_id () << std::dec << std::endl;
	  std::cout << "Data words: " << r->rod_ndata () << std::endl;
	  std::cout << std::hex;

	  const uint32_t *bs = r->rod_data ();

	  for (unsigned int i = 0; i < r->rod_ndata (); ++i)
	  {
	    std::cout << "\t" << bs[i];
	    if (i % 4 == 3)
	      std::cout << std::endl;
	  }

	  std::cout << std::dec;
	}

	// Decode the ROB fragment (including sanity check)

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now ();
	Muon::nsw::NSWCommonDecoder nsw_decoder (*r);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();

	unsigned int time_elapsed = std::chrono::duration_cast <std::chrono::microseconds> (end - begin).count ();
	float time_elapsed_ms = static_cast <float> (time_elapsed) / 1000;

	if (statistics.stat_events < statistics.max_stat)
	  statistics.elapsed_vector.push_back (time_elapsed_ms);

	time_elapsed_event += time_elapsed;

	if (params.printout)
	  std::cout << "Time difference = " << time_elapsed_ms << " [ms]" << std::endl;

	// Check direct access to channels and tree access to elinks and channels

	// Check the number of channels by accessing in both ways

	unsigned int nchan = 0;
	const std::vector <Muon::nsw::NSWElink *>& links = nsw_decoder.get_elinks ();
	for (auto i = links.begin (); i != links.end (); ++i)
	  nchan += (*i)->get_channels ().size ();

	if (nchan != nsw_decoder.get_channels ().size ())
	  ers::error (eformat::InconsistentChannelNumber (ERS_HERE, nchan, nsw_decoder.get_channels ().size ()));

	if (params.printout_level > 2)
	  std::cout << "Hit number = " << nchan << std::endl;

	nchan_event += nchan;

	// How to access information about detector elements and channels using the tree view

	if (params.tree_view)
	{
	  for (auto i = links.begin (); i != links.end (); ++i)
	  {
	    if (! (*i)->isNull ())
	    {
	      uint16_t l1Id  = (*i)->l1Id ();
	      uint16_t bcId  = (*i)->bcId ();

	      uint8_t sector = (*i)->elinkId ()->sector (); // (*i)->elinkId () returns a pointer to a Muon::nsw::ResourceId object
	      uint8_t layer  = (*i)->elinkId ()->layer ();
	      uint8_t radius = (*i)->elinkId ()->radius ();
	      uint8_t elink  = (*i)->elinkId ()->elink ();  // elink number is not needed to decode channel position

	      // Offline ID components

	      std::string station_name;

	      if (is_mmg)
		station_name = (*i)->elinkId ()->is_large_station () ? "MML" : "MMS";
	      else
		station_name = (*i)->elinkId ()->is_large_station () ? "STL" : "STS";

	      int8_t   station_eta    = (*i)->elinkId ()->station_eta ();
	      uint8_t  station_phi    = (*i)->elinkId ()->station_phi ();
	      uint8_t  multi_layer    = (*i)->elinkId ()->multi_layer ();
	      uint8_t  gas_gap        = (*i)->elinkId ()->gas_gap ();

	      const std::vector <Muon::nsw::VMMChannel *> channels = (*i)->get_channels ();
	      for (auto j = channels.begin (); j != channels.end (); ++j)
	      {
		uint16_t vmm_number  = (*j)->vmm ();
		uint16_t vmm_channel = (*j)->vmm_channel ();
		uint16_t rel_bcid    = (*j)->rel_bcid ();
		uint16_t pdo         = (*j)->pdo ();
		uint16_t tdo         = (*j)->tdo ();
		bool     parity      = (*j)->parity ();
		bool     neighbor    = (*j)->neighbor ();

		// Get offline information

		uint8_t  channel_type   = (*j)->channel_type ();
		uint16_t channel_number = (*j)->channel_number ();

		if (params.printout)
		{
		  if (params.printout_level > 1)
		  {
		    std::cout << "ROD header:" << std::endl;
		    std::cout << "\t\tROD fragment size (words)  " << r->rod_fragment_size_word () << std::endl;
		    std::cout << "\t\tROD header size (words)    " << r->rod_header_size_word () << std::endl;
		    std::cout << "\t\tROD trailer size (words)   " << r->rod_trailer_size_word () << std::endl;
		    std::cout << "\t\tROD format version          0x" << std::hex << r->rod_version () << std::dec << std::endl;
		    std::cout << "\t\tROD source ID               0x" << std::hex << r->rod_source_id () << std::dec << std::endl;
		    std::cout << "\t\tROD L1ID                   " << r->rod_lvl1_id () << std::endl;
		    std::cout << "\t\tROD BCID                   " << r->rod_bc_id () << std::endl;
		    std::cout << "\t\tROD trigger type           " << r->rod_lvl1_trigger_type () << std::endl;
		    std::cout << "\t\tROD run number             " << r->rod_run_no () << std::endl;
		    std::cout << "\t\tROD detector event type    " << r->rod_detev_type () << std::endl;
		    std::cout << "\t\tROD number of status words " << r->rod_nstatus () << std::endl;
		  }

		  std::cout << "Online decoding of hit word 0x" << std::hex << (*j)->vmm_word ()
			    << " on link 0x" << (*i)->elinkWord () << std::dec << std::endl;
		  std::cout << "Parity " << parity << " Calculated parity " << (*j)->calculate_parity () << std::endl;
		  std::cout << "L1ID " << l1Id << " BCID " << bcId << " Sector " << static_cast <unsigned int> (sector)
			    << " Layer " << static_cast <unsigned int> (layer) << " Radius " << static_cast <unsigned int> (radius)
			    << " Elink " << static_cast <unsigned int> (elink) << std::endl;
		  std::cout << "VMM " << vmm_number << " Channel " << vmm_channel << " Relative BCID " << rel_bcid 
			    << " Pdo " << pdo << " Tdo " << tdo << " Parity " << parity << " Neighbor " << neighbor << std::endl;
		  std::cout << "Offline decoding of hit word 0x" << std::hex << (*j)->vmm_word ()
			    << " on link 0x" << (*i)->elinkWord () << std::dec << std::endl;
		  std::cout << "Station name " << station_name << " Station eta " << static_cast <int> (station_eta)
			    << " Station phi " << static_cast <unsigned int> (station_phi) << std::endl;
		  std::cout << "Multilayer " << static_cast <unsigned int> (multi_layer) << " Gas gap " << static_cast <unsigned int> (gas_gap)
			    << " Channel type " << static_cast <unsigned int> (channel_type)
			    << " Channel Number " << channel_number << std::endl;
		}
	      }
	    }
	  }
	}

	// The same information can be accessed through the list of all the channels for that ROB as follows

	if (params.flat_view)
	{
	  const std::vector <Muon::nsw::VMMChannel *>& channels = nsw_decoder.get_channels ();
	  for (auto j = channels.begin (); j != channels.end (); ++j)
	  {
	    const Muon::nsw::NSWElink *link = (*j)->elink ();

	    uint16_t l1Id  = link->l1Id ();
	    uint16_t bcId  = link->bcId ();

	    uint8_t sector = link->elinkId ()->sector (); // (*i)->elinkId () returns a pointer to a Muon::nsw::ResourceId object
	    uint8_t layer  = link->elinkId ()->layer ();
	    uint8_t radius = link->elinkId ()->radius ();
	    uint8_t elink  = link->elinkId ()->elink ();  // elink number is not needed to decode channel position

	    uint16_t vmm_number  = (*j)->vmm ();
	    uint16_t vmm_channel = (*j)->vmm_channel ();
	    uint16_t rel_bcid    = (*j)->rel_bcid ();
	    uint16_t pdo         = (*j)->pdo ();
	    uint16_t tdo         = (*j)->tdo ();
	    bool     parity      = (*j)->parity ();
	    bool     neighbor    = (*j)->neighbor ();

	    // Offline ID components

	    std::string station_name;
	    if (is_mmg)
	      station_name = (*j)->is_large_station () ? "MML" : "MMS";
	    else
	      station_name = (*j)->is_large_station () ? "STL" : "STS";

	    int8_t   station_eta    = (*j)->station_eta ();
	    uint8_t  station_phi    = (*j)->station_phi ();
	    uint8_t  multi_layer    = (*j)->multi_layer ();
	    uint8_t  gas_gap        = (*j)->gas_gap ();
	    uint8_t  channel_type   = (*j)->channel_type ();
	    uint16_t channel_number = (*j)->channel_number ();

	    if (params.printout)
	    {
	      if (params.printout_level > 1)
	      {
		std::cout << "ROD header:" << std::endl;
		std::cout << "\t\tROD fragment size (words)  " << r->rod_fragment_size_word () << std::endl;
		std::cout << "\t\tROD header size (words)    " << r->rod_header_size_word () << std::endl;
		std::cout << "\t\tROD trailer size (words)   " << r->rod_trailer_size_word () << std::endl;
		std::cout << "\t\tROD format version          0x" << std::hex << r->rod_version () << std::dec << std::endl;
		std::cout << "\t\tROD source ID               0x" << std::hex << r->rod_source_id () << std::dec << std::endl;
		std::cout << "\t\tROD L1ID                   " << r->rod_lvl1_id () << std::endl;
		std::cout << "\t\tROD BCID                   " << r->rod_bc_id () << std::endl;
		std::cout << "\t\tROD trigger type           " << r->rod_lvl1_trigger_type () << std::endl;
		std::cout << "\t\tROD run number             " << r->rod_run_no () << std::endl;
		std::cout << "\t\tROD detector event type    " << r->rod_detev_type () << std::endl;
		std::cout << "\t\tROD number of status words " << r->rod_nstatus () << std::endl;
	      }

	      std::cout << "Online decoding of hit word 0x" << std::hex << (*j)->vmm_word () << " on link 0x" << link->elinkWord () << std::dec << std::endl;
	      std::cout << "Parity " << parity << " Calculated parity " << (*j)->calculate_parity () << std::endl;
	      std::cout << "L1ID " << l1Id << " BCID " << bcId << " Sector " << static_cast <unsigned int> (sector)
			<< " Layer " << static_cast <unsigned int> (layer) << " Radius " << static_cast <unsigned int> (radius)
			<< " Elink " << static_cast <unsigned int> (elink) << std::endl;
	      std::cout << "VMM " << vmm_number << " Channel " << vmm_channel << " Relative BCID " << rel_bcid 
			<< " Pdo " << pdo << " Tdo " << tdo << " Parity " << parity << " Neighbor " << neighbor << std::endl;
	      std::cout << "Offline decoding of hit word 0x" << std::hex << (*j)->vmm_word () << " on link 0x" << link->elinkWord () << std::dec << std::endl;
	      std::cout << "Station name " << station_name << " Station eta " << static_cast <int> (station_eta)
			<< " Station phi " << static_cast <unsigned int> (station_phi) << std::endl;
	      std::cout << "Multilayer " << static_cast <unsigned int> (multi_layer) << " Gas gap " << static_cast <unsigned int> (gas_gap)
			<< " Channel type " << static_cast <unsigned int> (channel_type)
			<< " Channel Number " << channel_number << std::endl;
	    }
	  }
	}
      }
    }
  }

  if (params.printout_level > 2)
    std::cout << "Hit number per event = " << nchan_event << std::endl;

  if (statistics.stat_events < statistics.max_stat)
  {
    float time_elapsed_event_ms = static_cast <float> (time_elapsed_event) / 1000;

    ++statistics.stat_events;
    statistics.elapsed_vector_event.push_back (time_elapsed_event_ms);
    statistics.nhits.push_back (nchan_event);
  }

  return errcode;
}

int test_nsw_common_decoder_loop (const Params &params, Statistics &statistics)
{
  int errcode = ERR_NOERR;

  for (const std::string &filename : params.file_names)
  {
    std::string data_file_name (filename);

    std::cout << "Reading file " << data_file_name << std::endl;
    std::unique_ptr <DataReader> reader (pickDataReader (data_file_name));

    if (!reader || !reader->good ())
    {
      ers::fatal (ers::CantOpenFile (ERS_HERE, data_file_name.c_str ()));
      return ERR_GENERIC;
    }

    while (!reader->endOfFile () && (params.max_events == 0 || statistics.nevents < params.max_events))
    {
      char *buf = nullptr;
      unsigned int size = 0;

      try
      {
        DRError err = reader->getData (size, &buf);

        if (err != EventStorage::DROK)
        {
          ers::fatal (ers::File (ERS_HERE, data_file_name.c_str ()));
          errcode = ERR_GENERIC;
          if (buf) delete [] buf;
          break;
        }

        eformat::read::FullEventFragment f (reinterpret_cast<unsigned int *>(buf));
        f.check ();

        if ((errcode = test_nsw_common_decoder_event (f, params, statistics)) != ERR_NOERR)
        {
          ers::error (ers::File (ERS_HERE, data_file_name.c_str ()));
          if (buf) delete [] buf;
          continue;
        }

        ++statistics.nevents;
      }

      catch (ers::Issue &ex)
      {
        ers::error (ers::File (ERS_HERE, data_file_name.c_str (), ex));
        errcode = ERR_GENERIC;
        if (buf) delete [] buf;
        break;
      }

      if (buf) delete [] buf;
    }
  }

  return errcode;
}

int main (int argc, char **argv)
{
  int err = ERR_NOERR;
  Params params;
  Statistics statistics;

  // Global statistics

  if ((err = test_nsw_common_decoder_opt (argc, argv, params)) != ERR_NOERR)
    return err;

  if ((err = test_nsw_common_decoder_init ()) != ERR_NOERR)
    return err;
  //coverity[TAINTED_SCALAR]
  if ((err = test_nsw_common_decoder_loop (params, statistics)) != ERR_NOERR)
    return err;

  if ((err = test_nsw_common_decoder_end (statistics)) != ERR_NOERR)
    return err;

  return err;
}
