#include "orbital.h"

static char *rcsid="$Id: orbital.c,v 1.29 2002/08/02 14:07:13 mfgu Exp $";
#if __GNUC__ == 2
#define USE(var) static void * use_##var = (&use_##var, (void *) &var) 
USE (rcsid);
#endif

/* closed Newton-Cotes formulae coeff. */
static double _CNC[5][5] = {
  {0, 0, 0, 0, 0},
  {0.5, 0.5, 0, 0, 0},
  {1.0/3.0, 4.0/3.0, 1.0/3.0, 0, 0},
  {3.0/8, 9.0/8, 9.0/8, 3.0/8, 0},
  {14.0/45, 64.0/45, 24.0/45, 64.0/45, 14.0/45,}
};

/* open Newton-Cotes formulae coeff. */
static double _ONC[9][9] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 2.0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1.5, 1.5, 0, 0, 0, 0, 0, 0},
  {0, 8./3, -4./3, 8./3, 0, 0, 0, 0, 0},
  {0, 55./24, 5./24, 5./24, 55./24, 0, 0, 0, 0},
  {0, 33./10, -42./10, 78./10, -42./10, 33./10, 0, 0, 0},
  {0, 4277./1440, -3171./1440, 3934./1440, 3934./440, 
   -3171./1440, 4277./1440, 0, 0},
  {0, 3680./945, -7632./945, 17568./945, -19672./945,
   17568./945, -7632./945, 3680./945, 0}
};

/* the following arrays provide storage space in the calculation */
static double _veff[MAX_POINTS];
static double ABAND[4*MAX_POINTS];
static double _dwork[MAX_POINTS];
static double _dwork1[MAX_POINTS];
static double _dwork2[MAX_POINTS];
 
static int max_iteration = 512;
static int nmax = 0;
static double wave_zero = 1E-10;

static int SetVEffective(int kl, POTENTIAL *pot);
static int TurningPoints(int n, double e, POTENTIAL *pot);
static int IntegrateRadial(double *p, double e, POTENTIAL *pot, 
			   int i1, double p1, int i2, double p2);
static double Amplitude(double *p, double e, int kl, POTENTIAL *pot, int i1);
static int Phase(double *p, POTENTIAL *pot, int i1, double p0);
static int DiracSmall(ORBITAL *orb, POTENTIAL *pot);

void dgbsv_(int *n, int *kl, int *ku, int *nrhs, double *a, 
	    int *lda, int *ipiv, double *b, int *ldb, int *info);
double dlogam_(double *x);
void y5n_(double *lambda, double *eta0, double *x0, 
	  double *y5, double *y5p, double *norm, int *ierr);
void lsode_(void (*f)(int *, double *, double *, double *), 
	    int *neq, double *y, double *r0, double *r, int *itol, 
	    double *rtol, double *atol, int *itask, int *istate, 
	    int *iopt, double *rwork, int *lrw, int *iwork, int *liw, 
	    void (*jac)(int *, double *, double *, int *, 
			int *, double *, int *), 
	    int *mf);

int GetNMax(void) {
  return nmax;
}
 
int RadialSolver(ORBITAL *orb, POTENTIAL *pot, double tol) {
  int ierr;
  
  if (orb->n > 0) {
    if (orb->n < nmax) {
      ierr = RadialBound(orb, pot, tol);
    } else {
      ierr = RadialRydberg(orb, pot, tol);
    }
  } else {
    ierr = RadialFree(orb, pot);
  }  
  return ierr;
}

double *GetVEffective(void) { 
  return _veff;
}

double EnergyH(double z, double n, int ka) {
  double a, np;

  ka = abs(ka);
  a = FINE_STRUCTURE_CONST*z;
  
  np = n + sqrt(ka*ka - a*a) - ka;
  a = a/np;
  a = (1.0/sqrt(1.0 + a*a)) - 1.0;
  a /= FINE_STRUCTURE_CONST2;
  
  return a;
}

