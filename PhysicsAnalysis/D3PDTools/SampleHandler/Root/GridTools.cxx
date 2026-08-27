/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


//
// includes
//

#include <SampleHandler/GridTools.h>

#include <AsgMessaging/MessageCheck.h>
#include <RootCoreUtils/Assert.h>
#include <RootCoreUtils/ShellExec.h>
#include <RootCoreUtils/StringUtil.h>
#include <SampleHandler/MetaObject.h>
#include <CxxUtils/checker_macros.h>
#include <TSystem.h>
#include <chrono>
#include <fstream>
#include <mutex>
#include <stdexcept>

namespace sh = RCU::Shell;

//
// method implementations
//

namespace SH
{
  ANA_MSG_SOURCE (msgGridTools, "SampleHandler_GridTools")
  using namespace msgGridTools;

  namespace
  {
    struct ProxyData
    {
      // the clock we use
      using clock = std::chrono::steady_clock;

      // don't really need a mutex as the code unlikely to be
      // multi-threaded, but may just as well put one to protect the
      // global/static variable
      std::recursive_mutex mutex;

      // whether we have confirmed that we do have a proxy
      bool haveProxy = false;

      // the expiration time of the proxy (if we have one)
      clock::time_point proxyExpiration;

      bool checkVomsProxy ()
      {
	std::lock_guard<std::recursive_mutex> lock (mutex);

	if (haveProxy == false)
	{
	  ANA_MSG_INFO ("checking for valid grid proxy");
	  int rc = 0;
	  std::string output =
	    RCU::Shell::exec_read ("voms-proxy-info --actimeleft", rc);
	  if (rc != 0)
	  {
	    ANA_MSG_INFO ("no valid proxy found");
	  } else
	  {
	    std::istringstream str (output);
	    unsigned seconds = 0;

	    if (!(str >> seconds))
	    {
          // Output format is more complicated if RPM isn't installed
          std::istringstream str2 (output.substr(output.rfind('\n',output.size()-2)+1,std::string::npos));

          if (!(str2 >> seconds)){
            ANA_MSG_INFO ("failed to parse command output: " << output);
          } else
          {
            proxyExpiration = clock::now() + std::chrono::seconds (seconds);
            haveProxy = true;
          } // Second try was successful

	    } else
	    {
	      proxyExpiration = clock::now() + std::chrono::seconds (seconds);
	      haveProxy = true;
	    } // First try was successful
	  }
	}

	return haveProxy &&
	  proxyExpiration > clock::now() + std::chrono::minutes (20);
      }

      void ensureVomsProxy (unsigned tries = 0)
      {
	std::lock_guard<std::recursive_mutex> lock (mutex);

	if (checkVomsProxy())
	  return;

	// rationale: cap the number of retries so that we do not loop
	//   forever if voms-proxy-init keeps succeeding but the
	//   resulting proxy stays too short-lived or unparseable.
	if (tries >= 3)
	  throw std::runtime_error ("failed to obtain a valid grid proxy after several attempts");

	if (haveProxy)
	{
	  ANA_MSG_INFO ("proxy expired or about to expire");
	} else
	{
	  ANA_MSG_INFO ("no proxy found");
	}
	ANA_MSG_INFO ("trying to set up a new proxy");
	haveProxy = false;
	RCU::Shell::exec ("voms-proxy-init -voms atlas");
	ensureVomsProxy (tries + 1);
      }
    };

    ProxyData& proxyData ()
    {
      // Methods of ProxyData() are thread-safe.
      static ProxyData result ATLAS_THREAD_SAFE;
      return result;
    }



    /// \brief read all lines beginning with a specific phrase
    /// (without the phrase itself)
    std::vector<std::string>
    readLineList (const std::string& text,
                  const std::string& begin)
    {
      std::vector<std::string> result;

      for (std::string::size_type split = 0;
           (split = text.find (begin, split)) != std::string::npos;
           ++ split)
      {
        if (split == 0 || text[split-1] == '\n')
        {
          split += begin.size();
          auto split2 = text.find ("\n", split);
          if (split2 == std::string::npos)
            split2 = text.size();
          std::string subresult = text.substr (split, split2 - split);
          // rationale: strip surrounding whitespace in O(n).  guard
          //   against an empty/all-whitespace value (front()/back() on
          //   an empty string is UB) and use find_first/last_not_of
          //   rather than isspace on a possibly-negative char.
          const char *const whitespace = " \t\n\r\f\v";
          const auto first = subresult.find_first_not_of (whitespace);
          if (first == std::string::npos)
            subresult.clear ();
          else
          {
            const auto last = subresult.find_last_not_of (whitespace);
            subresult = subresult.substr (first, last - first + 1);
          }
          result.push_back (std::move (subresult));
        }
      }
      return result;
    }



