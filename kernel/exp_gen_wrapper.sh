#!/bin/bash

"$2" "$3" <(nm --defined-only "$1")
