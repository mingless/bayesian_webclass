#ifndef FAIF_VALUE_HPP_
#define FAIF_VALUE_HPP_

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc9.0 warnings for boost::concept_check
#pragma warning(disable:4100)
#endif

#include <ostream>
#include <string>
#include <list>
#include <iterator>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/concept_check.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/nvp.hpp>

#include "ExceptionsFaif.hpp"

namespace faif {

    /** \brief nominal attribute trait (equality comparable), modeled as element in unordered set */
    struct nominal_tag {};

    /** \brief ordered attribute trait (equality comparable, less than comparable) , modeled as element in ordered set */
    struct ordinal_tag : public nominal_tag {};

    /** \brief interval attribute trait (equality comparable, less than comparable, distance), integer numbers */
    struct interval_tag : public ordinal_tag {};

    /** \brief nominal attribute trait (equality comparable, less than comparable, distance, continuous), real numbers */
    struct ratio_tag : public interval_tag {};

	/**
	   \brief the value concept
	 */
	template<typename Val>
	struct ValueConcept
		: boost::EqualityComparable<Val>, boost::DefaultConstructible<Val>,
		boost::CopyConstructible<Val>, boost::Assignable<Val> {

		typedef typename Val::ValueTag ValueTag; 		//the ValueType is required
        typedef typename Val::Value Value;              //the Value is required
		typedef typename Val::DomainType DomainType;    //the DomainType is required

		BOOST_CONCEPT_USAGE(ValueConcept)
		{
			Val v; //default constructable is required
			v.get(); //the get() method is required
			v.getDomain(); //the getDomain() method is required
			v.isUnknown(); //the isUnknown() method is required
		}

	};

    /** \brief forward declaration */
    template <typename Val> class DomainEnumerate;

