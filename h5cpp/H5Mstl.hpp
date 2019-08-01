
/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 *
 */

/**
 * @file This file contains meta functions and normal functions
 * to deal with c++ types. (e.g. to get the element type.)
 */

#ifndef  H5CPP_STL_HPP 
#define  H5CPP_STL_HPP

namespace h5::impl {
    template <template<class...> class Temp, class T, class = void>
    struct enable_if_specialized {};
    
    template <template<class...> class Temp, class T>
    struct enable_if_specialized<Temp, T, std::void_t<decltype(&Temp<T>::call)>> {
        template <class R, class...Args> static R get_return_type(R (*)(Args...));
        using type = decltype (get_return_type(&Temp<T>::call));
    };
    
    template <template<class...> class Temp, class T>
    using enable_if_specialized_t = typename enable_if_specialized<Temp, T>::type;
    
    
    
    template <class T, class = void> struct decay{ typedef T type; };    
    template <class T, class = void> struct rank : public std::integral_constant<size_t,0>{};
    template <class T, class = void> struct read_data_access;
    template <class T, class = void> struct write_data_access;
    template <class T, class = void> struct size_of_dimensions;
    template <class T, class = void> struct is_supported_element_type : std::false_type {};    
    
    template <class T> using decay_t = typename decay<T>::type;
    
    template <class T> static constexpr bool rank_v = rank<T>::value;
    template <class T> static constexpr bool is_supported_element_type_v = is_supported_element_type<T>::value;
    
    
    }

namespace h5::impl::detail {
/* This structure contains meta information about a type.
     * If specialized these members have to be added:
     * - static constexpr bool close_type  -> whether the type should be closed
     * - static ::hid_t create()           -> creates the type (does not close it)
     * 
     *      static       T* data(T& ref)
     *      static const T* data(const T& ref)
     * 
     *      static constexpr bool has_static_rank  = false; // -> constexpr std::size_t static_rank
     *      static constexpr bool has_static_size  = false; // -> constexpr std::array<std::size_t,>...
     * 
     *      static std::array<std::size_t, 
     */
template <class T, class = void>
struct type_meta_info {
    using type = T;
    static constexpr bool is_specialized = false;
    static constexpr bool close_type  = false;
    static ::hid_t create() { return H5I_UNINIT; }
    
    static constexpr bool has_static_rank  = false; // -> constexpr std::size_t static_rank
    static constexpr bool has_static_size  = false; // -> constexpr std::array<std::size_t,>...
    
    
};
}
/* Specializations of type_meta_info for all bsic types 
     */
#define basic_type_meta_info(T, H5T)                                            \
    namespace h5::impl::detail {                                                \
        template <>                                                             \
        struct type_meta_info<T, void> {                                        \
            using type = T;                                                     \
            using element_type = T;                                             \
            static constexpr bool is_specialized = true;                        \
                                                                                \
            static constexpr bool has_static_rank = true;                       \
            static constexpr std::size_t static_rank = 0;                       \
                                                                                \
            static constexpr bool has_static_size = true;                       \
            static constexpr std::array<std::size_t, 0> static_size = {};       \
            static auto size(const T) { return static_size; }                   \
                                                                                \
            static constexpr bool close_type = false;                           \
            static ::hid_t create() { return H5T; }                             \
                                                                                \
            static T* data(T& ref) { return &ref; }                             \
            static const T* data(const T& ref) { return &ref; }                 \
        };                                                                      \
    }                                                                           \
    namespace h5::impl {                                                        \
        template <>                                                             \
        struct is_supported_element_type<T, void> : std::true_type {};           \
    }
basic_type_meta_info(          char     , H5T_NATIVE_CHAR    )
basic_type_meta_info( unsigned char     , H5T_NATIVE_UCHAR   )
basic_type_meta_info(          short    , H5T_NATIVE_SHORT   )
basic_type_meta_info( unsigned short    , H5T_NATIVE_USHORT  )
basic_type_meta_info(          int      , H5T_NATIVE_INT     )
basic_type_meta_info( unsigned int      , H5T_NATIVE_UINT    )
basic_type_meta_info(          long     , H5T_NATIVE_LONG    )
basic_type_meta_info( unsigned long     , H5T_NATIVE_ULONG   )
basic_type_meta_info(          long long, H5T_NATIVE_LLONG   )
basic_type_meta_info( unsigned long long, H5T_NATIVE_ULLONG  )
basic_type_meta_info( float             , H5T_NATIVE_FLOAT   )
basic_type_meta_info( double            , H5T_NATIVE_DOUBLE  )
basic_type_meta_info( long double       , H5T_NATIVE_LDOUBLE )
basic_type_meta_info( bool              , H5T_NATIVE_HBOOL   )
#undef basic_type_meta_info

