//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//TAGE predictor settings
int initial_history_bits; // Starting history length
int num_histories; // Number of BHTs

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

//Tournament predictor data structures
uint8_t *gpt;
uint8_t *cpt;
uint8_t *lpt;
uint8_t *lht;
uint64_t phistory;

//
uint8_t *bht_gshare;
uint64_t ghistory;

//Double pointer to store multiple branch history tables and their corresponding tags
//For TAGE custom predictor
uint8_t **bht_ctrs;
uint8_t **bht_tags;
uint8_t **bht_usebits;
int tage_index_bits;
uint64_t tage_table_size;
int prediction_count;
uint8_t useful_bit_clear;

//For bimodal predictor
uint8_t *bimodal_table;
int bimodal_index_bits;
int bimodal_table_size;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

//Gshare functions 




void init_tourn(){

	int g_entries = 1 << ghistoryBits;
	int lpt_entries = 1 << lhistoryBits;
	int lht_entries = 1 << pcIndexBits;
	gpt = (uint8_t*)malloc(g_entries * sizeof(uint8_t));
	cpt = (uint8_t*)malloc(g_entries * sizeof(uint8_t));
	lpt = (uint8_t*)malloc(lpt_entries * sizeof(uint8_t));
	lht = (uint8_t*)malloc(lht_entries * sizeof(uint8_t));
	phistory=0;

	int i = 0;
	for(i = 0; i< g_entries; i++){
		gpt[i] = WN;
		cpt[i] = WT;
	}
	for(i = 0; i< lpt_entries; i++){
		lpt[i] = WN;
	}
	for(i = 0; i< lht_entries; i++){
		lht[i] = 0;
	}
}


uint8_t findChoice(uint32_t pc){

	uint32_t andbits=1<<pcIndexBits;
	uint32_t pc_lower_bits=pc&(andbits-1);
	uint32_t l_val=lpt[lht[pc_lower_bits]];
	uint32_t g_val=gpt[phistory];

	uint32_t g_pred=NOTTAKEN;
	uint32_t l_pred=NOTTAKEN;
	//printf("%d %d\n",g_val,l_val);
	switch(l_val){
			case WN:
				l_pred= NOTTAKEN;
				break;
			case SN:
				 l_pred= NOTTAKEN;
				 break;
			case WT:
				l_pred= TAKEN;
				break;
			case ST:
				 l_pred=  TAKEN;
				 break;
			default:
				printf("Warning: Undefined state of entry in l_val!\n");
				
		}
		
	switch(g_val){
			case WN:
				g_pred= NOTTAKEN;
				break;
			case SN:
				 g_pred= NOTTAKEN;
				 break;
			case WT:
				g_pred= TAKEN;
				break;
			case ST:
				 g_pred=  TAKEN;
				 break;
			default:
				printf("Warning: Undefined state of entry in g_val\n");
				
		}

	uint32_t choice= (g_pred << 1) | l_pred;

	return choice;
}

uint8_t make_prediction_tourn(uint32_t pc){
	uint32_t choice= findChoice(pc);
	switch(choice){
			case WN:
				if(cpt[phistory]>1)
					return NOTTAKEN;
				else 
					return TAKEN;
			case SN:
				 return NOTTAKEN;
			case WT:
				if(cpt[phistory]>1)
					return TAKEN;
				else
					return NOTTAKEN;
			case ST:
				 return TAKEN;
			default:
				printf("Warning: Undefined state of entry in choice!\n");
				return NOTTAKEN;
		}
}

void train_tourn(uint32_t pc, uint8_t outcome){
	uint32_t choice= findChoice(pc);
	uint32_t andbits=1<<pcIndexBits;
	uint32_t pc_lower_bits=pc&(andbits-1);
	int g_entries = 1 << ghistoryBits;
	int lht_entries = 1 << pcIndexBits;

	if(outcome==TAKEN){
		if(lpt[lht[pc_lower_bits]]<3)
			lpt[lht[pc_lower_bits]]+=1;

		if(gpt[phistory]<3)
			gpt[phistory]+=1;
	}
    	
	if(outcome==NOTTAKEN){
		if(lpt[lht[pc_lower_bits]]>0)
			lpt[lht[pc_lower_bits]]-=1;

		if(gpt[phistory]>0)
			gpt[phistory]-=1;
	}
    	
	lht[pc_lower_bits]=(lht[pc_lower_bits]<<1) | outcome;
	lht[pc_lower_bits]= lht[pc_lower_bits] & (lht_entries -1);
    
	int pred_choice = cpt[phistory];	
	switch(choice){
    case WN:
    	if(outcome == 1){
    		cpt[phistory] = (pred_choice>0)?pred_choice-1:pred_choice;
    	}
    	else{
    	   cpt[phistory] = (pred_choice<3)?pred_choice+1:pred_choice;
    	}
      break;
    case SN:
      break;
    case WT:
    	if(outcome == 1){
    		cpt[phistory] = (pred_choice<3)?pred_choice+1:pred_choice;
    	}
    	else{
    	  cpt[phistory] = (pred_choice>0)?pred_choice-1:pred_choice;
    	}
      break;
    case ST:
      break;
    default:
      printf("Warning: Undefined state of entry in train!\n");
  }
	phistory = (phistory << 1) | outcome;
	phistory = phistory & (g_entries - 1);
}



