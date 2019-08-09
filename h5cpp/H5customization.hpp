/**
 * @file This contains customization points for h5cpp.
 */

#ifndef  H5CPP_CUSTOMIZATION_HPP 
#define  H5CPP_CUSTOMIZATION_HPP

namespace h5::customization {

}

namespace h5::impl {
	/**
	 * @brief This meta function returns the element type of a class
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */

    template <class T, class = void> struct decay{ typedef T type; };
    
    /**
	 * @brief This struct can be specialized to provide a static constexpr member 
	 * value returning the size of the dimensions encapsulated in all objects of a
	 * given type.
	 * 
	 * The member is an std::array<std::size_t, rank>.
	 * The default is std::array<std::size_t, 0>
	 * 
	 * Example an array T[4] would return std::array<std::size_t, 1>{4}
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
    template <class T, class = void> struct static_size_of_dimensions {
        static constexpr std::array<std::size_t, 0> value{};
    };
    
    /**
	 * @brief This struct can be specialized to provide a static function 'call' 
	 * returning the sizes of the dimensions encapsulated in the given object.
	 *
	 * It defaults to static_size_of_dimensions<T>::size which defaults to
	 * std::array<std::size_t, 0>.
	 * 
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
    template <class T, class = void> struct size_of_dimensions {
        static inline auto call(const T& ref )
        { return static_size_of_dimensions<T>::value; }
    };
    
    /**
	 * @brief This struct can be specialized to provide a static constexpr member
	 * value containing the number of dimensions encapsulated in the given type.
	 *
	 * It defaults to the rank of the array returned by size_of_dimensions<T>::call
	 * which defaults to 0.
	 * 
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
    template <class T, class = void> struct rank {
        using return_t = decltype(size_of_dimensions<T>::call(std::declval<T>()));
        static constexpr std::size_t value = return_t{}.size();
    };
    
    template <class T, class = void> struct read_data_access;
    template <class T, class = void> struct write_data_access;
    
    template <class T, class = void> struct is_supported_element_type : std::false_type {};
    
    //access
    
    template <class T> using decay_t = typename decay<T>::type;
    
    template <class T> static constexpr bool rank_v = rank<T>::value;
    template <class T> static constexpr bool is_supported_element_type_v = is_supported_element_type<T>::value;


}

namespace h5 {
	/**
	 * @brief Type-name helper class for compile time id and printout.
	 *
	 * \note This is a customization point for new types.
	 *
	 * @ingroup customization-point
	 */
	template <class T> struct name {
		static constexpr char const * value = "n/a";
	};
}
#endif
