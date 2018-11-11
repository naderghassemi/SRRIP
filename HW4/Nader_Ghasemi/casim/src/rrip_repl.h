#ifndef RRIP_REPL_H_
#define RRIP_REPL_H_

#include "repl_policies.h"

#define CACHE_HIT               0
#define TRUE                    1
#define SEARCHING_FOR_VICTIM    1
#define FALSE                   0
#define CLEAR_CACHE_ENTRY       0   // set entry instruction Address to 0 to indicate an invalid PC entry                  
#define CLEAR_RPV               -1  // set Re-reference prediction value to invalid entry




/* From repl_policies.h:
 *
 * Generic replacement policy interface. A replacement policy is initialized by the cache 
 * (by calling setTop/BottomCC) and used by the cache array. Usage follows two models:
 * --> On lookups, update() is called if the replacement policy is to be updated on a hit
 * --> On each replacement, rank() is called with the req and a list of replacement candidates.
 * --> When the replacement is done, replaced() is called. (See below for more detail.)
 */

/* Add DECL_RANK_BINDINGS to each class that implements the new interface,
 * then implement a single, templated rank() function (see below for examples)
 * This way, we achieve a simple, single interface that is specialized transparently
 * to each type of array (this code is performance-critical)
 */


// Static RRIP
class SRRIPReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        int32_t* value_array;
        uint32_t* instruction_array; // Array for the lineID i.e. the instruction in the cache
        uint32_t MAX_VALUE;          // The value at of an cache entry for which it will be replace by on a cache miss
               // Holds the immediacy value (i.e. timestamp) corresponding to the instruction_array; range = 0 to 3
        uint32_t numLines;           // number of entries in the cache
        uint32_t is_new_entry;

         public:
        // add member methods here, refer to repl_policies.h
		// 10/23 DA: Need to implement rank(), update(), and replaced()
    
        // Constructor
		explicit SRRIPReplPolicy(uint32_t _numLines, uint32_t _MAX_VALUE) : numLines(_numLines), MAX_VALUE(_MAX_VALUE) 
                 {
         instruction_array = gm_calloc<uint32_t>(numLines);
                 value_array = gm_calloc<uint32_t>(numLines);
			for(uint32_t i = 0; i <= numLines; i++)
			{
		value_array[i] = MAX_VALUE;
			 }

                 }
		          ~SRRIPReplPolicy() {
                      gm_free(instruction_array);
			     gm_free(value_array);
     }
// input: instructionID = the index in the array of the hit/miss we want to update
		      void update(uint32_t instructionID, const MemReq* req)
		  {
                 // Reset is_new_entry to be false
             if(is_new_entry) 
                 {
                       assert(value_array[instructionID] == (uint32_t)CLEAR_RPV);
                  value_array[instructionID] = MAX_VALUE - 1;
              }
              else // update() called after a HIT!!!
         {
                 value_array[instructionID] = CACHE_HIT;
        }

                    is_new_entry = FALSE;
            
                    assert(value_array[instructionID] >= 0);
                 assert(value_array[instructionID] <= MAX_VALUE);
		              }


                      void replaced(uint32_t instructionID)
              {           
                 assert(instructionID >= 0);
                assert(instructionID <= numLines); 
            
                      is_new_entry = TRUE; // used to set value in update()
           instruction_array[instructionID] = CLEAR_CACHE_ENTRY; // set to invalid until it gets set in the calling function postinsert()  
            value_array[instructionID] = (uint32_t)CLEAR_RPV;            
         }
          template <typename C> inline uint32_t rank(const MemReq* req, C cands) 
        {
                 uint32_t instructionID;
            
            // Will remain in here until it finds the earliest entry with MAX_VALUE
            // If no entry has MAX_VALUE it will increment the all values in the array
            // until eventually there is a match
                     while (SEARCHING_FOR_VICTIM)
                 {
                for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) 
                     {
                     instructionID = *ci;
                if (value_array[instructionID] == MAX_VALUE) // 3
                             {
                                  assert(value_array[instructionID] >= 0);
                                assert(value_array[instructionID] <= MAX_VALUE);
                 return instructionID; // FOUND VICTIM, return its ID
                            }
                                  }

                // No match found! Increment each RRPV value in cache and search again
                     for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) 
                     {
                     instructionID = *ci;
                        value_array[instructionID]++;
                     }                          
             }
                }
    
        DECL_RANK_BINDINGS; 
#endif // RRIP_REPL_H_
