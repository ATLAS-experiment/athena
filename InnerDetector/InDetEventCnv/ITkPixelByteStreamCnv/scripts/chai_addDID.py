#!/usr/bin/env python3
"""
Test CREST Tag Content: ITkPixModIDMap-RUN4-00-00-TEST

This script connects to the CREST server at https://atlas-crest-dev.cern.ch/api-v6.4
and investigates the tag ITkPixModIDMap-RUN4-00-00-TEST.
It can create the tag ITkPixModIDMap-RUN4-00-00-TEST, its payload specifications and upload IOV.

Usage:
    python chai_addDID.py
    python chai_addDID.py --since 0
    python chai_addDID.py --tag ITkPixModIDMap-RUN4-00-00-TEST --db crest:https://atlas-crest-dev.cern.ch/api-v6.4
"""

import chai
import argparse
import sys
import json
import base64
import gzip

DEFAULT_TAG = "ITkPixModIDMap-RUN4-00-00-TEST"
DEFAULT_DB  = "crest:https://atlas-crest-dev.cern.ch/api-v6.4"


def sep(title=""):
    line = "=" * 70
    print(f"\n{line}")
    if title:
        print(title)
        print(line)



def create_container_with_clob(payload_spec, clob_dataA: str):
    """
    Create a container with the clob data.

    Args:
        payload_spec: PayloadSpec for the container
        clob_data: bytes object to store

    Returns:
        Container with the clob data
    """
    container = chai.Container(payload_spec)

    # Base64 encode the clob data
    # clob_base64 = base64.b64encode(clob_data).decode('ascii')
    print(f"Type of dataA is {type(clob_dataA)}")
    # Add as string to channel 0
    container[0].push_string(clob_dataA)
    return container

