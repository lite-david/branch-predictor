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
      return NOTTAKEN;
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
      break;
    case CUSTOM:
      cleanup_tage();
      break;
    default:
      break;
  }
}
