#ifndef FOLDED_MATRIX_H
#define FOLDED_MATRIX_H

/* the classes and functions to calculate and store energy matrix (in Nussinov algorithm).
   The classes and function for internal use, by FoldedChain.h and FoldedPair.h (are not the library interface).
*/

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//visual studio 8.0 - konwersja pomiedzy unsigned int a size_t
#pragma warning(disable:4267)
//visual studio 8.0 - arytmetyka dla iteratorow przy konwersji na inta
#pragma warning(disable:4244)
#endif

#include <iterator>
#include <algorithm>
#include <cassert>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>


#include "Chain.h"
#include "SecStruct.h"

namespace faif {
    namespace dna {

        /** \brief matrix - 2D array with indexing */
        class Matrix : boost::noncopyable {
        public:
			Matrix(unsigned int length) : len_(length) {
                unsigned int size_ = len_*len_;
                m_ = new EnergyValue[size_];
                std::fill(m_, m_ + size_, 0);
                // for(unsigned int i=0;i<size_;++i) m_[i] = 0;
            }
            ~Matrix() { delete [] m_; }
            const EnergyValue& get(int x, int y) const{ return m_[x*len_+y]; }
            void set(int x, int y, EnergyValue val) { m_[x*len_ + y] = val; }
            unsigned int getLength() const { return len_; }
        private:
            unsigned int len_;
            EnergyValue* m_;
        };

        class SecStructProxy;
        typedef boost::scoped_ptr<SecStructProxy> SecStructProxyPtr;

        /** strategy to use folded matrix algorithm with single chain and with two chains */
        class FoldedMatrixStrategy {
        public:
			FoldedMatrixStrategy(const EnergyNucleo& energy) : energy_(energy) {}
            virtual ~FoldedMatrixStrategy() {}

            /** return the iterator for nuleotide of i-th index in matrix */
            virtual Chain::const_iterator getNucleotide(int index) const = 0;

            /** returns the split index (the first index belonging to the second chain) */
            virtual int getSplitIndex() const = 0;

            /** returns the number of nucleotides in chain (chains) */
            virtual int getLength() const = 0;

            /** calculates energy between two nucleotides of given indexes */
            int getEnergy(int index_A, int index_B) const {
                return energy_.getEnergy( *getNucleotide(index_A), *getNucleotide(index_B) );
            }
        private:
            FoldedMatrixStrategy& operator=(const FoldedMatrixStrategy&); //!< private, reference members
            const EnergyNucleo& energy_;
        };

        /** dna chain or two dna chains with energy matrix */
        class FoldedMatrix : boost::noncopyable {
        public:
            // c-tor for single strand
            FoldedMatrix(const FoldedMatrixStrategy& strategy, unsigned int max_foldings);

            ~FoldedMatrix() {}

            /** calculates secondary structures (backtracking in energy matrix) */
            const SecStructures& getStructures();

            EnergyValue getSecStructEnergy() { return structure_energy_; }

            /** calculates one secondary structure (energy matrix view in depth) */
            SecStruct findInDepth() const {
                SecStruct structure;
                findDepthRecur(structure, 0, strategy_.getLength() - 1);
                return structure;
            }

            std::ostream& printMatrix(std::ostream& os, int print_width) const ;

            std::ostream& printStructures(std::ostream& os, int print_width) const;
        private:
            const FoldedMatrixStrategy& strategy_;
            unsigned int max_foldings_;

            Matrix matrix_;

            /** secondary structure energy */
            EnergyValue structure_energy_;
            SecStructProxyPtr proxy_;

            void makeMatrix();

            //SecStruct& findDepthRecur(SecStruct& structure, const ConnectPair& curr_point) const;
            SecStruct& findDepthRecur(SecStruct& structure, int first_idx, int second_idx ) const;
        };


        //----------------------------------------------------------------------------------------------
        //
        //  implementation
        //
        //----------------------------------------------------------------------------------------------


        namespace {

            /** partly calculated sec structure and counter */
            class SecStructCount {
            public:
                explicit SecStructCount(const SecStruct& sec_struct)
                    : counter_(1), sec_struct_(sec_struct) {}
                SecStructCount(const SecStructCount& c)
                    : counter_(c.counter_), sec_struct_(c.sec_struct_) {}
                ~SecStructCount() {}

