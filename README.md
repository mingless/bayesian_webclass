# bayesian_webclass
Bayesian website classifier for the ZPR course at the EiTI faculty of Warsaw University of Technology.

If needed single files can be compiled with something similar to:
```
g++ tidy.cpp -o tidy -ltidy `pkg-config --cflags --libs libxml++-2.6 libcurl` -std=c++11
```

CMakeLists info:

Since neither libtidy nor libxml++ have official CMake Modules added and found FindTidy.cmake isn't working it is recommended to add new libs with global path.
Compilation may require exporting:
    export CONFIGURE_ENV=LDFLAGS="${LDFLAGS}"
    export LDFLAGS=-L/usr/local/lib


On Unix:
    to find the path use:   `dpkg -L yourlibname`
    preferrably .so should be use, f.e. : `target_link_libraries(targetname /usr/lib/yourlibname.so)`


File installation:

Currently all files are to be installed via ${CATKIN_PACKAGE_LIB_DESTINATION} which places everything neatly in catkin_ws/install/lib/bayesian_webclass/.
Current installation folders and rules:
    files from bayesian_webclass/txt/ matching pattern "*.txt"
    files from bayesian_webclass/xml/ matching pattern "*.xml"

Executables:
    
`src/test.cpp` - opens csv/dns.csv file, saves its content to map: id -> domain_name  and checks wheather all these links can be opened. Valid links saves to file `valid_domains.csv` (no checks only 30, comment for loop and uncomment another version to download all)
   
Libraries:
    
`http_downloader.cpp` => Currently in progress, could not work properly
    
`csv.cpp`  one method to tokenize csv file, take wanted columns and save to map.

