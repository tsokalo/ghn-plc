#!/bin/bash


find . -type f -name "*.cc" -exec sed -i 's/GdothnMacBackoff/GhnPlcMacBackoff/g' {} +
find . -type f -name "*.h" -exec sed -i 's/GdothnMacBackoff/GhnPlcMacBackoff/g' {} +
