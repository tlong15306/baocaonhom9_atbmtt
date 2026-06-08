# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\moi_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\moi_autogen.dir\\ParseCache.txt"
  "moi_autogen"
  )
endif()