int RadialBound(ORBITAL *orb, POTENTIAL *pot, double tol) {
  double z, z0, e, e0, emin, emax;
  int i, kl, nr, nodes, niter;
  int i2, i2p, i2m, i2p2, i2m2, ierr;
  double *p, p1, p2, qi, qo, delta, ep, norm2, fact, eps;
  
  kl = orb->kappa;
  kl = (kl < 0)? (-kl-1):kl;

  if (kl < 0 || kl >= orb->n) {
    printf("Invalid orbital angular momentum, L=%d\n", kl);
    return -1;
  }
  
  p = malloc(sizeof(double)*2*MAX_POINTS);
  if (!p) return -1;

  niter = -1;
  nr = orb->n - kl -1;
  ierr = 0;

  z0 = pot->Z[MAX_POINTS-1];
  z = (z0 - pot->N + 1.0);
  e = EnergyH(z, orb->n, orb->kappa);
  emax = e*0.95;
  emin = EnergyH(z0, orb->n, orb->kappa);
  emin *= 1.05;

  SetPotentialW(pot, e, orb->kappa);   
  SetVEffective(kl, pot);

  while (1) {
    i2 = TurningPoints(orb->n, e, pot);
    if (i2 == -2) {
      e *= 0.9;
      continue;
    }
    
    i2p = i2 + 1;
    i2m = i2 - 1;
    i2p2 = i2 + 2;
    i2m2 = i2 - 2;
    nodes = IntegrateRadial(p, e, pot, 0, 0.0, i2p2, 1.0);
    
    if (nodes != nr) {
      if (nodes > nr) {
	ep = emin;
	emax = e*0.95;
      } else {
	ep = emax;
	emin = e*1.05;
      }
      p1 = nodes + kl + 1;
      p1 = fabs(p1 - orb->n)/(p1 + orb->n);
      p2 = 1.0 - p1;
      e = p2*e + p1*ep;
      SetPotentialW(pot, e, orb->kappa);
      SetVEffective(kl, pot);
      continue;
    }   
 
    qo = (-4.0*p[i2m2-1] + 30.0*p[i2m2] - 120.0*p[i2m]
	  + 40.0*p[i2] + 60.0*p[i2p] - 6.0*p[i2p2])/120.0;
    p1 = p[i2m2];
    for (i = 0; i < i2p; i++) {
      p[i] = p[i] * pot->dr_drho2[i];
    }
    p2 = p[i2];
    norm2 = InnerProduct(0, i2p, p, p, pot);

    IntegrateRadial(p, e, pot, i2m2, p1, MAX_POINTS-1, 0.0);
    qi = (6.0*p[i2m2] - 60.0*p[i2m] - 40.0*p[i2] + 120.0*p[i2p]
	  - 30.0*p[i2p2] + 4.0*p[i2p2+1])/120.0;
    for (i = i2m2; i < MAX_POINTS; i++) {
      p[i] = p[i] * pot->dr_drho2[i];
    }
    fact = p2/p[i2];
    qi *= fact;
    norm2 += fact*fact*InnerProduct(i2, MAX_POINTS-i2, p, p, pot);

    fact = 1.0 / pot->dr_drho2[i2];
    qo = qo * fact;
    qi = qi * fact;
    
    niter++;
    delta = 0.5*p2*(qo - qi)/norm2;
    ep = e+delta;
    if (ep >= emax) {
      ep = e + (emax - e)*0.1;
      emin = e;
    }
    if (ep <= emin) {
      ep = e + (emin - e)*0.1;
      emax = e;
    }
    e0 = e;
    e = ep;
    ep = Max(tol, fabs(ep)*tol);
    if ((fabs(delta) < ep || fabs(e-e0) < ep)) break;
    if (niter > max_iteration) {
      printf("MAX iteration reached in Bound.\n");
      printf("%10.3E %10.3E %10.3E %10.3E\n", e, e0, qo, qi);
      ierr = -4;
      break;
    }
    SetPotentialW(pot, e, orb->kappa);
    SetVEffective(kl, pot);
  }
  if (ierr) {
    free(p);
    return ierr;
  }
  
  qi = sqrt(norm2);
  fact = 1.0/qi;
  if (IsOdd(nodes)) {
    fact = -fact;
  }
 
  qi *= wave_zero;     
  for (i = MAX_POINTS-1; i >= 0; i--) {
    if (fabs(p[i]) > qi) break;
  }
  if (IsEven(i)) i++;
  orb->ilast = i;
       
  for (i = 0; i < MAX_POINTS; i++) {    
    p[i] *= fact;
  }
 
  orb->energy = e;
  orb->wfun = p;

  orb->qr_norm = 1.0;
  DiracSmall(orb, pot);
 
  return 0;
}

