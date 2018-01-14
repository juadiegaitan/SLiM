//
//  eidos_rng.h
//  Eidos
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014-2017 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/slim/
//

//	This file is part of Eidos.
//
//	Eidos is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	Eidos is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with Eidos.  If not, see <http://www.gnu.org/licenses/>.

/*
 
 Eidos uses a globally shared random number generator called gEidos_rng.  This file defines that global and relevant helper functions.
 
 */

#ifndef __Eidos__eidos_rng__
#define __Eidos__eidos_rng__


// We have our own private copy of (parts of) the GSL library, so that we don't have link dependencies.
// See the _README file in the gsl directory for information on the local copy of the GSL included in this project.
#include "gsl_rng.h"
#include "gsl_randist.h"

#include <stdint.h>
#include <cmath>
#include "eidos_global.h"


// This code is copied and modified from taus.c in the GSL library because we want to be able to inline taus_get().
// Random number generation can be a major bottleneck in many SLiM models, so I think this is worth the grossness.
typedef struct
{
	unsigned long int s1, s2, s3;
}
taus_state_t;

inline __attribute__((always_inline)) unsigned long
taus_get_inline (void *vstate)
{
	taus_state_t *state = (taus_state_t *) vstate;
	
#define TAUS_MASK 0xffffffffUL
#define TAUSWORTHE(s,a,b,c,d) (((s &c) <<d) &TAUS_MASK) ^ ((((s <<a) &TAUS_MASK)^s) >>b)
	
	state->s1 = TAUSWORTHE (state->s1, 13, 19, 4294967294UL, 12);
	state->s2 = TAUSWORTHE (state->s2, 2, 25, 4294967288UL, 4);
	state->s3 = TAUSWORTHE (state->s3, 3, 11, 4294967280UL, 17);
	
	return (state->s1 ^ state->s2 ^ state->s3);
}

#undef TAUS_MASK
#undef TAUSWORTHE


// This is a globally shared random number generator.  Note that the globals for random bit generation below are also
// considered to be part of the RNG state; if the Context plays games with swapping different RNGs in and out, those
// globals need to get swapped as well.  Likewise for the last seed value; this is part of the RNG state in Eidos.
extern gsl_rng *gEidos_rng;
extern int gEidos_random_bool_bit_counter;
extern uint32_t gEidos_random_bool_bit_buffer;
extern unsigned long int gEidos_rng_last_seed;				// unsigned long int is the type used by gsl_rng_set()


// generate a new random number seed from the PID and clock time
unsigned long int Eidos_GenerateSeedFromPIDAndTime(void);

// set up the random number generator with a given seed
void Eidos_InitializeRNGFromSeed(unsigned long int p_seed);


// get a random bool from a random number generator
//static inline bool Eidos_RandomBool(gsl_rng *p_r) { return (bool)(taus_get(p_r->state) & 0x01); }

// optimization of this is possible assuming each bit returned by taus_get() is independent and usable as a random boolean.
// I can't find a hard guarantee of this for gsl_rng_taus2, but it is generally true of good modern RNGs...
static inline __attribute__((always_inline)) bool Eidos_RandomBool(gsl_rng *p_r)
{
	bool retval;
	
	if (gEidos_random_bool_bit_counter > 0)
	{
		gEidos_random_bool_bit_counter--;
		gEidos_random_bool_bit_buffer >>= 1;
		retval = gEidos_random_bool_bit_buffer & 0x01;
	}
	else
	{
		gEidos_random_bool_bit_buffer = (uint32_t)taus_get_inline(p_r->state);	// gsl_rng_taus2 is in fact limited to unsigned 32-bit, according to its docs
		retval = gEidos_random_bool_bit_buffer & 0x01;
		gEidos_random_bool_bit_counter = 31;				// 32 good bits originally, and now we've used one
	}
	
	return retval;
}

// The gsl_rng_uniform() function is a bit slow because of the indirection it goes through to get the
// function pointer, so this is a customized version that should be faster.  Basically it just hard-codes
// taus_get(); otherwise its logic is the same.  The taus_get_double() function called by gsl_rng_uniform()
// has the advantage of inlining the taus_get() function, but on the other hand, Eidos_rng_uniform() is
// itself inline, which gsl_rng_uniform()'s call to taus_get_double() cannot be, so that should be a wash.
inline __attribute__((always_inline)) double Eidos_rng_uniform(gsl_rng *p_r)
{
	return taus_get_inline(p_r->state) / 4294967296.0;
}

// Basically ditto; faster than gsl_rng_uniform_pos() by avoiding indirection.
inline __attribute__((always_inline)) double Eidos_rng_uniform_pos(const gsl_rng *p_r)
{
	double x;
	
	do
	{
		x = taus_get_inline(p_r->state) / 4294967296.0;
	}
	while (x == 0);
	
	return x;
}

