#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Tests for AthRemoteExecCore.RemoteExecFragments machinery"""

import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthRemoteExecCore.RemoteExecCodecs import Codec
from AthRemoteExecCore.RemoteExecFragments import RemoteExecTop
from test_utils import INTS, test_IntsCodec


def _algsToCA(*algorithms) -> ComponentAccumulator:
    """Convenience function that wraps list of individual algorithms into ComponentAccumulator"""

    cfg = ComponentAccumulator()

    for algorithm in algorithms:
        cfg.addEventAlgo(algorithm)

    return cfg


def _flags():
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.lock()
    return flags


def _sum(name, a, b, out):
    return CompFactory.AthRemoteExec.test_SumAlg(name, IntA=a, IntB=b, IntSum=out)


def _ints(*keys: str) -> dict[str, Codec]:
    return {key: test_IntsCodec([key]) for key in keys}


def _keys(declarations):
    return [key for key, _, _ in declarations]


class TestDerivation(unittest.TestCase):
    """Test whether RemoteExecTop.add properly resolves boundaries"""

    def setUp(self):
        self.flags = _flags()
        self.menu = RemoteExecTop()

    def tearDown(self):
        cfg = self.menu.build(self.flags)
        for name in self.menu.properties["Sequences"]:
            self.assertIsNotNone(
                cfg.getSequence(name), f"{name} was advertised but not built"
            )
        cfg.wasMerged()

    def test_menu_add(self):
        fragment = self.menu.add(
            "RemoteExecSeqT1",
            _algsToCA(_sum("T1Alg", "a", "b", "sum")),
            outputs=["sum"],
            boundaries=_ints("a", "b", "sum"),
        )

        self.assertEqual(_keys(fragment.inputs), ["a", "b"])
        self.assertEqual(_keys(fragment.outputs), ["sum"])
        self.assertEqual(
            fragment.input_specs, [f"a#protobuf#{INTS}", f"b#protobuf#{INTS}"]
        )
        self.assertEqual(fragment.output_specs, [f"sum#protobuf#{INTS}"])

        inputCnvAlg = fragment.inputCnvAlg("RemoteExecRequest")
        outputCnvAlg = fragment.outputCnvAlg("RemoteExecRequest")
        self.assertFalse(getattr(inputCnvAlg, "ExtraOutputs", None))
        self.assertEqual(
            [tool.Direction for tool in inputCnvAlg.Decoders],
            ["WriteToSG", "WriteToSG"],
        )
        self.assertTrue(outputCnvAlg is not None)
        self.assertEqual(
            [tool.Direction for tool in outputCnvAlg.Encoders],
            ["ReadFromSG"],
        )

    def test_alg_sequence_one_output(self):
        """For multiple algorithms check if intemediate keys are not seen as inputs or outputs"""

        fragment = self.menu.add(
            "RemoteExecSeqT2",
            _algsToCA(
                _sum("T2First", "p", "q", "pq"), _sum("T2Second", "pq", "r", "chain")
            ),
            outputs=["chain"],
            boundaries=_ints("p", "q", "r", "chain"),
        )

        self.assertEqual(_keys(fragment.inputs), ["p", "q", "r"])
        self.assertNotIn("pq", _keys(fragment.inputs))
        self.assertEqual(_keys(fragment.outputs), ["chain"])

    def test_alg_sequence_two_outputs(self):
        """For multiple algorithms check if we can declare intermediate as an output"""

        fragment = self.menu.add(
            "RemoteExecSeqT2",
            _algsToCA(
                _sum("T2First", "p", "q", "pq"), _sum("T2Second", "pq", "r", "chain")
            ),
            outputs=["chain", "pq"],
            boundaries=_ints("p", "q", "r", "pq", "chain"),
        )

        self.assertEqual(_keys(fragment.inputs), ["p", "q", "r"])
        self.assertEqual(_keys(fragment.outputs), ["chain", "pq"])

    def test_ignore_inputs(self):
        """Test if ignore_inputs works"""

        fragment = self.menu.add(
            "RemoteExecSeqTIgnore",
            _algsToCA(_sum("TIgnoreAlg", "a", "b", "sum")),
            outputs=["sum"],
            ignore_inputs=["b"],
            boundaries=_ints("a", "sum"),
        )
        self.assertEqual(_keys(fragment.inputs), ["a"])

    def test_nonexistent_ignored_inputs(self):
        """Non-existent inputs declared as ignored_inputs are treated as defined"""

        with self.assertRaises(ValueError) as caught:
            self.menu.add(
                "RemoteExecSeqTStale",
                payload := _algsToCA(_sum("TStaleAlg", "a", "b", "sum")),
                outputs=["sum"],
                ignore_inputs=["sum"],
                boundaries=_ints("a", "b", "sum"),
            )

        self.assertEqual("Unmet inputs found: ['sum']", str(caught.exception))

        # Cleanup
        payload.wasMerged()

    def test_missing_input_boundary(self):
        """Check if all inputs have their boundaries defined"""

        with self.assertRaises(ValueError) as caught:
            self.menu.add(
                "RemoteExecSeqTIgnoreBad",
                payload := _algsToCA(_sum("TIgnoreBadAlg", "a", "b", "sum")),
                outputs=["sum"],
                boundaries=_ints("a", "sum"),
            )

        self.assertEqual(
            "Fragment 'RemoteExecSeqTIgnoreBad' does not say how ['b'] are (de)serialised: give each a Codec in boundaries={}",
            str(caught.exception),
        )

        # Cleanup
        payload.wasMerged()

    def test_unused_boundaries(self):
        """Adding extra unused boundaries is not accepted"""

        with self.assertRaises(ValueError) as caught:
            self.menu.add(
                "RemoteExecSeqTStrayBoundary",
                payload := _algsToCA(_sum("TStrayAlg", "a", "b", "sum")),
                outputs=["sum"],
                boundaries=_ints("a", "b", "sum", "nonesuch"),
            )

        self.assertEqual(
            "Fragment 'RemoteExecSeqTStrayBoundary' declares unused boundaries for ['nonesuch']; it crosses ['a', 'b', 'sum']",
            str(caught.exception),
        )

        # Cleanup
        payload.wasMerged()

    def test_carry_multiple_objects(self):
        """Check if we can carry multiple objects in a single message"""

        fragment = self.menu.add(
            "RemoteExecSeqT4",
            _algsToCA(_sum("T4Alg", "a", "b", "sum")),
            outputs=["total"],
            boundaries={
                "addends": test_IntsCodec(["a", "b"]),
                "total": test_IntsCodec(["sum"]),
            },
        )

        self.assertEqual(_keys(fragment.inputs), ["addends"])
        self.assertEqual(fragment.input_specs, [f"addends#protobuf#{INTS}"])
        self.assertEqual(_keys(fragment.outputs), ["total"])
        self.assertEqual(fragment.output_specs, [f"total#protobuf#{INTS}"])

    def test_output_boundary_mismatch(self):
        """Mismatch between what Codec expects and what remoteCfg produces is not allowed"""

        with self.assertRaises(ValueError) as caught:
            self.menu.add(
                "RemoteExecSeqT4b",
                payload := _algsToCA(_sum("T4bAlg", "a", "b", "sum")),
                outputs=["total"],
                boundaries={
                    "addends": test_IntsCodec(["a", "b"]),
                    "total": test_IntsCodec(["nonesuch"]),
                },
            )

        self.assertEqual(
            "The remoteCfg does not produce keys that Codec expects: ['total']. It writes ['sum']",
            str(caught.exception),
        )

        # Cleanup
        payload.wasMerged()

    def test_input_assetion(self):
        """Check if input= works as an assertion"""

        with self.assertRaises(ValueError) as caught:
            self.menu.add(
                "RemoteExecSeqT7",
                payload := _algsToCA(_sum("T7Alg", "a", "b", "sum")),
                outputs=["sum"],
                inputs=["a"],
                boundaries=_ints("a", "b", "sum"),
            )

        self.assertEqual(
            "Fragment 'RemoteExecSeqT7' declares inputs ['a'] but its remoteCfg leaves ['a', 'b'] unmet",
            str(caught.exception),
        )

        # Cleanup
        payload.wasMerged()

    def test_properties(self):
        """Test properties for EventLoopMgr"""

        self.menu.add(
            "RemoteExecSeqT8",
            _algsToCA(_sum("T8Alg", "a", "b", "sum")),
            outputs=["sum"],
            boundaries=_ints("a", "b", "sum"),
        )
        props = self.menu.properties

        self.assertEqual(props["Sequences"], ["RemoteExecSeqT8"])
        self.assertEqual(props["RequestKey"], self.menu.request_key)
        self.assertEqual(props["ReplyKeys"], {"RemoteExecSeqT8": "RemoteExecReplyT8"})
        self.assertEqual(
            props["Decoders"],
            {"RemoteExecSeqT8": ["a#protobuf#test_ints", "b#protobuf#test_ints"]},
        )
        self.assertEqual(
            props["Encoders"], {"RemoteExecSeqT8": ["sum#protobuf#test_ints"]}
        )


if __name__ == "__main__":
    unittest.main()