                //comparison
                bool operator==(const SecStructCount& s) const { return sec_struct_ == s.sec_struct_; }
                //equiwalance
                bool operator<(const SecStructCount& s) const { return sec_struct_ < s.sec_struct_; }

                //accessor
                const SecStruct& get() const { return sec_struct_; }
                //modyfikacja przechowywanej struktury
                void addPair(const ConnectPair& pair) { sec_struct_.addPair(pair); }

                //zmnijesza licznik, zwraca true, jezeli licznik > 0
                bool dec() const { --counter_; return counter_ > 0; }
                //zwieksza licznik
                void inc() const { ++counter_; }
                //odczyt licznika
                int getCounter() const { return counter_; }
            private:
                SecStructCount& operator=(const SecStructCount&); //zabroniony
                mutable int counter_; //licznik odwolan
                SecStruct sec_struct_; //przechowywana struktura drugorzedowa

            };

            //for debugging
            inline std::ostream& operator<<(std::ostream& os, const SecStructCount& sec_struct_count) {
                os << sec_struct_count.get() << "[" << sec_struct_count.getCounter() << "]";
                return os;
            }

            typedef std::set<SecStructCount> SecStructParts;
            typedef SecStructParts::iterator StructPartIter;

            /** zarzadza czesciowo obliczonymi strukturami drugorzedowymi.
                Usuwa te, ktore juz nie sa potrzebne, dodaje nowe (jezeli nie przekroczy ich maksymalnej liczby) */
            class StructPartManager : boost::noncopyable {
            public:
                StructPartManager(unsigned int max_foldings) : max_foldings_(max_foldings) { }

                StructPartIter begin() { return structures_.begin(); }
                StructPartIter end() { return structures_.end(); }

                unsigned int size() const { return structures_.size(); }

                /** modyfikuje wybrana strukture lub dodaje nowa (jezeli jest wiele do niej odwolan) */
                StructPartIter addPair(StructPartIter iter, const ConnectPair& pair ) {
                    SecStructCount new_struct(iter->get());
                    new_struct.addPair(pair);
                    remove(iter);
                    return insert(new_struct);
                }

                /** dodaje nowa strukture (jezeli istniala - to zwieksza licznik, jezeli za duzo struktur - to nie dodaje */
                StructPartIter insert(const SecStructCount& sec_struct_count) {
                    StructPartIter new_iter = structures_.find(sec_struct_count);
                    if(new_iter != structures_.end() )
                        new_iter->inc();
                    else if(structures_.size() < max_foldings_)
                        new_iter = structures_.insert(sec_struct_count).first;
                    return new_iter;
                }
                /** zmniejsza licznik (a jezeli trzeba to usuwa strukture */
                void remove(StructPartIter iter) {
                    if(! iter->dec() )
                        structures_.erase(iter);
                }
            private:
                SecStructParts structures_;
                unsigned int max_foldings_;
            };

            //kolekcja iteratorow bedzie uporzadkowana nie dla iteratorow a dla wartosci
            struct SecStructIterLessFunctor {
                bool operator()(const StructPartIter& itA, const StructPartIter& itB) const {
                    return itA->get() < itB->get();
                }
            };



            /** set of partly calculated secondary structures (iterators to sec structures, not full objects)
             */
            struct StructPartCollection : public std::set<StructPartIter,SecStructIterLessFunctor> {
            public:
                //zwieksza licznik dla kazdej przechowywanej struktury drugorzedowej
                void incPartsCount() {
                    std::for_each( begin(), end(), boost::bind( &SecStructCount::inc, boost::bind(&StructPartIter::operator*,_1) ) );
                }

                //zmniejsza licznik dla kazdej przechowywanej struktury drugorzedowej
                void decPartsCount() {
                    std::for_each( begin(), end(), boost::bind( &SecStructCount::dec, boost::bind(&StructPartIter::operator*,_1) ) );
                }

                //add single partly calculated secondary structure.
                //if containter has already this structure the counter is decreased
                //(because this structure is collected twice)
                void joinOne(const StructPartIter& sec_struct) {
                    StructPartCollection::iterator found = find(sec_struct);
                    if(found != end() )
                        (*found)->dec();
                    else
                        insert(sec_struct);
                }

                //adds sec structs from object and the argument
                void join(const StructPartCollection& collection) {
					std::for_each(collection.begin(), collection.end(), boost::bind(&StructPartCollection::joinOne, this, _1) );
                }

