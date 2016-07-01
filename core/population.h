//
//  population.h
//  SLiM
//
//  Created by Ben Haller on 12/13/14.
//  Copyright (c) 2014-2016 Philipp Messer.  All rights reserved.
//	A product of the Messer Lab, http://messerlab.org/slim/
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
 
 The class Population represents the entire simulated population as a map of one or more subpopulations.  This class is where much
 of the simulation logic resides; the population is called to put events into effect, to evolve, and so forth.
 
 */

#ifndef __SLiM__population__
#define __SLiM__population__


#include <vector>
#include <map>
#include <string>

#include "slim_global.h"
#include "subpopulation.h"
#include "substitution.h"
#include "chromosome.h"
#include "slim_global.h"
#include "slim_eidos_block.h"


class SLiMSim;


#ifdef SLIMGUI
// This struct is used to hold fitness values observed during a run, for display by GraphView_FitnessOverTime
// The Population keeps the fitness histories for all the subpopulations, because subpops can come and go, but
// we want to remember their histories and display them even after they're gone.
typedef struct {
	double *history_ = nullptr;						// mean fitness, recorded per generation; generation 1 goes at index 0
	slim_generation_t history_length_ = 0;			// the number of entries in the fitness_history buffer
} FitnessHistory;
#endif


class Population : private std::map<slim_objectid_t,Subpopulation*>		// OWNED POINTERS
{
	//	This class has its copy constructor and assignment operator disabled, to prevent accidental copying.

public:
	
	// We use private inheritance from std::map<slim_objectid_t,Subpopulation*> to avoid issues with Population being treated polymorphically
	// as a std::map, and forward only the minimal set of std::map methods that users of Population actually want.
	// See http://stackoverflow.com/questions/4353203/thou-shalt-not-inherit-from-stdvector for discussion.
	using std::map<slim_objectid_t,Subpopulation*>::find;
	using std::map<slim_objectid_t,Subpopulation*>::begin;
	using std::map<slim_objectid_t,Subpopulation*>::end;
	using std::map<slim_objectid_t,Subpopulation*>::size;
	
	SLiMSim &sim_;											// We have a reference back to our simulation
	Genome mutation_registry_;								// OWNED POINTERS: a registry of all mutations that have been added to this population
	slim_refcount_t total_genome_count_ = 0;				// the number of modeled genomes in the population; a fixed mutation has this frequency
#ifdef SLIMGUI
	slim_refcount_t gui_total_genome_count_ = 0;			// the number of modeled genomes in the selected subpopulations in SLiMgui
#endif
	
	// Cache info for TallyMutationReferences(); see that function
	std::vector<Subpopulation*> last_tallied_subpops_;		// NOT OWNED POINTERS
	slim_refcount_t cached_genome_count_ = 0;
	
	std::vector<Substitution*> substitutions_;				// OWNED POINTERS: Substitution objects for all fixed mutations
	bool child_generation_valid_ = false;					// this keeps track of whether children have been generated by EvolveSubpopulation() yet, or whether the parents are still in charge
	
	std::vector<Subpopulation*> removed_subpops_;			// OWNED POINTERS: Subpops which are set to size 0 (and thus removed) are kept here until the end of the generation
	
#ifdef SLIMGUI
	// information-gathering for various graphs in SLiMgui
	slim_generation_t *mutation_loss_times_ = nullptr;		// histogram bins: {1 bin per mutation-type} for 10 generations, realloced outward to add new generation bins as needed
	uint32_t mutation_loss_gen_slots_ = 0;					// the number of generation-sized slots (with bins per mutation-type) presently allocated
	
	slim_generation_t *mutation_fixation_times_ = nullptr;	// histogram bins: {1 bin per mutation-type} for 10 generations, realloced outward to add new generation bins as needed
	uint32_t mutation_fixation_gen_slots_ = 0;				// the number of generation-sized slots (with bins per mutation-type) presently allocated
	
	std::map<slim_objectid_t,FitnessHistory> fitness_histories_;	// fitness histories indexed by subpopulation id (or by -1, for the Population history)
	
	// true if gui_selected_ is set for all subpops, otherwise false; must be kept in synch with subpop flags!
	bool gui_all_selected_ = true;
#endif
	
	Population(const Population&) = delete;					// no copying
	Population& operator=(const Population&) = delete;		// no copying
	Population(void) = delete;								// no default constructor
	Population(SLiMSim &p_sim);								// our constructor: we must have a reference to our simulation
	~Population(void);										// destructor
	
	// add new empty subpopulation p_subpop_id of size p_subpop_size
	Subpopulation *AddSubpopulation(slim_objectid_t p_subpop_id, slim_popsize_t p_subpop_size, double p_initial_sex_ratio);
	
	// add new subpopulation p_subpop_id of size p_subpop_size individuals drawn from source subpopulation p_source_subpop_id
	Subpopulation *AddSubpopulation(slim_objectid_t p_subpop_id, Subpopulation &p_source_subpop, slim_popsize_t p_subpop_size, double p_initial_sex_ratio);
	
	// set size of subpopulation p_subpop_id to p_subpop_size
	void SetSize(Subpopulation &p_subpop, slim_popsize_t p_subpop_size);
	
