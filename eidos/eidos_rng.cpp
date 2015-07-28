//
//  eidos_rng.cpp
//  Eidos
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
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


#include "eidos_rng.h"

#include <unistd.h>
#include <time.h>


gsl_rng *gEidos_rng;
int gEidos_random_bool_bit_counter = 0;
unsigned long int gEidos_random_bool_bit_buffer = 0;


int GenerateSeedFromPIDAndTime(void)
{
	long pid = getpid();
	time_t t;
	
	time(&t);
	t += pid;
	
	return static_cast<int>(t);
}

void InitializeRNGFromSeed(int p_seed)
{
	// Allocate the RNG if needed
	if (!gEidos_rng)
		gEidos_rng = gsl_rng_alloc(gsl_rng_taus2);
	
	// Then set the seed as requested
	gsl_rng_set(gEidos_rng, static_cast<long>(p_seed));
	
	// These need to be zeroed out, too; they are part of our RNG state
	gEidos_random_bool_bit_counter = 0;
	gEidos_random_bool_bit_buffer = 0;
}
































































