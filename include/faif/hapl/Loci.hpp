#ifndef FAIF_HAPL_LOCI_HPP
#define FAIF_HAPL_LOCI_HPP

#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>

#include "../Value.hpp"

namespace faif {

    /** \brief Population genetics (haplotype and marker analyzis) primitives and algorithms  */
    namespace hapl {

        /** \brief the helping structure, implements the Allele members */
        struct AlleleImpl : public std::pair<std::string, bool> {

            /** \brief c-tor */
            AlleleImpl(const std::string& name = std::string(""), bool is_silent = false)
                : std::pair<std::string,bool>(name, is_silent) {}

            /** \brief equality comparison */
            bool operator==(const AlleleImpl& a) const {
                return first == a.first;
            }

            /** \brief non-equality comparison */
            bool operator!=(const AlleleImpl& a) const {
                return first != a.first;
            }
        };

		/** \brief ostream operator */
		inline std::ostream& operator<<(std::ostream& os, const AlleleImpl& a) {
			os << a.first;
			if(a.second)
				os  << "(silent)";
			return os;
		}

        /** \brief Allele, variant of a gene */
        typedef ValueNominal<AlleleImpl> Allele;

        /** \brief accessor */
        inline std::string getName(const Allele& a) {
            return a.get().first;
		}

        /** \brief accessor */
        inline bool isSilent(const Allele& a) {
            return a.get().second;
		}

        /** \brief Locus, place on DNA contatining the Allele */
        typedef DomainEnumerate<Allele> Locus;

        namespace {

            /** \brief generates the allele name */
			std::string generateAlleleName(const std::string& locus_name, int allele_number) {
                return locus_name + boost::lexical_cast<std::string>(allele_number);
            }

        }

        /** \brief helping method - create the entire locus, with number variants.

            Variants name are the locus name and the variant number,
            i.e. silent variant has name 'name0', first non silent variant has name 'name1' etc.
        */
        Locus createLocus(const std::string& name, int num_variants, bool has_silent) {
            Locus l(name);
            int num = 0;
            //the silent allele
            if(has_silent)
                l.insert( AlleleImpl(generateAlleleName(name, num), true) );

            //not sillent alleles
            for(num = 1; num < num_variants; num++) {
                l.insert( AlleleImpl( generateAlleleName(name, num), false) );
            }

            //if the locus does not have silent allele, another allele (because always create num alleles)
            if(!has_silent)
                l.insert( AlleleImpl( generateAlleleName(name, num), false) );

            return l;
		}

        /** \brief Loci, the collection(sequence) of Locus, e.g. genotype */
		typedef std::vector<Locus> Loci;

        /** ostream operator for loci */
        inline std::ostream& operator<<(std::ostream& os, const Loci& loci) {
			std::copy(loci.begin(), loci.end(), std::ostream_iterator<Locus>(os,";") );
			return os;
		}

    } //namespace hapl
} //namespace faif

#endif //FAIF_HAPL_LOCI_HPP