// The gsl_rng_uniform_int() function is very slow, so this is a customized version that should be faster.
// Basically it is faster because (1) the range of the taus2 generator is hard-coded, (2) the range check
// is done only on #if DEBUG, (3) it uses uint32_t, and (4) it calls taus_get() directly; otherwise the
// logic is the same.
inline __attribute__((always_inline)) uint32_t Eidos_rng_uniform_int(gsl_rng *p_r, uint32_t p_n)
{
	uint32_t scale = UINT32_MAX / p_n;
	uint32_t k;
	
#if DEBUG
	if ((p_n > INT32_MAX) || (p_n <= 0)) 
	{
		GSL_ERROR_VAL ("invalid n, either 0 or exceeds maximum value of generator", GSL_EINVAL, 0) ;
	}
#endif
	
	do
	{
		k = ((uint32_t)(taus_get_inline(p_r->state))) / scale;		// taus_get is used by the taus2 RNG
	}
	while (k >= p_n);
	
	return k;
}



// Fast Poisson drawing, usable when mu is small; algorithm from Wikipedia, referenced to Luc Devroye,
// Non-Uniform Random Variate Generation (Springer-Verlag, New York, 1986), chapter 10, page 505.
// The GSL Poisson code does not allow us to precalculate the exp() value, it is more than three times
// slower for some mu values, it doesn't let us force a non-zero draw, and it is not inlined (without
// modifying the GSL code), so there are good reasons for us to roll our own.  However, our version is
// safe only for mu < ~720, and the GSL's version is faster for mu > 250 anyway, so we cross over
// to using the GSL for mu > 250.  This is done on a per-draw basis.

// If someone does not trust our Poisson code, they can define USE_GSL_POISSON at compile time
// (i.e., -D USE_GSL_POISSON) to use the gsl_ran_poisson() instead.  It does make a substantial
// speed difference, though, so that option is turned off by default.  Note that at present the
// rpois() Eidos function always uses the GSL in any case.

//#define USE_GSL_POISSON

#ifndef USE_GSL_POISSON

static inline __attribute__((always_inline)) unsigned int Eidos_FastRandomPoisson(double p_mu)
{
	// Defer to the GSL for large values of mu; see comments above.
	if (p_mu > 250)
		return gsl_ran_poisson(gEidos_rng, p_mu);
	
	unsigned int x = 0;
	double p = exp(-p_mu);
	double s = p;
	double u = Eidos_rng_uniform(gEidos_rng);
	
	while (u > s)
	{
		++x;
		p *= (p_mu / x);
		s += p;
		
		// If p_mu is too large (> ~720), this loop can hang as p underflows to zero.
	}
	
	return x;
}

// This version allows the caller to supply a precalculated exp(-mu) value
static inline __attribute__((always_inline)) unsigned int Eidos_FastRandomPoisson(double p_mu, double p_exp_neg_mu)
{
	// Defer to the GSL for large values of mu; see comments above.
	if (p_mu > 250)
		return gsl_ran_poisson(gEidos_rng, p_mu);
	
	// Test consistency; normally this is commented out
	//if (p_exp_neg_mu != exp(-p_mu))
	//	EIDOS_TERMINATION << "ERROR (Eidos_FastRandomPoisson): p_exp_neg_mu incorrect." << EidosTerminate(nullptr);
	
	unsigned int x = 0;
	double p = p_exp_neg_mu;
	double s = p;
	double u = Eidos_rng_uniform(gEidos_rng);
	
	while (u > s)
	{
		++x;
		p *= (p_mu / x);
		s += p;
		
		// If p_mu is too large (> ~720), this loop can hang as p underflows to zero.
	}
	
	return x;
}

// This version specifies that the count is guaranteed not to be zero; zero has been ruled out by a previous test
static inline __attribute__((always_inline)) unsigned int Eidos_FastRandomPoisson_NONZERO(double p_mu, double p_exp_neg_mu)
{
	// Defer to the GSL for large values of mu; see comments above.
	if (p_mu > 250)
	{
		unsigned int result;
		
		do
		{
			result = gsl_ran_poisson(gEidos_rng, p_mu);
		}
		while (result == 0);
		
		return result;
	}
	
	// Test consistency; normally this is commented out
	//if (p_exp_neg_mu != exp(-p_mu))
	//	EIDOS_TERMINATION << "ERROR (Eidos_FastRandomPoisson_NONZERO): p_exp_neg_mu incorrect." << EidosTerminate(nullptr);
	
	unsigned int x = 0;
	double p = p_exp_neg_mu;
	double s = p;
	double u = Eidos_rng_uniform_pos(gEidos_rng);	// exclude 0.0 so u != s after rescaling
	
	// rescale u so that (u > s) is true in the first round
	u = u * (1.0 - s) + s;
	
	// do the first round, since we now know u > s
	++x;
	p *= p_mu;	// divided by x, but x is now 1
	s += p;
	
	while (u > s)
	{
		++x;
		p *= (p_mu / x);
		s += p;
		
		// If p_mu is too large (> ~720), this loop can hang as p underflows to zero.
	}
	
	return x;
}

double Eidos_FastRandomPoisson_PRECALCULATE(double p_mu);	// exp(-mu); can underflow to zero, in which case the GSL will be used


#endif // USE_GSL_POISSON


#endif /* defined(__Eidos__eidos_rng__) */




































































