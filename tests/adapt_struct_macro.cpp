/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */

#define GOOGLE_STRIP_LOG 1

#include <fstream>

namespace SomeNameSpace {
 struct StructType;
}





//force enable everything
#define H5CPP_USE_DLIB
#define H5CPP_USE_BLAZE
#define H5CPP_USE_EIGEN3
#define H5CPP_USE_ITPP_MATRIX
#define H5CPP_USE_ITPP_VECTOR
#define H5CPP_USE_UBLAS_MATRIX
#define H5CPP_USE_UBLAS_VECTOR

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <dlib/matrix.h>
#include <blaze/Blaze.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <itpp/itbase.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <gtest/gtest.h>

#include <h5cpp/core>
#include "struct.h"
#include <h5cpp/io>

#include "abstract.h"

// ////////////////////////////////////////////////////////////////////////// //
// ------------------------------- data types ------------------------------- //
// ////////////////////////////////////////////////////////////////////////// //

typedef unsigned long long int MyUInt;

using array_s_c_eigen = std::array<Eigen::Matrix3f[2], 3>;
using array_c_s_eigen = std::array<Eigen::Vector3f, 2>[3];
using array_s_s_eigen = std::array<std::array<Eigen::Matrix4f, 3>, 2>;
using array_c_c_eigen = Eigen::RowVector3f[2][3];

using array_s_s_int = std::array<std::array<int, 3>, 2>;
using array_s_c_int = std::array<int[3], 2>;
using array_c_s_int = std::array<int, 3>[2];
using array_c_c_int = int[2][3];

namespace Namespace
{
    struct InNamespace
    {
        std::uint64_t idx;
        float x;
        float y;
        float z;
    };
}

struct Simple {
    MyUInt              idx;
    char                _char;
    unsigned char       _uchar;
    short               _short;
    unsigned short      _ushort;
    int                 _int;
    unsigned int        _uint;
    long                _long;
    unsigned long       _ulong;
    long long           _llong;
    unsigned long long  _ullong;
    float               _float;
    double              _double;
    long double         _ldouble;
    bool                _bool;
};

struct SimpleWithPadding {
    MyUInt              idx;
    char                _char;
    double              _double;
};

struct SimpleEigen {
    MyUInt                    idx;
    Eigen::Matrix4f           _mat4f;
    Eigen::Quaternion<double> _quat;
    Eigen::Vector3f           _vec3f;
};

struct SimpleEigenWithPadding {
    MyUInt              idx;
    Eigen::Vector3f     _vec3f;
    Eigen::Matrix4f     _mat4f;
};

struct Inner {
    MyUInt              idx;
    Simple              _simple;
    SimpleEigen         _eigen;
//    sn::struct_type     _snst;
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
        Inner   ar[2]; // array of custom type
        Namespace::InNamespace inname;
    };
    MyUInt                idx;            // typedef type
    Mid                   ar[2][3]; // array of arrays
};

// ////////////////////////////////////////////////////////////////////////// //
// ----------------------------- helper macros ------------------------------ //
// ////////////////////////////////////////////////////////////////////////// //
#define VAR_SEQ_FOR_EACH(macro, data, ...)                                      \
    BOOST_PP_SEQ_FOR_EACH(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define DO_OUT(R, data, child) << t.child << " "
#define DO_CMP(R, data, child) binaryEqual(l.child, r.child) &&

#define MAKE_COUT_AND_COMPARE(T,...)                            \
    std::ostream& operator<<(std::ostream& out, const T& t) {   \
        out VAR_SEQ_FOR_EACH(DO_OUT, , __VA_ARGS__);            \
        return out;                                             \
    }                                                           \
    bool binaryEqual(const T& l, const T& r) {                \
        return VAR_SEQ_FOR_EACH(DO_CMP, , __VA_ARGS__) true;    \
    }

#define H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(T,...)    \
    MAKE_COUT_AND_COMPARE(T, __VA_ARGS__)                       \
    H5CPP_ADAPT_AND_REGISTER_STRUCT(T, __VA_ARGS__)
// ////////////////////////////////////////////////////////////////////////// //
// -------------------- output / compare for base types --------------------- //
// ////////////////////////////////////////////////////////////////////////// //


template<class T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const std::array<T,N>& t)
{
    for(std::size_t i = 0; i < N;++i) out << t.at(i) << " ";
    return out;
}
template<class T, std::size_t N>
std::ostream& operator<<(std::ostream& out, const T t[N] )
{
    for(std::size_t i = 0; i < N;++i) out << t[i] << " ";
    return out;
}

