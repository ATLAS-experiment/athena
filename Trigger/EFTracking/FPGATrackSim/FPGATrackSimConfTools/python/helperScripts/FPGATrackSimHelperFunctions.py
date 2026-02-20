#!/usr/bin/env python3
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import re

def convertRegionsExpressionToArray(expression, min_value=0, max_value=1279):
    if not expression or str(expression).strip() == "":
        raise ValueError("No region expression provided.")
    
    # Check if the expression is already a list or list-like string
    if isinstance(expression, (list, tuple)):
        result = [int(x) for x in expression if min_value <= int(x) <= max_value]
        if not result:
            raise ValueError("No valid regions after filtering by range.")
        return result
    
    # Check if it's a string representation of a list
    if isinstance(expression, str):
        expression_stripped = expression.strip()
        if expression_stripped.startswith('[') and expression_stripped.endswith(']'):
            try:
                # Parse the list string
                list_content = expression_stripped[1:-1].strip()
                if list_content:
                    values = [int(x.strip()) for x in list_content.split(',')]
                    if all(min_value <= x <= max_value for x in values):
                        return values
                    else:
                        out_of_range = [x for x in values if x < min_value or x > max_value]
                        raise ValueError(f"Values out of range [{min_value}-{max_value}]: {out_of_range}")
                else:
                    raise ValueError("Empty list provided.")
            except ValueError:
                raise ValueError(f"Invalid list format: {expression}")
    
    # Check if the expression is a single integer
    if str(expression).isdigit():
        num = int(expression)
        if min_value <= num <= max_value:
            return [num]  # Return the single integer as a list
        else:
            raise ValueError(f"Invalid choise: {expression}. Number out of range [{min_value}-{max_value}].")
    
    # Start with all numbers within min_value to max_value
    numbers = set(range(min_value, max_value + 1))

    include_set = set()
    exclude_set = set()

    print(f"Initial expression: {str(expression)}")
    if isinstance(expression, tuple) or isinstance(expression, list):
        expression = ",".join(map(str, expression))
    
    # Handle asterisk to return all values
    if expression.strip() == "*":
        return list(range(min_value, max_value + 1))
    
    parts = expression.split(",")  # Handle single-element expressions
    print(f"Parts after split: {parts}")  
    for part in parts:
        part = part.strip()
        

        # Exclude regions (e.g. "!5-30")
        if part.startswith("!"):
            exclude_set.update(parse_range_or_wildcard(part[1:], min_value, max_value))
        else:
            include_set.update(parse_range_or_wildcard(part, min_value, max_value))

    # Apply inclusion if non-empty; otherwise, use full range
    result = include_set if include_set else numbers
    result -= exclude_set  # Remove exclusions

    # Ensure final result stays within valid bounds
    result = {num for num in result if min_value <= num <= max_value}
    
    if not result:
        raise ValueError(f"No valid regions after applying expression: {expression}")
    
    return list(sorted(result))  # Always return a sorted list

def parse_range_or_wildcard(expr, min_value, max_value):  
    # Check for range (e.g., "5-30")
    if "-" in expr and "*" not in expr:
        start, end = map(int, expr.split("-"))
        start, end = max(min_value, start), min(max_value, end)  # Apply min/max limits
        return set(range(start, end + 1))
    
    # Check for explicit number
    if expr.isdigit():
        num = int(expr)
        return {num} if min_value <= num <= max_value else set()  # Ignore if out of range

    if "eta" in expr:   
        etanum = int(expr.replace("eta",""))
        # the lowest bits at the 32 phi regions    
        return parse_range_or_wildcard(f"{32*etanum}-{32*(etanum+1)-1}",min_value, max_value)

    # Handle wildcards
    pattern = "^" + expr.replace("*", ".*") + "$"
    regex = re.compile(pattern)

    return {num for num in range(min_value, max_value + 1) if regex.match(str(num))}

if __name__ == "__main__":
    # Cases to validate
    test_cases = [
        ("5", [5]),  # Single number
        ("5-10", [5, 6, 7, 8, 9, 10]),  # Range
        ("!5-10", list(range(0, 5)) + list(range(11, 1280))),  # Exclusion range
        ("5,10,15", [5, 10, 15]),  # Multiple numbers
        ("5-10,!7", [5, 6, 8, 9, 10]),  # Range with exclusion
        ("*", list(range(1280))),  # Wildcard for all
        ("!5", list(range(0, 5)) + list(range(6, 1280))),  # Exclude single number
        ("5-10,!7-8", [5, 6, 9, 10]),  # Range with exclusion range
        ("1*", [num for num in range(1280) if re.match(r"^1.*$", str(num))]),  # Wildcard pattern
        ("!9*", [num for num in range(1280) if not re.match(r"^9.*$", str(num))]),  # Exclude wildcard pattern
        ("[5,6,8,10]", [5, 6, 8, 10]),  # List format
    ]

    for expression, expected in test_cases:
        try:
            result = convertRegionsExpressionToArray(expression)
            assert result == expected, f"Test failed for {expression}: {result} != {expected}"
        except Exception as e:
            print(f"Test failed for {expression}: {e}")
            import sys
            sys.exit(1)
