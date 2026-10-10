# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/appQtShowcase_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/appQtShowcase_autogen.dir/ParseCache.txt"
  "appQtShowcase_autogen"
  )
endif()