void cleanup_tourn(){
  free(gpt);
  free(cpt);
  free(lpt);
  free(lht);
}




void init_gshare(){
  // Allocate an array to use as BHT
  // Size of BHT depends on number of bits used 
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t make_prediction_gshare(uint32_t pc){
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  if(verbose){
    printf("bht_entries: %d, pc_lower_bits: %d, ghistory: %lu, index: %d \n", bht_entries, pc_lower_bits, ghistory, index);
  }
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome){
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void cleanup_gshare(){
  free(bht_gshare);
}

// Bimodal branch prediction

void init_bimodal(){
  bimodal_table_size = 1<<bimodal_index_bits;
  bimodal_table = (uint8_t*)malloc(bimodal_table_size * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bimodal_table_size; i++ ){
    bimodal_table[i] = WN;
  }
}

uint8_t make_prediction_bimodal(uint32_t pc){
  switch(bimodal_table[pc%bimodal_table_size]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in bimodal table!\n");
      return NOTTAKEN;
  }
}

void train_bimodal(uint32_t pc, uint8_t outcome){
  switch(bimodal_table[pc%bimodal_table_size]){
    case WN:
      bimodal_table[pc%bimodal_table_size] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bimodal_table[pc%bimodal_table_size] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bimodal_table[pc%bimodal_table_size] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bimodal_table[pc%bimodal_table_size] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: In training, Undefined state of entry in Bimodal table!\n");
  }
}

void cleanup_bimodal(){
  free(bimodal_table);
}

// TAGE functions

void init_tage(){
  //TAGE uses the gshare with 2 bit history as a base predictor 
  bimodal_index_bits = 12;
  tage_index_bits = 11;
  prediction_count = 0;
  useful_bit_clear = 0xFE;
  tage_table_size = 1<<tage_index_bits;
  init_bimodal();
  //Improves upon base prediction with additional varying length tagged history tables 
  num_histories = 3;

  
  //Allocate additional BHTs and tag 
  bht_ctrs = (uint8_t**)malloc(num_histories*sizeof(uint8_t*));
  bht_usebits = (uint8_t**)malloc(num_histories*sizeof(uint8_t*));
  bht_tags = (uint8_t**)malloc(num_histories*sizeof(uint8_t*));
  int i =0;
  for(i = 0; i< num_histories; i++){
    bht_ctrs[i] = (uint8_t*)malloc(tage_table_size * sizeof(uint8_t));
    bht_tags[i] = (uint8_t*)malloc(tage_table_size * sizeof(uint8_t));
    bht_usebits[i] = (uint8_t*)malloc(tage_table_size * sizeof(uint8_t));
  }
}

uint8_t get_tag_tage(uint32_t pc){
    return ((pc & 0xF0) >> 4);
}

uint32_t get_index_tage(uint32_t pc, int history_bits){
  uint32_t pc_lower_bits = pc & (tage_table_size-1);
  uint32_t running_xor = tage_index_bits;
  uint32_t ghistory_lower_bits = ghistory & (tage_table_size -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  uint64_t temp64;
  uint32_t temp32;
  while(running_xor < history_bits){
    temp64 = ghistory & ((tage_table_size -1) << running_xor);
    temp64 = temp64 >> (running_xor - history_bits);
    temp32 = temp64;
    index = index ^ temp32;
    running_xor += tage_index_bits;
  }
  if(verbose){
    printf("pc_lower_bits: %d, ghistory %lu, index: %d, pc: %d, history_bits: %d\n", pc_lower_bits, ghistory, index, pc, history_bits);
  }
  return index;
}

uint8_t make_prediction_tage(uint32_t pc){
  prediction_count++;
  uint8_t prediction = make_prediction_bimodal(pc);
  int i = 0;
  for(i = 0; i< num_histories; i++){
    int history_bits = tage_index_bits + tage_index_bits*i; 
    uint32_t index = get_index_tage(pc, history_bits);
    uint8_t tag = get_tag_tage(pc);
    if(bht_tags[i][index] == tag){
      switch(bht_ctrs[i][index]){
        case WN:
          prediction = NOTTAKEN;
          break;
        case SN:
          prediction = NOTTAKEN;
          break;
        case WT:
          prediction = TAKEN;
          break;
        case ST:
          prediction = TAKEN;
          break;
        default:
          printf("Warning: In prediction, Undefined state of entry in TAGE BHT!\n");
          prediction = NOTTAKEN;
      }
    }
  }
  return prediction;
}

void train_tage(uint32_t pc, uint8_t outcome){
  //Train bimodal predictor
  uint8_t prediction = make_prediction_bimodal(pc);
  uint8_t altpred_prediction = prediction;
  train_bimodal(pc, outcome);
  int i = 0;
  int pred_bht = -1;
  int altpred_bht = -1;
  // Find pred and alt_pred bhts
  for(i = 0; i< num_histories; i++){
    int history_bits = tage_index_bits + tage_index_bits*i; 
    uint32_t index = get_index_tage(pc, history_bits);
    uint8_t tag = get_tag_tage(pc);
    if(bht_tags[i][index] == tag){
      altpred_bht = pred_bht; 
      altpred_prediction = prediction;
      pred_bht = i;
      switch(bht_ctrs[i][index]){
        case WN:
          prediction = NOTTAKEN;
          break;
        case SN:
          prediction = NOTTAKEN;
          break;
        case WT:
          prediction = TAKEN;
          break;
        case ST:
          prediction = TAKEN;
          break;
        default:
          printf("Warning: In prediction, Undefined state of entry in TAGE BHT!\n");
          prediction = NOTTAKEN;
      }
    }
  }
  if(pred_bht > -1){
    int pred_history_bits = tage_index_bits + tage_index_bits*pred_bht; 
    uint32_t pred_index = get_index_tage(pc, pred_history_bits);
    // If there's a prediction provider update it's counters. 
    switch(bht_ctrs[pred_bht][pred_index]){
      case WN:
        bht_ctrs[pred_bht][pred_index] = (outcome==TAKEN)?WT:SN;
        break;
      case SN:
        bht_ctrs[pred_bht][pred_index] = (outcome==TAKEN)?WN:SN;
        break;
      case WT:
        bht_ctrs[pred_bht][pred_index] = (outcome==TAKEN)?ST:WN;
        break;
      case ST:
        bht_ctrs[pred_bht][pred_index] = (outcome==TAKEN)?ST:WT;
        break;
      default:
        printf("Warning: In training, Undefined state of entry in Bimodal table!\n");
    }
    // if altpred differes from prediction, update the usefulness counter accordingly
    if(altpred_prediction != prediction){
      if(prediction == outcome)
        bht_usebits[pred_bht][pred_index] = fmin(3, bht_usebits[pred_bht][pred_index] + 1);
      else
        bht_usebits[pred_bht][pred_index] = fmax(0, bht_usebits[pred_bht][pred_index] - 1);
    }
  }
  // If the prediction provider isn't from the bht with longest history, (try to) allocate a new entry
  if(pred_bht < num_histories - 1){
    int allocated_entry = 0;
    for(i = pred_bht+1; i< num_histories; i++){
      int new_entry_history_bits = tage_index_bits + tage_index_bits*i; 
      uint32_t new_entry_index = get_index_tage(pc, new_entry_history_bits);
      uint8_t new_entry_tag = get_tag_tage(pc);
      if(bht_usebits[i][new_entry_index] == 0){
        bht_ctrs[i][new_entry_index] = (outcome == TAKEN)?WT:WN;
        bht_tags[i][new_entry_index] = new_entry_tag;
        allocated_entry = 1;
        break;
      }
    }
    if(!allocated_entry){
      for(i = pred_bht+1; i< num_histories; i++){
        int new_entry_history_bits = tage_index_bits + tage_index_bits*i; 
        uint32_t new_entry_index = get_index_tage(pc, new_entry_history_bits);
        uint8_t new_entry_tag = get_tag_tage(pc);
        bht_usebits[i][new_entry_index]--;
      }
    }
  }
  //Update history
  ghistory = ((ghistory << 1) | outcome); 
  if(prediction_count > 256000){
    for(i = 0; i< num_histories; i++){
      int j = 0;
      for(j = 0; j< tage_table_size; j++){
        bht_usebits[i][j] = bht_usebits[i][j] & useful_bit_clear;
      }
    }
    useful_bit_clear = (useful_bit_clear == 0xFD)?0xFE:0xFD;
    prediction_count = 0;
  }
}

void cleanup_tage(){
  cleanup_bimodal();
  int i =0;
  for(i = 0; i< num_histories; i++){
    free(bht_ctrs[i]);
    free(bht_tags[i]);
    free(bht_usebits[i]);
  }
  free(bht_ctrs);
  free(bht_tags);
  free(bht_usebits);
}


// Initialize the predictor
void init_predictor() {
  //TODO: Initialize Branch Predictor Data Structures
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
  
    	init_tourn();
      break;
    case CUSTOM:
      init_tage();
      break;
    default:
      break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
uint8_t make_prediction(uint32_t pc){
  //TODO: Implement prediction scheme
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return make_prediction_gshare(pc);
    case TOURNAMENT:
  return make_prediction_tourn(pc);
  //return NOTTAKEN;
    case CUSTOM:
      return make_prediction_tage(pc);
    default:
      return NOTTAKEN;
  }
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
void train_predictor(uint32_t pc, uint8_t outcome){
  //TODO: Implement Predictor training
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_gshare(pc, outcome);
      break;
    case TOURNAMENT:
  		train_tourn(pc,outcome);
      break;
    case CUSTOM:
      train_tage(pc, outcome);
      break;
    default:
      break;
  }
}


// Free any dynamically allocated Datastructures
void cleanup() {
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      cleanup_gshare();
      break;
    case TOURNAMENT:
    cleanup_tourn();
      break;
    case CUSTOM:
      cleanup_tage();
      break;
    default:
      break;
  }
}
