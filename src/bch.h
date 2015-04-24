
#ifndef __BCH_H__
#define __BCH_H__

//////////////////////////// MACRO //////////////////////////////
// It returns corrective capacity of the code with (n,k) given //
/////////////////////////////////////////////////////////////////

#define t(n,k)  ( ((n)-(k)) / (16) )

#define MAXR 192 // max r bits
#define P 8      // degree of parallelism
#define MAXN ((1<<16)-1)  // primitive code length
#define MAXT 12         // Max corrective capacity
#define DRIFT 0 // adding extra errors

class BCH_BM
{
public:
	// @param m	Base 2 logarithm of cardinality of the Field
	BCH_BM(int t2, int m, FILE *o3); 
	~BCH_BM();
	void initialize();
	void release();

protected:
/*********************** PN bit source **************************************/
int lfsr(unsigned long int *seed);

/*********************** Loading matrices routine ***************************/
void load_matrices(int n, int k);

/******************  Input comb network  ********************************/
int comb_c(int index, int *input);

/******************  State comb network  ********************************/
int comb_n(int index,int r, int *reg_old);
int comb_k(int index, int *reg_old);

/*********************** BCH parellel encoder n clock ticks *****************/
void BCHnclk_par(int n,int k, int* message, int* codeword);

/*********************** BCH parellel encoder k clock ticks *****************/
void BCHkclk_par(int n,int k, int* message, int* codeword);

public:

/*********************** Creation of GF(2^m)  *******************************/
void gfField(int m, // Base 2 logarithm of cardinality of the Field
	int poly// primitive polynomial of the Field in decimal form
	);

/*********************** Error detection   *******************************/
bool error_detection(int* codeword);

/*********************** Error correction   *******************************/
void BerlMass();

// Random variable uniformly distributed between 0.0 and 1.0
double uniform01(long *pSeed );

/*********************** Insertion sort  *******************************/
void elSort(int dim, int* err);

/*********************** print msg and code  *******************************/
void printNK(int n,int k, int* message, int* codeword, int length);

/*********************** verify result  *******************************/
bool verifyResult(int n, int k, int* message, int* messageRef);

/*********************** final step of BCH decoder ********************************/
void BCH_final_dec(int n, int k, int* message, int* codeword);

/*********************** Message generator **********************************/
void message_gen(int n,int k, unsigned long int  *seed, int* message);

/*********************** Serial BCH encoder ********************************/
void BCH_s_enc(int n, int k, int* message, int* codeword);

void decode(int n, int k, int* message, int* codeword);

private:
	int *powAlpha, *indexAlpha;
	int m;
	int t;

	int n,k;
	int *el;

	FILE* o3;
};

#endif