int RadialRydberg(ORBITAL *orb, POTENTIAL *pot, double tol) {
  double z, e, e0, emin, emax;
  int i, kl, niter, ierr;
  double lambda, eta0, x0, y5, y5p, y5norm;
  int i2, i2p, i2m, i2p2, i2m2, nodes, nr;
  double qo, qi, norm2, delta, dk, zp, *p;
  double ep, p1, p2, fact;

  z = (pot->Z[MAX_POINTS-1] - pot->N + 1.0);
  kl = orb->kappa;
  kl = (kl < 0)? (-kl-1):kl;
  if (kl < 0 || kl >= orb->n) {
    printf("Invalid orbital angular momentum, L=%d\n", kl);
    return -1;
  }
  e = EnergyH(z, orb->n, orb->kappa);
  emax = 0.95*e;
  emin = EnergyH(z, orb->n-0.95, orb->kappa);
  SetPotentialW(pot, e, orb->kappa);
  SetVEffective(kl, pot);
  p = malloc(sizeof(double)*2*MAX_POINTS);
  if (!p) return -1;
  nr = orb->n - kl - 1;

  niter = -1;
  zp = z*FINE_STRUCTURE_CONST;
  lambda = kl + 0.5;
  lambda = lambda*lambda - zp*zp;
  if (lambda > 0) {
    lambda = sqrt(lambda) - 0.5;
  } else {
    lambda = kl;
  }
  while (niter < max_iteration) {
    niter++;
    i2 = TurningPoints(orb->n, e, pot);
    if (i2 < 0) {
      printf("The orbital angular momentum = %d too high\n", kl);
      return -2;
    }
    i2p = i2 + 1;
    i2m = i2 - 1;
    i2p2 = i2 + 2;
    i2m2 = i2 - 2;      
    nodes = IntegrateRadial(p, e, pot, 0, 0.0, i2p2, 1.0);
    if (i2 < MAX_POINTS-24) {
      qo = (-4.0*p[i2m2-1] + 30.0*p[i2m2] - 120.0*p[i2m]
	    + 40.0*p[i2] + 60.0*p[i2p] - 6.0*p[i2p2])/120.0;
      p1 = p[i2m2];
      for (i = 0; i < i2p; i++) {
	p[i] = p[i] * pot->dr_drho2[i];
      }
      p2 = p[i2];
      norm2 = InnerProduct(0, i2p, p, p, pot);
      
      IntegrateRadial(p, e, pot, i2m2, p1, MAX_POINTS-1, 0.0);
      qi = (6.0*p[i2m2] - 60.0*p[i2m] - 40.0*p[i2] + 120.0*p[i2p]
	    - 30.0*p[i2p2] + 4.0*p[i2p2+1])/120.0;
      for (i = i2m2; i < MAX_POINTS; i++) {
	p[i] = p[i] * pot->dr_drho2[i];
      }
      fact = p2/p[i2];
      qi *= fact;
      norm2 += fact*fact*InnerProduct(i2, MAX_POINTS-i2, p, p, pot);
      
      fact = 1.0/pot->dr_drho2[i2];
      qo = qo * fact;
      qi = qi * fact;
      delta = 0.5*p2*(qo - qi)/norm2;
      ep = e+delta;
      if (ep >= emax) {
	ep = e + (emax - e)*0.1;
	emin = e;
      }
      if (ep <= emin) {
	ep = e + (emin - e)*0.1;
	emax = e;
      }
      e0 = e;
      e = ep;
      ep = Max(tol, fabs(ep)*tol);
      if ((fabs(delta) < ep || fabs(e-e0) < ep)) {
	fact = 1.0/sqrt(norm2);
	if (IsOdd(nodes)) fact = -fact;
	for (i = 0; i < MAX_POINTS; i++) {
	  p[i] *= fact;
	}
	break;
      }
      if (niter > max_iteration) {
	printf("MAX iteration reached in Bound.\n");
	printf("%10.3E %10.3E %10.3E %10.3E\n", e, e0, qo, qi);
	return -4;
      }
      SetPotentialW(pot, e, orb->kappa);
      SetVEffective(kl, pot);
    } else {
      for (i = i2m2-1; i <= i2p2; i++) {
	_dwork[i] = p[i] * pot->dr_drho2[i];
      }
      qo = (-4.0*_dwork[i2m2-1] + 30.0*_dwork[i2m2] - 120.0*_dwork[i2m]
	    + 40.0*_dwork[i2] + 60.0*_dwork[i2p] - 6.0*_dwork[i2p2])/120.0;
      qo /= _dwork[i2]*pot->dr_drho[i2];

      zp = FINE_STRUCTURE_CONST2*e;
      dk = sqrt(-2.0*e*(1.0 + 0.5*zp));
      zp = z*(1.0 + zp);
      eta0 = zp/dk;
      x0 = dk*pot->rad[i2];
      y5n_(&lambda, &eta0, &x0, &y5, &y5p, &y5norm, &ierr);
      e0 = eta0 - lambda;
      norm2 = dlogam_(&e0);
      e0 = eta0 + lambda + 1.0;
      norm2 += dlogam_(&e0);
      e0 = zp/(eta0*eta0);
      norm2 = -norm2 + log(e0);
      norm2 = 0.5*norm2+y5norm; 
      norm2 = exp(norm2)*y5;
      qi = dk*y5p/y5;

      delta = 0.5*norm2*norm2*(qo-qi);
      ep = e+delta;
      if (ep >= emax) {
	ep = e + (emax - e)*0.1;
	emin = e;
      }
      if (ep <= emin) {
	ep = e + (emin - e)*0.1;
	emax = e;
      }
      e0 = e;
      e = ep;
      ep = Max(tol, fabs(ep)*tol);
      if ((fabs(delta) < ep || fabs(e-e0) < ep)) {
	p1 = p[i2p2];
	IntegrateRadial(p, e, pot, i2p2, p1, MAX_POINTS-1, 0.0);
	fact = norm2/_dwork[i2];
	if (IsOdd(nodes)) fact = -fact;
	for (i = 0; i < MAX_POINTS; i++) {
	  p[i] *= pot->dr_drho2[i] * fact;
	}
	break;
      }
      if (niter > max_iteration) {
	printf("MAX iteration reached in Bound.\n");
	printf("%10.3E %10.3E %10.3E %10.3E\n", e, e0, qo, qi);
	return -5;
      }
      SetPotentialW(pot, e, orb->kappa);
      SetVEffective(kl, pot);
    }
  }

  if (niter == max_iteration) {
    printf("MAX iteration reached in RadialRydberg, for N=%d, Kappa=%d\n",
	   orb->n, orb->kappa);
    return -3;
  } 

  for (i = MAX_POINTS-1; i >= 0; i--) {
    if (fabs(p[i]) > wave_zero) break;
  }
  if (IsEven(i)) i++;
  orb->ilast = i;
  orb->energy = e;
  orb->wfun = p;
  
  e0 = InnerProduct(0, MAX_POINTS, p, p, pot);
  orb->qr_norm = 1.0/e0;
  DiracSmall(orb, pot);
  
  return 0;
}  
  

