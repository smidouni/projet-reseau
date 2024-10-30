# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\projet-reseau_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\projet-reseau_autogen.dir\\ParseCache.txt"
  "projet-reseau_autogen"
  )
endif()