    /// \brief read the line beginning with a specific phrase (without
    /// the phrase itself)
    std::string readLine (const std::string& text,
                          const std::string& begin)
    {
      auto lines = readLineList (text, begin);
      if (lines.empty())
        throw std::runtime_error ("failed to find line starting with: " + begin);
      if (lines.size() > 1)
        throw std::runtime_error ("multiple lines starting with: " + begin);
      return lines.at(0);
    }



    /// \brief read the line beginning with a specific phrase (without
    /// the phrase itself)
    unsigned readLineUnsigned (const std::string& text,
                               const std::string& begin)
    {
      const auto line = readLine (text, begin);
      std::istringstream str (line);
      unsigned result = 0;
      if (!(str >> result) || !str.eof())
        throw std::runtime_error ("failed to convert " + line + " into an unsigned");
      return result;
    }



    /// \brief the command for setting up rucio
    std::string rucioSetupCommand ()
    {
      return "source $ATLAS_LOCAL_ROOT_BASE/user/atlasLocalSetup.sh -q && lsetup --force 'rucio -w'";
    }
  }



  const std::string& downloadStageEnvVar ()
  {
    static const std::string result = "SAMPLEHANDLER_RUCIO_DOWNLOAD";
    return result;
  }



  bool checkVomsProxy ()
  {
    return proxyData().checkVomsProxy();
  }



  void ensureVomsProxy ()
  {
    proxyData().ensureVomsProxy();
  }