/* note that the free states are normalized to have asymptotic 
   amplitude of 1/sqrt(k), */
int RadialFree(ORBITAL *orb, POTENTIAL *pot) {
  int i, kl, nodes;
  int i2, i2p, i2m, i2p2, i2m2;
  double *p, po, qo, e;
  double dfact, da, cs, si, phase0;

  e = orb->energy;
  if (e < 0.0) { 
    printf("Energy < 0 in Free\n");
    return -1;
  }
  kl = orb->kappa;
  if (orb->kappa == 0) {
    printf("Kappa == 0 in Free\n");
    return -1;
  }
  SetPotentialW(pot, e, kl);
  kl = (kl < 0)? (-kl-1):kl;  
  p = malloc(2*MAX_POINTS*sizeof(double));
  if (!p) return -1;
  SetVEffective(kl, pot);
  
  i2 = TurningPoints(0, e, pot);
  i2m = i2 - 1;
  i2p = i2 + 1;
  i2m2 = i2 - 2;
  i2p2 = i2 + 2;
  nodes = IntegrateRadial(p, e, pot, 0, 0.0, i2p2, 1.0);
  
  for (i = i2p2; i >= 0; i--) {
    p[i] *= pot->dr_drho2[i];
  }
  dfact = 1.0 / pot->dr_drho[i2];
  qo = (-4.0*p[i2m2-1] + 30.0*p[i2m2] - 120.0*p[i2m]
	+ 40.0*p[i2] + 60.0*p[i2p] - 6.0*p[i2p2])/120.0;
  qo *= dfact;
  po = p[i2];

  da = Amplitude(p, e, orb->kappa, pot, i2);
  cs = p[i2] * qo - da*po;
  si = po / p[i2];
  dfact = (si*si + cs*cs);
  dfact = 1.0/sqrt(dfact);
  phase0 = atan2(si, cs);
  if (phase0 < 0) phase0 += TWO_PI;
    
  if (IsOdd(nodes)) {
    phase0 = (phase0 < PI)?(phase0 + PI):(phase0-PI);
    dfact = -dfact;
  }
  
  for (i = 0; i < i2; i++) {
    p[i] *= dfact;
  }
    
  Phase(p, pot, i2, phase0);
  orb->ilast = i2m;
  orb->wfun = p;
  orb->phase = NULL;

  orb->qr_norm = 1.0;
  DiracSmall(orb, pot);
  return 0;
}

