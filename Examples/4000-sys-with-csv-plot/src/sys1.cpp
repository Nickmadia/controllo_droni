#include "main.h"


int Sys1::init()
{
  int i;
  
  x = (double *) malloc(state_size*sizeof(double));
  if (x == NULL) {fprintf(stderr, "Error: x is NULL\n"); return (-1);}

  tmp_x = (double *) malloc(state_size*sizeof(double));
  if (tmp_x == NULL) {fprintf(stderr, "Error: tmp_x is NULL\n"); return (-1);}

  y = (double *) malloc(output_size*sizeof(double));
  if (y == NULL) {fprintf(stderr, "Error: y is NULL\n"); return (-1);}
 
  for (i = 0; i < state_size; i++) {x[i] = 1;}

  return (0);
  
} // init()


void Sys1::update_state(double *u)
{
  tmp_x[0] = 0.1*x[0] + 0.95*x[1] ;
  tmp_x[1] = -0.95*x[0] + 0.1*x[1];
  tmp_x[2] = 0.1*x[2] + 2*u[0];

  tmp_buf = x;

  x = tmp_x;

  tmp_x = tmp_buf;
  
  
}  // update_state()



void Sys1::update_output()
{
  
  y[0] = x[0];
  
} // update_output()