namespace h5::impl::detail {
    /* Specializations of type_meta_info for all carray types.
         * The array can be multidimensional and mixed with std::array.
         */
    template <class T, std::size_t N>
    struct type_meta_info <std::array<T, N>, void> {
    private:
        /* These templates determine the dimensions of a multidimensional carray
             */
        template<class ElemT, std::size_t...Ds>
        struct array_info {
            static constexpr hsize_t rank = sizeof...(Ds);
            static constexpr hsize_t bounds[rank] = {Ds...};
            
            static constexpr std::array<std::size_t, rank> bounds_std_array = {Ds...};
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
        using info = array_info<T,N>;
    public:
        using element_type = typename info::type;
        using type = std::array<T, N>;
        static constexpr bool is_specialized = true;
        static constexpr bool close_type = true;
        
        static constexpr bool has_static_rank  = true;
        static constexpr std::size_t static_rank = info::rank;
        
        static constexpr bool has_static_size  = true;
        static constexpr std::array<std::size_t, static_rank> static_size = info::bounds_std_array;
        static auto size(const type&) { return static_size; }
                
        static ::hid_t create() {
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
        
        static element_type* data(type& ref) { return ref.data(); }
        static const element_type* data(const type& ref) { return ref.data(); }
    };
    
    /* Specializations of type_meta_info for all std::array types.
         * The array can be multidimensional and mixed with carray.
         */
    template <class T, std::size_t N>
    struct type_meta_info <T[N], void> : type_meta_info <std::array<T, N>>
    {
    private:
        using super = type_meta_info <std::array<T, N>>;
    public:
        using type = T[N];
        using element_type = typename super::element_type;
        
        static auto size(const type&) { return super::static_size; }
        static element_type* data(type& ref) { return ref; }
        static const element_type* data(const type& ref) { return ref; }
    };
    
    /* Inserts the type with the given name and offset into the parent type.
         * This function is a convenience function.
         */
    template<class T, class C>
    inline void type_meta_info_insert(::hid_t parent, const char* n, size_t o, T C::*)
    {
        static_assert(type_meta_info<T>::is_specialized);
        ::hid_t t = type_meta_info<T>::create();
        H5Tinsert(parent, n, o, t);
        if constexpr(type_meta_info<T>::close_type) {
            H5Tclose(t);
        }
    }
}
namespace h5::impl {
    template <class T, std::size_t N>
    struct is_supported_element_type<T[N], void> : std::true_type {};
    template <class T, std::size_t N>
    struct is_supported_element_type<std::array<T, N>, void> : std::true_type {};
}
namespace h5::utils {
template <class T, std::size_t N>
constexpr bool is_supported<std::array<T, N>> = true;
template <class T, std::size_t N>
constexpr bool is_supported<T[N]> = true; 
    }
    
namespace h5::impl {

/*STL: */
	// 2.) filter is_xxx_type
	// 4.) write access
	// 5.) obtain dimensions of extents
	// 6.) ctor with right dimensions

