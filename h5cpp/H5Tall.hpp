/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */

#ifndef H5CPP_TALL_HPP
#define H5CPP_TALL_HPP

#include <boost/current_function.hpp>
#include <boost/core/demangle.hpp>


namespace h5 {
    template<class T> hid_t register_struct(){ return H5I_UNINIT; }
}

namespace h5::impl::detail {
    /* This structure contains meta information about a type.
     * If specialized these members have to be added:
     * - static constexpr bool close_type  -> whether the type should be closed
     * - static ::hid_t create()           -> creates the type (does not close it)
     */
    template <class T>
    struct type_meta_info {
        static constexpr bool specialized = false;
        static constexpr bool close_type  = false;
        static ::hid_t create() { return H5I_UNINIT; }
    };

    /* Specializations of type_meta_info for all bsic types 
     */
    #define basic_type_meta_info(T, H5T)                                        \
        template <>                                                             \
        struct type_meta_info<T> {                                              \
            static constexpr bool specialized = true;                           \
            static constexpr bool close_type = false;                           \
            static ::hid_t create() { return H5T; }                             \
        }
    basic_type_meta_info(          char     , H5T_NATIVE_CHAR    );
    basic_type_meta_info( unsigned char     , H5T_NATIVE_UCHAR   );
    basic_type_meta_info(          short    , H5T_NATIVE_SHORT   );
    basic_type_meta_info( unsigned short    , H5T_NATIVE_USHORT  );
    basic_type_meta_info(          int      , H5T_NATIVE_INT     );
    basic_type_meta_info( unsigned int      , H5T_NATIVE_UINT    );
    basic_type_meta_info(          long     , H5T_NATIVE_LONG    );
    basic_type_meta_info( unsigned long     , H5T_NATIVE_ULONG   );
    basic_type_meta_info(          long long, H5T_NATIVE_LLONG   );
    basic_type_meta_info( unsigned long long, H5T_NATIVE_ULLONG  );
    basic_type_meta_info( float             , H5T_NATIVE_FLOAT   );
    basic_type_meta_info( double            , H5T_NATIVE_DOUBLE  );
    basic_type_meta_info( long double       , H5T_NATIVE_LDOUBLE );
    basic_type_meta_info( bool              , H5T_NATIVE_HBOOL   );
    #undef basic_type_meta_info

    /* Specializations of type_meta_info for all carray types.
     * The array can be multidimensional and mixed with std::array.
     */
    template <class T, std::size_t N>
    struct type_meta_info <std::array<T, N>> {
        using type = std::array<T, N>;
        /* These templates determine the dimensions of a multidimensional carray
         */
        template<class ElemT, std::size_t...Ds>
        struct array_info {
            static constexpr hsize_t rank = sizeof...(Ds);
            static constexpr hsize_t bounds[rank] = {Ds...};
            using type = ElemT;
            using type_info = type_meta_info<ElemT>;
        };
        template<class ElemT, std::size_t D, std::size_t...Ds>
        struct array_info<ElemT[D], Ds...> :
            array_info<ElemT, Ds..., D>
        {};
        template<class ElemT, std::size_t D, std::size_t...Ds>
        struct array_info<std::array<ElemT, D>, Ds...> :
            array_info<ElemT, Ds..., D>
        {};
        static constexpr bool specialized = true;
        static constexpr bool close_type = true;
        static ::hid_t create() {
            using info = array_info<T,N>;
            using element_type = typename info::type;
            ::hid_t subt = info::type_info::create();
            ::hid_t t = H5Tarray_create(
                subt,
                info::rank,
                info::bounds
            );
            if constexpr(info::type_info::close_type) {
                H5Tclose(subt);
            }
            return t;
        }
    };

    /* Specializations of type_meta_info for all std::array types.
     * The array can be multidimensional and mixed with carray.
     */
    template <class T, std::size_t N>
    struct type_meta_info <T[N]> : type_meta_info <std::array<T, N>>
    {};

