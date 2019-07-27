
/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 *
 */

/**
 * @file This file adds support for the itpp library by specializing
 * customization points.
 */

#ifndef  H5CPP_ITPP_HPP 
#define  H5CPP_ITPP_HPP


#if defined(MAT_H) || defined(H5CPP_USE_ITPP_MATRIX)
namespace h5::itpp {
		template<class T> using rowmat = ::itpp::Mat<T>;
		template <class Object, class T = impl::decay_t<Object>>
			using is_supported = std::bool_constant<std::is_same_v<Object,h5::itpp::rowmat<T>>>;
}
namespace h5::impl {
	// 1.) object -> H5T_xxx
	template <class T> struct decay<h5::itpp::rowmat<T>>{ typedef T type; };

	// get read access to datastaore
	template <class Object, class T = impl::decay_t<Object>> inline
	std::enable_if_t< h5::itpp::is_supported<Object>::value, const T*>
	data(const Object& ref ){
			return ref._data();
	}
	// read write access
	template <class Object, class T = impl::decay_t<Object>> inline
	std::enable_if_t< h5::itpp::is_supported<Object>::value, T*>
	data( Object& ref ){
			return ref._data();
	}
	template<class T> struct rank<h5::itpp::rowmat<T>> : public std::integral_constant<size_t,2>{};
	template <class T> inline std::array<size_t,2> size( const h5::itpp::rowmat<T>& ref ){ return {(hsize_t)ref.cols(),(hsize_t)ref.rows()};}
	template <class T> struct get<h5::itpp::rowmat<T>> {
		static inline h5::itpp::rowmat<T> ctor( std::array<size_t,2> dims ){
			return h5::itpp::rowmat<T>( dims[1], dims[0] );
	}};
}
#endif

#if defined(VEC_H) || defined(H5CPP_USE_ITPP_VECTOR)
namespace h5::itpp {
		template<class T> using rowvec = ::itpp::Vec<T>;
		template <class Object, class T = impl::decay_t<Object>>
			using is_supported_v = std::bool_constant<std::is_same_v<Object,h5::itpp::rowvec<T>>>;
}
namespace h5::impl {
	template <class T> struct decay<h5::itpp::rowvec<T>>{ typedef T type; };
	template <class Object, class T = impl::decay_t<Object>> inline
	std::enable_if_t< h5::itpp::is_supported_v<Object>::value, const T*>
	data(const Object& ref ){
			return ref._data();
	}
	template <class Object, class T = impl::decay_t<Object>> inline
	std::enable_if_t< h5::itpp::is_supported_v<Object>::value, T*>
	data( Object& ref ){
			return ref._data();
	}
	template<class T> struct rank<h5::itpp::rowvec<T>> : public std::integral_constant<size_t,2>{};
	template <class T> inline std::array<size_t,1> size( const h5::itpp::rowvec<T>& ref ){ return { (hsize_t)ref.size() };}
	template <class T> struct get<h5::itpp::rowvec<T>> {
		static inline h5::itpp::rowvec<T> ctor( std::array<size_t,1> dims ){
			return h5::itpp::rowvec<T>( dims[0] );
	}};
}
#endif
#endif
