#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Does a fragment's boundary really fall out of its payload?

RemoteExecMenu derives the wire inputs and every boundary's StoreGate type from the
payload's own handles, which removes most of a fragment declaration -- and moves
the failure mode. A wrong derivation would not be a configuration error; it
would be a server quietly asking clients for the wrong thing. So each rule is
checked against a payload built for the purpose, including the cases where the
right answer is to refuse rather than to guess.

The one thing deliberately *not* derived is the wire contract itself. A
boundary's encoding and schema are stated, and a fragment that forgets to state
one must fail loudly here rather than fall back to a default -- there is no
default, and the tests below say so.
"""

import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthExRemoteExec.RemoteExecDemoCodecs import DOUBLES, INTS, DemoDoubles, DemoInts
from AthExRemoteExec.RemoteExecFragments import RemoteExecMenu, RemoteExecPayloadCfg


def _flags():
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.lock()
    return flags


def _sum(name, a, b, out):
    return CompFactory.AthExRemoteExec.SumAlg(name, A=a, B=b, Sum=out)


def _ints(*keys):
    """A codec for each of `keys`, each carrying just its own key.

    The simple case, where a boundary is one StoreGate object and its name and
    that object's key are the same string. Most codecs are only ever this.
    """
    return {key: DemoInts([key]) for key in keys}


def _keys(declarations):
    return [key for key, _, _ in declarations]


class TestDerivation(unittest.TestCase):

    def setUp(self):
        self.flags = _flags()
        self.menu = RemoteExecMenu()

    def test_unmet_reads_become_inputs(self):
        """What the payload reads but does not write is what the client sends."""
        fragment = self.menu.add("RemoteExecSeqT1",
                                 RemoteExecPayloadCfg(_sum("T1Alg", "a", "b", "sum")),
                                 outputs=["sum"],
                                 boundaries=_ints("a", "b", "sum"))
        self.assertEqual(fragment.input_specs,
                         [f"a#protobuf#{INTS}", f"b#protobuf#{INTS}"])
        self.assertEqual(fragment.output_specs, [f"sum#protobuf#{INTS}"])

    def test_internal_intermediates_are_not_boundaries(self):
        """A key both written and read inside the fragment never crosses."""
        fragment = self.menu.add(
            "RemoteExecSeqT2",
            RemoteExecPayloadCfg(_sum("T2First", "p", "q", "pq"),
                          _sum("T2Second", "pq", "r", "chain")),
            outputs=["chain"],
            boundaries=_ints("p", "q", "r", "chain"))
        self.assertEqual(_keys(fragment.inputs), ["p", "q", "r"])
        self.assertNotIn("pq", _keys(fragment.inputs))
        # 'pq' is produced, so it *could* be returned; it simply was not asked
        # for. That is the choice the outputs list exists to make.
        self.assertEqual(_keys(fragment.outputs), ["chain"])

    def test_conditions_reads_are_not_client_inputs(self):
        """A ReadCondHandle is served by AthCondSeq, not by the wire."""
        payload = ComponentAccumulator()
        payload.addEventAlgo(CompFactory.AthExRemoteExec.OffsetAlg(
            "T3Alg", Input="x", Offset="RemoteExecCondOffset", Output="offsetted"))
        payload.addCondAlgo(CompFactory.AthExRemoteExec.RemoteExecCondAlg(
            "T3CondAlg", Offset="RemoteExecCondOffset"))
        fragment = self.menu.add("RemoteExecSeqT3", payload, outputs=["offsetted"],
                                 boundaries=_ints("x", "offsetted"))
        self.assertEqual(_keys(fragment.inputs), ["x"])

    def test_a_conditions_folder_key_is_not_a_client_input(self):
        """A COOL folder key carries no store prefix, and is still conditions.

        The prefix test alone said otherwise. A ReadCondHandleKey set to
        '/Indet/Align' looks exactly like an unmet event-store read, so the
        alignment became a wire input -- and the job then refused to start,
        because '/' is not allowed in a StoreGateSvc key. Nothing is produced
        for this key here on purpose: an unmet conditions read must be left
        alone rather than charged to the client.
        """
        payload = ComponentAccumulator()
        payload.addEventAlgo(CompFactory.AthExRemoteExec.OffsetAlg(
            "TCondFolderAlg", Input="x", Offset="/RemoteExec/Offset",
            Output="offsetted"))
        fragment = self.menu.add("RemoteExecSeqTCondFolder", payload,
                                 outputs=["offsetted"],
                                 boundaries=_ints("x", "offsetted"))
        self.assertEqual(_keys(fragment.inputs), ["x"])

    def test_an_optional_read_can_be_exempted(self):
        """ignore_inputs takes an unmet read off the client's bill."""
        fragment = self.menu.add(
            "RemoteExecSeqTIgnore",
            RemoteExecPayloadCfg(_sum("TIgnoreAlg", "a", "b", "sum")),
            outputs=["sum"], ignore_inputs=["b"],
            boundaries=_ints("a", "sum"))
        self.assertEqual(_keys(fragment.inputs), ["a"])

    def test_an_exemption_that_hides_nothing_is_refused(self):
        """An exemption for a key that is not unmet is stale, and says so.

        Otherwise a payload could stop reading the optional key -- or start
        producing it -- and the exemption would sit there indefinitely,
        suppressing nothing and documenting a dependency that no longer exists.
        """
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqTStale",
                          RemoteExecPayloadCfg(_sum("TStaleAlg", "a", "b", "sum")),
                          outputs=["sum"], ignore_inputs=["sum"],
                          boundaries=_ints("a", "b", "sum"))
        self.assertIn("sum", str(caught.exception))

    def test_a_boundary_without_a_codec_is_refused(self):
        """There is no default wire format, and no guessing at one.

        A boundary whose serialisation is inferred from a C++ type is a
        boundary whose serialisation changes when somebody edits a handle --
        silently, and on the wire.
        """
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqTNoEncoding",
                          RemoteExecPayloadCfg(_sum("TNoEncodingAlg", "a", "b", "sum")),
                          outputs=["sum"], boundaries=_ints("a", "sum"))
        self.assertIn("b", str(caught.exception))

    def test_a_boundary_for_a_key_that_does_not_cross_is_refused(self):
        """A declaration for a key nobody sends is a half-finished rename."""
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqTStrayBoundary",
                          RemoteExecPayloadCfg(_sum("TStrayAlg", "a", "b", "sum")),
                          outputs=["sum"],
                          boundaries=_ints("a", "b", "sum", "nonesuch"))
        self.assertIn("nonesuch", str(caught.exception))

    def test_a_converting_boundary_carries_keys_of_another_name(self):
        """One message, several objects, and the wire name is none of them.

        The awkward case, and the only one: a codec that converts rather than
        carries has a boundary name that is not a StoreGate key at all. What the
        payload leaves unmet is 'a' and 'b'; what the client is asked for is the
        message carrying both.
        """
        fragment = self.menu.add(
            "RemoteExecSeqT4",
            RemoteExecPayloadCfg(_sum("T4Alg", "a", "b", "sum")),
            outputs=["total"],
            boundaries={"addends": DemoInts(["a", "b"]),
                        "total": DemoInts(["sum"])})
        self.assertEqual(_keys(fragment.inputs), ["addends"])
        self.assertEqual(fragment.input_specs, [f"addends#protobuf#{INTS}"])
        self.assertEqual(_keys(fragment.outputs), ["total"])

    def test_a_converting_output_is_produced_via_the_keys_it_carries(self):
        """An output nothing writes under its own name is still produced."""
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqT4b",
                          RemoteExecPayloadCfg(_sum("T4bAlg", "a", "b", "sum")),
                          outputs=["total"],
                          boundaries={"addends": DemoInts(["a", "b"]),
                                      "total": DemoInts(["nonesuch"])})
        self.assertIn("total", str(caught.exception))

    def test_nothing_restates_a_scheduler_dependency(self):
        """The codec declares what it touches; the configuration does not.

        Two statements of the same fact are two things that can disagree, so
        there is one: the component that will do the recording is the component
        that says the key will be recorded.
        """
        fragment = self.menu.add(
            "RemoteExecSeqT4c",
            RemoteExecPayloadCfg(_sum("T4cAlg", "a", "b", "sum")),
            outputs=["sum"], boundaries=_ints("a", "b", "sum"))
        gate = fragment.gate("RemoteExecRequest")
        self.assertFalse(getattr(gate, "ExtraOutputs", None))
        # ...and the codec is told which way it carries, because the same class
        # serves both ends of the boundary.
        self.assertEqual([tool.Direction for tool in gate.Inputs],
                         ["Write", "Write"])
        self.assertEqual(
            [tool.Direction for tool in fragment.pack("RemoteExecRequest").Outputs],
            ["Read"])

    def test_a_vector_boundary_needs_no_type_either(self):
        """A schema the fragment states is not derived from a handle."""
        fragment = self.menu.add(
            "RemoteExecSeqT4d",
            RemoteExecPayloadCfg(CompFactory.AthExRemoteExec.ScaleVectorAlg(
                "T4dAlg", Input="values", Output="scaled")),
            outputs=["scaledPoints"],
            boundaries={"points": DemoDoubles("values"),
                        "scaledPoints": DemoDoubles("scaled")})
        self.assertEqual(fragment.input_specs, [f"points#protobuf#{DOUBLES}"])

    def test_an_unproduced_output_is_refused(self):
        """Asking for a key the payload never writes is a configuration error."""
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqT6",
                          RemoteExecPayloadCfg(_sum("T6Alg", "a", "b", "sum")),
                          outputs=["nonesuch"],
                          boundaries=_ints("a", "b", "nonesuch"))
        self.assertIn("nonesuch", str(caught.exception))

    def test_a_declared_input_that_is_not_unmet_is_refused(self):
        """inputs= is an assertion; a disagreement must not pass quietly."""
        with self.assertRaises(ValueError) as caught:
            self.menu.add("RemoteExecSeqT7",
                          RemoteExecPayloadCfg(_sum("T7Alg", "a", "b", "sum")),
                          outputs=["sum"], inputs=["a"],
                          boundaries=_ints("a", "b", "sum"))
        self.assertIn("unmet", str(caught.exception))

    def test_menu_and_sequences_come_from_one_object(self):
        """The properties the loop manager is given describe what was built."""
        self.menu.add("RemoteExecSeqT8", RemoteExecPayloadCfg(_sum("T8Alg", "a", "b", "sum")),
                      outputs=["sum"], boundaries=_ints("a", "b", "sum"))
        self.menu.add("RemoteExecSeqT9",
                      RemoteExecPayloadCfg(CompFactory.AthExRemoteExec.FailAlg("T9Alg")))
        props = self.menu.properties

        self.assertEqual(props["Sequences"], ["RemoteExecSeqT8", "RemoteExecSeqT9"])
        self.assertEqual(props["RequestKey"], self.menu.request_key)
        # A fragment with no outputs gets no reply key and no pack algorithm.
        self.assertNotIn("RemoteExecSeqT9", props["ReplyKeys"])
        self.assertNotIn("RemoteExecSeqT9", props["Outputs"])

        cfg = self.menu.build(self.flags)
        for name in props["Sequences"]:
            self.assertIsNotNone(cfg.getSequence(name),
                                 f"{name} was advertised but not built")
        self.assertIsNotNone(cfg.getEventAlgo("RemoteExecSeqT8Pack"))
        cfg.wasMerged()


if __name__ == "__main__":
    unittest.main()
