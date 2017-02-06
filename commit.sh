#!/bin/bash

if [ $# -ne 1 ]; then
    echo "$0: usage: commit.sh <git comment for the commit>"
    exit 1
fi

echo "Commiting with comment: $1"

git add -A
git commit -m "$1"
git push origin coded-llc-flow