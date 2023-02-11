# Run with cmake -P
# Parameters (-D): TOP_SOURCE_DIR, P_BINARY_DIR, P_SOURCE_DIR, P_HEADERONLY, P_BUILDSTATIC

file( MAKE_DIRECTORY ${P_BINARY_DIR}/argumentum )

file( COPY ${TOP_SOURCE_DIR}/src/
   DESTINATION ${P_BINARY_DIR}/argumentum/inc/
   FILES_MATCHING PATTERN "*.h"
   )

if( P_BUILDSTATIC )
   file( READ ${P_SOURCE_DIR}/argumentum/argparse.h
      main_header )

   string( REPLACE "../../src/" "inc/"
      main_header "${main_header}" )

   file( WRITE ${P_BINARY_DIR}/argumentum/argparse.h
      "${main_header}" )
endif()

if( P_HEADERONLY )
   file( READ ${P_SOURCE_DIR}/argumentum/argparse-h.h
      main_header )

   string( REPLACE "../../src/" "inc/"
      main_header "${main_header}" )

   file( WRITE ${P_BINARY_DIR}/argumentum/argparse-h.h
      "${main_header}" )
endif()