int DiracSmall(ORBITAL *orb, POTENTIAL *pot) {
  int i, i1, kappa;
  double xi, e, *p, a, b;

  e = orb->energy;
  kappa = orb->kappa;
  p = orb->wfun;
  i1 = orb->ilast+1;

  for (i = 0; i < i1; i++) {
    xi = e - pot->Vc[i] - pot->U[i];
    xi = xi*FINE_STRUCTURE_CONST2*0.5;
    _dwork[i] = 1.0 + xi;
    _dwork1[i] = sqrt(_dwork[i])*p[i];
    _dwork2[i] = 1.0/(24.0*pot->dr_drho[i]);
  }
  
  for (i = 0; i < i1; i++) {
    if (fabs(_dwork1[i]) < 1E-16) {
      p[i+MAX_POINTS] = 0.0;
      continue;
    } 
    if (i == 0) {
      b = -50.0*_dwork1[0];
      b += 96.0*_dwork1[1];
      b -= 72.0*_dwork1[2];
      b += 32.0*_dwork1[3];
      b -= 6.0 *_dwork1[4];
    } else if (i == 1) {
      b = -6.0*_dwork1[0];
      b -= 20.0*_dwork1[1];
      b += 36.0*_dwork1[2];
      b -= 12.0*_dwork1[3];
      b += 2.0 *_dwork1[4];
    } else if (i == i1-1) {
      b = -50.0*_dwork1[i];
      b += 96.0*_dwork1[i-1];
      b -= 72.0*_dwork1[i-2];
      b += 32.0*_dwork1[i-3];
      b -= 6.0 *_dwork1[i-4];
      b = -b; 
    } else if (i == i1-2) {
      b = -6.0*_dwork1[i+1];
      b -= 20.0*_dwork1[i];
      b += 36.0*_dwork1[i-1];
      b -= 12.0*_dwork1[i-2];
      b += 2.0 *_dwork1[i-3];
      b = -b;
    } else {
      b = 2.0*(_dwork1[i-2] - _dwork1[i+2]);
      b += 16.0*(_dwork1[i+1] - _dwork1[i-1]);
    }
    b *= _dwork2[i];
    b += _dwork1[i]*kappa/pot->rad[i];
    b /= (2.0*_dwork[i]);
    p[i] = _dwork1[i];
    p[i+MAX_POINTS] = b*FINE_STRUCTURE_CONST;
  } 
  if (orb->n > 0) {
    for (i = i1; i < MAX_POINTS; i++) {
      xi = e - pot->Vc[i] - pot->U[i];
      xi = xi*FINE_STRUCTURE_CONST2*0.5;
      _dwork[i] = 1.0 + xi;
      p[i] = sqrt(_dwork[i])*p[i];
    }
    a = InnerProduct(0, i1, p+MAX_POINTS, p+MAX_POINTS, pot);
    b = InnerProduct(0, MAX_POINTS, p, p, pot);    
    a *= orb->qr_norm;
    b *= orb->qr_norm;
    a = sqrt(a+b);
    orb->qr_norm = a/sqrt(b);
    a = 1.0/a;
    for (i = 0; i < i1; i++) {
      p[i] *= a;
      p[i+MAX_POINTS] *= a;
    }
    for (i = i1+MAX_POINTS; i < 2*MAX_POINTS; i++) {
      p[i] = 0.0;
    }
    return 0;
  }
  
  for (i = i1; i < MAX_POINTS; i += 2) {
    xi = e - pot->Vc[i] - pot->U[i];
    xi = xi*FINE_STRUCTURE_CONST2*0.5;
    _dwork[i] = 1.0 + xi;
    _dwork1[i] = sqrt(_dwork[i])*p[i];
    _dwork2[i] = 0.25/(pot->dr_drho[i]);
  }

  for (i = i1; i < MAX_POINTS; i += 2) {
    if (i == i1) {
      b = -3.0*_dwork1[i] + 4.0*_dwork1[i+2] - _dwork1[i+4];
    } else if (i == MAX_POINTS-2) {
      b = -3.0*_dwork1[i] + 4.0*_dwork1[i-2] - _dwork1[i-4];
      b = -b;
    } else {
      b = _dwork1[i+2] - _dwork1[i-2];
    }
    b *= _dwork2[i];
    b += _dwork1[i]*kappa/pot->rad[i];
    a = _dwork1[i]/(p[i]*p[i]);
    p[i] = _dwork1[i];
    p[i+MAX_POINTS] = FINE_STRUCTURE_CONST*a/(2.0*_dwork[i]); 
    p[i+1+MAX_POINTS] = FINE_STRUCTURE_CONST*b/(2.0*_dwork[i]);
  }

  b = FINE_STRUCTURE_CONST2*orb->energy;
  orb->qr_norm = sqrt((1.0 + b)/(1.0 + 0.5*b));
  return 0;
}

void DerivODE(int *neq, double *t, double *y, double *ydot) {
  double w0, w;
  double t0, s, e;
  
  t0 = y[2];
  w0 = y[3];
  s = y[4];
  e = y[5];

  w = w0 + (*t - t0)*s;
  w = 2.0*(e - w/(*t));
  
  ydot[0] = y[1];
  ydot[1] = 1.0/(y[0]*y[0]*y[0]) - y[0]*w;  
}
  
double Amplitude(double *p, double e, int ka, POTENTIAL *pot, int i0) {
  int i, n, kl1;
  double a, b, xi, r2, r3;
  double z, dk, r0, r1, r, w, v1;
  
  int neq, itol, itask, istate, iopt, lrw, iwork[22], liw, mf;
  double y[6], rtol, atol, *rwork;

  n = MAX_POINTS-1;
  z = pot->Z[n] - pot->N + 1.0;
  kl1 = ka*(ka+1);
  r1 = pot->rad[n];
  _dwork[0] = r1;
  _dwork1[0] = _veff[n];
  dk = sqrt(2.0*e);
  dk = EPS3*e*dk;
  for (i = 1; i < MAX_POINTS; i++) {
    r = _dwork[i-1]*1.02;
    _dwork[i] = r;
    r2 = r*r;
    r3 = r2*r;
    a = -z/r;
    b = e - a;
    xi = sqrt(1.0 + 0.5*FINE_STRUCTURE_CONST2*b);
    xi = xi*xi;
    b = b*b;
    v1 = z/r2;
    w = (-2.0*v1/r + 0.75*FINE_STRUCTURE_CONST2*v1*v1/xi - 2*ka*v1/r);
    w /= 4.0*xi;
    _dwork1[i] = a + 0.5*kl1/r2 - 0.5*FINE_STRUCTURE_CONST2*(b - w);

    a = z/r2;
    b = kl1/r3;
    if (a < dk && b < dk) break;
  }

  r0 = r;
  w = 2.0*(e - _dwork1[i]);
  y[0] = pow(w, -0.25);
  a = FINE_STRUCTURE_CONST2*e;
  b = FINE_STRUCTURE_CONST*z;
  y[1] = 0.5*(y[0]/w)*(z*(1+a)/r2-(kl1-b*b)/r3);

  rwork = _dwork2;
  lrw = MAX_POINTS;
  liw = 22;
  neq = 2;
  itol = 1;
  rtol = EPS5;
  atol = EPS10;
  itask = 1;
  istate = 1;
  iopt = 0;
  mf = 10;

  i--;
  for (; i >= 0; i--) {
    r = _dwork[i];
    y[2] = r0;
    y[3] = _dwork1[i+1]*r0;
    y[4] = (_dwork1[i]*r - y[3])/(r - r0);
    y[5] = e;
    lsode_(DerivODE, &neq, y, &r0, &r, &itol, &rtol, &atol, 
	   &itask, &istate, &iopt, rwork, &lrw, iwork, &liw, NULL, &mf);
    r0 = r;
  }

  p[n] = y[0];
  for (i = n-1; i >= i0; i--) {
    r = pot->rad[i];
    y[2] = r0;
    y[3] = _veff[i+1]*r0;
    y[4] = (_veff[i]*r - y[3])/(r - r0);
    y[5] = e;
    lsode_(DerivODE, &neq, y, &r0, &r, &itol, &rtol, &atol, 
	   &itask, &istate, &iopt, rwork, &lrw, iwork, &liw, NULL, &mf);
    r0 = r;
    p[i] = y[0];
  }
  
  return y[1];
}    