    /**
	   \brief nominal attribute template (equality comparable)
     */
    template <typename Val> class  ValueNominal {
    public:
        BOOST_CONCEPT_ASSERT(( boost::EqualityComparable<Val> ));
        BOOST_CONCEPT_ASSERT(( boost::DefaultConstructible<Val> ));
        BOOST_CONCEPT_ASSERT(( boost::CopyConstructible<Val> ));
        BOOST_CONCEPT_ASSERT(( boost::Assignable<Val> ));

        /** \brief the attribute trait */
        typedef nominal_tag ValueTag;

        typedef Val Value;

        typedef DomainEnumerate< ValueNominal<Value> > DomainType;

        /** \brief c-tor. Create UNKNOWN Value */
		ValueNominal() : val_(), domain_(0L) {}

        /** \brief c-tor */
		ValueNominal(const Value& val, DomainType* d) : val_(val), domain_(d) {}

        /** \brief c-tor */
		ValueNominal(const  ValueNominal& a) : val_(a.val_), domain_(a.domain_) {}

        /** \brief assign operator */
		ValueNominal& operator=(const  ValueNominal& a) {
            val_ = a.val_;
            domain_ = a.domain_;
			return *this;
        }

        /** \brief d-tor */
        ~ValueNominal(){}

        /** \brief accessor  */
        Value& get() { return val_; }

        /** \brief accessor  */
        const Value& get() const { return val_; }

        /** \brief accessor  */
		bool isUnknown() const { return domain_ == 0L; }

        /** \brief accessor - domain */
        const DomainType* getDomain() const { return domain_; }

        /** \brief the equality comparison */
        bool operator==(const  ValueNominal& v) const { return val_ == v.val_ && domain_ == v.domain_; }

        /** \brief the equality comparison */
        bool operator!=(const  ValueNominal& v) const { return val_ != v.val_ || domain_ != v.domain_; }

		/** \brief return the unknown value object */
		static ValueNominal& getUnknown() {
			static ValueNominal unknown;
			return unknown;
		}

		/** \brief serialization using boost::serialization */
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */){
			ar & boost::serialization::make_nvp("ValueVal", val_ );
			ar & boost::serialization::make_nvp("ValueDomain", domain_ );
		}
    private:
        Value val_;
        DomainType* domain_;
    };

    typedef  ValueNominal<std::string> ValueNominalString;
    typedef  ValueNominal<int>  ValueNominalInt;

    /**
	   \brief the domain concept
     */
    template<typename Dom>
    struct DomainConcept : boost::EqualityComparable<Dom>, boost::DefaultConstructible<Dom>,
        boost::CopyConstructible<Dom>, boost::Assignable<Dom> {

        typedef typename Dom::ValueTag ValueTag;              //the ValueTag is required
        typedef typename Dom::Value Value;                    //the type of Value is required
        typedef typename Dom::ValueId ValueId;                //the ValueId is required
        typedef typename Dom::iterator iterator;              //the iterator is required
        typedef typename Dom::const_iterator const_iterator;  //the const_iterator is required

        BOOST_CONCEPT_USAGE(DomainConcept)
        {
            Dom d; //default constructable is required
            d.getId(); //the getId() method is required
        }
    };

    /** \breif the domain of nominal attributes.

        Finite set of unique equally comparable values. No erase/remove operations on this set is available.
    */
    template <typename Val>
    class DomainEnumerate {
    public:
        BOOST_CONCEPT_ASSERT((ValueConcept<Val>));

        typedef typename Val::ValueTag ValueTag;
        typedef Val Value;
        typedef const Val* ValueId;  //value identifier
        typedef Val* ValueIdSerialize; //value identifier for serialization
        typedef ValueNominalString* AttrIddSerialize;         // !< \brief  for serialization the const not works correctly

        typedef std::list<Val> Container;
        typedef typename Container::iterator iterator;
        typedef typename Container::const_iterator const_iterator;

        DomainEnumerate(const std::string& id = "") : id_(id) {}

        /** \brief copy c-tor (transform the parent pointers) */
        DomainEnumerate(const DomainEnumerate& d);

        /** \brief assignment (transform the parent pointers) */
        DomainEnumerate& operator=(const DomainEnumerate&);

        ~DomainEnumerate() {}

        /** \brief accessor */
        std::string getId() const { return id_; }

        /** \brief return the value id for given value iterator */
        static ValueId getValueId(const_iterator it) {
            return &(*it);
        }

        /** \brief return the value id for unknown value */
        static ValueId getUnknownId() {
            return &(Value::getUnknown());
        }

        /** \brief accessor */
        int getSize() const { return static_cast<int>(values_.size()); }

        /** the domain equality operator (the id is compared, not all attributes) */
        bool operator==(const DomainEnumerate& d) const { return id_ == d.id_; }

        /** the domain not equality operator (the id is compared, not all attributes) */
        bool operator!=(const DomainEnumerate& d) const { return id_ != d.id_;}

        /** \brief accessor */
        iterator begin() { return values_.begin(); }
        /** \brief accessor */
        iterator end() { return values_.end(); }

        /** \brief accessor */
        const_iterator begin() const { return values_.begin(); }
        /** \brief accessor */
        const_iterator end() const { return values_.end(); }

        /** \brief search for value equal to given, if found return the value identifier,
            otherwise insert new into collection and return the inserted value identifier */
        ValueId insert(const typename Val::Value& value);

        /** \brief return the first iterator to attribute equal to given */
        ValueId find(const typename Val::Value& value) const;

        /** \brief serialization using boost::serialization */
        template<class Archive>
        void serialize(Archive & ar, const unsigned int /* file_version */){
            ar & boost::serialization::make_nvp("DomainId", id_ );
            ar & boost::serialization::make_nvp("DomainValues", values_ );
        }
    private:
        std::string id_; //!< the domain id
        Container values_;
    };

    /** \brief create the domain with attributes.
        \return the new domain
    */
    template <typename T>
    DomainEnumerate< ValueNominal<T> > createDomain(const std::string& id, T* begin, T* end) {
        DomainEnumerate< ValueNominal<T> > domain(id);
        for(T* i = begin; i != end; ++i) {
            domain.insert(*i);
        }
        return domain;
    }

    /** the ostream operator for attribute, defined here because it uses the domain method */
    template <typename Value> std::ostream& operator<<(std::ostream& os, const ValueNominal<Value>& a) {
		if(a.getDomain() == 0L)
			os << "Unknown";
		else
			os << a.getDomain()->getId() << "=" << a.get(); // << " addr:" << &a << ' ';
        return os;
    }

    /** the ostream operator for pointer to attribute, defined here because it uses the domain method */
    template <typename Value> std::ostream& operator<<(std::ostream& os, const ValueNominal<Value>* a) {
		if(a) {
			return operator<<(os, *a);
		} else {
			return os << "Null";
		}
    }

    /** \brief the ostream operator for domain */
    template <typename Val>
    std::ostream& operator<<(std::ostream& os, const DomainEnumerate<Val>& domain) {
        std::copy(domain.begin(), domain.end(), std::ostream_iterator<Val>(os,","));
        return os << ';';
    }

    namespace {

        template <typename Val>
        Val TransformFunction(const Val& a, typename Val::DomainType* d) {
            return Val(a.get(),d);
        }

    } //namespace


    /** \brief copy c-tor (transform the parent pointers  */
    template <typename Val>
    DomainEnumerate<Val>::DomainEnumerate(const DomainEnumerate& d)
        : id_(d.id_) {

        std::transform(d.values_.begin(), d.values_.end(), std::back_inserter<Container>(values_),
                       boost::bind(&TransformFunction<Val>, _1, this) );
    }

    /** \brief assignment (transform the parent pointers  */
    template <typename Val>
    DomainEnumerate<Val>&
    DomainEnumerate<Val>::operator=(const DomainEnumerate& d) {
        if(&d == this)
            return *this;

        id_ = d.id_;
        values_.clear();

        std::transform(d.values_.begin(), d.values_.end(), std::back_inserter<Container>(values_),
                       boost::bind(&TransformFunction<Val>, _1, this) );
        return *this;
    }

    /** search for attribute equal to given. If found return the iterator,
        otherwise insert new into collection and return the inserted attribute */
    template< typename Val>
    typename DomainEnumerate<Val>::ValueId
    DomainEnumerate<Val>::insert(const typename Val::Value& value) {
        Val attr(value, this);
        const_iterator found = std::find( values_.begin(), values_.end(), attr );
        if( found != values_.end() )
            return getValueId(found);
        //inserts element on the end (if not found)
        return getValueId( values_.insert( values_.end(), Val(value, this) ) );
    }

    /** return the first iterator to attribute equal to given
     // \throws NotFoundException
    */
    template<typename Val>
    typename DomainEnumerate<Val>::ValueId
    DomainEnumerate<Val>::find(const typename Val::Value& value) const {
        Val attr(value, const_cast<DomainEnumerate<Val>*>(this)); //ValueNominal requires non-const pointer to domain
        const_iterator found = std::find( values_.begin(), values_.end(), attr );
        if( found != values_.end() ) {
            return getValueId( found);
        }
        throw NotFoundException( getId().c_str() );
    }

} //namespace faif

#endif //FAIF_VALUE_HPP_
