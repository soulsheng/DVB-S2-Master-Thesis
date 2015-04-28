#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>

#include "bch_bm.h"

#if 0
/****************************************************************************/
/*********************** Global Variable  **********************************/
/***************************************************************************/


int Ap_n[MAXR][MAXR];	// (n-k) rows, (n-k) col
int Ap_k[MAXR][MAXR];	// 192 rows, 192 col
int C[MAXR][P];			// 192 rows, 8 col
int S[(MAXT + DRIFT)*2];          // Syndrome vector

/****************************************************************************/
/*********************** PN bit source **************************************/
/***************************************************************************/
#endif
int BCH_BM::lfsr(unsigned long int *seed)
{
	int b,c;

	b = ( ((*seed) & (1 << 31) ) >> 31 ) ;

	c =   ((*seed) & 1) ^ ( ((*seed) & (1 << 1)) >> 1 ) ^ ( ((*seed) & (1 << 21)) >> 21 ) ^ b ;

	(*seed) = ((*seed) << 1) | c;

	return(b);
}

/****************************************************************************/
/*********************** Message generator **********************************/
/***************************************************************************/

void BCH_BM::message_gen(int n,int k, unsigned long int  *seed, int* message)
{
	int i;
    // Message bits pseudo random generation
	for (i=n-1;i>=n-k;i--)
		message[i] = lfsr(seed);
	// Zero padding
	for(i = 0; i < n-k; i++)
		message[i] = 0;
}

/****************************************************************************/
/*********************** Polynomial Generators *****************************/
/***************************************************************************/
// Note: only used by algorithm emulating the serial architecture (n-clock cycles)

const unsigned int gen12[] =
{1,1,1,0,0,1,1,1,1,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,
 1,1,1,0,1,1,1,1,1,0,1,0,0,0,0,1,1,1,1,0,0,0,1,0,1,1,0,0,0,0,0,0,
 1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,1,0,1,1,0,0,0,0,1,1,1,0,1,1,
 0,0,0,1,1,0,1,1,0,0,1,1,0,1,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,0,1,0,
 0,0,1,1,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1,1,1,0,1,0,0,0,1,0,0,0,0,1,
 1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,0,0,1,1,0,0,1,0,0,0,1,1,1,0,0,1,0,
 1};
// i.e. gen(x) = a_0*x^0 + a_1*x^1 + ... + a_(r-1)*x^(r-1) + a_r*x^r

const unsigned int gen10[] =
{1,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,1,1,0,1,1,1,0,1,0,0,0,1,1,1,0,1,
 1,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1,0,0,0,1,0,1,1,1,1,1,1,0,1,1,1,
 1,1,0,0,0,0,0,0,1,1,1,0,1,0,1,0,0,0,0,1,1,1,1,0,0,1,0,1,0,1,1,0,
 1,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,0,1,0,1,0,1,0,0,0,0,1,1,1,1,1,1,
 1,0,1,1,0,1,1,1,0,0,1,1,0,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,0,1,1,0,
 1};

const unsigned int gen8[] =
{1,1,0,1,0,1,0,0,0,1,1,0,0,1,1,0,1,0,0,1,1,1,1,1,0,0,1,0,0,0,0,0,
 1,0,1,0,1,1,1,0,1,0,1,1,0,1,1,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,
 1,0,1,1,1,1,0,1,1,1,1,0,1,0,0,1,1,1,1,0,0,1,0,0,1,0,0,0,1,1,1,0,
 1,1,1,1,1,0,1,0,1,0,1,0,0,1,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,
 1};


const unsigned int gen12_s[] = 
{1,0,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,1,1,
1,1,1,0,1,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,1,0,1,0,0,1,0,1,0,
1,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1,1,1,0,0,0,1,0,1,1,1,0,0,0,1,0,0,
1,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,0,1,0,0,1,1,0,1,1,0,0,1,0,1,1,0,
0,0,0,1,1,0,0,1,0,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,0,0,0,1,1,0,
0,0,0,0,0,0,1,0,
1};	// short frame

