
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>

#include <bch_bm.h>




/****************************************************************************/
/*********************** Global Variable  **********************************/
/***************************************************************************/

/*************************************************************************/
/*******************        MAIN FUNCTION       **************************/
/*************************************************************************/


int main()
{

	int message[MAXN];				// information bits
	int codeword[MAXN]={0};			// codeword bits
	int messageRecv[MAXN]={0};		// information received
	int err[MAXT+DRIFT];          // array of random error location
	FILE *o1, *o2;//, *o3;
	int n,k,i,s;
	unsigned long int seed;
	long seed2;
	char *outfile = new char[120];


	//o1 = fopen("outserial.txt","w");
	//o2 = fopen("outpnclk.txt","w");

	// n = 57600;  k = 57472;  //
	// n = 16200; k=16008; // n = 21600; k = 21408; // n = 43200; k = 43040;
	CODE_RATE_TAG code_rate = RATE_8_9;

	CODE_TYPE_TAG code_type = CODE_TYPE_NORMAL;

	BCH_BM	bch;
	bch.initialize();


	/** Simulation Loop **/
	for(s = 0; s <2; s++)
	{
	
	bch.setCode( code_rate, code_type );

	n = bch.getN();
	k = bch.getK();

	bch.message_gen(n,k,&seed,message);

	bch.encode(n,k, message, codeword);

	std::cout << "init msg & code: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );


	fprintf(stdout,"\nSIM #%d\n",s+1);


	fprintf(stdout,"\nSimulation #%d\nLocation of the pseudo-random errors:\n ",s+1);

	int tCapacity = 0;
	if ( code_type == CODE_TYPE_NORMAL )
		tCapacity = t(n,k) + DRIFT;
	else
		tCapacity = 12 + DRIFT;
	// Random error pattern generator
	for(i = 0; i < tCapacity; i++){
		// bit flipping
		seed2 = (s+1)*(i+1);
		srand( seed2 ) ;
		err[i] = rand()%k+n-k;
		codeword[ err[i] ] ^= 1;
		fprintf(stdout,"%d\t",err[i]);
	}


	std::cout << "add error to code: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );

	bch.decode(n,k, messageRecv, codeword);

	std::cout << "decode: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );

	bool	bRight = bch.verifyResult(n,k, message, messageRecv);
	if ( bRight )
		std::cout << s+1 << "# message recovered!" << std::endl << std::endl;
	else
		std::cout << s+1 << "# message not recovered!" << std::endl << std::endl;

	}


	return 0;
}