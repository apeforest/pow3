extern double **Transpose(double **a,int n, int m);
extern double **Multiply(double **a, double **b, int n, int m, int l);
extern double **Padding(double **a, int n);
extern void inverse(double**,int);
extern void ludcmp(double**, int, int*, double*);
extern void lubksb(double**, int, int*, double*);
extern void print_matrix(double **array, int n, int m);
extern void print_vector(double *array, int n);
