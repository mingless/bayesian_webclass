#ifndef FAIF_FUSION_HPP
#define FAIF_FUSION_HPP

#include <vector>
#include <ostream>
#include <map>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <limits>
#include <functional>
#include <boost/bind.hpp>

#include "Belief.hpp"

namespace faif {

    namespace ml {

        /**
           \brief typedef's for Dempster-Shafer combination rule (for fusion) and some internal (helping) methods
        */
        template<typename Belief> struct FusionInternal {

            // /< attribute id representation in fusion
            typedef typename Belief::ValueId AttrIdd;

            //collection of pair (AttrIdd, Probability)
            typedef typename Belief::Beliefs Beliefs;

            // the helping type - internal for fusion calculations
            typedef std::map<AttrIdd, std::vector<Probability> > Evidence;

            //the helping function - return the initial key-value pair for evidence container
            static typename Evidence::value_type InitTransformFunction(const Belief& in) {
                std::vector<Probability> v;
                v.push_back(in.getProbability());
                return typename Evidence::value_type(in.getValue(), v);
            }

        };

        //the helping functor - complete the evidence records by adding the probabilities from given vector
        template<typename Belief>
        struct AddEvidenceFunctor : public std::unary_function<void, typename Belief::Beliefs> {

            typedef typename FusionInternal<Belief>::Evidence Evidence;
            typedef typename FusionInternal<Belief>::Beliefs Beliefs;
            typedef typename FusionInternal<Belief>::AttrIdd AttrIdd;

            Evidence& evidence_;

            AddEvidenceFunctor(Evidence& evidence) : evidence_(evidence) {}

            void operator()(const Beliefs& probs) {
                //adds the probability to the evidence - try to find the item and read the probability. If item not found NaN is added.
                for(typename Evidence::iterator i = evidence_.begin(); i != evidence_.end(); ++i ) {
                    Probability probability_ = std::numeric_limits<Probability>::quiet_NaN();
                    typename Beliefs::const_iterator found =
                        std::find_if(probs.begin(), probs.end(),
                                     boost::bind(std::equal_to<AttrIdd>(), boost::bind(&Belief::getValue, _1), i->first ) );
                    if( found != probs.end() )
                        probability_ = found->getProbability();
                    //put the found probability
                    i->second.push_back(probability_);
                }
                // std::for_each( evidence_.begin(), evidence_.end(), boost::bind(AddEvidenceFunctor<Belief>::AddProbFunction, _1, probs) );



                //look for NaN and change the NaN value to (1.0 - sum)/count , where 'count' is number of NaN values
                Probability sum = 0.0;
                int count = 0; //number of NaN
                for(typename Evidence::const_iterator i = evidence_.begin(); i != evidence_.end(); ++i ) {
                    const Probability& v = i->second.back();
                    if ( v == v ) { //if not is NaN
                        sum += v;
                    } else { // found NaN
                        ++count;
                    }
                }
                //update the NaN to (1.0 - sum)/ count
                if(count > 0) {
                    Probability probForUnknown = (1.0 - sum)/static_cast<Probability>(count);
                    if(probForUnknown < 0)
                        probForUnknown = 0;
                    for(typename Evidence::iterator i = evidence_.begin(); i != evidence_.end(); ++i ) {
                        Probability& v = i->second.back();
                        if ( v != v ) //if NaN
                            v = probForUnknown;
                    }
                }
            }
        private:
            AddEvidenceFunctor& operator=(const AddEvidenceFunctor&); //forbidden
        };

		// the helping class - return the probabilities divided by sum (so they summarizes to 1)
        template<typename Belief, typename EvidenceValue>
        struct NormalizationFunctor : std::binary_function<Belief, Probability, EvidenceValue> {

			NormalizationFunctor(Probability sum) : sum_(sum) {}

			Belief operator()(const Probability& prob, const EvidenceValue& evidence) {
				return Belief(evidence.first, prob / sum_ );
			}
			Probability sum_;
		};

        // } //namespace

        /** \brief connect categories using the Dempster-Shafer combination rule, e.g.
            bel(cat) = bel1(cat) * bel2(cat) * ... * beln(cat) / SUM(bel)
        **/
        template<typename Belief> typename Belief::Beliefs fusion(const typename std::vector<typename Belief::Beliefs>& input) {

            BOOST_CONCEPT_ASSERT((BeliefConcept<Belief>));

            typedef typename FusionInternal<Belief>::Evidence Evidence;
            typedef typename FusionInternal<Belief>::Beliefs Beliefs;

            Beliefs output;

            if(input.empty() )
                return output; //empty output

            Evidence evidence;
            const Beliefs& front = input.front();             //assert: input front exists
            std::transform( front.begin(), front.end(), std::inserter(evidence, evidence.begin() ), FusionInternal<Belief>::InitTransformFunction );
            AddEvidenceFunctor<Belief> add_evidence(evidence);
            std::for_each( ++input.begin(), input.end(), add_evidence );

            std::vector<Probability> combination;
            for(typename Evidence::const_iterator i = evidence.begin(); i != evidence.end(); ++i ) {
                const std::vector<Probability>& in = i->second;
                combination.push_back( std::accumulate(in.begin(), in.end(), 1.0, std::multiplies<Probability>() ) );
            }
            Probability sum = std::accumulate(combination.begin(), combination.end(), 0.0);
            // normalize the probability to get sum of probabilities connected with AttrIdd equal to 1
            NormalizationFunctor<Belief, typename Evidence::value_type> nfunctor(sum);
            std::transform( combination.begin(), combination.end(), evidence.begin(), std::back_inserter(output), nfunctor );
            return output;
        }

    } //namespace ml
} //namespace faif

#endif //FAIF_FUSION_HPP
