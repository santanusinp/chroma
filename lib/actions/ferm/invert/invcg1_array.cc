// $Id: invcg1_array.cc,v 1.2 2004-05-27 11:21:23 bjoo Exp $
/*! \file
 *  \brief Conjugate-Gradient algorithm for a generic Linear Operator
 */

#include "chromabase.h"
#include "actions/ferm/invert/invcg1_array.h"


//! Conjugate-Gradient (CGNE) algorithm for a generic Linear Operator
/*! \ingroup invert
 * This subroutine uses the Conjugate Gradient (CG) algorithm to find
 * the solution of the set of linear equations
 *
 *   	    Chi  =  A . Psi
 *
 * where       A is hermitian
 *
 * Algorithm:

 *  Psi[0]  :=  initial guess;    	       Linear interpolation (argument)
 *  r[0]    :=  Chi - M^dag . M . Psi[0] ;     Initial residual
 *  p[1]    :=  r[0] ;	       	       	       Initial direction
 *  IF |r[0]| <= RsdCG |Chi| THEN RETURN;      Converged?
 *  FOR k FROM 1 TO MaxCG DO    	       CG iterations
 *      a[k] := |r[k-1]|**2 / <Mp[k],Mp[k]> ;
 *      Psi[k] += a[k] p[k] ;   	       New solution vector
 *      r[k] -= a[k] M^dag . M . p[k] ;        New residual
 *      IF |r[k]| <= RsdCG |Chi| THEN RETURN;  Converged?
 *      b[k+1] := |r[k]|**2 / |r[k-1]|**2 ;
 *      p[k+1] := r[k] + b[k+1] p[k];          New direction
 *
 * Arguments:
 *
 *  \param M       Linear Operator    	       (Read)
 *  \param chi     Source	               (Read)
 *  \param psi     Solution    	    	       (Modify)
 *  \param RsdCG   CG residual accuracy        (Read)
 *  \param MaxCG   Maximum CG iterations       (Read)
 *  \param n_count Number of CG iteration      (Write)
 *
 * Local Variables:
 *
 *  p   	       Direction vector
 *  r   	       Residual vector
 *  cp  	       | r[k] |**2
 *  c   	       | r[k-1] |**2
 *  k   	       CG iteration counter
 *  a   	       a[k]
 *  b   	       b[k+1]
 *  d   	       < p[k], A.p[k] >
 *  Mp  	       Temporary for  M.p
 *
 * Subroutines:
 *                             +               
 *  A       Apply matrix M or M  to vector
 *
 * Operations:
 *
 *  2 A + 2 Nc Ns + N_Count ( 2 A + 10 Nc Ns )
 */

#undef PRINT_5D_RESID
template<typename T>
void InvCG1_a(const LinearOperator< multi1d<T> >& A,
	      const multi1d<T> & chi,
	      multi1d<T>& psi,
	      const Real& RsdCG, 
	      int MaxCG, 
	      int& n_count)
{
  const int N = psi.size();
  const OrderedSubset& s = A.subset();

  Real chi_sq =  Real(norm2(chi,s));

  QDPIO::cout << "chi_norm = " << sqrt(chi_sq) << endl;
  Real rsd_sq = (RsdCG * RsdCG) * chi_sq;

  //                                            
  //  r[0]  :=  Chi - A . Psi[0]    where  A is hermitian
    
  //                     
  //  r  :=  [ Chi  -  A. psi ]
  multi1d<T> r(N), Ap(N);
  A(Ap, psi, PLUS);
  for(int n=0; n < N; ++n)
    r[n][s] = chi[n] - Ap[n];

#ifdef PRINT_5D_RESID 
  for(int n=0; n < N; n++) {
    Double norm_r = norm2(r[n],s);
    if( toBool( norm_r > Double(1.0e-20)) ) {
      QDPIO::cout << "Iteration 0  r[" << n << "] = " << norm_r << endl;
    }
  }
#endif


  //  p[1]  :=  r[0]
  multi1d<T> p(N);
  for(int n=0; n < N; ++n) {
    p[n][s] = r[n];
  }

  //  Cp = |r[0]|^2
  Double cp = norm2(r, s);   	       	   /* 2 Nc Ns  flops */

  QDPIO::cout << "InvCG: k = 0  cp = " << cp << "  rsd_sq = " << rsd_sq << endl;

  //  IF |r[0]| <= RsdCG |Chi| THEN RETURN;
  if ( toBool(cp  <=  rsd_sq) )
  {
    n_count = 0;
    return;
  }

  //
  //  FOR k FROM 1 TO MaxCG DO
  //
  Real a, b;
  Double c, d;
  
  for(int k = 1; k <= MaxCG; ++k)
  {
    //  c  =  | r[k-1] |**2
    c = cp;

    //  a[k] := | r[k-1] |**2 / < p[k], Ap[k] > ;
    //      	       	       	       	       	  
    //  First compute  d  =  < p, A.p > 
    //  Mp = M(u) * p

    A(Ap, p, PLUS);

    //  d = | mp | ** 2
    d=Double(0);

    // This is probably inefficient. It would be nice
    // to have an inner product real with multi1d<>
    // the best way is to have a local inner product 
    // and then do one big global sum at the end.
    for(int n=0; n < N; ++n) {
      d += innerProductReal(p[n], Ap[n], s);	/* 2 Nc Ns  flops */
    }

    a = Real(c)/Real(d);

    //  Psi[k] += a[k] p[k]
    for(int n=0; n < N; ++n) {
      psi[n][s] += a * p[n];	/* 2 Nc Ns  flops */
    }

    //  r[k] -= a[k] A . p[k] ;
    //      	      
    for(int n=0; n < N; ++n) {
      r[n][s] -= a * Ap[n];
    }

#ifdef PRINT_5D_RESID
    for(int n=0; n < N; n++) {
      Double norm_r = norm2(r[n],s);
      if( toBool( norm_r > Double(1.0e-20)) )
	QDPIO::cout << "Iteration " << k << " r[" << n << "] = " << norm_r << endl;
    }
#endif

    //  IF |r[k]| <= RsdCG |Chi| THEN RETURN;

    //  cp  =  | r[k] |**2
    cp = norm2(r, s);	                /* 2 Nc Ns  flops */

    QDPIO::cout << "InvCG: k = " << k << "  cp = " << cp << endl;

    if ( toBool(cp  <=  rsd_sq) )
    {
      n_count = k;
      return;
    }

    //  b[k+1] := |r[k]|**2 / |r[k-1]|**2
    b = Real(cp) / Real(c);

    //  p[k+1] := r[k] + b[k+1] p[k]
    for(int n=0; n < N; ++n)
      p[n][s] = r[n] + b*p[n];	/* Nc Ns  flops */
  }
  n_count = MaxCG;
  QDPIO::cerr << "Nonconvergence Warning" << endl;
}


// Fix here for now
template<>
void InvCG1(const LinearOperator< multi1d<LatticeFermion> >& A,
	    const multi1d<LatticeFermion>& chi,
	    multi1d<LatticeFermion>& psi,
	    const Real& RsdCG, 
	    int MaxCG, 
	    int& n_count)
{
  InvCG1_a(A, chi, psi, RsdCG, MaxCG, n_count);
}