                //creates new second structure (append pair into sec_struct by manager)
                //inserts it into collection only if the manager adds new iterator (the number of sec structs not
                //exceeded the max_foldings parameter in manager
                bool addNewSecStruct(const StructPartIter& sec_struct, const ConnectPair& pair,
                                     StructPartManager& manager) {
                    StructPartIter new_iter = manager.addPair(sec_struct, pair);
                    if(new_iter == manager.end())
                        return false;
                    insert(new_iter);
                    return true;
                }

                //append the connect pair to all partly calculated second structures stored in collection
                //the new colection with connected pair is returned by reference to argument (new_collection argument)
                //returns 'true' if succeed (i.e. number of sec structures not exceed the 'max_foldings'
                bool addPair(const ConnectPair& pair,
                             StructPartManager& manager, StructPartCollection& new_collection) const {

                    for(StructPartCollection::const_iterator it = begin(); it != end(); ++it )
                        if(!new_collection.addNewSecStruct(*it, pair, manager) )
                            return false;
                    return true;
                }

            };

            /** stream operator for debugging */
            inline std::ostream& operator<<(std::ostream& os, const StructPartCollection& collection) {
				std::transform(collection.begin(), collection.end(),
							   std::ostream_iterator<SecStructCount>(os,""),
							   boost::bind(&StructPartIter::operator*,_1) );
                return os;
            }

            /** the point of analyze in secondary structure search */
            struct ActivePoint : public std::pair<int,int> {

                explicit ActivePoint(int x, int y) : std::pair<int,int>(x,y) {}

                //pary sa uporzadkowane w kolejnosci przegladania, tzn. (0,15) < (0,14) < (1,16)
                bool operator<(const ActivePoint& p) {
                    if(first != p.first)
                        return first < p.first;
                    return p.second < second;
                }
            };

            //funkcja pomocnicza - operator strumieniowy
            std::ostream& operator<<(std::ostream& os, const ActivePoint& p) {
                os << '(' << p.first << ',' << p.second << ')';
                return os;
            }

            /** the point or set of points (if brancing) in secondary structure search.
                It contain connection to partly calculated secondary structures.
            */
            class  FoldActivePoints {
            public:
                //tworzy poczatkowy stan
                explicit FoldActivePoints(const ActivePoint& start, const StructPartIter start_struct_iter) {
                    addActivePoint(start);
                    folded_parts_.insert(start_struct_iter);
                }
                //tworzy kolejny stan (kopia)
                FoldActivePoints(const FoldActivePoints& bef)
                    : active_points_(bef.active_points_), folded_parts_(bef.folded_parts_) {}

                ~FoldActivePoints() { }
                bool operator==(const FoldActivePoints& p) const { return active_points_ == p.active_points_; }
                bool operator<(const FoldActivePoints& p) const { return active_points_ < p.active_points_; }

                //czy ma jeszcze punkty aktywne
                bool hasActivePoint() const { return !active_points_.empty(); }

                //pobiera pierwszy punkt aktywny (usuwa go z kolekcji)
                ActivePoint getFirstActivePoint() {
                    assert( !active_points_.empty() );
                    ActivePoint out = *(active_points_.begin());
                    active_points_.erase( active_points_.begin() );
                    return out;
                }

                //dodaje nowy punkt do analizy
                void addActivePoint(const ActivePoint& active_point) { active_points_.insert(active_point); }

                //laczy przechowywane struktury drugorzedowe
                void join(const FoldActivePoints& f) const { folded_parts_.join( f.folded_parts_ ); }

                //dodaje pare do czesciowo obliczonych struktur
                //zwraca true, jezeli udalo sie dodac (tzn. nie przekroczono liczby struktur)
                bool addPair(const ConnectPair& pair, StructPartManager& manager) {
                    StructPartCollection new_folded_parts_;
                    bool out = folded_parts_.addPair(pair, manager, new_folded_parts_ );
                    swap( folded_parts_, new_folded_parts_ );
                    return out;
                }

                /** accessor and mutator (dangerous) */
                StructPartCollection& getFoldedParts() { return folded_parts_; }

                /** accessor */
                const StructPartCollection& getFoldedParts() const { return folded_parts_; }

