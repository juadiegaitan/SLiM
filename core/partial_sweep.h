//
//  partial_sweep.h
//  SLiM
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/software/
//

//	This file is part of SLiM.
//
//	SLiM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//
//	SLiM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with SLiM.  If not, see <http://www.gnu.org/licenses/>.

/*
 
 The class PartialSweep represents a request, in the input file, that a particular mutation should sweep only partway to fixation.
 Once the mutation has swept to the requested prevalence, it becomes neutral – its selection coefficient is set to 0.
 
 */

#ifndef __SLiM__partial_sweep__
#define __SLiM__partial_sweep__


#include <iostream>


class PartialSweep
{
public:
	
	int mutation_type_;
	int position_;
	double target_prevalence_;
	
	PartialSweep(int p_mutation_type, int p_position, double p_target_prevalence);
};

// support stream output of PartialSweep, for debugging
std::ostream &operator<<(std::ostream &p_outstream, const PartialSweep &p_partial_sweep);


#endif /* defined(__SLiM__partial_sweep__) */



































































