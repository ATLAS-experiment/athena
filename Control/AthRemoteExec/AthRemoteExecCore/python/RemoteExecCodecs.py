# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Configuration for Codec tools"""

from enum import StrEnum

from AthenaCommon.Configurable import Configurable

Source = StrEnum("Source", ["explicit", "derived", "none"])


class Direction(StrEnum):
    ReadFromSG = "ReadFromSG"
    WriteToSG = "WriteToSG"


class Codec:
    """Configuration helper for Codec tools. Each codec configuration should derive from this class."""

    # Codec encoding protocol, see BoundaryInfo in AthRemoteExecWire/Protocol.h
    encoding: str | None = None
    # C++ Codec tool, e.g. CompFactory.AthRemoteExec.MyCodecTool
    tool_type = None
    # Where Codec's schema comes from:
    # - explicit -> from Codec: boundaries = {"name": SomeCodec("we_use_this")}
    # - derived -> from SG:     boundaries = {"we_use_this": AnotherCodec()}
    # - none -> codec carries one specific payload type and needs no schema, e.g. bytestream:
    #                           boundaries = {"name": YetAnotherCodec()}
    schema_source: Source = Source.explicit

    def __init__(self, schema: str | None = None, **properties) -> None:
        """Construct Codec. To be called by the derived class.

        :param schema see Protocol.h
        :param properties all properties that need to be passed to the tool
        """

        if self.encoding is None:
            raise TypeError(f"{type(self).__name__}: encoding is not set")

        if self.tool_type is None:
            raise TypeError(f"{type(self).__name__}: C++ tool is not set")

        if self.schema_source is Source.explicit and not schema:
            raise ValueError(
                f"{type(self).__name__} needs a schema: one component of this "
                "class can carry more than one thing, so the encoding tag does "
                "not name the payload by itself"
            )

        if self.schema_source is not Source.explicit and schema:
            raise ValueError(
                f"{type(self).__name__} takes no schema; it is "
                + (
                    "derived from the remoteCfg's own handles"
                    if self.schema_source is Source.derived
                    else "named completely by its encoding tag"
                )
            )

        self.schema = schema
        self.properties = properties

    def resolvedSchema(self, sg_type: str = "") -> str | None:
        """The schema string this boundary actually declares."""

        if self.schema_source is Source.explicit:
            return self.schema
        if self.schema_source is Source.derived:
            if not sg_type:
                raise ValueError(
                    f"{type(self).__name__} takes its schema from the "
                    "StoreGate type, which could not be derived for this key"
                )
            return sg_type

        return ""

    def sgKeys(self, key: str) -> list[str]:
        """The StoreGate keys this codec sees. To be overriden in derived class."""

        return [key]

    def directionProperties(self, direction: Direction) -> dict:
        """Properties that depends on which way this Codec instance acts."""

        return {}

    def spec(self, key: str, sg_type: str = "") -> str:
        """Complete C++ BoundaryInfo formatted as string: 'key#encoding#schema'"""

        return f"{key}#{self.encoding}#{self.resolvedSchema(sg_type)}"

    def tool(
        self,
        name: str,
        key: str,
        direction: Direction,
        sg_type: str = "",
    ) -> Configurable:
        """Create Codec tool instance.

        :param name: tool name
        :param key: boundary key
        :param direction: direction in which this tools instance acts
        :param sg_type: type, for resolving schema
        """

        if not isinstance(direction, Direction):
            raise ValueError(
                f"direction must be 'ReadFromSG' or 'WriteToSG', not {direction!r}"
            )
        if not self.tool_type:
            raise RuntimeError("Tool type is not set!")
        return self.tool_type(
            name,
            Key=key,
            Schema=self.resolvedSchema(sg_type),
            Direction=direction,
            **self.directionProperties(direction),
            **self.properties,
        )

    def __repr__(self):
        return f"{type(self).__name__}({self.schema!r})"