    #if defined(EIGEN_CORE_H) || defined(H5CPP_USE_EIGEN3)
    /* Specializations of type_meta_info for all fixed size Eigen matrice types.
     */
    template <class Scalar, int Rs, int Cs, int Options>
    struct type_meta_info <Eigen::Matrix<Scalar, Rs, Cs, Options, Rs, Cs >> {
        using type = Eigen::Matrix<Scalar, Rs, Cs, Options, Rs, Cs>;
        static_assert(Rs > 0);
        static_assert(Cs > 0);
        static constexpr bool specialized = true;
        static constexpr bool close_type = true;
        static ::hid_t create() {
            ::hid_t subt = type_meta_info<Scalar>::create();
            constexpr hsize_t bounds[2] = {Rs, Cs};
            ::hid_t t = H5Tarray_create(
                subt,
                2,
                bounds
            );
            if constexpr(type_meta_info<Scalar>::close_type) {
                H5Tclose(subt);
            }
            return t;
        }
    };
    #endif
    
    
    /* Inserts the type with the given name and offset into the parent type.
     * This function is a convenience function.
     */
    template<class T, class C>
    inline void type_meta_info_insert(::hid_t parent, const char* n, size_t o, T C::*)
    {
        static_assert(type_meta_info<T>::specialized);
        ::hid_t t = type_meta_info<T>::create();
        H5Tinsert(parent, n, o, t);
        if constexpr(type_meta_info<T>::close_type) {
            H5Tclose(t);
        }
    }
}
//#if __has_include(<boost/preprocessor/seq/for_each.hpp>)
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define _h5cpp_stringify_impl(...) #__VA_ARGS__
#define _h5cpp_stringify(...) _h5cpp_stringify_impl(__VA_ARGS__)

#define _h5cpp_variadic_for_each(macro, data, ...) BOOST_PP_SEQ_FOR_EACH(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define _h5cpp_type_elem(R, data, child)  type_meta_info_insert(t, _h5cpp_stringify(child), HOFFSET(data, child), &data::child);
#define H5CPP_ADAPT_AND_REGISTER(T,...)                                         \
    namespace h5::impl::detail {                                                \
        template <> struct type_meta_info<T> {                                  \
            /* required for HOFFSET */                                          \
            static_assert(std::is_standard_layout_v<T>);                        \
            static constexpr bool specialized = true;                           \
            static constexpr bool close_type = true;                            \
            static ::hid_t create() {                                           \
                ::hid_t t = H5Tcreate(H5T_COMPOUND, sizeof(T));                 \
                _h5cpp_variadic_for_each(_h5cpp_type_elem, T, __VA_ARGS__)      \
                return t;                                                       \
            }                                                                   \
        };                                                                      \
    }                                                                           \
    namespace h5 {                                                              \
        template <> struct name<T> {                                            \
            static constexpr char const * value = _h5cpp_stringify(T);          \
        };                                                                      \
    }
//#endif

/* template specialization from hid_t< .. > type which provides syntactic sugar in the form
 * h5::dt_t<int> dt; 
 * */
namespace h5::impl::detail {
	template<class T> // parent type, data_type is inherited from, see H5Iall.hpp top section for details 
	using dt_p = hid_t<T,H5Tclose,true,true,hdf5::any>;
	/*type id*/
	template<class T>
	struct hid_t<T,H5Tclose,true,true,hdf5::type> : public dt_p<T> {
		using parent = dt_p<T>;
		using parent::hid_t; // is a must because of ds_t{hid_t} ctor 
		using hidtype = T;
		using info = type_meta_info<hidtype>;
		hid_t() : parent( info::specialized ? info::create() : H5I_UNINIT){};
	};
	template <class T> using dt_t = hid_t<T,H5Tclose,true,true,hdf5::type>;
}

/* template specialization is for the preceding class, and should be used only for HDF5 ELEMENT types
 * which are in C/C++ the integral types of: char,short,int,long, ... and C POD types. 
 * anything else, the ones which are considered objects/classes are broken down into integral types + container 
 * then pointer read|write is obtained to the continuous slab and delegated to h5::read | h5::write.
 * IF the data is not in a continuous memory region then it must be copied! 
 */

