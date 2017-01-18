#ifndef FAIF_UTILS_POWER
#define FAIF_UTILS_POWER

namespace faif {

	/** the template metaprogramming for calculating power of double,
		based on sqare and multiply method, i.e. if n = 2^m (n is m-th power of 2),
		x^n = (...((x^2)^2)...)^2 (m-times)
	*/
	template <unsigned n> double int_power(double x) {
		return int_power<2>( int_power<n/2>(x) )*int_power<n%2>(x);
	}

	/** the specialisation for power = 2 */
	template <> double int_power<2>(double x) {
		return x*x;
	}

	/** the specialisation for power = 1 */
	template <> double int_power<1>(double x) {
		return x;
	}

	/** the template specialisation for power = 0 */
	template <> double int_power<0>(double) {
		return 1.0;
	}
}


#endif