	// 1.) object -> H5T_xxx
	/**
	 * @brief This meta function returns the element type of a class
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
//	template <class T, class...> struct decay{ typedef T type; };
//	template <class T, class...Ts> using decay_t = typename decay<T, Ts...>::type;

	template <class T> struct decay<const T>{ typedef T type; };
	template <class T> struct decay<const T*>{ typedef T* type; };
	template <class T> struct decay<std::basic_string<T>>{ typedef T* type; };
	template <class T, signed N> struct decay<const T[N]>{ typedef T* type; };
	template <class T, signed N> struct decay<T[N]>{ typedef T* type; };

	template <class T> struct decay<std::initializer_list<const T*>>{ typedef const T* type; };
	template <class T> struct decay<std::initializer_list<T*>>{ typedef T* type; };
	template <class T> struct decay<std::initializer_list<T>>{ typedef T type; };

	template <class T> struct decay<std::vector<const T*>>{ typedef const T* type; };
	template <class T> struct decay<std::vector<T*>>{ typedef T* type; };
	template <class T> struct decay<std::vector<T>>{ typedef T type; };

	// helpers
	template< class T >
	inline constexpr bool is_scalar_v = std::is_integral_v<T> || std::is_pod_v<T> || std::is_same_v<T,std::string>;

	template <class T, class B = impl::decay_t<T>>
		using is_rank01 = std::bool_constant<
			std::is_same_v<T,std::initializer_list<B>> ||
			std::is_same_v<T,std::vector<B>> >;

	/**
	 * @brief This meta function returns the number of dimensions encapsulated
	 * in the given c++ type.
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
//	template<class T, class = void> struct rank : public std::integral_constant<size_t,0>{};
	template<> struct rank<std::string>: public std::integral_constant<size_t,1>{};
	template<class T> struct rank<T*>: public std::integral_constant<size_t,1>{};
	template<class T> struct rank<std::vector<T>>: public std::integral_constant<size_t,1>{};

	// 3.) read access
	/**
	 * @brief This function provides const read access to the data buffer
	 * of a type.
	 *
	 * The access is provided as a const pointer to the element type.
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
//    template <class T, class = void>
//    struct read_data_access;
    
    
    template <class T>
    struct read_data_access<T, std::enable_if_t<detail::type_meta_info<T>::is_specialized>>
    {
        static inline auto call( const T& ref ){ return detail::type_meta_info<T>::data(ref); }
    };
    
    template <class T>
    struct read_data_access
    <
        T,
        std::enable_if_t
        <
            !detail::type_meta_info<T>::is_specialized
            && (std::is_integral_v<T> || std::is_pod_v<T>)
        >
    >
    {
        static inline auto call( const T& ref ){ return &ref; }
    };
    
    template <class T>
    struct read_data_access<std::initializer_list<T>, std::enable_if_t<impl::is_scalar_v<T>>>
    {
        static inline auto call( const std::initializer_list<T>& ref ){ return ref.begin(); }
    };
    
    template <>
    struct read_data_access<std::initializer_list<const char*>, void>
    {
        static inline auto call( const std::initializer_list<const char*>& ref ){ return ref.begin(); }
    };
    
    template <>
    struct read_data_access<std::string, void>
    {
        static inline auto call( const std::string& ref ){ return ref.c_str(); }
    };
    
    template <class T>
    struct read_data_access<std::vector<T>>
    {
        static inline auto call( const std::vector<T>& ref ){ return ref.data(); }
    };
    
    template <class T> inline enable_if_specialized_t<read_data_access, T>
    data( const T& ref ){ return read_data_access<T>::call(ref); } ///TODO enable_if
    
	// 4.) write access
	/**
	 * @brief This function provides read / write access to the data buffer
	 * of a type.
	 *
	 * The access is provided as a pointer to the element type.
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
//    template <class T, class = void>
//    struct write_data_access;
    
    template <class T>
    struct write_data_access<T, std::enable_if_t<detail::type_meta_info<T>::is_specialized>>
    {
        static inline auto call(T& ref ){ return detail::type_meta_info<T>::data(ref); }
    };
    
    template <class T>
    struct write_data_access
        <
            T,
            std::enable_if_t
            <
                !detail::type_meta_info<T>::is_specialized
                && (std::is_integral_v<T> || std::is_pod_v<T>)
                >
            >
    {
        static inline auto call( T& ref ){ return &ref; }
    };
    
    template <class T>
    struct write_data_access<std::vector<T>, void>
    {
        static inline auto call(std::vector<T>& ref ){ return ref.data(); }
    };
    
    template <class T> inline enable_if_specialized_t<write_data_access, T>
    data( T& ref ){ return write_data_access<T>::call(ref); }
        
	// 5.) obtain dimensions of extents
	/**
	 * @brief This function provides the sizes of the dimensions encapsulated in
	 * the given type.
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
//    template <class T, class = void>
//    struct size_of_dimensions;
    
    template <class T>
    struct size_of_dimensions<T, std::enable_if_t<detail::type_meta_info<T>::is_specialized>>
    {
        static inline auto call(const T& ref ){ return detail::type_meta_info<T>::size(ref); }
    };
    
    template <class T>
    struct size_of_dimensions<T, std::enable_if_t<
                                     !detail::type_meta_info<T>::is_specialized
                                     && impl::is_scalar_v<T>
                                     >>
    {
        static inline auto call(const T& ref ){ return std::array<size_t,0>{}; }
    };
    
    template <class T>
    struct size_of_dimensions<T, std::enable_if_t<
                                     !detail::type_meta_info<T>::is_specialized
                                     && impl::is_rank01<T>::value
                                     >>
    {
        static inline auto call(const T& ref ){ return std::array<size_t,1>{ref.size()}; }
    };
    
    template <class T> inline enable_if_specialized_t<size_of_dimensions, T>
    size( const T& ref ){ return size_of_dimensions<T>::call(ref); }
    
	// 6.) ctor with right dimensions
	/**
	 * @brief This function calls the constructor of a type initializing any
	 * encapsulated dimensions to the given sizes.
	 * @param dims Size of encapsulated dimensions
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
	template <class T> struct get {
	   	static inline T ctor( std::array<size_t,impl::rank<T>::value> dims ){
			return T(); }};
	template<class T>
	struct get<std::vector<T>> {
		static inline std::vector<T> ctor( std::array<size_t,1> dims ){
			return std::vector<T>( dims[0] );
	}};
}
#endif

#if ! defined (_detail_H5CPP_type_meta_info_eigen) && (defined(EIGEN_CORE_H) || defined(H5CPP_USE_EIGEN3))
#define _detail_H5CPP_type_meta_info_eigen
namespace h5::impl::detail {
    /* Specializations of type_meta_info for all fixed size Eigen matrice types.
     */
    template <class Scalar, int Rs, int Cs, int Options>
        struct type_meta_info <
            Eigen::Matrix<Scalar, Rs, Cs, Options, Rs, Cs >,
        std::enable_if_t<(Rs > 0 && Cs > 0 )>> {
        using type = Eigen::Matrix<Scalar, Rs, Cs, Options, Rs, Cs>;
        
        static constexpr bool has_static_rank  = true;
        static constexpr std::size_t static_rank = 2;
        
        static constexpr bool has_static_size  = true;
        static constexpr std::array<std::size_t, static_rank> static_size = {Rs, Cs};
        static auto size(const type&) { return static_size; }
        
        static constexpr bool is_specialized = true;
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
        
        static Scalar* data(type& ref) { return ref.data(); }
        static const Scalar* data(const type& ref) { return ref.data(); }
    };
    
