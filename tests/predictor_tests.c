#include <stdio.h>
#include <assert.h>
#include "predictor.h"

void test_gshare_basic(){
  int pc_stream[8] = {0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF};
  int outcomes[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  //Settings 
  ghistoryBits = 10;
  bpType = GSHARE;

  init_predictor();
  // The first prediction should be NOTTAKEN since initialization sets history to NT and predictor as WN
  assert(make_prediction(pc_stream[0]) == NOTTAKEN);
  cleanup();
  printf("test_gshare_basic passed! \n");
}

void test_gshare_2bit_basic(){
  int pc_stream[8] = {0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF,
                      0xCAFECAFE, 0xDEADBEEF};
  int outcomes[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  //Settings 
  ghistoryBits = 2;
  bpType = GSHARE;

  init_predictor();
  // The first prediction should be NOTTAKEN since initialization sets history to NT and predictor as WN
  assert(make_prediction(pc_stream[0]) == NOTTAKEN);
  // WN -> WT for index 2 in BHT. history is set to 1.
  train_predictor(pc_stream[0], outcomes[0]);
  // The second prediction should be TAKEN since pc is 3, but 3 xor 1 = 2. Index 2 is WT
  assert(make_prediction(pc_stream[1]) == TAKEN);
  cleanup();
  printf("test_gshare_2bit_basic passed! \n");

}

void test_gshare(){
  test_gshare_basic();
  test_gshare_2bit_basic();
}

int main(){
  test_gshare();
  return 0;
}
