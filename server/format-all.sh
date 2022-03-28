#!/bin/bash
find ./ -iname *.hpp | xargs clang-format -i
find ./ -iname *.cpp | xargs clang-format -i
