
#ifndef __BCH_H__
#define __BCH_H__

//////////////////////////// MACRO //////////////////////////////
// It returns corrective capacity of the code with (n,k) given //
/////////////////////////////////////////////////////////////////

#define t(n,k)  ( ((n)-(k)) / (16) )

#define MAXR 192 // max r bits
#define P 8      // degree of parallelism
//#define MAXN ((1<<16)-1)  // primitive code length
#define MAXT 12         // Max corrective capacity
#define DRIFT 0 // adding extra errors


enum	CODE_RATE
{
	C1_4,	//	n = 16200; k=16008; t=12;
	C1_3,	//	n = 21600; k=21408; t=12;
	C2_5,	//	n = 25920; k=25728; t=12;
	C1_2,	//	n = 32400; k=32208; t=12;
	C3_5,	//	n = 38880; k=38688; t=12;
	C2_3,	//	n = 43200; k=43040; t=10;
	C3_4,	//	n = 48600; k=48408; t=12;
	C4_5,	//	n = 51840; k=51648; t=12;
	C5_6,	//	n = 54000; k=53840; t=10;
	C8_9,	//	n = 57600; k=57472; t=8;
	C9_10,	//	n = 58320; k=58192; t=8;
	CCOUNT
};

enum	FRAME_TYPE
{
	FECFRAME_NORMAL,
	FECFRAME_SHORT
};

class BCH_BM
{
public:
	// @param m	Base 2 logarithm of cardinality of the Field
	BCH_BM(); 
	~BCH_BM();

protected:
/*********************** PN bit source **************************************/
int lfsr(unsigned long int *seed);
#if 0
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
#endif
/*********************** Creation of GF(2^m)  *******************************/
void gfField(int m, // Base 2 logarithm of cardinality of the Field
	int poly, // primitive polynomial of the Field in decimal form
	int* powAlpha,
	int* indexAlpha);

/*********************** Error detection   *******************************/
bool error_detection(char* codeword);

/*********************** Error correction   *******************************/
void BerlMass();

/*********************** final step of BCH decoder ********************************/
void BCH_final_dec(char* message, char* codeword);

void release();

public:

/*********************** print msg and code  *******************************/
void printNK(char* message, char* codeword, int length);

/*********************** verify result  *******************************/
bool verifyResult(char* message, char* messageRef);

/*********************** Message generator **********************************/
void message_gen(unsigned long int  *seed, char* message);

public:

void initialize();

void setCode( CODE_RATE rate, FRAME_TYPE type );

int getN( );
int getK( );

/*********************** Serial BCH encoder ********************************/
void encode(char* message, char* codeword);

void decode(char* message, char* codeword);

private:
	int *powAlphaNormal, *indexAlphaNormal;
	const int mNormal;

	int *powAlphaShort, *indexAlphaShort;
	const int mShort;

	int *powAlpha, *indexAlpha;
	int m;
	int MAXN;

	int n,k;
	int *el;
	int *reg;

	int S[(MAXT + DRIFT)*2];          // Syndrome vector

	CODE_RATE	code_rate; 
	FRAME_TYPE	code_type; 
};

#endif