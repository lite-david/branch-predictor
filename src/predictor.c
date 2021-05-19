//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
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

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint8_t *bht_gshare;
uint32_t ghistory;


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
  int bht_entries = 1 << ghistoryBits;
  int pc_lower_bits = pc & (bht_entries-1);
  //Get index into bht table by xor pc and history
  int index = pc_lower_bits ^ ghistory;
  if(verbose){
    printf("bht_entries: %d, pc_lower_bits: %d, ghistory: %d, index: %d \n", bht_entries, pc_lower_bits, ghistory, index);
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
  int bht_entries = 1 << ghistoryBits;
  int pc_lower_bits = pc & (bht_entries-1);
  int index = pc_lower_bits ^ ghistory;

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
  ghistory = ghistory & (bht_entries -1);
}

void cleanup_gshare(){
  free(bht_gshare);
}

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  init_gshare();
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return make_prediction_gshare(pc);
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_gshare(pc, outcome);
      break;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}

void 
cleanup() {
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      cleanup_gshare();
      break;
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }
}