/****************************************************************************/
/*********************** Serial BCH encoder ********************************/
/***************************************************************************/

void BCH_BM::encode(int n, int k, int* message, int* codeword)
{

	const unsigned int *g;
	
	int mem,app,i,j;

/***************  Mode Selection (t-error-correction) ***********************/

	switch(n-k) {
	case 192:
		g = gen12;
		break;
	case 160:
		g = gen10;
		break;
	case 128:
		g = gen8;
		break;
	case 168:
		g = gen12_s;
		break;
	default:
		fprintf(stdout,"Error:simulation aborted!\n");
		fprintf(stdout,"Please insert a n-k couple provided by DVB-S2 FEC\n");
		exit(0);
	}
	
	memset( reg, 0, sizeof(int)*MAXR );

/*********************** Encoding serial algorithm ********************************/
/**************************   n clock ticks **************************************/

/************************* Computing remainder **********************************/

	for (i=n-1; i>=0; i--)
	{
		mem=reg [n-k-1];
		for (j=n-k-2; j>=0; j--)
		{
			app=mem & g[j+1];
			reg[j+1]=reg[j]^app;
		}

		reg[0]= message[i]^(mem & g[0]);

	}

/*********************** Codeword in systematic form ********************************/

	for (i=n-1;i>=n-k;i--)
		codeword[i] = message[i];
	for (i=n-k-1; i >=0; i--)
		codeword[i] = reg[i];

}
#if 0
/****************************************************************************/
/*********************** Loading matrices routine ***************************/
/***************************************************************************/

void BCH_BM::load_matrices(int n, int k)
{
	FILE *input_Ap_k, *input_C, *input_Ap_n;
	int i,j;

/***********************  Mode Selection (t-error-correction) ***********************/

	switch(n-k) {
	case 192:
		input_Ap_k = fopen ("Matrices/ADVBS2_nclk_t12.txt","r");
		input_Ap_n = fopen ("Matrices/ADVBS2_nclk_t12.txt","r");
		input_C = fopen("Matrices/CDVBS2_kclk_t12.txt","r");
		break;
	case 160:
		input_Ap_k = fopen ("Matrices/ADVBS2_kclk_t10.txt","r");
		input_Ap_n = fopen ("Matrices/ADVBS2_nclk_t10.txt","r");
		input_C = fopen("Matrices/CDVBS2_kclk_t10.txt","r");
		break;
	case 128:
		input_Ap_k = fopen ("Matrices/ADVBS2_kclk_t8.txt","r");
		input_Ap_n = fopen ("Matrices/ADVBS2_nclk_t8.txt","r");
		input_C = fopen("Matrices/CDVBS2_kclk_t8.txt","r");
		break;
	default:
		fprintf(stdout,"Error: loading of matrices failed!\n");
		fprintf(stdout,"Please insert a n-k couple provided by DVB-S2 FEC\n");
		exit(0);
	}



/********************* Loading matrix Ap_n ***********************************/
/////////// Note: ONLY this matrix size is variable //////////////////////////


	for ( i=0;i<n-k;i++){
		for ( j=0;j<n-k;j++){
			//fscanf(input_Ap_k,"%d\t",&(Ap_k[i][j]));
			fscanf(input_Ap_n,"%d\t",&(Ap_n[i][j]));
			//power_A[i][j] = load_i;
		}
		//fscanf(input_Ap_k,"\n");
		fscanf(input_Ap_n,"\n");
	}

/********************* Loading matrix Ap_k ***********************************/

	for ( i=0;i<MAXR;i++)
		for ( j=0;j<MAXR;j++)
			fscanf(input_Ap_k,"%d\t", &(Ap_k[i][j]));
		fscanf(input_Ap_k,"\n");




/********************* Loading matrix C ***********************************/
	for (i=0;i<MAXR;i++)
	{
		for (j=0;j<P;j++)
		{
			fscanf(input_C,"%d\t",&(C[i][j]));
			//comb_C[i][j] = load_c;
		}

		fscanf (input_C,"\n");
	}


	fclose(input_C);
	fclose(input_Ap_n);
	fclose(input_Ap_k);

}

