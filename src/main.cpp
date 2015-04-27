
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

enum	CODE_RATE_TAG
{
	RATE_1_4,	//	n = 16200; k=16008; t=12;
	RATE_1_3,	//	n = 21600; k=21408; t=12;
	RATE_2_5,	//	n = 25920; k=25728; t=12;
	RATE_1_2,	//	n = 32400; k=32208; t=12;
	RATE_3_5,	//	n = 38880; k=38688; t=12;
	RATE_2_3,	//	n = 43200; k=43040; t=10;
	RATE_3_4,	//	n = 48600; k=48408; t=12;
	RATE_4_5,	//	n = 51840; k=51648; t=12;
	RATE_5_6,	//	n = 54000; k=53840; t=10;
	RATE_8_9,	//	n = 57600; k=57472; t=8;
	RATE_9_10,	//	n = 58320; k=58192; t=8;
	RATE_COUNT
};

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
	}


	BCH_BM	bch;
	bch.initialize();


	/** Simulation Loop **/
	for(s = 0; s <2; s++)
	{

	bch.message_gen(n,k,&seed,message);

	bch.encode(n,k, message, codeword);

	std::cout << "init msg & code: " << std::endl;
	bch.printNK( n,k, message, codeword, 100 );


	fprintf(stdout,"\nSIM #%d\n",s+1);


	fprintf(stdout,"\nSimulation #%d\nLocation of the pseudo-random errors:\n ",s+1);

	// Random error pattern generator
	for(i = 0; i < t(n,k) + DRIFT; i++){
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