int Phase(double *p, POTENTIAL *pot, int i1, double phase0) {
  int i;
  double fact;
  
  fact = 1.0 / 3.0;

  for (i = i1; i < MAX_POINTS; i++) {
    _dwork[i] = 1.0/p[i];
    _dwork[i] *= _dwork[i];
    _dwork[i] *= pot->dr_drho[i];
  }

  i = i1+1;
  p[i] = phase0;
  for (i = i1+3; i < MAX_POINTS; i += 2) {
    p[i] = p[i-2] + (_dwork[i-3] + 4.0*_dwork[i-2] + _dwork[i-1])*fact;
  }
  return 0;
}

int SetVEffective(int kl, POTENTIAL *pot) {
  double kl1;
  int i;
  double r;

  kl1 = 0.5*kl*(kl+1);
 
  for (i = 0; i < MAX_POINTS; i++) {
    r = pot->rad[i];
    r *= r;
    _veff[i] = pot->Vc[i] + pot->U[i] + kl1/r;
    _veff[i] += pot->W[i];
  }

  return 0;
}

static int TurningPoints(int n, double e, POTENTIAL *pot) {
  int i, i2;
  double x, a, b;

  if (n <= 0) {
    for (i = 10; i < MAX_POINTS-5; i++) {
      x = e - _veff[i];
      if (x <= 0) continue;
      b = 1.0/pot->rad[i];
      a = 20.0/(0.5*pot->ar*sqrt(b) + pot->br*b);
      x = TWO_PI/sqrt(2.0*x);
      if (x < a) break;
    }
    i2 = i-2;
    if (IsOdd(i2)) (i2)--;
  } else {
    for (i = MAX_POINTS-1; i > 10; i--) {
      if (e > _veff[i]) break;
    }
    if (i <= 10) return -2;
    i2 = i - 4;
  } 
  return i2;
}

static int IntegrateRadial(double *p, double e, POTENTIAL *pot,
			   int i1, double p1, int i2, double p2){
  double a, b, r, x, y, z, p0;
  int kl=1, ku=1, nrhs=1;
  int i, info, n, m, j, k;
  int ipiv[MAX_POINTS];
  
  m = i2 - i1 - 1;
  if (m < 0) return 0;
  if (m == 0) {
    p[i1] = p1;
    p[i2] = p2;
    return 0;
  }

  j = 1;
  n = 2*kl + ku + 1;
  k = kl + ku;
  for (i = i1+1; i < i2; i++, j++, k += n) {
    r = pot->rad[i];
    x = 2.0*(_veff[i] - e);
    x *= 4.0*r*r;
    a = pot->ar;
    b = pot->br;
    z = sqrt(r);
    y = a*z + 2.0*b;
    y = 1/(y*y);
    z = (0.75*a*a*r + 5.0*a*b*z +4.0*b*b);
    x += z*y;    
    x *= y / 12.0;
    a = 1.0 - x;
    b = -2.0*(1.0 + 5.0*x);
    ABAND[k-1] = a;
    ABAND[k] = b;
    ABAND[k+1] = a;
    p[i] = 0.0;
  }
    
  i = i1;
  r = pot->rad[i];
  x = 2.0*(_veff[i] - e);
  x *= 4.0*r*r;
  a = pot->ar;
  b = pot->br;
  z = sqrt(r);
  y = a*z + 2.0*b;
  y = 1/(y*y);
  z = (0.75*a*a*r + 5.0*a*b*z +4.0*b*b);
  x += z*y;    
  x *= y / 12.0;
  a = 1.0 - x;
  p[i1] = p1;
  p[i1+1] = -a*p1;

  i = i2;
  r = pot->rad[i];
  x = 2.0*(_veff[i] - e);
  x *= 4.0*r*r;
  a = pot->ar;
  b = pot->br;
  z = sqrt(r);
  y = a*z + 2.0*b;
  y = 1/(y*y);
  z = (0.75*a*a*r + 5.0*a*b*z +4.0*b*b);
  x += z*y;    
  x *= y / 12.0;
  a = 1.0 - x;
  p[i2] = p2;
  p[i2-1] = -a*p2;
  
  dgbsv_(&m, &kl, &ku, &nrhs, ABAND, &n, ipiv, p+i1+1, &m, &info);
  if (info) {
    printf("Error in Integrating the radial equation: %d\n", info);
    exit(1);
  }
  
  n = 0;
  for (i = i2; i >= i1; i--) {
    if (e >= _veff[i]) break;
  }
  if (i == i1) return n;

  p0 = p[i];
  for (; i >= i1; i--) {
    if (e < _veff[i]) break;
    a = fabs(p[i]);
    if (a > wave_zero) {
      if ((p0 > 0 && p[i] < 0) ||
	  (p0 < 0 && p[i] > 0)) {
	n++;
	p0 = p[i];
      }
    }
  }
    
  return n;
}