/************************************************************************/
/***********  Combinatorial blocks emulation ****************************/
/***********************************************************************/

/************************************************************************/
/******************  Input comb network  ********************************/
/***********************************************************************/

int BCH_BM::comb_c(int index, int *input)
{
	int out,f,ind;

	out=0;

	ind=P-1;

	for (f=0; f<P; f++)
	{
		out= out ^ ((C[index][f]) & (input[f]));
		ind--;
	}

	return(out);
}

/************************************************************************/
/******************  State comb network  ********************************/
/***********************************************************************/

int BCH_BM::comb_n(int index,int r, int *reg_old)
{
	int out,f;

	out=0;

	for (f=0; f<P; f++)

	{
		out=out^(Ap_n[index][r-f-1] & reg_old[r-f-1]);
	}

	return(out);
}

int BCH_BM::comb_k(int index, int *reg_old)
{
	int out,f;

	out=0;

	for (f=0; f<P; f++)

	{
		out=out^(Ap_k[index][MAXR-f-1] & reg_old[MAXR-f-1]);
	}

	return(out);
}



/****************************************************************************/
/*********************** BCH parellel encoder *******************************/
/*********************** n clock ticks        *******************************/
/***************************************************************************/

void BCH_BM::BCHnclk_par(int n,int k, int* message, int* codeword)
{
	int clock_ticks;
	int *reg, *reg_old;

	int input[P]; // parallel input bits

/***************  Mode Selection (t-error-correction) ***********************/

	switch(n-k) {
	case 192:
		reg = (int*)calloc(n-k,sizeof(int));
		reg_old = (int*)calloc(n-k,sizeof(int));
		break;
	case 160:
		reg = (int*)calloc(n-k,sizeof(int));
		reg_old = (int*)calloc(n-k,sizeof(int));
		break;
	case 128:
		reg = (int*)calloc(n-k,sizeof(int));
		reg_old = (int*)calloc(n-k,sizeof(int));
		break;
	default:
		fprintf(stdout,"Error:simulation aborted!\n");
		fprintf(stdout,"Please insert a n-k couple provided by DVB-S2 FEC\n");
		exit(0);
	}
	/// Computation of clock ticks required to compute the remainder after division////
	clock_ticks = n/P;



/************************* Computing remainder **********************************/
	int z=0;

	for (int i=0; i<clock_ticks; i++)
	{
		///// refresh of state  /////// ///////
		for (int m=0; m<n-k; m++)
			reg_old[m]=reg[m];
		///////////////////////////////////////
		/////// loading of parallel input //////
		for (int count=P-1; count>=0; count--)
		{
			z++;
			input[count] = message[n-z];
		}
		///////////////////////////////////////////
		/// Computing of next values of state /////
		if (clock_ticks >0)
		{

			int m;
			for (m=0; m<n-k; m++)
			{
				if (m<P)

					reg[m] = input[m]^comb_n(m,n-k,reg_old);

				else

					reg[m] = comb_n(m,n-k,reg_old)^reg_old[m-P];

			}
		}
		/////////////////////////////////////////////
	}
/************************************************************************************/

/*********************** Codeword in systematic form ********************************/
	int i;
	for (i=n-1; i>n-k-1; i--)
		codeword[i] = message[i];

	for (i=n-k-1; i>=0; i--)
		codeword[i] =  reg[i];

	free(reg);	free(reg_old);
}

/****************************************************************************/
/*********************** BCH parellel encoder *******************************/
/*********************** k clock ticks        *******************************/
/***************************************************************************/

