# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""The demonstration fragment's own codecs.

These are here, and not in RemoteExecCodecs, because they are the *fragment's*. This
package's framework implements no encoding: it carries bytes, and what those
bytes mean is the business of whoever published the schema. A real fragment
would ship these from its own package, and the transport, the loop manager, the
gate and the pack algorithm would be unchanged by their existence.

They are also the awkward shape, deliberately kept in one place. Every other
encoding *carries* its payload -- the bytes are the StoreGate object, so the
boundary key is the object's key and there is one of them. A protobuf boundary
*converts*, so the two come apart: ``addends`` is what the boundary is called on
the wire, while ``a`` and ``b`` are what the fragment's algorithms read. The
framework is not shaped to accommodate that; these descriptors are.
"""

from AthenaConfiguration.ComponentFactory import CompFactory

from AthExRemoteExec.RemoteExecCodecs import Codec

__all__ = ["DemoInts", "DemoDoubles", "INTS", "DOUBLES"]

#: Message names, matching DemoSchema.cxx. The one name no C++ type supplies.
INTS = "athexremoteexec.demo.v1.Ints"
DOUBLES = "athexremoteexec.demo.v1.Doubles"


class DemoInts(Codec):
    """Named integers, as one protobuf message.

    One message becomes several StoreGate objects, matched by name rather than
    by position, so a fragment that gains an input does not silently
    reinterpret the ones it had::

        boundaries={"addends": DemoInts(["a", "b"])}

    The same list is what the tool is configured with and what tells the
    fragment derivation that ``a`` and ``b`` arrive over the wire rather than
    being unmet reads. Stated once.
    """

    encoding = "protobuf"
    tool_type = CompFactory.AthExRemoteExec.DemoIntsCodec

    def __init__(self, values):
        if not values:
            raise ValueError("DemoInts carries at least one named integer")
        super().__init__(INTS)
        self.values = list(values)

    def sgKeys(self, key):
        return list(self.values)

    def directionProperties(self, direction):
        # Which half is live depends on which way this instance carries; the
        # other stays empty, and the C++ side refuses a direction whose keys
        # were not given.
        if direction == "Write":
            return {"Decoded": list(self.values)}
        return {"Encoded": list(self.values)}

    def __repr__(self):
        return f"DemoInts({self.values!r})"


class DemoDoubles(Codec):
    """One named vector of doubles.

    The same shape as :class:`DemoInts` for a message that happens to carry a
    single object, which is a property of the message rather than of the
    mechanism.
    """

    encoding = "protobuf"
    tool_type = CompFactory.AthExRemoteExec.DemoDoublesCodec

    def __init__(self, values):
        if not values:
            raise ValueError("DemoDoubles needs the key its vector lives under")
        super().__init__(DOUBLES)
        self.values = values

    def sgKeys(self, key):
        return [self.values]

    def directionProperties(self, direction):
        if direction == "Write":
            return {"Decoded": self.values}
        return {"Encoded": self.values}

    def __repr__(self):
        return f"DemoDoubles({self.values!r})"