                //do debuggowania
                friend std::ostream& operator<<(std::ostream& os, const FoldActivePoints& state);
            private:
                //punkty analizy (moze ich byc wiele - przy skokach)
                std::set<ActivePoint> active_points_;
                //czesciowo obliczone struktury drugorzedowe dla danego zbioru punktow aktywnych
                mutable StructPartCollection folded_parts_;
            };

            //for debugging
            inline std::ostream& operator<<(std::ostream& os, const FoldActivePoints& state) {
                os << "points: ";
                std::copy(state.active_points_.begin(), state.active_points_.end(), std::ostream_iterator<ActivePoint>(os,"") );
                os << std::endl << "structs: " << state.getFoldedParts() << std::endl;
                return os;
            }

            /** rozatrywane zbiory punktow, ktore tworza struktury drugorzedowe */
            class States {
            public:
                typedef std::set<FoldActivePoints> StatesColection;

                bool empty() const { return states_.empty(); }

                unsigned int size() const { return states_.size(); }

                //pobiera nastepny stan do rozpatrzenia (w odpowiedniej kolejnosci!).
                //Usuwa go z kolekcji. Kolekcja nie moze byc pusta.
                FoldActivePoints getFirstState() {
                    assert( !states_.empty() );
                    FoldActivePoints out = *(states_.begin());
                    states_.erase( states_.begin() );
                    return out;
                }

                //dodaje nowy stan
                void addState(const FoldActivePoints& new_state) {
                    addOrJoin(new_state);
                }

                //dodaje stan dla left lub up lub diag
                void addStateMove(const FoldActivePoints& state,
                                  const ActivePoint& new_point) {
                    FoldActivePoints new_state(state);
                    new_state.addActivePoint(new_point);
                    addOrJoin(new_state);
                }
                void addStateJump(const FoldActivePoints& state,
                                  const ActivePoint& point_jump1,
                                  const ActivePoint& point_jump2) {
                    FoldActivePoints new_state(state);
                    new_state.addActivePoint(point_jump1);
                    new_state.addActivePoint(point_jump2);
                    addOrJoin(new_state);
                }
            private:
                StatesColection states_;

                //dodaje nowy stan do kolekcji stanow. Jezeli taki stan juz istnial - to laczy odowiednie struktury
                void addOrJoin(const FoldActivePoints& new_state) {
                    StatesColection::iterator found = states_.find(new_state);
                    if(found != states_.end() )
                        found->join(new_state);
                    else
                        states_.insert(new_state);
                }
            };

        } //namespace

        //klasa, ktora przechowuje struktury drugorzedowe dla danego lancucha
        class SecStructProxy : boost::noncopyable {
        public:
            SecStructProxy( const Matrix& matrix, const FoldedMatrixStrategy& strategy, int max_foldings )
				: matrix_(matrix), strategy_(strategy)
            {
                StructPartManager sec_struct_manager(max_foldings);
                SecStruct sec_struct_empty;
                SecStructCount start_sec_struct(sec_struct_empty);
                StructPartIter start_struct_iter = sec_struct_manager.insert(start_sec_struct);
                ActivePoint start_point(0, strategy_.getLength() - 1);
                States states;
                states.addState( FoldActivePoints(start_point, start_struct_iter) );
				find( states, sec_struct_manager );

                //kopiuje wyniki
				std::transform( sec_struct_manager.begin(), sec_struct_manager.end(),
								std::inserter( possible_structures_, possible_structures_.begin() ),
								bind(&SecStructCount::get, _1) );
            }

            const SecStructures& getStructures() const { return possible_structures_; }

            std::ostream& printStructures(std::ostream& os, int print_width) const;
        private:
            const Matrix& matrix_;
            const FoldedMatrixStrategy& strategy_;

            SecStructures possible_structures_;

            //przeszukuje macierz, znajduje struktury drugorzedowe
            void find(States& states, StructPartManager& sec_struct_manager) const;
        };

        inline FoldedMatrix::FoldedMatrix(const FoldedMatrixStrategy& strategy, unsigned int max_foldings )
            : strategy_(strategy), max_foldings_(max_foldings), matrix_(strategy_.getLength() ), proxy_(0L)
        {
            makeMatrix();
            structure_energy_ = matrix_.get(0,matrix_.getLength()-1 );
        }

        inline const SecStructures& FoldedMatrix::getStructures() {
            if( proxy_.get() == 0L )
                proxy_.reset( new SecStructProxy(matrix_, strategy_, max_foldings_) );
            return proxy_->getStructures();
        }

