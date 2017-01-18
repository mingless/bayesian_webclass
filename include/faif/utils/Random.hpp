#ifndef FAIF_RANDOM_H
#define FAIF_RANDOM_H

// the random generators based on boost::random ( mt19937),
// the generator is a synchronized (boost::mutex) singleton

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc9.0 generuje smieci dla boost/date_time
#pragma warning(disable:4244)
//msvc9.0 generuje smieci dla boost/random
#pragma warning(disable:4512)

#endif

#include <algorithm>
#include <boost/random.hpp>
#include <boost/thread/mutex.hpp>

namespace faif {

    // /** macros defined in stdandard library, but here are defined again,
    //     because of Visual Studio incompatibility with standard library.
    //     VC defines macro min and max in windows.h
    // */
    // template<typename T> inline const T& min_val(const T& a, const T& b) {
    //     if(b < a) return b;
    //     return a;
    // }

    // template<typename T> inline const T& max_val(const T& a, const T& b) {
    //     if(a < b) return b;
    //     return a;
    // }

	/** \brief the singleton, synchronized proxy to boost::Random */
	class RandomSingleton {
	public:

		/** the sileton getInstance method.
			Not using double-check pattern, because it is assumed, that random generator is initialized
			in single thread
		*/
		static RandomSingleton& getInstance() {
			static RandomSingleton singleton;
			return singleton;
		}
		//accessor - mutex, access to generator
		boost::mutex& getAccess(){ return access_; }

		//accessor - random generator from boost
		boost::mt19937& getRng(){ return rng_;}
	private:
		//private constructor
		RandomSingleton() {
			//init the random generator
			rng_.seed( static_cast<unsigned int>( time( 0L ) ) );
		}
		//noncopyable
		RandomSingleton(const RandomSingleton&);
		//noncopyable
		RandomSingleton& operator=(const RandomSingleton&);
		//private destructor
		~RandomSingleton() { }

		//mutex, access to generator
		boost::mutex access_;

		//random generator from boost
		boost::mt19937 rng_;
	};


    /** \brief the uniform distribution for double, in given range, e.g. <0,1), uses RandomSingleton */
    class RandomDouble  {
    public:
        /** \brief the c-tor random variable generator in range <0,1), uniform distribution */
        explicit RandomDouble()
			: access_(RandomSingleton::getInstance().getAccess() ),
			  gen_(RandomSingleton::getInstance().getRng(),
				   boost::uniform_real<double>(0.0, 1.0) )
		{ }

        /** \brief the c-tor random variable generator in range <min,max), uniform distribution */
        explicit RandomDouble(double min_v, double max_v)
			: access_(RandomSingleton::getInstance().getAccess() ),
			  gen_(RandomSingleton::getInstance().getRng(),
				   boost::uniform_real<double>( (std::min)(min_v, max_v), (std::max)(min_v,max_v) ) )
		{ }


        RandomDouble(const RandomDouble& r) : access_(r.access_), gen_(r.gen_) {}
        ~RandomDouble(){}
        /** \brief the method to generate the random variable in given range, uniform distribution */
        double operator()() {
			boost::mutex::scoped_lock scoped_lock(access_);
			return gen_();
        }
    private:
        typedef boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > Generator;

        RandomDouble& operator=(const RandomDouble&); //!< assignment not allowed
        boost::mutex& access_;
        Generator gen_;
    };

    /** \brief the uniform distribution for int, in range <min,max>, uses RandomSingleton */
    class RandomInt {
    public:
        explicit RandomInt(int min, int max)
			: access_(RandomSingleton::getInstance().getAccess() ),
			  gen_( RandomSingleton::getInstance().getRng(), boost::uniform_int<int>(min, max) )
		{ }

		RandomInt(const RandomInt& r) : access_(r.access_), gen_(r.gen_) {}
        ~RandomInt(){}

        /** \brief the method to generate the random variable in range <min, max>, uniform distribution */
        int operator()() {
			boost::mutex::scoped_lock scoped_lock(access_);
			return gen_();
        }
    private:
        typedef boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > Generator;

        RandomInt& operator=(const RandomInt&); //!< assignment not allowed
        boost::mutex& access_;
        Generator gen_;
    };

    /** \brief the normal distribution for double, for given mean (mi) and standard deviation (sigma),
        uses RandomSingleton
    */
    class RandomNormal  {
    public:
        /** \brief the c-tor random variable generator, normal distribution */
        explicit RandomNormal(double mi, double sigma)
			: access_(RandomSingleton::getInstance().getAccess() ),
			  gen_( RandomSingleton::getInstance().getRng(), boost::normal_distribution<double>(mi, sigma) )
		{}

        RandomNormal(const RandomNormal& r) : access_(r.access_), gen_(r.gen_) {}
        ~RandomNormal(){}
        /** \brief the method to generate the random variable with normal distribution */
        double operator()() {
			boost::mutex::scoped_lock scoped_lock(access_);
			return gen_();
        }
    private:
        typedef boost::variate_generator<boost::mt19937&, boost::normal_distribution<double> > Generator;

        RandomDouble& operator=(const RandomNormal&); //!< assignment not allowed
        boost::mutex& access_;
        Generator gen_;
    };


} //namespace faif

#endif //FAIF_RANDOM_H
