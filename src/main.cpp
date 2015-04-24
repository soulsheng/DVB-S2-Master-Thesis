
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

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
	BCH_BM	bch;

	int message[MAXN];				// information bits
	int codeword[MAXN]={0};			// codeword bits
	int messageRecv[MAXN]={0};		// information received
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

	int tVal = t(n,k) + DRIFT;
	int *el = (int*) calloc(tVal*2,sizeof(int));

#ifndef SERIAL
	load_matrices(n,k);
#endif
#if defined (TESTDEC)
	sprintf(outfile,"DecTest/outdec_%d_%d.txt",n,k);
	o3 = fopen(outfile,"w");
#endif
	// Galois Field Creation
	bch.gfField(16,
		32+8+4+1,
		&pow,
		&index
		);

#if defined (TESTDEC)
	sprintf(outfile,"DecTest/outdec_%d_%d.txt",n,k);
	o3 = fopen(outfile,"w");
#endif

	/** Simulation Loop **/
	for(s = 0; s <2; s++)
	{

	bch.message_gen(n,k,&seed,message);
#ifdef SERIAL
	bch.BCH_s_enc(n,k, message, codeword);
#endif

	std::cout << "init msg & code: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );

#ifdef NPARALLEL
	BCHnclk_par(n,k);
#endif

#ifdef KPARALLEL
	BCHkclk_par(n,k);
#endif


	fprintf(stdout,"\nSIM #%d\n",s+1);

#if defined (TESTDEC)
	fprintf(o3,"\nSimulation #%d\nLocation of the pseudo-random errors:\n ",s+1);

	// Random error pattern generator
	for(i = 0; i < tVal; i++){
		// bit flipping
		seed2 = (s+1)*(i+1);
		srand( seed2 ) ;
		err[i] = (rand()%n+192)%n;
		codeword[ err[i] ] ^= 1;
		fprintf(o3,"%d\t",err[i]);
	}
	// Sort of the error locations in decreasing order:
	// it will be useful to check the corrispondence with errors detected
	bch.elSort(tVal, err);
#endif

	std::cout << "add error to code: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );

	if(bch.error_detection(pow,index,tVal, codeword) ) {
		fprintf(stdout,"Errors detected!\nDecoding by Berlekamp-Massey algorithm.....\n");
		fprintf(o3,"\n\nErrors detected!\nDecoding by Berlekamp-Massey algorithm.....\n");
		bch.BerlMass(tVal*2,pow,index, el);

		bool success = true;
		fprintf(o3,"\nPosition of errors detected:\n");
		for(i = 0; i <tVal*2; i++) {
			if(el[i] != err[i]) {success=false;}
			fprintf(o3,"%d\t",el[i]);

			codeword[ el[i] ] ^= 1;
		}
		if(success) {fprintf(o3,"\nSuccessful decoding!");
		fprintf(stdout,"\nSuccessful decoding!\n----------------------\n");};
		fprintf(o3,"\n\n-------------------------------------");

		std::cout << "correct error for code: " << std::endl;
		bch.printNK( n,k, message, codeword, 100 );

	}
	else
		fprintf(stdout,"\n\nNo errors detected!\n------------------------------\n");


	bch.BCH_final_dec(n,k, messageRecv, codeword);

	bool	bRight = bch.verifyResult(n,k, message, messageRecv);
	if ( bRight )
		std::cout << s+1 << "# message recovered!" << std::endl << std::endl;
	else
		std::cout << s+1 << "# message not recovered!" << std::endl << std::endl;

	}

	//free(pow);	free(index);

	return 0;
}