template <class S>
std::ostream& operator<<(std::ostream& out, const Eigen::Quaternion<S>& q)
{
    out << q.w() << ' '<< q.x() << ' '<< q.y() << ' '<< q.z() << ' ';
    return out;
}
template<class T>
std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, bool>
binaryEqual(const T& l, const T& r)
{
    return (std::memcmp( (const void*) &l, (const void*) &r, sizeof(T)) == 0);
}

template <class S, int R, int C, int O, int Mr, int Mc>
bool binaryEqual(const Eigen::Matrix<S,R,C,O,Mr,Mc>& l, const Eigen::Matrix<S,R,C,O,Mr,Mc>& r)
{
    if(l.cols() != r.cols() || l.rows() != r.rows())
        return false;
    
    for(int ir = 0; ir < l.rows(); ++ ir)
        for(int ic = 0; ic < l.cols(); ++ ic)
            if(!binaryEqual(r(ir, ic), l(ir, ic)))
            {
                return false;
            }
    return true;
}

template <class S>
bool binaryEqual(const Eigen::Quaternion<S>& l, const Eigen::Quaternion<S>& r)
{
    return binaryEqual(l.x(),r.x()) &&
           binaryEqual(l.y(),r.y()) &&
           binaryEqual(l.z(),r.z()) &&
           binaryEqual(l.w(),r.w());
}

template<class T, std::size_t N>
bool binaryEqual(const std::array<T,N>& l, const std::array<T,N>& r);

template<class T, std::size_t N>
bool binaryEqual(T (&l)[N], T (&r)[N]) {
    for(std::size_t i = 0; i < N;++i)
        if(!binaryEqual(l[i], r[i]))
            return false;
    return true;
}

template<class T, std::size_t N>
bool binaryEqual(const std::array<T,N>& l, const std::array<T,N>& r) {
    for(std::size_t i = 0; i < N;++i)
        if(!binaryEqual(l.at(i), r.at(i)))
            return false;
    return true;
}


// ////////////////////////////////////////////////////////////////////////// //
// ------------------------------- macro calls ------------------------------ //
// ////////////////////////////////////////////////////////////////////////// //
MAKE_COUT_AND_COMPARE(
    SomeNameSpace::StructType,
    field1, field2, field3, field4, field5,
    field6, field7, field8, field9
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    Namespace::InNamespace,
    idx, x, y, z
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    Simple,
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
    _bool
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    SimpleWithPadding,
    idx,
    _char,
    _double
    )

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    SimpleEigen,
    idx,
    _mat4f,
    _quat,
    _vec3f
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    SimpleEigenWithPadding,
    idx,
    _vec3f,
    _mat4f
    )
H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    Inner,
    idx,
    _simple,
    _eigen,
    _ar_s_c_eigen,
    _ar_c_s_eigen,
    _ar_s_s_eigen,
    _ar_c_c_eigen,
    _ar_s_s_int,
    _ar_s_c_int,
    _ar_c_s_int,
    _ar_c_c_int
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    Outer::Mid,
    idx, ar, inname
)

H5CPP_ADAPT_AND_REGISTER_STRUCT_AND_MAKE_COUT(
    Outer,
    idx, ar
)





// ////////////////////////////////////////////////////////////////////////// //
// ------------------------------- test setup ------------------------------- //
// ////////////////////////////////////////////////////////////////////////// //