    /* Specializations of type_meta_info for all fixed size Eigen matrice types.
     */
    template <class Scalar, int Options>
    struct type_meta_info <Eigen::Quaternion<Scalar, Options>, void> {
        using type = Eigen::Quaternion<Scalar, Options>;
        
        static constexpr bool has_static_rank  = true;
        static constexpr std::size_t static_rank = 1;
        
        static constexpr bool has_static_size  = true;
        static constexpr std::array<std::size_t, 1> static_size = {4};
        static auto size(const type&) { return static_size; }
        
        static constexpr bool is_specialized = true;
        static constexpr bool close_type = true;
        static ::hid_t create() {
            ::hid_t subt = type_meta_info<Scalar>::create();
            constexpr hsize_t bounds[1] = {4};
            ::hid_t t = H5Tarray_create(subt, 1, bounds);
            if constexpr(type_meta_info<Scalar>::close_type) {
                H5Tclose(subt);
            }
            return t;
        }
        
        static Scalar* data(type& ref) { return ref.data(); }
        static const Scalar* data(const type& ref) { return ref.data(); }
    };
}
namespace h5::utils { 
        template <class Scalar, int Rs, int Cs, int Options>
static constexpr bool is_supported<Eigen::Matrix<Scalar, Rs, Cs, Options, Rs, Cs >> = true;
        
        template <class Scalar, int Options>
        static constexpr bool is_supported<Eigen::Quaternion<Scalar, Options>> = true; 
    }
#endif

#if ! defined (_h5cpp_stringify_impl) && __has_include(<boost/preprocessor/seq/for_each.hpp>)
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#define _h5cpp_stringify_impl(...) #__VA_ARGS__
#define _h5cpp_stringify(...) _h5cpp_stringify_impl(__VA_ARGS__)

#define _h5cpp_variadic_for_each(macro, data, ...) BOOST_PP_SEQ_FOR_EACH(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define _h5cpp_type_elem(R, data, child)  type_meta_info_insert(t, _h5cpp_stringify(child), HOFFSET(data, child), &data::child);
#define H5CPP_ADAPT_AND_REGISTER_STRUCT(T,...)                                  \
    namespace h5::impl::detail {                                                \
        template <> struct type_meta_info<T> {                                  \
            /* required for HOFFSET */                                          \
            static_assert(std::is_standard_layout_v<T>);                        \
            static constexpr bool is_specialized = true;                        \
                                                                                \
            static constexpr bool has_static_rank = true;                       \
            static constexpr std::size_t static_rank = 0;                       \
                                                                                \
            static constexpr bool has_static_size = true;                       \
            static constexpr std::array<std::size_t, 0> static_size = {};       \
            static auto size(const T) { return static_size; }                   \
                                                                                \
            static constexpr bool close_type = true;                            \
            static ::hid_t create() {                                           \
                ::hid_t t = H5Tcreate(H5T_COMPOUND, sizeof(T));                 \
                _h5cpp_variadic_for_each(_h5cpp_type_elem, T, __VA_ARGS__)      \
                return t;                                                       \
            }                                                                   \
                                                                                \
            static T* data(T& ref) { return &ref; }                             \
            static const T* data(const T& ref) { return &ref; }                 \
        };                                                                      \
    }                                                                           \
    namespace h5 {                                                              \
        template <> struct name<T> {                                            \
            static constexpr char const * value = _h5cpp_stringify(T);          \
        };                                                                      \
    }                                                                           \
    namespace h5::utils {                                                       \
        template <>                                                             \
        constexpr bool is_supported<T> = true;                                  \
    }                                                                           \
    namespace h5::impl {                                                        \
        template <>                                                             \
        struct is_supported_element_type<T, void> : std::true_type {};          \
    }
#endif
