# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os

from AthenaCommon.Logging import logging
log = logging.getLogger('IOVDbAutoCfgFlags')

def getLastGlobalTag(prevFlags):
    if not prevFlags.Input.Files:
        return ""

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    globaltag = GetFileMD(prevFlags.Input.Files).get("IOVDbGlobalTag", None)
    if globaltag is None:
        return ""

    if isinstance(globaltag, list):  # if different tags have been used at different steps
        globaltag = globaltag[-1]


    #With the migration to crest, the global tag name also changed,
    #even for tags that are straight copies from COOL. Adjust global
    #tag names read from input metadata
    if prevFlags.IOVDb.UseCREST and globaltag.startswith("OFL"):
        globaltag=globaltag[3:]
        
    return globaltag


def getDatabaseInstanceDefault(flags):
    # MC
    if flags.Input.isMC:
        return "OFLP200"

    # real-data
    try:
        year = int(flags.Input.ProjectName[4:6])
    except Exception:
        log.warning("Failed to extract year from project tag %s. Assuming CONDBR2.", flags.Input.ProjectName)
        return "CONDBR2"

    if year > 13:
        return "CONDBR2"
    else:
        return "COMP200"


def getCrestServer():
    """Return default Crest server URL (can be set via ${CREST_SERVER})"""
    from urllib.parse import urlsplit

    url = os.getenv("CREST_SERVER", "").strip()
    if not url:
        return "https://crest.cern.ch"

    # Ensure we have a protocol, default to https
    if not urlsplit(url).scheme:
        url = "https://" + url.lstrip("/")

    return url


def getCrestAPI():
    """Return default Crest API version"""
    return "api-v6.0"


def getCrestConnection(server=None, api=None):
    """Build connection string for Crest server based on server name and api.
    If server is a valid file name, return that instead."""

    from urllib.parse import urljoin, urlsplit

    # Split URL assuming default protocol
    url = urlsplit(server or getCrestServer(), scheme='https')

    # Valid URL with server name
    if url.netloc:
        return urljoin(url.geturl(), api or getCrestAPI())

    # Or check if there is a local file
    if os.access(url.path, os.F_OK):
        return url.path

    raise RuntimeError(f"Invalid URL or non-existent file: {server}")


#
# Unit tests:
#
if __name__ == '__main__':
    import tempfile
    import unittest
    from unittest.mock import patch

    class Test(unittest.TestCase):

        def test_no_envvar(self):
            with patch.dict(os.environ):
                os.environ.pop("CREST_SERVER", None)
                self.assertEqual(getCrestServer(), "https://crest.cern.ch")

        @patch.dict(os.environ, {"CREST_SERVER": ""})
        def test_empty(self):
            self.assertEqual(getCrestServer(), "https://crest.cern.ch")
            self.assertEqual(getCrestConnection(), "https://crest.cern.ch/" + getCrestAPI())

        @patch.dict(os.environ, {"CREST_SERVER": "myserver.com"})
        def test_no_protocol(self):
            self.assertEqual(getCrestServer(), "https://myserver.com")
            self.assertEqual(getCrestConnection(), "https://myserver.com/" + getCrestAPI())

        @patch.dict(os.environ, {"CREST_SERVER": "http://myserver.com/"})
        def test_with_protocol(self):
            self.assertEqual(getCrestServer(), "http://myserver.com/")
            self.assertEqual(getCrestConnection(), "http://myserver.com/" + getCrestAPI())

        def test_file_nonexistent(self):
            with self.assertRaises(RuntimeError):
                getCrestConnection("mydb.txt")

        def test_file_exists(self):
            with tempfile.NamedTemporaryFile() as f:
                self.assertEqual(getCrestConnection(f.name), f.name)

    unittest.main()
