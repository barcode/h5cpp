# check if the correct version of hdf5 available
#cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

### match hdf5 versioning x.x.x.h5cpp-version
#project(h5cpp-tests VERSION 1.10.4.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(h5cpp_test target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target}
        PRIVATE
            ${HDF5_C_INCLUDE_DIRS}
    )
    target_link_libraries(${target}
        PRIVATE
            h5cpp

            profiler
            glog
            gtest
            gtest_main
            boost_date_time
            ${HDF5_C_LIBRARIES}
            ${HDF5_C_HL_LIBRARIES}
            z
            dl
            m
            pthread
    )

    add_test(NAME ${target} COMMAND ${target} --gtest_color=auto --gtest_print_time=0)
endfunction()

h5cpp_test(test-stl         stl.cpp         event_listener.hpp abstract.h)
h5cpp_test(test-integral    integral.cpp    event_listener.hpp abstract.h)
h5cpp_test(test-pipeline    pipeline.cpp    event_listener.hpp abstract.h)
h5cpp_test(test-armadillo   armadillo.cpp   event_listener.hpp abstract.h)
h5cpp_test(test-packettable packettable.cpp event_listener.hpp abstract.h struct.h)

find_package(Eigen3 QUIET)
find_package(blaze QUIET)
find_package(dlib QUIET)
if(Eigen3_FOUND AND blaze_FOUND AND dlib_FOUND)
    h5cpp_test(test-adapt-macro adapt_struct_macro.cpp abstract.h)
    target_link_libraries(test-adapt-macro PRIVATE Eigen3::Eigen)
    target_link_libraries(test-adapt-macro PRIVATE blaze::blaze)
    target_link_libraries(test-adapt-macro PRIVATE dlib::dlib)
endif()
