#include <armadillo>
#include <cstdint>
#include <eigen3/Eigen/Core>
#include <h5cpp/all>


typedef unsigned long long int MyUInt;

using array_s_c_eigen = std::array<Eigen::Matrix3f[2], 3>;
using array_c_s_eigen = std::array<Eigen::Vector3f, 2>[3];
using array_s_s_eigen = std::array<std::array<Eigen::Matrix4f, 3>, 2>;
using array_c_c_eigen = Eigen::RowVector3f[2][3];

using array_s_s_int = std::array<std::array<int, 3>, 2>;
using array_s_c_int = std::array<int[3], 2>;
using array_c_s_int = std::array<int, 3>[2];
using array_c_c_int = int[2][3];

struct Inner {
    MyUInt              idx;
    char       _char;
    unsigned char       _uchar;
    short      _short;
    unsigned short      _ushort;
    int        _int;
    unsigned int        _uint;
    long       _long;
    unsigned long       _ulong;
    long long  _llong;
    unsigned long long  _ullong;
    float               _float;
    double              _double;
    long double         _ldouble;
    bool                _bool;
    Eigen::Matrix3f     _m3f;
    array_s_c_eigen     _ar_s_c_eigen;
    array_c_s_eigen     _ar_c_s_eigen;
    array_s_s_eigen     _ar_s_s_eigen;
    array_c_c_eigen     _ar_c_c_eigen;
    array_s_s_int       _ar_s_s_int;
    array_s_c_int       _ar_s_c_int;
    array_c_s_int       _ar_c_s_int;
    array_c_c_int       _ar_c_c_int;
};



struct Outer {
    struct Mid {
        MyUInt  idx;   // typedef type 
        Inner   ar[4]; // array of custom type
    };
    MyUInt                idx;            // typedef type 
    Mid                   ar[3][8]; // array of arrays
};

//generate glue code required by h5cpp

H5CPP_ADAPT_AND_REGISTER(
    Inner,
    idx,
    _char,
    _uchar,
    _short,
    _ushort,
    _int,
    _uint,
    _long,
    _ulong,
    _llong,
    _ullong,
    _float,
    _double,
    _ldouble,
    _bool,
    _m3f,
    _ar_s_c_eigen,
    _ar_c_s_eigen,
    _ar_s_s_eigen,
    _ar_c_c_eigen,
    _ar_s_s_int,
    _ar_s_c_int,
    _ar_c_s_int,
    _ar_c_c_int
    )

H5CPP_ADAPT_AND_REGISTER(
    Outer::Mid,
    idx, ar
    )

H5CPP_ADAPT_AND_REGISTER(
    Outer,
    idx, ar
    )

template<class T>
std::vector<T> get_test_data( size_t n ){
    std::vector<T> vec (n);
    std::mt19937 gen{0}; // to enable us to do compare the read data
    std::uniform_int_distribution<std::uint8_t> dist{0, 255};
    std::uint8_t* begin = static_cast<std::uint8_t*>(static_cast<void*>(&vec.front()));
    std::uint8_t* end = static_cast<std::uint8_t*>(static_cast<void*>(&vec.back() + 1));
    for(std::uint8_t* it = begin; it < end; ++it )
        *it = dist(gen);
    if constexpr(
        std::is_same_v<T, Inner> ||
        std::is_same_v<T, Outer::Mid> ||
        std::is_same_v<T, Outer>
        )
        for(int i=0; i<n; i++ )
            vec[i].idx = i;
    return vec;
}

#define CHUNK_SIZE 5
#define NROWS 4*CHUNK_SIZE
#define NCOLS 1*CHUNK_SIZE

int main(){
	//RAII will close resource, noo need H5Fclose( any_longer ); 
	h5::fd_t fd = h5::create("example-bbost-macro.h5",H5F_ACC_TRUNC);
    
    std::vector<Outer> vec = get_test_data<Outer>(5);
    h5::write(fd, "struct", vec );

	{ // read entire dataset back
		using T = std::vector<Outer>;
		auto data = h5::read<T>(fd,"/struct");
		std::cerr <<"reading back data previously written:\n\t";
		for( std::size_t i = 0; i < data.size(); ++i ) {
			std::cerr << data.at(i).idx <<" ";
			if(std::memcmp((const void*) &data.at(i), (const void*) &vec.at(i), sizeof(vec.at(i))) != 0 )
				std::cerr << "\n ERROR AT INDEX " << i << std::endl;
		}
		std::cerr << std::endl;
	}

    h5::write(fd, "eigen3f", get_test_data<Eigen::Matrix3f>(2));
//    h5::write(fd, "carray-struct", get_test_data<Outer[2]>(2));
    h5::write(fd, "std-array-struct", get_test_data<std::array<Outer, 2>>(2));
}
