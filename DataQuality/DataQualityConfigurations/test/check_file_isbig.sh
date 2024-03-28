#!/bin/bash

[[ $(wc -l $1 | awk '{ print $1 }') -gt $2 ]] && exit 0
exit 1