double InnerProduct(int i1, int n, double *p1, double *p2, POTENTIAL *pot) {
  int i, k;

  for (k = i1, i = 0; i < n; i++, k++) {
    _dwork[i] = p1[k]*p2[k] * pot->dr_drho[k];
  }
  return Simpson(_dwork, 0, n-1);
}

double Simpson(double *y, int ia, int ib) {
  int i;
  double a;

  a = 0.0;
  
  for (i = ia; i < ib - 1; i += 2) {
    a += (y[i] + 4.0*y[i+1] + y[i+2])/3.0;
  }
  if (i < ib) a += 0.5 * (y[i] + y[ib]);

  return a;
}

/* integration by newton-cotes formula */
int NewtonCotes(double *r, double *x, int i0, int i1, int m) {
  int i, j, n, k;
  double a;

  for (i = i0; i <= i1-4; i += 4) {
    a = 0.0;
    for (j = 0, k = i; j <= 4; j++, k++) {
      a += _CNC[4][j] * x[k];
    }
    r[i+4] = r[i] + a;
  }
  
  if (i1 < MAX_POINTS-1) {
    if (i > i0) {
      k = i - 3;
      n = i1 - i + 5;
    } else {
      k = i + 1;
      n = i1 - i + 1;
    }
    a = 0.0;
    for (j = 1; j < n; j++, k++) {
      a += _ONC[n][j] * x[k];
    }
    r[i1+1] = r[i1+1-n] + a;
  }

  if (m >= 0) {
    n = i1 - i - 1;
    if (n > 0) {
      a = 0.0;
      for (j = 0, k = i; j <= n; j++, k++) {
	a += _CNC[n][j] * x[k];
      }
      r[i1-1] = r[i] + a;
    }
    n++;
    a = 0.0;
    for (j = 0, k = i; j <= n; j++, k++) {
      a += _CNC[n][j] * x[k];
    }
    r[i1] = r[i] + a;
  } else {
    for (i = i0; i <= i1; i += 4) {
      for (n = 1; n <= Min(3, i1-i); n++) {
	a = 0.0;
	for (j = 0, k = i; j <= n; j++, k++) {
	  a += _CNC[n][j] * x[k];
	}
	r[i+n] = r[i] + a;
      }
    }
  }

  return 0;
}

int SetOrbitalRGrid(POTENTIAL *pot, double rmin, double rmax) {
  int i;  
  double z0, z, d1, d2, del;
  double a, b;

  z0 = GetAtomicNumber();
  z = z0;
  if (pot->N > 0) z = (z - pot->N + 1);
  if (pot->flag == 0) pot->flag = -1; 

  if (rmin <= 0.0) rmin = 1E-5;
  if (rmax <= 0.0) rmax = 5E+2;
  nmax = sqrt(rmax)/2.0;
  if (nmax < 10) {
    printf("rmax not large enough\n");
    exit(1);
  }

  rmin /= z0;
  rmax /= z;
  
  d1 = log(rmax/rmin);
  d2 = sqrt(rmax) - sqrt(rmin);

  a = 16.0*sqrt(2.0*z)/PI;
  b = (MAX_POINTS - 1.0 - (a*d2))/d1;
  if (b < 1.0/log(1.1)) {
    printf("Not enough radial mesh points, ");
    printf("enlarge to at least %d\n", (int) (1 + a*d2 + d1/log(1.1)));
    exit(1);
  }

  d1 = b*d1;
  d2 = a*d2;
  del = (d1 + d2)/(MAX_POINTS - 1);
  pot->rad[0] = rmin;
  d1 = a*sqrt(rmin) + b*log(rmin);
  for (i = 1; i < MAX_POINTS; i++) {
    d1 += del;
    pot->rad[i] = GetRFromRho(d1, a, b, pot->rad[i-1]);
  }

  pot->ar = a;
  pot->br = b;

  for (i = 0; i < MAX_POINTS; i++) {
    d1 = a * sqrt(pot->rad[i]);
    d2 = 2.0*pot->rad[i];
    pot->dr_drho[i] = d2/(d1 + 2.0*b);
    pot->dr_drho2[i] = sqrt(pot->dr_drho[i]);
  }

  return 0;
}