        inline std::ostream& FoldedMatrix::printStructures(std::ostream& os, int print_width) const {
            if(proxy_.get() != 0L )
                proxy_->printStructures(os,print_width);
            else
                os << "sec struct not calculated" << std::endl;
            return os;
        }

		namespace {
			/** support function to find maximum value for given call in dynamic programming algorithms */
			inline int findMaxVal(const Matrix& matrix,
								  const FoldedMatrixStrategy& strategy,
								  int i, int j) {
				int left=matrix.get(i,j-1);
				int down=matrix.get(i+1,j);
				int downleft= matrix.get(i+1,j-1) + strategy.getEnergy(i,j);
				return (std::max)( (std::max)(left, down), downleft );
			}

		}

        //tworzy macierz energii dla jednego lancucha
        inline void FoldedMatrix::makeMatrix() {

            int length = matrix_.getLength();
			int split_point = strategy_.getSplitIndex(); //first index belonging to the second chain
			int n; //odleglosc (i,j)

			//wypelnia macierz w okolicy podzialu dwu czasteczek (split point)
			for(n = 1; n < 4 && n < length; ++n) {
				int i = (std::max)( split_point - n, 0);
				int j = i + n;
				for(;(i < split_point) && (i < length-1) && (j < length); ++i,++j) {
                    int max_v  = findMaxVal( matrix_, strategy_, i, j);
                    matrix_.set(i,j,max_v);
				}
			}

			//wypelnia dla pozostalych
            for(n=4; n<length; ++n){
				int i=0;
				int j=n;
                for( ;(i < length-1) && ( j<length );++i, ++j) {
                    int max_val  = findMaxVal( matrix_, strategy_, i, j);

                    for(int k=0; k<j-i; ++k) {
                        int loopval = matrix_.get(i,i+k)+matrix_.get(k+i+1,j);
                        if( loopval > max_val)
                            max_val = loopval;
                    }
                    matrix_.set(i,j,max_val);
                }
            }
        }

        //drukuje macierz na wskazany strumien
        inline std::ostream& FoldedMatrix::printMatrix(std::ostream& os, int print_width) const {
            for(int i=0;i < strategy_.getLength();++i) {
                os.width(print_width);
                os <<i;
            }
            os.width(7);
            os << "j/"<< std::endl;
            os.width(print_width);
            for(int i=0;i < strategy_.getLength();++i){
                os.width(print_width);
                os <<*strategy_.getNucleotide(i);
            }
            os.width(7);
            os << "i"<< std::endl;
            {
                int i = 0;
				for(;i < strategy_.getSplitIndex();++i){
					os.width(print_width);
					os.fill('-');
					os <<"-";
				}
				os << '+';
				for(;i < strategy_.getLength(); ++i) {
					os.width(print_width);
					os.fill('-');
					os <<"-";
				}
            }
            os.fill(' ');
            os << std::endl;
            for(int i=0;i<strategy_.getLength();++i) {
                for(int j=0;j<strategy_.getLength();++j){
                    os.width(print_width);
                    if(j>=i)
                        os << matrix_.get(i,j);
                    else
                        os << ".";
                }
                os<<"|";
                os.width(2);
                os<<*strategy_.getNucleotide(i);
                os.width(4);
                os<< i << std::endl;
            }
            return os;
        }


        //drukuje struktury na wskazany strumien
        inline std::ostream& SecStructProxy::printStructures(std::ostream& os, int print_width) const {
            os << "sequence length: "<<  strategy_.getLength() << std::endl;
            os <<"found: " << possible_structures_.size() << " structures" << std::endl;

            for(int i = 0; i < strategy_.getLength(); ++i ) {
                os.width(print_width);
                os << *strategy_.getNucleotide(i);
            }
            os << std::endl;
			{
				int i = 0;
				for(; i < strategy_.getSplitIndex(); ++i){
					os.width(print_width);
					os<<i;
				}
				for(; i < strategy_.getLength();++i) {
					os.width(print_width);
					os<<i - strategy_.getSplitIndex();
				}
			}
			os << std::endl;

			std::copy( possible_structures_.begin(), possible_structures_.end(), std::ostream_iterator<SecStruct>(os,"\n") );
            return os;
        }

