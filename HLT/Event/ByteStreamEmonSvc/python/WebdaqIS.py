#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
"""Read IS objects via the TDAQ webproxy (webdaq REST API), without TDAQ python modules.

Python equivalent of webdaq::is::get(). The webproxy is taken from $TDAQ_WEBDAQ_BASE.

  from ByteStreamEmonSvc.WebdaqIS import readISObject
  runparams = readISObject("ATLAS", "RunParams.RunParams")
  if runparams:
      run_number = runparams["run_number"]
"""

import os
import requests
from urllib.parse import quote

from AthenaCommon.Logging import logging
log = logging.getLogger("WebdaqIS")


def readISObject(partition, name, base=None, timeout=10):
    """Read the IS object 'server.name' (e.g. 'RunParams.RunParams') from partition.

    Returns the attributes as dictionary, or None if the object (or the
    webproxy) is not available.
    """
    base = base or os.getenv("TDAQ_WEBDAQ_BASE")
    if not base:
        log.warning("TDAQ_WEBDAQ_BASE not set, cannot read %s", name)
        return None

    server = name.split(".", 1)[0]
    url = f"{base.rstrip('/')}/info/current/{quote(partition)}/is/{quote(server)}/{quote(name)}"
    try:
        r = requests.get(url, params={"format": "compact"}, timeout=timeout)
        r.raise_for_status()
        # compact format: [name, type, time, attributes]
        return r.json()[3]
    except (requests.RequestException, ValueError, IndexError, KeyError) as e:
        log.warning("Cannot read %s from partition %s: %s", name, partition, e)
        return None
