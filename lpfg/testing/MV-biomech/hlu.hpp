#ifndef _HLU_HPP
#define _HLU_HPP

/*********
 * HLUTriple: holds a triple (H,L,U) of type T
 *********/
template<class T>
struct HLUTriple
{
  T H,L,U;
  HLUTriple(void) : H(), L(), U() {}
  HLUTriple(const T& _H, const T& _L, const T& _U)
    : H(_H), L(_L), U(_U) {}

  HLUTriple& operator=(const HLUTriple& hlu)
  {
    H = hlu.H; L = hlu.L; U = hlu.U;
    return *this;
  }

  template<class S>
  HLUTriple operator*(const S& s) const
  {
    HLUTriple ans;
    ans.H = H * s;  ans.L = L * s;  ans.U = U * s;
    return ans;
  }
};

/*********
 * Orientation class: holds a turtle orientation (H,L,U)
 *********/
struct Orientation
  : public HLUTriple<V3f>
{
  Orientation(void) : HLUTriple<V3f>(V3f(1,0,0) , V3f(0,1,0) , V3f(0,0,1)) {}
  Orientation(const V3f& _H, const V3f& _L, const V3f& _U)
    : HLUTriple<V3f>(_H,_L,_U) {}
  Orientation reoriented(const Qf& q) const
  {
    return Orientation(q.rotateVector(H),
		       q.rotateVector(L),
		       q.rotateVector(U));
  }
};

/**********
 * DiagTensor: holds a tensor with principal directions H,L,U
 **********/
typedef HLUTriple<float> DiagTensor;

#endif // _HLU_HPP
