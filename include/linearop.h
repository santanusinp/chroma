// -*- C++ -*-
// $Id: linearop.h,v 1.13 2003-10-20 20:24:46 edwards Exp $

/*! @file
 * @brief Linear Operators
 */

#ifndef __linearop_h__
#define __linearop_h__

using namespace QDP;

enum LinOpSign {PLUS = 1, MINUS = -1};


//! Linear Operator
/*! @ingroup linop
 *
 * Supports creation and application for linear operators that
 * hold things like Dirac operators, etc.
 */

template<typename T>
class LinearOperator
{
public:
  //! Apply the operator onto a source vector
  virtual T operator() (const T& psi, enum LinOpSign isign) const = 0;

  //! Return the subset on which the operator acts
  virtual const OrderedSubset& subset() const = 0;

  //! Virtual destructor to help with cleanup;
  virtual ~LinearOperator() {}
};


//! Dslash-like Linear Operator
/*! @ingroup linop
 *
 * These are concessions/optimizations for red-black checkboarding 
 */
template<typename T>
class DslashLinearOperator : public LinearOperator<T>
{
public:
  //! Apply operator on both checkerboards (entire lattice)
  virtual T operator() (const T& psi, enum LinOpSign isign) const
    {
      T d;

      d[rb[0]] = apply(psi, isign, 0);
      d[rb[1]] = apply(psi, isign, 1);

      return d;
    }

  //! Apply checkerboarded linear operator
  /*! 
   * To avoid confusion (especially of the compilers!), call the checkerboarded
   * apply instead of operator()
   */
  virtual T apply (const T& psi, enum LinOpSign isign, int cb) const = 0;

  //! Virtual destructor to help in cleanup
  virtual ~DslashLinearOperator() {}
};


#endif
