#!/bin/bash

SCRIPTPATH=$(dirname "$SCRIPT")
pushd "$SCRIPTPATH" > /dev/null

cat .gitignore | sed '/^#.*/ d' | sed '/^\s*$/ d' | sed 's/^/rm -rf /' | bash > /dev/null

popd > /dev/null
