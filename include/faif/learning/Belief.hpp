#ifndef FAIF_BELIEF_HPP_
#define FAIF_BELIEF_HPP_

#include <ostream>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

#include "../Value.hpp"

namespace faif {

    /** \brief the probability type */
    typedef double Probability;

    namespace ml {

		/**
		   \brief the belief concept
		*/
        template<typename Bel>
        struct BeliefConcept : boost::DefaultConstructible<Bel>, boost::CopyConstructible<Bel>,
            boost::Assignable<Bel>
        {

            typedef typename Bel::Value Value;              //the Value type is required
            typedef typename Bel::ValueId ValueId;          //the ValueId type is required
            typedef typename Bel::Beliefs Beliefs;          //the Belief collection type is required

            BOOST_CONCEPT_USAGE(BeliefConcept)
            {
				BOOST_CONCEPT_ASSERT((ValueConcept<Value>));
				Bel b; //default constructable is required
				b.getValue(); //the getValue() method is required
				b.getProbability(); //the getProbability() method is required
			}

		};

        /** \brief belief is value id with probability */
        template <typename Val> class Belief {
            BOOST_CONCEPT_ASSERT((ValueConcept<Val>));
        public:
            typedef Val Value;

            typedef typename Val::DomainType::ValueId ValueId;

            //collection of pair (AttrIdd, Probability)
            typedef typename std::vector<Belief<Val> > Beliefs;

            Belief() : value_(Val::DomainType::getUnknownId() ), probability_(0.0) {}

            Belief(ValueId value, Probability probability) : value_(value), probability_(probability) {}

            ValueId getValue() const { return value_; }
            Probability getProbability() const { return probability_; }

            /** \brief serialization using boost::serialization */
            friend class boost::serialization::access;

            template<class Archive>
            void save(Archive & ar, const unsigned int /* file_version */) const {
                ar & boost::serialization::make_nvp("Value", value_ );
                ar & boost::serialization::make_nvp("Probability", probability_ );
            }

            template<class Archive>
            void load(Archive & ar, const unsigned int /* file_version */) {
                typename Val::DomainType::ValueIdSerialize i;
                ar >> boost::serialization::make_nvp("Value", i);
                value_ = const_cast<ValueId>(i);
                ar & boost::serialization::make_nvp("Probability", probability_ );
            }

            template<class Archive>
            void serialize( Archive &ar, const unsigned int file_version ){
                boost::serialization::split_member(ar, *this, file_version);
            }

            //ordering from the best towards week
            bool operator<( const Belief &c) const { return probability_ > c.probability_; }
        private:
            ValueId value_;                 //public member
            Probability probability_;        //public member
        };

        //ostream iterator for debugging
        template <typename Val> std::ostream& operator<<(std::ostream& os, const Belief<Val>& b) {
            os << "Value:" << b.getValue() << " Prob:" << b.getProbability();
            return os;
        }

        /** stream operator - for debugging */
        template <typename Val> std::ostream& operator<<(std::ostream& os, const std::vector<Belief<Val> >& c) {
            std::copy(c.begin(), c.end(), std::ostream_iterator<Belief<Val> >(os,";") );
            return os;
        }

    } //namespace ml

} // namespace faif


#endif //FAIF_BELIEF_HPP_
