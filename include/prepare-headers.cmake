# Run with cmake -P
# Parameters (-D): TOP_SOURCE_DIR, P_BINARY_DIR, P_SOURCE_DIR

file( MAKE_DIRECTORY ${P_BINARY_DIR}/argumentum )

file( COPY ${TOP_SOURCE_DIR}/src/
   DESTINATION ${P_BINARY_DIR}/argumentum/inc/
   FILES_MATCHING PATTERN "*.h"
   )

file( READ ${P_SOURCE_DIR}/argumentum/argparse.h
   main_header )

string( REPLACE "../../src/" "inc/" 
   main_header "${main_header}" )

file( WRITE ${P_BINARY_DIR}/argumentum/argparse.h
   "${main_header}" )
