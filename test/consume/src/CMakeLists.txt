
add_executable( basic )
target_sources( basic
   PRIVATE
   basic.cpp
   )
target_link_libraries( basic
   PRIVATE
   Argumentum::argumentum
   )

