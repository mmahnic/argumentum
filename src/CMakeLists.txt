
add_library(argparse-s STATIC
   argparser.cpp
   )

install( TARGETS argparse-s
   ARCHIVE DESTINATION lib
   LIBRARY DESTINATION lib
   )
