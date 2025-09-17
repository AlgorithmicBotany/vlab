#ifndef UTIL_FORALL_HPP
#define UTIL_FORALL_HPP
#include <iterator>
#include <utility>

namespace util
{
  namespace ForAll
    {
    struct BaseForwardIter
      {
      virtual ~BaseForwardIter() {}
      };

    template <typename iterator>
      struct ForwardIter : public BaseForwardIter
        {
        typedef typename std::iterator_traits<iterator>::value_type value_type;
        typedef typename std::iterator_traits<iterator>::reference reference;

        ForwardIter( const iterator& fst, const iterator& lst )
          : it( fst )
          , end( lst )
          , brk( 0 ) {}

        const reference value() const { ++brk; return *it; }
        bool is_end() const { return ( it == end ) || brk; }
        mutable iterator it;
        iterator end;
        mutable int brk;
        };

    template <typename Pair, typename iterator>
      inline ForwardIter<iterator>
      forwardIter( const Pair& range, const iterator* )
        {
        return ForwardIter<iterator>( range.first, range.second );
        }

    template <typename iterator>
      inline const ForwardIter<iterator>*
      castForwardIter( const BaseForwardIter* base, const iterator* )
        {
        return static_cast< const ForwardIter<iterator>* >( base );
        }

    template <typename T> inline T* pointer( const T& ) { return 0; }

    template <typename Container>
      std::pair<typename Container::iterator, typename Container::iterator> make_range( Container& cont )
        {
        return std::make_pair( cont.begin(), cont.end() );
        }

    template <typename Container>
      std::pair<typename Container::const_iterator, typename Container::const_iterator> make_range( const Container& cont )
        {
        return std::make_pair( cont.begin(), cont.end() );
        }

#define forall_pointer( obj ) ( true ? 0 : util::ForAll::pointer( obj ) )

    /**
     * Macro allowing for automatic iteration over range.first -> range.second 
     * (range being a std::pair)
     */
#define forall_range( typed_var, range ) \
    for( const util::ForAll::BaseForwardIter& iter = util::ForAll::forwardIter( range, forall_pointer( (range).first ) ) ; \
         !util::ForAll::castForwardIter( &iter, forall_pointer( (range).first ) )->is_end() ;\
         ++( util::ForAll::castForwardIter( &iter, forall_pointer( (range).first ) )->it ) ) \
    for( typed_var = util::ForAll::castForwardIter( &iter, forall_pointer( (range).first ) )->value() ; \
         util::ForAll::castForwardIter( &iter, forall_pointer( (range).first ) )->brk ; \
         --( util::ForAll::castForwardIter( &iter, forall_pointer( (range).first ) )->brk ) )

    /**
     * Macro iterating over a standard container from cont.begin() to cont.end()
     */
#define forall( typed_var, cont ) forall_range( typed_var, util::ForAll::make_range( cont ) )

    /**
     * Macro iterating avec a a container but from cont.begin_name() to 
     * cont.end_name()
     *
     * \warning In the current implementation, cont is evaluated twice!
     */
#define forall_named( typed_var, cont, name ) forall_range( typed_var, std::make_pair( (cont).begin_##name(), (cont).end_##name() ) )

    }
}

#endif // UTIL_FORALL_HPP
