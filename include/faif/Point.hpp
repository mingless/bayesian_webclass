#ifndef FAIF_POINT_HPP_
#define FAIF_POINT_HPP_

#include <ostream>
#include <vector>
#include <list>
#include <algorithm>
#include <utility>

#include <boost/bind.hpp>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include "Value.hpp"

namespace faif {

	/** \brief Point in n-space, each component of the same type */
	template<typename Val> class Point : public std::vector<typename Val::DomainType::ValueId> {
		BOOST_CONCEPT_ASSERT((ValueConcept<Val>));

	public:
		/** \brief serialization using boost::serialization */
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */){
			ar & boost::serialization::make_nvp("PointVect",
												boost::serialization::base_object< std::vector<typename Val::DomainType::ValueId> >(*this) );
		}
	};

	/** stream operator - calls the write method */
	template<typename Val>
	std::ostream& operator<<(std::ostream& os, const Point<Val>& p) {
		for(typename std::vector<typename Val::DomainType::ValueId>::const_iterator i = p.begin(); i != p.end(); ++i) {
			os << **i << ' ';
		}
		return os;
	}

	/**
	   \brief feature init policy - use default constructor
	*/
	template<typename Feature> struct FeatureInitDefault {
		static Feature init() {
			return Feature();
		}
	};

	/** \brief point and some feature

		examples: train example or point with fitness
	*/
	template<typename Value, typename Feature,
			 template <typename> class FeatureInit = FeatureInitDefault  >
	class PointAndFeature : public Point<Value> {
		BOOST_CONCEPT_ASSERT((ValueConcept<Value>));
	public:
		PointAndFeature() : Point<Value>(), feature_(FeatureInit<Feature>::init()) { }

		PointAndFeature(const Point<Value>& p, Feature f) : Point<Value>(p), feature_(f) { }

		PointAndFeature(const PointAndFeature<Value, Feature, FeatureInit>& pf) : Point<Value>(pf), feature_(pf.feature_) { }
		PointAndFeature& operator=(const PointAndFeature<Value, Feature, FeatureInit>& right) {
                Point<Value>::operator=(right);
                feature_ = right.feature_;
                return *this;
		}
		Feature getFeature() const { return feature_; }

		/** \brief serialization using boost::serialization */
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */){
			ar & boost::serialization::make_nvp("Point", boost::serialization::base_object< Point<Value> >(*this) );
			ar & boost::serialization::make_nvp("Feature", feature_ );
		}
	private:
		Feature feature_;
	};


	/** ostream operator - for debugging */
	template <typename Value, typename Feature, template <typename> class FeatureInit >
	std::ostream& operator<<(std::ostream& os, const PointAndFeature<Value, Feature, FeatureInit>& pf) {
		os << static_cast<const Point<Value>&>(pf) << ": " << pf.getFeature();
		return os;
	}

	/** \brief Space n-dimensional, each domain of the same type */
	template<typename Domain> class Space : public std::list<Domain> {
		BOOST_CONCEPT_ASSERT((DomainConcept<Domain>));

		typedef typename Domain::Value Value;
	public:
		Space() {}

        /** \brief create the test example from iterator range or C-like table of values */
        template<typename It>
        Point<Value> createPoint(It begin_range, It end_range) const {
            Point<Value> point;
            typename Space::const_iterator j = this->begin();
            for(It i = begin_range; i != end_range && j != this->end(); ++i, ++j) {
                point.push_back( j->find(*i) );
            }
            return point;
        }

        /** \brief create the test example from collection of pairs: attribute(domain) identifier and attribute value */
		Point<Value> createPoint(const std::vector<std::pair<std::string, typename Value::Value> >& collection) const {
            typedef std::pair<std::string, typename Value::Value> AttrValPair;
            typedef std::vector<AttrValPair> VecAttrValPair;

            Point<Value> point;
            for(typename Space::const_iterator j = this->begin(); j != this->end(); ++j ) {
                const typename Value::DomainType& d = *j;
                typename VecAttrValPair::const_iterator k = std::find_if(collection.begin(), collection.end(),
																		 boost::bind(&AttrValPair::first, _1) == boost::bind(&Domain::getId, d) );

				typename Domain::ValueId id = Domain::getUnknownId(); //if not found given domain id the 'unknown' is inserted
                if( k != collection.end() ) {
					try {
						id = d.find( k->second );
					} catch(NotFoundException&) //if not found value the 'unknown' is inserted
					{ }
				}
				point.push_back(id);
            }
            return point;
        }

        /** \brief create the test example from collection of pairs: attribute(domain) identifier and attribute value
			\throws NotFoundException if domain id is not correct or atribute value is not correct */
		Point<Value> createPointStrict(const std::vector<std::pair<std::string, typename Value::Value> >& collection) const {
            typedef std::pair<std::string, typename Value::Value> AttrValPair;
            typedef std::vector<AttrValPair> VecAttrValPair;

            Point<Value> point;
            for(typename Space::const_iterator j = this->begin(); j != this->end(); ++j ) {
                const typename Value::DomainType& d = *j;
                typename VecAttrValPair::const_iterator k = std::find_if(collection.begin(), collection.end(),
																		 boost::bind(&AttrValPair::first, _1) == boost::bind(&Domain::getId, d) );
				if( k == collection.end() )
					throw NotFoundException( d.getId().c_str() );
				point.push_back( d.find( k->second ) ); //find throws NotFoundException if fails finding the value
            }
            return point;
        }

	};


} //namespace faif

#endif //FAIF_POINT_HPP_
