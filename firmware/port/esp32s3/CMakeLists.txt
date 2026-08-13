# Herus on ESP32-S3 + SX1262 (LilyGO T3-S3 for Phases 0-3, own PCB from Phase 4).
#
#   idf.py set-target esp32s3
#   idf.py build flash monitor
#
# Everything portable lives in ../../core and ../../net and is compiled from here
# rather than copied: one source of truth, and the host proof suite exercises the
# exact same files.
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(herus)
