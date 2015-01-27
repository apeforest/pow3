#include <stdio.h>
#include <string.h>
#include <math.h>
#include "global.h"

/************** begin forward function prototype declaration ************/
double **Transpose(double **a,int n, int m);
double **Multiply(double **a, double **b, int n, int m, int l);
void inverse(double**,int);
void ludcmp(double**, int, int*, double*);
void lubksb(double**, int, int*, double*);
/************** end function prototype declaration **********************/

/************************************************
   Transpose of a square matrix, do it in place
*************************************************/
double **Transpose(double **a,int n, int m)
{
   int i,j;
   double **b;
   b = (double **)malloc(m*sizeof(double *));
   for (i=0;i<m;i++)
     b[i] = (double *)malloc(n*sizeof(double));

   for (i=0;i<n;i++)
     for (j=0;j<m;j++)
       b[j][i] = a[i][j];
   return b;
}

/*************************************************
  Two matrix multiplication
**************************************************/
double **Multiply(double **a, double **b, int n, int m, int l)
{
  int i,j,k;
  double **c;
  c = (double **)malloc(n*sizeof(double *));
  for (i=0;i<n;i++)
    c[i] = (double *)malloc(m*sizeof(double));
  for(i=0;i<n;i++)
    for(j=0;j<m;j++)
      c[i][j] = 0;
  for (i=0;i<n;i++)
    for (j=0; j < m; j++)
      for (k=0; k < l; k++)
	c[i][j] += a[i][k] * b[k][j];
  return c;
}

/*****************************************************
  b= a - I and padd b's last one column by 1
*****************************************************/
double **Padding(double **a, int n)
{
  int i,j;
  double **b;
  b = (double **)malloc(n*sizeof(double *));
  for (i=0;i<n;i++)
    b[i] = (double *)malloc((n+1)*sizeof(double));
  for(i=0;i<n;i++)
    for(j=0;j<n;j++)
      {
	if(i==j)
	  b[i][j] = a[i][j] - 1;
	else
	  b[i][j] = a[i][j];
      }
  for(i=0;i<n;i++)
    b[i][n]=1;
  return b;
}


/**************************************************
  Inversion of a matrix using Gaussian method
***************************************************/
void inverse(double **mat, int dim)
{
 int i,j,*indx;
 double **y,d,*col;

 y = (double **)malloc(dim*sizeof(double *));
 for(i=0;i<dim;i++)
   y[i]=(double *)malloc(dim*sizeof(double));
 
 indx = (int *)malloc((unsigned)(dim*sizeof(int)));
 col = (double *)malloc((unsigned)(dim*sizeof(double)));
 ludcmp(mat,dim,indx,&d);
 for (j=0;j<dim;j++)
    {
     for (i=0;i<dim;i++) col[i] = 0.0;
     col[j] = 1.0;
     lubksb(mat,dim,indx,col);
     for (i=0;i<dim;i++) y[i][j] = col[i];
    }
 for (i=0;i<dim;i++)
    for (j=0;j<dim;j++)
       mat[i][j] = y[i][j];
 
 for(i=0;i<dim;i++)
   free(y[i]);
 free(col);
 free(indx);
}

/********************************************
LU decomposition
********************************************/
void ludcmp(double **a, int n, int *indx, double *d)
{
  int i,imax,j,k;
  double   big,dum,sum,temp;
  double   *vv;

  vv = (double*)malloc((unsigned)(n*sizeof(double)));
  if (!vv) {
    fprintf(stderr,"Error Allocating Vector Memory\n");
    exit(1);
  }
  *d = 1.0;
  for (i=0;i<n;i++) {
    big = 0.0;
    for (j=0;j<n;j++) {
      if ((temp=fabs(a[i][j])) > big) big = temp;
    }
    if (big == 0.0) {
      fprintf(stderr,"Singular Matrix in Routine LUDCMP\n");
      for (j=0;j<n;j++) printf(" %f ",a[i][j]); printf("/n");
      exit(1);
    }
    vv[i] = 1.0/big;
  }
  for (j=0;j<n;j++) {
    for (i=0;i<j;i++) {
      sum = a[i][j];
      for (k=0;k<i;k++) sum -= a[i][k] * a[k][j];
      a[i][j] = sum;
    }
    big = 0.0;
    for (i=j;i<n;i++) {
      sum = a[i][j];
      for (k=0;k<j;k++) sum -= a[i][k] * a[k][j];
      a[i][j] = sum;
      if ((dum=vv[i]*fabs(sum)) >= big) {
	big = dum;
	imax = i;
      }
    }
    if (j != imax) {
      for (k=0;k<n;k++) {
	dum = a[imax][k];
	a[imax][k] = a[j][k];
	a[j][k] = dum;
      }
      *d = -(*d);
      vv[imax] = vv[j];
    }
    indx[j] = imax;
    if (a[j][j] == 0.0) a[j][j] = TINY;
    if (j != n-1) {
      dum = 1.0 / a[j][j];
      for (i=j+1;i<n;i++) a[i][j] *= dum;
    }
  }
  free(vv);
}

/************************************************
LU back substitute
*************************************************/
void lubksb(double **a, int n, int *indx, double *b)
{
  int i,ip,j,ii=-1;
  double   sum;

  for (i=0;i<n;i++) {
    ip = indx[i];
    sum = b[ip];
    b[ip] = b[i];
    if (ii>=0)
      for (j=ii;j<i;j++) sum -= a[i][j] * b[j];
    else if (sum) ii = i;
    b[i] = sum;
  }
  for (i=n-1;i>=0;i--) {
    sum = b[i];
    for (j=i+1;j<n;j++) sum -= a[i][j] * b[j];
    b[i] = sum / a[i][i];
  }
}

/************************************
Print out a double matrix
*************************************/
void print_matrix(double **array, int n, int m)
{
  int i, j;
  
  for(i = 0; i < n; i++) {
    for(j = 0;  j < m; j++)
      printf("%.2f ", array[i][j]);
    printf("\n");
  }
}

/************************************
Print out a double 1-D array
*************************************/
void print_vector(double *array, int n)
{
  int i;
  
  for(i = 0; i < n; i++) 
    printf("%.2f ", array[i]);
  printf("\n");
}
