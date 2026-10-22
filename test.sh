#!/usr/bin/env bash
set -e

# Change directory to project root (where this script resides)
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

echo "Building and running automated test suite..."
make test
