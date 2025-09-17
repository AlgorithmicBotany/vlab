#ifndef STATIC_ASSERT_HPP
#define STATIC_ASSERT_HPP

namespace util
{

  // Extracted from boost static_asset.hpp ...
  template <bool x> struct STATIC_ASSERTION_FAILURE;
  template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
  template<int x> struct static_assert_test{};

#define JOIN( X, Y ) DO_JOIN( X, Y )
#define DO_JOIN( X, Y ) DO_JOIN2(X,Y)
#define DO_JOIN2( X, Y ) X##Y

//#define STATIC_ASSERT( B ) \
   typedef util::static_assert_test<\
      sizeof(util::STATIC_ASSERTION_FAILURE< (bool)( B ) >)>\
         JOIN(static_assert_typedef_, __LINE__)


#define STATIC_ASSERT( B ) \
   util::static_assert_test<\
      sizeof(util::STATIC_ASSERTION_FAILURE< (bool)( B ) >)>\
         JOIN(static_assert_typedef_, __LINE__)

}


#endif // STATIC_ASSERT_HPP