def main(args = None):
    parser = argparse.ArgumentParser(
        description="Create ITkPixModIDMap-RUN4-00-00-TEST tag content via chai",
    )
    parser.add_argument("--tag", default=DEFAULT_TAG, help=f"Tag name (default: {DEFAULT_TAG})")
    parser.add_argument("--db",  default=DEFAULT_DB,  help=f"DB connection string (default: {DEFAULT_DB})")
    parser.add_argument("--since", type=int, default=0,
                        help="Start time for IOVs (default: 0)")
    parser.add_argument("--until", type=lambda x: int(x, 0), default=0xFFFFFFFFFFFFFFFF,
                        help="End time for IOVs (default: 0xFFFFFFFFFFFFFFFF)")
    parser.add_argument("--dataA", type=str, default="data.json",
                        help="Path to the JSON file containing the data for A-side (default: data.json)")
    args = parser.parse_args(args)
    since = args.since if args.since > 0 else None
    until = args.until if args.until > 0 else None

    if(args.since > 0 and args.until > 0 and until < since):
        print(f"✗ Error: until = {until} and since = {since}")
        sys.exit(1)

    sep("CHAI – Tag Content Creation")
    print(f"  Tag : {args.tag}")
    print(f"  DB  : {args.db}")
    print(f"  Data, side-A: {args.dataA}")
    # ------------------------------------------------------------------
    # 1. Connect
    # ------------------------------------------------------------------
    sep("1. Connect to CREST")
    try:
        db = chai.Database(args.db)
        print(f"✓ Connected")
    except Exception as exc:
        print(f"✗ Connection failed: {exc}")
        sys.exit(1)

    # ------------------------------------------------------------------
    # 2. Retrieve tag
    # ------------------------------------------------------------------
    sep("2. Retrieve / Create a tag")
    
    try:
        # Try to get the tag
        existing_tag = db.get_tag(args.tag)
        print(f"  Tag '{args.tag}' already exists")
        print(f"  Removing existing tag...")
        db.delete_tag(args.tag)
        print(f"  ✓ Tag removed successfully")
    except RuntimeError as e:
        # Tag doesn't exist, which is fine
        print(f"  Tag '{args.tag}' does not exist (this is expected)")

    # ------------------------------------------------------------------
    # 3. Tag metadata
    # ------------------------------------------------------------------
    sep("3. Tag metadata")
    # Define payload specification with 2 columns (one string, one int)
    print("  Creating payload specification:")
    print("    - Column 1: 'cabling' (String)")

    fields = [
        chai.Field("cabling", chai.Type.String),
#        chai.Field("cabling", chai.Type.String),
    ]
    field_spec = chai.FieldSpec(fields)
    
    # Define 2 channels
    print("  Creating channel specification:")
    print("    - Channel 0: 'Channel_AC'")
    
    channels = [
        chai.Channel(0, "Channel_AC"),
    ]
    channel_spec = chai.ChannelSpec(channels)
    
    # Combine into payload specification
    payload_spec = chai.PayloadSpec(field_spec, channel_spec)
    print(f"  ✓ Payload specification created with {len(field_spec)} fields and {len(channel_spec)} channels")
    
    # Create the tag
    try:
        tag = db.create_tag(
            args.tag,                          # tag_name
            "Test tag for demonstrating tag management operations",  # description
            payload_spec,                           # payload_spec
            "",                                     # node description
            chai.IovType.Time,                      # iov_type
            "crest-json-single-iov",                # object_type
            chai.Synchronization.All,               # synchronization
            chai.TagStatus.Unlocked                 # status
        )
        print(f"  ✓ Tag created successfully: {tag.name}")
        print(f"    - Description: {tag.description}")
        print(f"    - IOV Type: {tag.iov_type}")
        print(f"    - Status: {tag.status}")
    except Exception as e:
        print(f"  ✗ Failed to create tag: {e}")
        sys.exit(1)


    print(f"  name          : {tag.name}")
    print(f"  description   : {tag.description}")
    print(f"  iov_type      : {tag.iov_type}")
    print(f"  object_type   : {tag.object_type}")
    print(f"  synchronization: {tag.synchronization}")
    print(f"  status        : {tag.status}")
    print(f"  # IOVs        : {len(tag)}")

    # ------------------------------------------------------------------
    # 4. Payload specification
    # ------------------------------------------------------------------
    sep("4. Payload specification")
    pspec = tag.payload_spec
    print(f"  Fields ({len(pspec.fields.fields)}):")
    for f in pspec.fields.fields:
        print(f"    {f.name}: {f.type}")

    channel_names = pspec.channels.channel_names
    print(f"\n  Channels ({len(channel_names)}):")
    items = list(channel_names.items())
    shown = items[:10]
    for cid, cname in shown:
        suffix = f" ({cname})" if cname else ""
        print(f"    {cid}{suffix}")
    if len(items) > 10:
        print(f"    ... and {len(items) - 10} more")

    # ------------------------------------------------------------------
    # 5. Upload IOVs
    # ------------------------------------------------------------------
    sep("5. IOVs")
    print(f"  Since: {args.since}")
    # Load JSON file from disk
    json_dataA = None
    with open(args.dataA, "r") as f:
        json_dataA = json.load(f)
    # Store a compact, compressed representation of the JSON payload. The backend
    # rejects very large raw JSON strings, but accepts compressed base64 content.
    json_textA = json.dumps(json_dataA, separators=(",", ":"), sort_keys=True)

    compressedA = gzip.compress(json_textA.encode("utf-8"))
    cdataA = base64.b64encode(compressedA).decode("ascii")

    #this is the uncompressed version
    cdataA = str(json.dumps(json_dataA).encode('utf-8'))

    print(f"  Created clob dataA: {len(cdataA)} characters (compressed JSON)")
    print(f"Type of dataA is {type(cdataA)}")
    # Create container
    container = create_container_with_clob(payload_spec, cdataA)
    print(f"  Container created with {container.num_channels} channel(s), {container.num_fields} field(s)")

    # Store the container directly using add_payload
    try:
        since = args.since
        until = args.until
        # Store the container payload
        tag.add_payload(container, since, until)

        payload_json = container.to_json()
        print(f"  ✓ Stored IOV: since={since}, until={until}")
        print(f"    Payload size: {len(payload_json)} bytes")
        print(f"    String length: {len(json.dumps(json_dataA))} characters")

    except Exception as e:
        print(f"  ✗ Failed to store IOV: {e}")

    sep()
    print("✓ Done")


if __name__ == "__main__":
    main()