#define TYPES_TO_CHECK                                          \
    /* check nothing broke / everything works as expected */    \
    H5CPP_TEST_PRIMITIVE_TYPES,                                 \
    SomeNameSpace::StructType,                                  \
                                                                \
    /* check c-array types */                                   \
    /*int[1],                                                   \
    int[1][2],                                                  \
    int[1][2][3],                                               \
    int[1][2][3][4],                                            \
    int[1][2][3][4][5],*/                                       \
                                                                \
    /* check combinations of c/std arrays */                    \
    array_s_c_int,                                            \
    /*array_c_s_int,*/                                              \
    array_s_s_int,                                              \
    /*array_c_c_int,*/                                              \
                                                                \
    /* check fixed size eigen types */                          \
    /*Eigen::Matrix3f,*/                                            \
    /*Eigen::Matrix4f,*/                                            \
    /*Eigen::RowVector3f,*/                                         \
    /*Eigen::Vector3f,*/                                            \
                                                                \
    /* check combinations of c/std arrays and eigen types */    \
    array_s_c_eigen,                                          \
    /*array_c_s_eigen,*/                                            \
    array_s_s_eigen,                                            \
    /*array_c_c_eigen,*/                                            \
                                                                \
    /* check adapted structure types */                         \
    Namespace::InNamespace,                                     \
    Simple,                                                     \
    SimpleWithPadding,                                          \
    SimpleEigen,                                                \
    SimpleEigenWithPadding,                                     \
    Inner,                                                      \
    Outer::Mid,                                                 \
    Outer                                                       \
                                                                \
    /* check tuple types */                                     \
    /*std::tuple<int>,                                          \
    std::tuple<int, char>,                                      \
    std::tuple<int, char, array_s_c_int, array_s_c_eigen>,      \
    std::tuple<Simple, SimpleEigen, Inner, Outer::Mid, Outer>*/

// ////////////////////////////////////////////////////////////////////////// //
// ------------------------------- unit tests ------------------------------- //
// ////////////////////////////////////////////////////////////////////////// //

typedef ::testing::Types<TYPES_TO_CHECK> BasicRdWrApTestTypes;

// the values have to be small (otherwise the output file will be large)
//used as chunk size
constexpr std::size_t chunk_sz = 4;
//amount of data generated by calling generate<>
constexpr std::size_t data_sz = 2 * chunk_sz;
//offset when writing into the middle of a chunk
constexpr std::size_t in_chunk_offset = chunk_sz / 2;

template<class, class = void>
struct has_idx : std::false_type {};

template<class T>
struct has_idx<T, std::void_t<decltype(&T::idx)>> : std::true_type {};

template<class T>
std::vector<T> generate(std::size_t seed) {
    std::vector<T> vec (data_sz);
    std::mt19937 gen{seed};
    std::uniform_int_distribution<std::uint8_t> dist{0, 255};
    std::uint8_t* begin = static_cast<std::uint8_t*>(static_cast<void*>(&vec.front()));
    std::uint8_t* end = static_cast<std::uint8_t*>(static_cast<void*>(&vec.back() + 1));
    for(std::uint8_t* it = begin; it < end; ++it )
        *it = dist(gen);
    if constexpr(has_idx<T>::value)
        for(int i=0; i < vec.size(); i++ )
            vec[i].idx = i;
    return vec;
}


#include "basic_read_write_append_test_cases.hpp"

TYPED_TEST(BasicRdWrApTest, WriteStructDirectly) {
    if constexpr(std::is_same_v<SimpleEigen,TypeParam>)
    {
        const auto fd = this->fd;
        const auto path = this->name;
        {
            const SimpleEigen data = generate<TypeParam>(0).front();
            h5::write(fd, path, data);
            const auto read = h5::read<TypeParam>(fd, path);
            //        EXPECT_TRUE(binaryEqual(data, read))
            //        binaryCompare(data, read);
        }
    }
}

/*----------- BEGIN TEST RUNNER ---------------*/
H5CPP_TEST_RUNNER( int argc, char**  argv );
/*----------------- END -----------------------*/