void BCH_BM::BCHkclk_par(int n,int k, int* message, int* codeword)
{
	int clock_ticks;
	int *reg, *reg_old;
	int offset,m;

	int input[P]; // parallel input bits

/***************  Mode Selection (t-error-correction) ***********************/

	switch(n-k) {
	case 192:
		reg = (int*)calloc(MAXR,sizeof(int));
		reg_old = (int*)calloc(MAXR,sizeof(int));
		offset = MAXR-192;
		break;
	case 160:
		reg = (int*)calloc(MAXR,sizeof(int));
		reg_old = (int*)calloc(MAXR,sizeof(int));
		offset = MAXR - 160;
		break;
	case 128:
		reg = (int*)calloc(MAXR,sizeof(int));
		reg_old = (int*)calloc(MAXR,sizeof(int));
		offset = MAXR-128;
		break;
	default:
		fprintf(stdout,"Error:encoding aborted!\n");
		fprintf(stdout,"Please insert a n-k couple provided by DVB-S2 FEC\n");
		exit(0);
	}
	/// Computation of clock ticks required to compute the remainder after division////
	clock_ticks = k/P;

/************************* Computing remainder **********************************/

	int z=0;
	for (int i=0; i<clock_ticks; i++)
	{
		for (m=0; m < MAXR; m++)
			reg_old[m]=reg[m];

		for (int count=P-1; count>=0; count--)
		{
			z++;
			input[count] = message[n-z];
		}


		if (clock_ticks>0)
		{

			for (m=0; m< MAXR; m++)
			{
				if (m<P)

					reg[m] = (comb_c(m,input))^(comb_k(m,reg_old));
				else
					reg[m] = (comb_c(m,input))^(comb_k(m,reg_old))^(reg_old[m-(P)]);
			}
		}
	}
/*********************** Codeword in systematic form ********************************/
	int i;
	for (i=n-1; i>n-k-1; i--)
		codeword[i] = message[i];

	for (i=n-k-1; i>=0 ; i--)
		codeword[i] =  reg[i+offset];

	/*  Check values of register
	FILE *de;
	de = fopen("debugkclk.txt","w");
	for(i = MAXR-1; i >=0; i--)
		fprintf(de,"%d\n",reg[i]);
	*/

	free(reg);	free(reg_old);
}
#endif
/****************************************************************************/
/*********************** Creation of GF(2^m)  *******************************/
/*********************** useful tables        *******************************/
/***************************************************************************/

void BCH_BM::gfField(int m, // Base 2 logarithm of cardinality of the Field
			 int poly, // primitive polynomial of the Field in decimal form
			 int* powAlpha,
			 int* indexAlpha)
{
	int reg,	// this integer of 32 bits, masked by a sequence of m ones,
				// contains the elements of Galois Field
		tmp,i;
	// sequence of m ones
	int mask = (1<<m) -1;  // 1(m) Bit Masking

	powAlpha[0] = 1;
	indexAlpha[0] = - 1; // we set -1
	indexAlpha[1] = 0;

	for (i = 0, reg = 1; i < (1<<m)-2; i++)
	{
			tmp = (int)(reg & (1<<(m-1))); // Get the MSB
            reg <<= 1;   // Register shifted
            if( tmp) { //
				reg ^= poly;
				//
				reg &= mask;
			}
			// Step-by-step writing of the tables
			powAlpha[i+1] = (int) reg;
			indexAlpha[(int)reg] = i+1;
    }


}


/****************************************************************************/
/*********************** Error detection   *******************************/
/***************************************************************************/

bool BCH_BM::error_detection( int n, int k, int* codeword)
{
	int tCapacity = 0;
	if ( code_type == CODE_TYPE_NORMAL )
		tCapacity = t(n,k) + DRIFT;
	else
		tCapacity = 12 + DRIFT;

	bool syn = false;
	for(int i = 0; i < tCapacity*2; i++)
	{
		S[i] = 0;
		for(int j = 0; j < n; j++){
			if(codeword[j])
				S[i] ^= powAlpha[((i+1)*j)%MAXN];
		}
		if((S[i] = indexAlpha[S[i]]) != -1)
			syn = true;

	}

	return syn;

}


/****************************************************************************/
/*********************** Error correction   *******************************/
/***************************************************************************/

void BCH_BM::BerlMass( int n, int k )

