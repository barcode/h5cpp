/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */

#define GOOGLE_STRIP_LOG 1

#include <Eigen/Core>
#include <h5cpp/all>

#include <gtest/gtest.h>
#include "abstract.h"


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
template<class T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const std::array<T,N>& t)
{
    for(std::size_t i = 0; i < N;++i) out << t.at(i) << " ";
    return out;
}
template<class T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const T t[N] )
{
    for(std::size_t i = 0; i < N;++i) out << t.at(i) << " ";
    return out;
}

#define _do_out(R, data, child) << t.child << " "
#define H5CPP_ADAPT_AND_REGISTER_AND_COUT(T,...)                \
    std::ostream& operator<<(std::ostream& out, const T& t)     \
    {                                                           \
        out _h5cpp_variadic_for_each(_do_out, T, __VA_ARGS__);  \
        return out;                                             \
    }                                                           \
    H5CPP_ADAPT_AND_REGISTER(T,__VA_ARGS__)


H5CPP_ADAPT_AND_REGISTER_AND_COUT(
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

H5CPP_ADAPT_AND_REGISTER_AND_COUT(
    Outer::Mid,
    idx, ar
)

H5CPP_ADAPT_AND_REGISTER_AND_COUT(
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

template <typename T> class AdaptStructTest : public AbstractTest<T>{};
typedef ::testing::Types<
    H5CPP_TEST_PRIMITIVE_TYPES, // check nothing broke / everything works as expected
    array_s_c_eigen,
////    array_c_s_eigen,
    array_s_s_eigen,
////    array_c_c_eigen,
    
    array_s_c_int,
////    array_c_s_int,
    array_s_s_int,
////    array_c_c_int,
    
    Eigen::Matrix3f,
    Eigen::Matrix4f,
    Eigen::RowVector3f,
    Eigen::Vector3f,
    
    Inner,
    Outer::Mid,
    Outer
> AdaptedTypes;

// instantiate for listed types
TYPED_TEST_CASE(AdaptStructTest, AdaptedTypes);

#include <fstream>

TYPED_TEST(AdaptStructTest, AdaptStructWrite) {
    const auto path = this->name;
    static_assert(std::is_standard_layout_v<TypeParam>);
    
    std::ofstream out{"gtest_mańual_log_" + std::string{typeid(TypeParam).name()}};
    out << "---------------------\nAdaptStructWrite\n---------------------\n";
    out <<path<<std::endl;
	h5::write(
        this->fd, path,
        get_test_data<TypeParam>(10),
        h5::stride{3}, h5::max_dims{H5S_UNLIMITED} 
    );
}

TYPED_TEST(AdaptStructTest, AdaptStructCreateAppend) {
    const auto path = this->name;
    static_assert(std::is_standard_layout_v<TypeParam>);
    
    std::ofstream out{"gtest_mańual_log_" + std::string{typeid(TypeParam).name()}, std::ios_base::app};
    out << "---------------------\nAdaptStructCreateAppend\n---------------------\n";
    out <<path<<std::endl;
    h5::pt_t pt = h5::create<TypeParam>(this->fd, path,
        h5::max_dims{H5S_UNLIMITED}, h5::chunk{32} | h5::gzip{9} );
    out << "created" << std::endl;
    const auto vec = get_test_data<TypeParam>(32);
    out << "rnged" << std::endl;
    h5::append(pt, vec);
    out << "appended" << std::endl;
}

TYPED_TEST(AdaptStructTest, AdaptStructRead) {
    const auto path = this->name;
    std::ofstream out{"gtest_mańual_log_" + std::string{typeid(TypeParam).name()}, std::ios_base::app};
    out << "---------------------\nAdaptStructRead\n---------------------\n";
    out <<path<<std::endl;
    
    
    
	std::vector<TypeParam> vec = get_test_data<TypeParam>(100);
    out << "rnged" << std::endl;
	h5::write(this->fd, path, vec);
    out << "written1" << std::endl;
	{  // READ
		const auto data =  h5::read<std::vector<TypeParam>>(this->fd, path);
        ASSERT_EQ(vec.size(), data.size());
        for(std::size_t i = 0; i < vec.size(); ++i) {
            const auto obj1 = static_cast<const void*>(&vec.at(i));
            const auto obj2 = static_cast<const void*>(&data.at(i));
            const auto cmp = std::memcmp(obj1, obj2, sizeof(TypeParam));
            if(cmp)
            out << "--\n"
                <<  "(" << i  << ")\t"
                << cmp << "\n" << vec.at(i) << "-\n" << data.at(i) << "\n--\n";
            EXPECT_EQ(cmp, 0);
        }
	}
    out << "read1" << std::endl;
	{ // PARTIAL READ 
		std::vector<TypeParam> data =  h5::read<std::vector<TypeParam>>(this->fd, path, h5::count{50}, h5::offset{10} );
        out << "pread  "<<data.size() << std::endl;
        //EXPECT_TRUE(data.size() == 50);
        out << __LINE__ << std::endl;
        if constexpr(
            std::is_same_v<TypeParam, Inner> ||
            std::is_same_v<TypeParam, Outer::Mid> ||
            std::is_same_v<TypeParam, Outer>
            )
            for(std::size_t i = 0; i < data.size(); ++i)
            {
                out << __LINE__ << std::endl;
                
                out << data.at(i).idx << std::endl;
            }
        out << __LINE__ << std::endl;
	}
    out << __LINE__ << std::endl;
}
/*----------- BEGIN TEST RUNNER ---------------*/
H5CPP_TEST_RUNNER( int argc, char**  argv );
/*----------------- END -----------------------*/

