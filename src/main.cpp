
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <bch.h>

#ifndef TESTENC
#define TESTDEC
#define SERIAL
#endif



/****************************************************************************/
/*********************** Global Variable  **********************************/
/***************************************************************************/

/*************************************************************************/
/*******************        MAIN FUNCTION       **************************/
/*************************************************************************/

int main()
{

	int message[MAXN];//, message[MAXN]; // information bits
	int codeword[MAXN]={0};
	int err[MAXT+DRIFT];          // array of random error location
	FILE *o1, *o2, *o3;
	int *pow, *index;
	int n,k,i,s;
	unsigned long int seed;
	long seed2;
	char *outfile = new char[120];


	//o1 = fopen("outserial.txt","w");
	//o2 = fopen("outpnclk.txt","w");

	n = 57600;  k = 57472;  //
	n = 16200; k=16008; // n = 21600; k = 21408; // n = 43200; k = 43040;

#ifndef SERIAL
	load_matrices(n,k);
#endif
#if defined (TESTDEC)
	sprintf(outfile,"DecTest/outdec_%d_%d.txt",n,k);
	o3 = fopen(outfile,"w");
#endif
	// Galois Field Creation
	gfField(16,
		32+8+4+1,
		&pow,
		&index
		);

#if defined (TESTDEC)
	sprintf(outfile,"DecTest/outdec_%d_%d.txt",n,k);
	o3 = fopen(outfile,"w");
#endif

	/** Simulation Loop **/
	for(s = 0; s <100; s++)
	{

	message_gen(n,k,&seed,message);
#ifdef SERIAL
	BCH_s_enc(n,k, message, codeword);
#endif


#ifdef NPARALLEL
	BCHnclk_par(n,k);
#endif

#ifdef KPARALLEL
	BCHkclk_par(n,k);
#endif


	fprintf(stdout,"SIM #%d\n",s+1);

#if defined (TESTDEC)
	fprintf(o3,"\nSimulation #%d\nLocation of the pseudo-random errors:\n ",s+1);

	// Random error pattern generator
	for(i = 0; i < t(n,k)+DRIFT; i++){
		// bit flipping
		seed2 = (s+1)*(i+1);
		err[i] = (int)floor(n*uniform01(&seed2));
		codeword[ err[i] ] ^= 1;
		fprintf(o3,"%d\t",err[i]);
	}
	// Sort of the error locations in decreasing order:
	// it will be useful to check the corrispondence with errors detected
	elSort(t(n,k)+DRIFT, err);
#endif

	if(error_detection(pow,index,t(n,k)+DRIFT, codeword) ) {
		fprintf(stdout,"Errors detected!\nDecoding by Berlekamp-Massey algorithm.....\n");
		fprintf(o3,"\n\nErrors detected!\nDecoding by Berlekamp-Massey algorithm.....\n");
		BerlMass((t(n,k)+DRIFT)*2,pow,index, err, o3);

	}
	else
		fprintf(stdout,"\n\nNo errors detected!\n------------------------------\n");
	}

	//free(pow);	free(index);

	return 0;
}