{
	int tCapacity = 0;
	if ( code_type == CODE_TYPE_NORMAL )
		tCapacity = t(n,k) + DRIFT;
	else
		tCapacity = 12 + DRIFT;

	int t2 = 2*tCapacity;
	int j,L,l,i;
	int d, dm, tmp;
	int *T, *c, *p, *lambda;
	// Allocation and initialization
	// Auto-Regressive-Filter coefficients computed at the previous step
	p = (int*) calloc(t2,sizeof(int));
	// Auto-Regressive-Filter coefficients computed at the current step
	c = (int*) calloc(t2,sizeof(int));
	// Temporary array
	T = (int*) calloc(t2,sizeof(int));
	// error location array (found by Chien Search)
	//el = (int*) calloc(t2,sizeof(int));
	// Error polynomial locator
	lambda = (int*) calloc(t2,sizeof(int));

	memset( el, -1, sizeof(int)*MAXT );

	// Inizialization step
	c[0] = 1;
	p[0] = 1;
	L = 0;
	l = 1;
	dm = 1;

/*********** Berlekamp-Massey Algorithm *******************/
	for (j = 0; j < t2; j++)
	{
		// Discrepancy computation
		if(S[j] == -1)
			d = 0;
		else
			d = powAlpha[S[j]];
		for(i = 1; i <= L;i++)
			if(S[j-i] >= 0 && c[i] > 0)
			d ^= powAlpha[(indexAlpha[c[i]]+ S[j-i])%MAXN];
			// exponential rule

		if( d == 0)
		{
			l++;
		}
		else
		{
			if(2*L > j)
			{
				for( i = l; i <t2; i++)
				{
					if(p[i-l] != 0)
						c[i] ^= powAlpha[(indexAlpha[d]-indexAlpha[dm]+indexAlpha[p[i-l]]+MAXN)%MAXN];
				}
				l++;
			}
			else
			{
				for( i = 0; i < t2; i++)
					T[i] = c[i];
				for( i = l; i <t2; i++)
				{
					if(p[i-l] != 0)
						c[i] ^= powAlpha[(indexAlpha[d]-indexAlpha[dm]+indexAlpha[p[i-l]]+MAXN)%MAXN];
				}
				L = j-L+1;
				for( i = 0; i < t2; i++)
					p[i] = T[i];
				dm = d;
				l = 1;
			}

		}
	}



/********** Storing of error locator polynomial coefficient **********/
	for(i = 0; i <=L; i++)
	{
		// Error storing
		lambda[i] = indexAlpha[c[i]];

	}

/**************    Chien search   **************************/
/*******************   Roots searching  ***********************/

	int kk = 0;
	for(i = 0; i < MAXN; i++)
	{
		for(j = 1, tmp = 0; j <=L; j++)
			tmp ^= powAlpha[(lambda[j]+i*j)%MAXN];
		if (tmp == 1)
			// roots inversion give the error locations
			el[kk++] = (MAXN-i)%MAXN;

	}
	


	free(T); free(c); free(p); free(lambda); //free(el);

}


/*********************** print msg and code  *******************************/
void BCH_BM::printNK( int n,int k, int* message, int* codeword, int length )
{
	std::cout << std::endl << "msg:" << std::endl;
	int nMax = n-k+length-1;
	for (int i=nMax;i>=n-k;i--)
		std::cout << message[i] << " ";

	std::cout << std::endl << "code:" << std::endl;
	for (int i=nMax;i>=n-k;i--)
		std::cout << codeword[i] << " ";

	std::cout << std::endl;
}

void BCH_BM::BCH_final_dec( int n, int k, int* message, int* codeword )
{
	for (int i=n-1;i>=n-k;i--)
		message[i] = codeword[i];
}

bool BCH_BM::verifyResult( int n, int k, int* message, int* messageRef )
{
	bool bSuccess = true;
	for (int i=n-1;i>=n-k;i--)	{
		if( message[i] != messageRef[i])	{
			bSuccess = false;
			break;
		}
	}

	return bSuccess;
}

