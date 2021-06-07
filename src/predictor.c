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
int history_growth; // factor for history length growth across BHTs
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
uint8_t **bht_set;
uint8_t **bht_tags;
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
      printf("Warning: Undefined state of entry in BHT!\n");
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
      printf("Warning: Undefined state of entry in BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void cleanup_gshare(){
  free(bht_gshare);
}

// TAGE functions

void init_tage(){
  //TAGE uses the gshare with 2 bit history as a base predictor 
  ghistoryBits = 2;
  init_gshare();
  //Improves upon base prediction with additional varying length tagged history tables 
  initial_history_bits = 4;
  history_growth = 2;
  num_histories = 2;
  
  //Allocate additional BHTs and tag 
  bht_set = (uint8_t**)malloc(num_histories*sizeof(uint8_t*));
  bht_tags = (uint8_t**)malloc(num_histories*sizeof(uint8_t*));
  int i =0;
  for(i = 0; i< num_histories; i++){
    int history_bits = initial_history_bits*pow(history_growth, i); 
    bht_set[i] = (uint8_t*)malloc(1<<history_bits * sizeof(uint8_t));
    bht_tags[i] = (uint8_t*)malloc(1<<history_bits * sizeof(uint8_t));
  }
}

uint8_t make_prediction_tage(uint32_t pc){
  uint8_t prediction = make_prediction_gshare(pc);
  int i = 0;
  for(i = 0; i< num_histories; i++){
    int history_bits = initial_history_bits*pow(history_growth, i); 
    uint32_t bht_entries = 1 << history_bits;
    uint32_t pc_lower_bits = pc & (bht_entries-1);
    uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
    uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
    uint8_t tag = ((pc & 0xFF0000) >> 16);
    if(bht_tags[i][index] == tag){
      switch(bht_set[i][index]){
        case WN:
          prediction = NOTTAKEN;
        case SN:
          prediction = NOTTAKEN;
        case WT:
          prediction = TAKEN;
        case ST:
          prediction = TAKEN;
        default:
          printf("Warning: Undefined state of entry in BHT!\n");
          prediction = NOTTAKEN;
      }
    }
  }
  return prediction;
}

void train_tage(uint32_t pc, uint8_t outcome){
  //TODO Implement training of rest of the BHTs
  train_gshare(pc, outcome);
}

void cleanup_tage(){
  cleanup_gshare();
  int i =0;
  for(i = 0; i< num_histories; i++){
    free(bht_set[i]);
    free(bht_tags[i]);
  }
  free(bht_set);
  free(bht_tags);
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