        /** znajduje pierwsza strukture (przeglada wglab) */
        inline SecStruct& FoldedMatrix::findDepthRecur(SecStruct& structure, int x, int y) const {

            EnergyValue current_energy = matrix_.get(x,y);

            //warunek stopu
            if(current_energy == 0)
                return structure;

            if(current_energy == matrix_.get(x,y-1) )
                return findDepthRecur( structure, x, y-1 );
            else if(current_energy == matrix_.get(x+1,y) )
                return findDepthRecur( structure, x+1, y);
            else if(current_energy == matrix_.get(x+1,y-1) + strategy_.getEnergy(x,y) ) {
                //tutaj tworzy curr_point
                structure.addPair( ConnectPair( strategy_.getNucleotide(x), strategy_.getNucleotide(y) ) );
                return findDepthRecur( structure, x+1, y-1);
            }
            else //znajduje skok
                for(int k=x; k < y; ++k)
                    if(current_energy == (matrix_.get(x,k)+matrix_.get(k+1,y) ) ) {
                        SecStruct struct_one;
                        findDepthRecur(struct_one, x, k);
                        SecStruct struct_two;
                        findDepthRecur(struct_two, k+1, y);
                        structure.append(struct_one);
                        structure.append(struct_two);
                        return structure;
                    }
            //tutaj nie powinien sie nigdy znalezc
            assert(0);
            return structure;
        }

        /** znajduje struktury drugorzedowe na podst. macierzy energii */
        inline void SecStructProxy::find(States& states, StructPartManager& sec_struct_manager) const
        {
            while(!states.empty() ) {
                // cout << "states:" << states.size() << " sec_structs:" << sec_struct_manager.size() << std::endl;
                //pobiera nastepny do rozpatrzenia (w odpowiedniej kolejnosci!). Usuwa go z kolekcji.
                FoldActivePoints state = states.getFirstState();
                // cout << state;

                //znajduje aktywny punkt (usuwa go z aktywnych punktow dla danego stanu)
                ActivePoint curr_point = state.getFirstActivePoint();

                int x = curr_point.first;
                int y = curr_point.second;

                EnergyValue current_energy = matrix_.get(x,y);

                //warunek stopu
                if(current_energy == 0) {

                    if( state.hasActivePoint() ) {
                        //dodaje stan z pozostalymi punktami aktywnymi
                        states.addState(state);
                    }
                    //jezeli nie - to nie dodaje (ale tez nie zmniejsza licznikow do struktur, wiec one zostana
                }
                else {
                    //tutaj nie stop, czyli trzeba rozpatrywac stany nastepne

                    //struktury przechowywane przez stan biezacy staja sie nieaktywne
                    state.getFoldedParts().decPartsCount();


                    bool left = current_energy == matrix_.get(x,y-1);
                    bool down = current_energy == matrix_.get(x+1,y);
                    bool diag = current_energy == (matrix_.get(x+1,y-1) + strategy_.getEnergy(x,y) );

                    if(left) {
                        //                      cout << "l";
                        state.getFoldedParts().incPartsCount();
                        states.addStateMove(state, ActivePoint(x, y-1) );
                    }
                    if(down) {
                        //                      cout << "d";
                        state.getFoldedParts().incPartsCount();
                        states.addStateMove(state, ActivePoint(x+1, y) );
                    }
                    if(diag) {
                        //                      cout << "x";
                        state.getFoldedParts().incPartsCount();
                        //dodaje pare do czesciowo obliczonych struktur
                        //gdy tworzy nowe struktury to zwraca true, jezeli udalo sie je dodac
                        ConnectPair current_pair( strategy_.getNucleotide(x), strategy_.getNucleotide(y) );
                        if( state.addPair(current_pair, sec_struct_manager) )
                            states.addStateMove(state, ActivePoint(x+1, y-1) );
                    }

                    if(!left && !down && !diag) {
                        //znajduje miejsce w ktorym byla petla
                        for(int k=x; k < y; ++k)
                            if(current_energy == (matrix_.get(x,k)+matrix_.get(k+1,y) ) ) {
                                // cout << "EA jump " << structure << " x = " << x << " y = " << y << " k = " << k << std::endl;
                                state.getFoldedParts().incPartsCount();
                                states.addStateJump(state, ActivePoint(x, k), ActivePoint(k+1, y) );
                            }
                    }
                }
                //              cout << std::endl << std::endl;
            }
        }


    } //namespace dna
} //namespace faif

#endif //FOLDED_MATRIX_H