#define H5CPP_REGISTER_TYPE_( C_TYPE, H5_TYPE )                                           \
namespace h5::impl::detail {                                                              \
	template <> struct hid_t<C_TYPE,H5Tclose,true,true,hdf5::type> : public dt_p<C_TYPE> {\
		using parent = dt_p<C_TYPE>;                                                      \
		using parent::hid_t;                                                              \
		using hidtype = C_TYPE;                                                           \
		hid_t() : parent( H5Tcopy( H5_TYPE ) ) { 										  \
			hid_t id = static_cast<hid_t>( *this );                                       \
			if constexpr ( std::is_pointer<C_TYPE>::value )                               \
					H5Tset_size (id,H5T_VARIABLE), H5Tset_cset(id, H5T_CSET_UTF8);        \
		}                                                                                 \
	};                                                                                    \
}                                                                                         \
namespace h5 {                                                                            \
	template <> struct name<C_TYPE> {                                                     \
		static constexpr char const * value = #C_TYPE;                                    \
	};                                                                                    \
}                                                                                         \

/* registering integral data-types for NATIVE ones, which means all data is stored in the same way 
 * in file and memory: TODO: allow different types for file storage
 * */
	H5CPP_REGISTER_TYPE_(bool,H5T_NATIVE_HBOOL)

	H5CPP_REGISTER_TYPE_(unsigned char, H5T_NATIVE_UCHAR) 			H5CPP_REGISTER_TYPE_(char, H5T_NATIVE_CHAR)
	H5CPP_REGISTER_TYPE_(unsigned short, H5T_NATIVE_USHORT) 		H5CPP_REGISTER_TYPE_(short, H5T_NATIVE_SHORT)
	H5CPP_REGISTER_TYPE_(unsigned int, H5T_NATIVE_UINT) 			H5CPP_REGISTER_TYPE_(int, H5T_NATIVE_INT)
	H5CPP_REGISTER_TYPE_(unsigned long int, H5T_NATIVE_ULONG) 		H5CPP_REGISTER_TYPE_(long int, H5T_NATIVE_LONG)
	H5CPP_REGISTER_TYPE_(unsigned long long int, H5T_NATIVE_ULLONG) H5CPP_REGISTER_TYPE_(long long int, H5T_NATIVE_LLONG)
	H5CPP_REGISTER_TYPE_(float, H5T_NATIVE_FLOAT) 					H5CPP_REGISTER_TYPE_(double, H5T_NATIVE_DOUBLE)
	H5CPP_REGISTER_TYPE_(long double,H5T_NATIVE_LDOUBLE)

	H5CPP_REGISTER_TYPE_(char*, H5T_C_S1)


#define H5CPP_REGISTER_STRUCT( POD_STRUCT ) H5CPP_REGISTER_TYPE_( POD_STRUCT, h5::register_struct<POD_STRUCT>() )

/* type alias is responsible for ALL type maps through H5CPP if you want to screw things up
 * start here.
 * template parameters:
 *  hid_t< C_TYPE being mapped, conversion_from_capi, conversion_to_capi, marker_for_this_type>
 * */


namespace h5 {
	template <class T> using dt_t = h5::impl::detail::hid_t<T,H5Tclose,true,true,h5::impl::detail::hdf5::type>;

	template<class T>
	hid_t copy( const h5::dt_t<T>& dt ){
		hid_t id = static_cast<hid_t>(dt);
		H5Iinc_ref( id );
		return id;
	}
}


template<class T>
inline std::ostream& operator<<(std::ostream &os, const h5::dt_t<T>& dt) {
	hid_t id = static_cast<hid_t>( dt );
	os << "data type: " << h5::name<T>::value << " ";
	os << ( std::is_pointer<T>::value ? "pointer" : "value" );
	os << ( H5Iis_valid( id ) > 0 ? " valid" : " invalid");
	return os;
}

#endif
