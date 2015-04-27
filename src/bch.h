
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
	BCH_BM(); 
	~BCH_BM();

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

/*********************** Creation of GF(2^m)  *******************************/
void gfField(int m, // Base 2 logarithm of cardinality of the Field
	int poly// primitive polynomial of the Field in decimal form
	);

/*********************** Error detection   *******************************/
bool error_detection( int n, int k, int* codeword);

/*********************** Error correction   *******************************/
void BerlMass( int n, int k );

/*********************** final step of BCH decoder ********************************/
void BCH_final_dec(int n, int k, int* message, int* codeword);

void release();

public:

/*********************** print msg and code  *******************************/
void printNK(int n,int k, int* message, int* codeword, int length);

/*********************** verify result  *******************************/
bool verifyResult(int n, int k, int* message, int* messageRef);

/*********************** Message generator **********************************/
void message_gen(int n,int k, unsigned long int  *seed, int* message);

public:

void initialize();

/*********************** Serial BCH encoder ********************************/
void encode(int n, int k, int* message, int* codeword);

void decode(int n, int k, int* message, int* codeword);

private:
	int *powAlpha, *indexAlpha;
	const int m;

	int n,k;
	int *el;
	int *reg;
};

#endif