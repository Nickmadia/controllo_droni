#ifndef dts1_h
#define dts1_h


#include "main.h"


class Sys1 {

private:
  int state_size = 3;
  int output_size = 1;
  
  double *tmp_x;  // tmp state
  double *tmp_buf;
  
  public:
  double *x;  // state
  double *y;  // output
  
  int init();
  void update_state(double *u);
  void update_output();
};


#endif