  std::vector<std::string>
  faxListFilesGlob (const std::string& name, const std::string& filter)
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    return faxListFilesRegex (name, RCU::glob_to_regexp (filter));
#pragma GCC diagnostic pop
  }



  std::vector<std::string>
  faxListFilesRegex (const std::string& name, const std::string& filter)
  {
    RCU_REQUIRE_SOFT (!name.empty());
    RCU_REQUIRE_SOFT (name.find('*') == std::string::npos);
    RCU_REQUIRE_SOFT (!filter.empty());

    ensureVomsProxy ();

    static const std::string separator = "------- SampleHandler Split -------";
    std::vector<std::string> result;

    ANA_MSG_INFO ("querying FAX for dataset " << name);
    std::string output = sh::exec_read ("source $ATLAS_LOCAL_ROOT_BASE/user/atlasLocalSetup.sh -q && lsetup --force fax && echo " + separator + " && fax-get-gLFNs " + sh::quote (name));
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);

    std::istringstream str (output.substr (split + separator.size() + 1));
    std::regex pattern (filter);
    std::string line;
    while (std::getline (str, line))
    {
      if (!line.empty())
      {
	if (!line.starts_with ("root:"))
	  throw std::runtime_error ("faxListFilesRegex: couldn't parse line: " + line);

	std::string::size_type split1 = line.rfind (":");
	std::string::size_type split2 = line.rfind ("/");
	if (split1 < split2)
	  split1 = split2;
	if (split1 != std::string::npos)
	{
	  if (RCU::match_expr (pattern, line.substr (split1+1)))
	    result.push_back (line);
	} else
	  throw std::runtime_error ("faxListFilesRegex: couldn't parse line: " + line);
      }
    }
    if (result.size() == 0)
      ANA_MSG_WARNING ("dataset " << name << " did not contain any files.  this is likely not right");
    return result;
  }



  std::vector<std::string>
  rucioDirectAccessGlob (const std::string& name, const std::string& filter,
                         const std::string& selectOptions)
  {
    return rucioDirectAccessRegex (name, RCU::glob_to_regexp (filter),
                                   selectOptions);
  }



  std::vector<std::string>
  rucioDirectAccessRegex (const std::string& name, const std::string& filter,
                          const std::string& selectOptions)
  {
    RCU_REQUIRE_SOFT (!name.empty());
    RCU_REQUIRE_SOFT (name.find('*') == std::string::npos);
    RCU_REQUIRE_SOFT (!filter.empty());

    ensureVomsProxy ();

    static const std::string separator = "------- SampleHandler Split -------";

    ANA_MSG_INFO ("querying rucio for dataset " << name);
    std::string output = sh::exec_read (rucioSetupCommand() + " && echo " + separator + " && rucio list-file-replicas --pfns --protocols root " + selectOptions + " " + sh::quote (name));
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);
    std::istringstream str (output.substr (split + separator.size() + 1));

    // this is used to avoid getting two copies of the same file.  we
    // first fill them in a map by filename, then copy them into a
    // vector
    std::map<std::string,std::string> resultMap;

    std::regex urlPattern ("^root://.*");
    std::regex pattern (filter);
    std::string line;
    while (std::getline (str, line))
    {
      if (line.empty())
      {
        // no-op
      } else if (!RCU::match_expr (urlPattern, line))
      {
        ANA_MSG_INFO ("couldn't handle line: " << line);
      } else
      {
	std::string::size_type split = line.rfind ("/");
	if (split != std::string::npos)
	{
          std::string filename = line.substr (split+1);
	  if (RCU::match_expr (pattern, filename))
	    resultMap[filename] = line;
	} else
	  throw std::runtime_error ("rucioDirectAccessRegex: couldn't parse line: " + line);
      }
    }

    std::vector<std::string> result;
    for (const auto& file : resultMap)
      result.push_back (file.second);
    if (result.size() == 0)
      ANA_MSG_WARNING ("dataset " + name + " did not contain any files.  this is likely not right");
    return result;
  }



  std::vector<RucioListDidsEntry> rucioListDids (const std::string& dataset)
  {
    RCU_REQUIRE_SOFT (!dataset.empty());

    ensureVomsProxy ();

    static const std::string separator = "------- SampleHandler Split -------";
    std::vector<RucioListDidsEntry> result;

    ANA_MSG_INFO ("querying rucio for dataset " << dataset);
    std::string output = sh::exec_read (rucioSetupCommand() + " && echo " + separator + " && rucio list-dids " + sh::quote (dataset));
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);

    std::istringstream str (output.substr (split + separator.size() + 1));
    std::regex pattern ("^\\| ([a-zA-Z0-9_.-]+):([a-zA-Z0-9_.-]+) +\\| ([a-zA-Z0-9_.-]+) +\\| *$");
    std::string line;
    while (std::getline (str, line))
    {
      std::smatch what;
      if (std::regex_match (line, what, pattern))
      {
	RucioListDidsEntry entry;
	entry.scope = what[1];
	entry.name = what[2];
	entry.type = what[3];
	result.push_back (entry);
      }
    }
    return result;
  }



  std::vector<RucioListFileReplicasEntry>
  rucioListFileReplicas (const std::string& dataset)
  {
    RCU_REQUIRE_SOFT (!dataset.empty());

    ensureVomsProxy ();

    static const std::string separator = "------- SampleHandler Split -------";
    std::vector<RucioListFileReplicasEntry> result;

    std::string command = rucioSetupCommand() + " && echo " + separator + " && rucio list-file-replicas --protocols root " + sh::quote (dataset);

    ANA_MSG_INFO ("querying rucio for dataset " << dataset);
    std::string output = sh::exec_read ( command );
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);

    std::istringstream str (output.substr (split + separator.size() + 1));
    std::regex pattern ("^\\| +([^ ]+) +\\| +([^ ]+) +\\| +([^ ]+ [^ ]+) +\\| +([^ ]+) +\\| +([^: ]+): ([^ ]+) +\\| *$");
    std::string line;
    while (std::getline (str, line))
    {
      std::smatch what;
      if (std::regex_match (line, what, pattern) &&
          what[1] != "SCOPE")
      {
	RucioListFileReplicasEntry entry;
	entry.scope    = what[1];
	entry.name     = what[2];
	entry.filesize = what[3];
	entry.adler32  = what[4];
	entry.disk     = what[5];
	entry.replica  = what[6];
	result.push_back (entry);
      }
    }
    return result;
  }



  std::map<std::string,std::unique_ptr<MetaObject> >
  rucioGetMetadata (const std::set<std::string>& datasets)
  {
    RCU_REQUIRE_SOFT (!datasets.empty());

    ensureVomsProxy ();

    static const std::string separator = "------- SampleHandler Split -------";
    std::map<std::string,std::unique_ptr<MetaObject> > result;

    std::string command = rucioSetupCommand() + " && echo " + separator + " && rucio get-metadata";
    for (auto& dataset : datasets)
    {
      RCU_REQUIRE_SOFT (!dataset.empty());
      command += " " + sh::quote (dataset);
    }

    ANA_MSG_INFO ("querying rucio for meta-data");
    std::string output = sh::exec_read (command);
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);

    std::istringstream str (output.substr (split + separator.size() + 1));
    std::regex pattern ("^([^:]+): *(.+)$");
    std::string line;
    auto meta = std::make_unique<MetaObject>();

    auto addMeta = [&] ()
    {
      std::string name = meta->castString ("scope") + ":" + meta->castString ("name");
      if (result.find (name) != result.end())
        throw std::runtime_error ("rucioGetMetadata: read " + name + " twice");
      result[name] = std::move (meta);
    };

    while (std::getline (str, line))
    {
      std::smatch what;
      if (line == "------")
      {
        addMeta ();
        meta = std::make_unique<MetaObject>();
      } else  if (std::regex_match (line, what, pattern))
      {
	if (meta->get (what[1]))
          throw std::runtime_error (std::string("duplicate entry: ") + what[1].str());
	meta->setString (what[1], what[2]);
      } else if (!line.empty())
      {
	ANA_MSG_WARNING ("couldn't parse line: " << line);
      }
    }
    addMeta ();

    for (auto& subresult : result)
    {
      if (datasets.find (subresult.first) == datasets.end())
        throw std::runtime_error ("received result for dataset not requested: " + subresult.first);
    }
    for (auto& dataset : datasets)
    {
      if (result.find (dataset) == result.end())
        throw std::runtime_error ("received no result for dataset: " + dataset);
    }

    return result;
  }



  RucioDownloadResult rucioDownload (const std::string& location,
                                     const std::string& dataset)
  {
    ensureVomsProxy ();
    
    const std::string separator = "------- SampleHandler Split -------";
    std::string command = rucioSetupCommand() + " && echo " + separator + " && cd " + sh::quote (location) + " && rucio download " + sh::quote (dataset) + " 2>&1";

    ANA_MSG_INFO ("starting rucio download " + dataset + " into " + location);
    std::string output = sh::exec_read (command);
    auto split = output.rfind (separator + "\n");
    if (split == std::string::npos)
      throw std::runtime_error ("couldn't find separator in: " + output);
    output = output.substr (split + separator.size() + 1);

    RucioDownloadResult result;
    result.did = readLine (output, "DID ");
    result.totalFiles = readLineUnsigned (output, "Total files (DID): ");
    result.downloadedFiles = readLineUnsigned (output, "Downloaded files: ");
    result.alreadyLocal = readLineUnsigned (output, "Files already found locally: ");
    result.notDownloaded = readLineUnsigned (output, "Files that cannot be downloaded: ");
    return result;
  }



  std::vector<RucioDownloadResult>
  rucioDownloadList (const std::string& location,
                     const std::vector<std::string>& datasets)
  {
    std::vector<RucioDownloadResult> result;
    for (auto& dataset : datasets)
      result.push_back (rucioDownload (location, dataset));
    return result;
  }



  std::vector<std::string>
  rucioCacheDatasetGlob (const std::string& location,
                         const std::string& dataset,
                         const std::string& fileGlob)
  {
    std::vector<std::string> result;

    std::string path = location;
    if (path.empty() || path.back() != '/')
      path += "/";
    if (dataset.find (':') != std::string::npos)
      path += dataset.substr (dataset.find (':')+1);
    else
      path += dataset;
    const std::string finished {
      path + "-finished"};

    // check if the finished file does not exist
    // note that AccessPathName has the weirdest calling convention
    //
    // rationale: this check-then-download is not safe against two jobs
    //   caching the same dataset into the same directory concurrently
    //   (they can both see the marker missing and download at the same
    //   time); guarding that properly would need an exclusive lock on
    //   the directory.  we do at least check that the marker file was
    //   created, so an unwritable directory fails loudly instead of
    //   silently re-downloading on every call.
    if (gSystem->AccessPathName (finished.c_str()) != 0)
    {
      RucioDownloadResult status = rucioDownload (location, dataset);
      if (status.downloadedFiles + status.alreadyLocal < status.totalFiles)
        throw std::runtime_error ("failed to download all files of " + dataset);
      //  this just creates an empty file
      std::ofstream finishedFile (finished.c_str());
      if (!finishedFile)
        throw std::runtime_error ("failed to create marker file: " + finished);
    }

    std::string output = sh::exec_read ("find " + sh::quote (path) + " -type f -name " + sh::quote (fileGlob));
    std::istringstream str (output);
    std::string line;
    while (std::getline (str, line))
    {
      if (!line.empty())
        result.push_back (line);
    }
    return result;
  }
}