BCH_BM::BCH_BM()
	:mNormal(16), mShort(14)
{
	// Allocation and initialization of the tables of the Galois Field
	powAlphaNormal = (int *)calloc((1<<mNormal)-2, sizeof(int));
	indexAlphaNormal = (int *)calloc((1<<mNormal)-1, sizeof(int));

	// Galois Field Creation
	gfField(mNormal, 32+8+4+1, powAlphaNormal, indexAlphaNormal);

	powAlphaShort = (int *)calloc((1<<mShort)-2, sizeof(int));
	indexAlphaShort = (int *)calloc((1<<mShort)-1, sizeof(int));

	// Galois Field Creation
	gfField(mShort, 32+8+2+1, powAlphaShort, indexAlphaShort);

}

BCH_BM::~BCH_BM()
{
	release();
}

void BCH_BM::initialize()
{
	el = (int*) calloc(MAXT*2,sizeof(int));
	reg = (int*)calloc(MAXR,sizeof(int));
}

void BCH_BM::release()
{
	//free(powAlpha);	free(indexAlpha);
	free( el );
	free( reg );
}

void BCH_BM::decode( int n, int k, int* messageRecv, int* codeword )
{
	if( error_detection( n, k, codeword) ) {
		fprintf(stdout,"Errors detected!\nDecoding by Berlekamp-Massey algorithm.....\n");

		BerlMass( n, k );

		bool success = true;
		fprintf(stdout,"\nPosition of errors detected:\n");
		for(int i = 0; i <MAXT; i++) 
		{
			if ( -1 != el[i] )
			{
				codeword[ el[i] ] ^= 1;
				fprintf(stdout,"%d\t",el[i]);
			}
		}

		if(success) {
		fprintf(stdout,"\nSuccessful decoding!\n----------------------\n");};
		
	}
	else
		fprintf(stdout,"\n\nNo errors detected!\n------------------------------\n");


	BCH_final_dec(n,k, messageRecv, codeword);

}

void BCH_BM::setCode( CODE_RATE_TAG rate, CODE_TYPE_TAG type )
{
	code_rate = rate;
	code_type = type;

	if( CODE_TYPE_NORMAL == code_type ) {
		switch( code_rate )
		{
		case RATE_1_4:	
			n = 16200; k=16008;
			break;
		case RATE_1_3:	
			n = 21600; k=21408; 
			break;
		case RATE_2_5:	
			n = 25920; k=25728; 
			break;
		case RATE_1_2:	
			n = 32400; k=32208; 
			break;
		case RATE_3_5:	
			n = 38880; k=38688; 
			break;
		case RATE_2_3:	
			n = 43200; k=43040; 
			break;
		case RATE_3_4:	
			n = 48600; k=48408; 
			break;
		case RATE_4_5:	
			n = 51840; k=51648; 
			break;
		case RATE_5_6:	
			n = 54000; k=53840; 
			break;
		case RATE_8_9:	
			n = 57600; k=57472; 
			break;
		case RATE_9_10:	
			n = 58320; k=58192;
			break;
		default:
			break;
		}// switch

		m = 16;
		powAlpha = powAlphaNormal;
		indexAlpha = indexAlphaNormal;

	}// if normal
	else	// short
	{
		switch( code_rate )
		{
		case RATE_1_4:	
			n = 3240; k=3072;
			break;
		case RATE_1_3:	
			n = 5400; k=5232; 
			break;
		case RATE_2_5:	
			n = 6480; k=6312; 
			break;
		case RATE_1_2:	
			n = 7200; k=7032; 
			break;
		case RATE_3_5:	
			n = 9720; k=9552; 
			break;
		case RATE_2_3:	
			n = 10800; k=10632; 
			break;
		case RATE_3_4:	
			n = 11880; k=11712; 
			break;
		case RATE_4_5:	
			n = 12600; k=12432; 
			break;
		case RATE_5_6:	
			n = 13320; k=13152; 
			break;
		case RATE_8_9:	
			n = 14400; k=14232; 
			break;
		default:
			break;
		}// switch

		m = 14;
		powAlpha = powAlphaShort;
		indexAlpha = indexAlphaShort;

	}// else short

	MAXN = (1<<m)-1;
}

int BCH_BM::getN( )
{
	return n;
}

int BCH_BM::getK( )
{
	return k;
}