double GetRFromRho(double rho, double a, double b, double r0) {
  double e, d1;
  int i;

  e = 1.0;
  i = 0;
  while (fabs(e) > 1E-10) {
    if (i > 100) {
      printf("Newton iteration failed to converge in GetRFromRho\n");
      exit(1);
    }
    d1 = sqrt(r0)*a;
    e = d1 + b*log(r0) - rho;
    e /= (0.5*d1 + b);
    r0 *= (1.0 - e);
    i++;
  }

  return r0;
}

int SetPotentialZ(POTENTIAL *pot, double c) {
  int i;

  c = 1.0+c;
  for (i = 0; i < MAX_POINTS; i++) {
    pot->Z[i] = c*GetAtomicEffectiveZ(pot->rad[i]);
  }
  return 0;
}

int SetPotentialVc(POTENTIAL *pot) {
  int i;
  double n, r, r2, v, x, y, a, b, v0;

  for (i = 0; i < MAX_POINTS; i++) {
    r = pot->rad[i];
    pot->Vc[i] = - (pot->Z[i] / r);
    r2 = r*r;
    pot->dVc[i] = pot->Z[i] / r2; 
    pot->dVc2[i] = -2.0 * pot->Z[i] / (r2*r);
  }

  n = pot->N - 1;
  if (n > 0 && (pot->a > 0 || pot->lambda > 0)) {
    for (i = 0; i < MAX_POINTS; i++) {
      r = pot->rad[i];
      r2 = r*r;
      v0 = n/r;
      x = 1.0 + r*pot->a;
      a = exp(-pot->lambda * r);
      v = v0 * (1.0 - a/x);
      pot->Vc[i] += v;      
      b = (pot->lambda + pot->a/x);
      y = -v/r + (v0 - v)*b;
      pot->dVc[i] += y;
      pot->dVc2[i] -= y/r;
      pot->dVc2[i] += v/r2;
      pot->dVc2[i] -= (v0/r + y) * b;
      pot->dVc2[i] -= (v0 - v)*(pot->a*pot->a)/(x*x);
    }
  }
  return 0;
}

int SetPotentialU(POTENTIAL *pot, int n, double *u) {
  int i;
  
  if (n < 0) {
    for (i = 0; i < MAX_POINTS; i++) { 
      pot->U[i] = 0.0;
      pot->dU[i] = 0.0;
      pot->dU2[i] = 0.0;
    }
    return 0;
  }

  for (i = 0; i < n; i++) {
    pot->U[i] = u[i];    
  }

  for (i = 1; i < MAX_POINTS-1; i++) {
    pot->dU[i] = pot->U[i+1] - pot->U[i-1];
    pot->dU[i] *= 0.5;
    pot->dU[i] /= pot->dr_drho[i];
  }
  pot->dU[0] = pot->dU[1];
  pot->dU[MAX_POINTS-1] = pot->dU[MAX_POINTS-2];
  
  for (i = 1; i < MAX_POINTS-1; i++) {
    pot->dU2[i] = pot->dU[i+1] - pot->dU[i-1];
    pot->dU2[i] *= 0.5;
    pot->dU2[i] /= pot->dr_drho[i];
  }
  pot->dU2[0] = pot->dU2[1];
  pot->dU2[MAX_POINTS-1] = pot->dU2[MAX_POINTS-2];
  
  return 0;
}

int SetPotentialW (POTENTIAL *pot, double e, int kappa) {
  int i;
  double xi, r, x, y, z;

  for (i = 0; i < MAX_POINTS; i++) {
    xi = e - pot->Vc[i] - pot->U[i];
    r = xi*FINE_STRUCTURE_CONST2*0.5 + 1.0;
  
    x = pot->dU[i] + pot->dVc[i];
    y = - 2.0*kappa*x/pot->rad[i];
    x = x*x*0.75*FINE_STRUCTURE_CONST2/r;
    z = (pot->dU2[i] + pot->dVc2[i]);
    pot->W[i] = x + y + z;
    pot->W[i] /= 4.0*r;
    x = xi*xi;
    pot->W[i] = x - pot->W[i];
    pot->W[i] *= 0.5*FINE_STRUCTURE_CONST2;
    pot->W[i] = -pot->W[i];
  }

  for (i = 1; i < MAX_POINTS-1; i++) {
    pot->dW[i] = pot->W[i+1] - pot->W[i-1];   
    pot->dW[i] *= 0.5;
    pot->dW[i] /= pot->dr_drho[i];
  }
  pot->dW[0] = pot->dW[1];
  pot->dW[MAX_POINTS-1] = pot->dW[MAX_POINTS-2];
  
  for (i = 1; i < MAX_POINTS-1; i++) {
    pot->dW2[i] = pot->dW[i+1] - pot->dW[i-1];
    pot->dW2[i] *= 0.5;
    pot->dW2[i] /= pot->dr_drho[i];
  }
  pot->dW2[0] = pot->dW2[1];
  pot->dW2[MAX_POINTS-1] = pot->dW2[MAX_POINTS-2];
  
  return 0;
}