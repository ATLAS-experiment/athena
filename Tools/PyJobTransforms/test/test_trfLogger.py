#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @Package test_trfLogger.py
#  @brief Unittests for trfLogger.py
#  @author Frank Winklmeier

import unittest


class trfLoggerTests(unittest.TestCase):

    def test_noDuplication(self):
        import contextlib
        import io

        # Capture stdout
        with contextlib.redirect_stdout(io.StringIO()) as output:
            from PyJobTransforms.trfLogger import msg
            msg.info("Hello world")
        output = output.getvalue().splitlines()

        # Ensure there is no output duplication, which can occur if the
        # AthenaCommon.Logging module is imported too early.
        self.assertEqual(len(output), 1)

        self.assertTrue(output[0].endswith("Hello world"))


if __name__ == '__main__':
    unittest.main()
