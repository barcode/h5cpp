/* Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */
#include <armadillo>
#include <cstdint>
#include "struct.h"
#include <h5cpp/core>
	// generated file must be sandwiched between core and io 
	// to satisfy template dependencies in <h5cpp/io>  
	#include "generated.h"
#include <h5cpp/io>
#include "utils.hpp"

#define CHUNK_SIZE 5
#define NROWS 4*CHUNK_SIZE
#define NCOLS 1*CHUNK_SIZE

int main(){
	//RAII will close resource, noo need H5Fclose( any_longer ); 
	h5::fd_t fd = h5::create("dump.h5",H5F_ACC_TRUNC);

	{
		h5::write(fd, "struct"   , h5::utils::get_test_data<sn::example::Record>(5) );
        h5::write(fd, "/path/int", h5::utils::get_test_data<int>(5) );
	}
    //dumping info
	{
        std::cout << "names at /:\n";
        for(const auto& n : h5::ls(fd, "/"))
            std::cout << "    " << n << '\n';
        std::cout << "names at /path/:\n";
        for(const auto& n : h5::ls(fd, "path"))
            std::cout << "    " << n << '\n';
	}
}
