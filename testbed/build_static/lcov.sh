#!/usr/bin/env bash

# Generate HTML coverage report

if ! which lcov > /dev/null
then
  echo "You must install lcov: sudo apt-get install lcov"
  exit 1
fi

lcov --capture --directory ./ --output-file coverage.info

genhtml coverage.info --output-directory coverage-html