	// set fraction p_migrant_fraction of p_subpop_id that originates as migrants from p_source_subpop_id per generation  
	void SetMigration(Subpopulation &p_subpop, slim_objectid_t p_source_subpop_id, double p_migrant_fraction);
	
	// execute a script event in the population; the script is assumed to be due to trigger
	void ExecuteScript(SLiMEidosBlock *p_script_block, slim_generation_t p_generation, const Chromosome &p_chromosome);
	
	// apply mateChoice() callbacks to a mating event with a chosen first parent; the return is the second parent index, or -1 to force a redraw
	slim_popsize_t ApplyMateChoiceCallbacks(slim_popsize_t p_parent1_index, Subpopulation *p_subpop, Subpopulation *p_source_subpop, std::vector<SLiMEidosBlock*> &p_mate_choice_callbacks);
	
	// apply modifyChild() callbacks to a generated child; a return of false means "do not use this child, generate a new one"
	bool ApplyModifyChildCallbacks(slim_popsize_t p_child_index, IndividualSex p_child_sex, slim_popsize_t p_parent1_index, slim_popsize_t p_parent2_index, bool p_is_selfing, bool p_is_cloning, Subpopulation *p_subpop, Subpopulation *p_source_subpop, std::vector<SLiMEidosBlock*> &p_modify_child_callbacks);
	
	// generate children for subpopulation p_subpop_id, drawing from all source populations, handling crossover and mutation
	void EvolveSubpopulation(Subpopulation &p_subpop, const Chromosome &p_chromosome, slim_generation_t p_generation, bool p_mate_choice_callbacks_present, bool p_modify_child_callbacks_present);
	
	// generate a child genome from parental genomes, with recombination, gene conversion, and mutation
	void DoCrossoverMutation(Subpopulation *p_subpop, Subpopulation *p_source_subpop, slim_popsize_t p_child_genome_index, slim_objectid_t p_source_subpop_id, slim_popsize_t p_parent1_genome_index, slim_popsize_t p_parent2_genome_index, const Chromosome &p_chromosome, slim_generation_t p_generation, IndividualSex p_child_sex, IndividualSex p_parent_sex);
	
	// generate a child genome from a single parental genome, without recombination or gene conversion, but with mutation
	void DoClonalMutation(Subpopulation *p_subpop, Subpopulation *p_source_subpop, slim_popsize_t p_child_genome_index, slim_objectid_t p_source_subpop_id, slim_popsize_t p_parent_genome_index, const Chromosome &p_chromosome, slim_generation_t p_generation, IndividualSex p_child_sex);
	
	// Recalculate all fitness values for the parental generation, including the use of fitness() callbacks
	void RecalculateFitness(slim_generation_t p_generation);
	
	// Tally mutations and remove fixed/lost mutations
	void MaintainRegistry(void);
	
	// step forward a generation: make the children become the parents
	void SwapGenerations(void);
	
	// count the total number of times that each Mutation in the registry is referenced by a population, and set total_genome_count_ to the maximum possible number of references (i.e. fixation)
	slim_refcount_t TallyMutationReferences(std::vector<Subpopulation*> *p_subpops_to_tally, bool p_force_recache);
	
	// handle negative fixation (remove from the registry) and positive fixation (convert to Substitution), using reference counts from TallyMutationReferences()
	void RemoveFixedMutations(void);
	
	// check the registry for any bad entries (i.e. zombies)
	void CheckMutationRegistry(void);
	
	// print all mutations and all genomes to a stream
	void PrintAll(std::ostream &p_out) const;
	void PrintAllBinary(std::ostream &p_out) const;
	
	// print sample of p_sample_size genomes from subpopulation p_subpop_id, using SLiM's own format
	void PrintSample_slim(std::ostream &p_out, Subpopulation &p_subpop, slim_popsize_t p_sample_size, bool p_replace, IndividualSex p_requested_sex) const;
	
	// print sample of p_sample_size genomes from subpopulation p_subpop_id, using "ms" format
	void PrintSample_ms(std::ostream &p_out, Subpopulation &p_subpop, slim_popsize_t p_sample_size, bool p_replace, IndividualSex p_requested_sex, const Chromosome &p_chromosome) const;
	
	// print sample of p_sample_size genomes from subpopulation p_subpop_id, using "vcf" format
	void PrintSample_vcf(std::ostream &p_out, Subpopulation &p_subpop, slim_popsize_t p_sample_size, bool p_replace, IndividualSex p_requested_sex, bool p_output_multiallelics) const;
	
	// remove subpopulations, purge all mutations and substitutions, etc.; called before InitializePopulationFrom[Text|Binary]File()
	void RemoveAllSubpopulationInfo(void);
	
	// additional methods for SLiMgui, for information-gathering support
#ifdef SLIMGUI
	void RecordFitness(slim_generation_t p_history_index, slim_objectid_t p_subpop_id, double fitness_value);
	void SurveyPopulation(void);
	void AddTallyForMutationTypeAndBinNumber(int p_mutation_type_index, int p_mutation_type_count, slim_generation_t p_bin_number, slim_generation_t **p_buffer, uint32_t *p_bufferBins);
#endif
};


#endif /* defined(__SLiM__population__) */




































































