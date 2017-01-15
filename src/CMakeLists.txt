project(bayesian_webclass)

add_library(CSV STATIC ../include/bayesian_webclass/csv.h csv.cpp)
add_library(HTTP STATIC ../include/bayesian_webclass/http_downloader.h http_downloader.cpp)

add_executable(test_p test.cpp)
target_link_libraries(test_p
        CSV
        HTTP
        ${catkin_LIBRARIES}
        ${LIBS}
        )