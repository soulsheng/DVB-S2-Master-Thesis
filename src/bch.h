
#ifndef __BCH_H__
#define __BCH_H__

//////////////////////////// MACRO //////////////////////////////
// It returns corrective capacity of the code with (n,k) given //
/////////////////////////////////////////////////////////////////

#define t(n,k)  ( ((n)-(k)) / (16) )


/*********************** PN bit source **************************************/
int lfsr(unsigned long int *seed);

/*********************** Message generator **********************************/
void message_gen(int n,int k, unsigned long int  *seed);

/*********************** Serial BCH encoder ********************************/
void BCH_s_enc(int n, int k);

/*********************** Loading matrices routine ***************************/
void load_matrices(int n, int k);

/******************  Input comb network  ********************************/
int comb_c(int index, int *input);

/******************  State comb network  ********************************/
int comb_n(int index,int r, int *reg_old);
int comb_k(int index, int *reg_old);

/*********************** BCH parellel encoder n clock ticks *****************/
void BCHnclk_par(int n,int k);

/*********************** BCH parellel encoder k clock ticks *****************/
void BCHkclk_par(int n,int k);

/*********************** Creation of GF(2^m)  *******************************/
void gfField(int m, // Base 2 logarithm of cardinality of the Field
	int poly, // primitive polynomial of the Field in decimal form
	int ** powOfAlpha, int ** indexAlpha);

/*********************** Error detection   *******************************/
bool error_detection(int *pow, int *index, int t);

/*********************** Error correction   *******************************/
void BerlMass(//int *S, // array of syndrome in exponential notation
	int t2, // length of array S
	int *pow,
	int *index);

// Random variable uniformly distributed between 0.0 and 1.0
double uniform01(long *pSeed );

/*********************** Insertion sort  *******************************/
void elSort(int dim);

#endif