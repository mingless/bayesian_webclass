#ifndef FAIF_GAUSS_ELIMINATOR
#define FAIF_GAUSS_ELIMINATOR

// plik zawiera deklaracje przeksztalcenia - prosty eliminator Gaussowski (niestety O(N^3) )

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc8.0 generuje warning dla boost::numeric::matrix
#pragma warning(disable:4996)
#endif

#include <cassert>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace faif {

	template<typename T>
    //parametry przekazywane przez referencje, sa niszczone podczas obliczen
	boost::numeric::ublas::vector<T> GaussEliminatorRef(boost::numeric::ublas::matrix<T>& m,
														boost::numeric::ublas::vector<T>& y) {

		//          std::assert( y.size() == m.size1() && y.size() == m.size2() );

		int n = static_cast<int>(y.size() ); //wielkosc

		//tworzy macierz trojkatna
		for (int i = 0; i < n; ++i) {
			int max = i; //maksymalny element
			for (int j = i + 1; j < n; ++j)
				if( m(j,i) > m(max,i) )
					max = j;

			for (int j = 0; j < n; ++j)
				std::swap( m(max,j), m(i,j) );
			std::swap( y(max), y(i) );

			for(int k = i + 1; k < n; ++k)
				y(k) -= m(k,i)/m(i,i) * y(i);
			for (int j = n-1; j >= i; --j)
				for (int k = i + 1; k < n; ++k)
					m(k,j) -= m(k,i)/m(i,i) * m(i,j);
		}

		//eliminacja - oblicza wyniki
		for (int i = n - 1; i >= 0; --i) {
			y(i) = y(i)/m(i,i);
			m(i,i) = 1;
			for (int j = i - 1; j >= 0; --j) {
				y(j) -= m(j,i)*y(i);
				m(j,i) = 0;
			}
		}
		return y;
	}

	template<typename T>
    //kopiuje parametry, wywoluje GaussEliminatorRef, parametry nie sa niszczone
	boost::numeric::ublas::vector<T> GaussEliminator(const boost::numeric::ublas::matrix<T>& m,
													 const boost::numeric::ublas::vector<T>& y) {
		typename boost::numeric::ublas::matrix<T> mm(m);
		typename boost::numeric::ublas::vector<T> yy(y);
		return GaussEliminatorRef(mm,yy);
	}


} //namespace faif

#endif //FAIF_GAUSS_ELIMINATOR
