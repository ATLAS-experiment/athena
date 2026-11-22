# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Examples of how to declare new Codecs"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthRemoteExecCore.RemoteExecCodecs import Codec, Direction, Encoding

#: Message schema, matching whatever name we use in C++
INTS = "athexremoteexec.demo.v1.Ints"
DOUBLES = "athexremoteexec.demo.v1.Doubles"
WORDS = "athremoteexec.demo.v1.Words"


class DemoInts(Codec):
    """Named integers in a one protobuf message. One message becomes several StoreGate objects matched by name, see DemoIntsCodec.h for more details.

    .add(..., boundaries={"addends": DemoInts(["a", "b"])})
    """

    encoding = Encoding.protobuf
    tool_type = CompFactory.AthRemoteExec.Example.DemoIntsCodec

    def __init__(self, values: list[str]):
        if not values:
            raise ValueError("DemoInts carries at least one named integer")
        super().__init__(INTS)
        self.values = list(values)

    def sgKeys(self, key: str) -> list[str]:
        return list(self.values)

    def directionProperties(self, direction: Direction) -> dict[str, list[str]]:
        if direction is Direction.WriteToSG:
            return {"Decoded": list(self.values)}
        return {"Encoded": list(self.values)}

    def __repr__(self):
        return f"DemoInts({self.values!r})"


class DemoDoubles(Codec):
    """One named vector of doubles."""

    encoding = Encoding.protobuf
    tool_type = CompFactory.AthRemoteExec.Example.DemoDoublesCodec

    def __init__(self, values: str):
        if not values:
            raise ValueError("DemoDoubles needs the key its vector lives under")
        super().__init__(DOUBLES)
        self.values = values

    def sgKeys(self, key: str) -> list[str]:
        return [self.values]

    def directionProperties(self, direction: Direction) -> dict[str, str]:
        if direction is Direction.WriteToSG:
            return {"Decoded": self.values}
        return {"Encoded": self.values}

    def __repr__(self):
        return f"DemoDoubles({self.values!r})"


class DemoWords(Codec):
    """Vector of unsigned 32-bit integers"""

    encoding = Encoding.protobuf
    tool_type = CompFactory.AthRemoteExec.Example.DemoWordsCodec

    def __init__(self, values: str):
        if not values:
            raise ValueError("DemoWords needs to be supplied with a key name")
        super().__init__(WORDS)
        self.values = values

    def sgKeys(self, key: str) -> list[str]:
        return [self.values]

    def directionProperties(self, direction: Direction) -> dict[str, str]:
        if direction is Direction.WriteToSG:
            return {"Decoded": self.values}
        return {"Encoded": self.values}

    def __repr__(self):
        return f"DemoWords({self.values!r})"
