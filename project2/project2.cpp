#include <omp.h>
#include <stdio.h>
#include <math.h>

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

#define N	0.70

// setting the number of threads:
#ifndef NUMT
#define NUMT		    2
#endif


// how many tries to discover the maximum performance:
#ifndef NUMTRIES
#define NUMTRIES	10
#endif

float Height( int, int );	// function prototype



float
Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r <= 0. )
	        return 0.;
	float height = pow( r, 1./(float)N );
	return height;
}



int main( int argc, char *argv[ ] )
{
#ifndef _OPENMP
	fprintf( stderr, "No OpenMP support!\n" );
	return 1;
#endif
	omp_set_num_threads( NUMT );
	// set the number of threads to use in parallelizing the for-loop:`

    // the area of a single full-sized tile:
    // (not all tiles are full-sized, though)

    float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
                ( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );

// get ready to record the maximum performance and the probability:
	double maxPerformance = 0.;	// must be declared outside the NUMTRIES loop
	double totalVolume=0; // must be declared outside the NUMTRIES loop
	// looking for the maximum performance:
	for( int tries = 0; tries < NUMNODES; tries++ )
    {
        double time0 = omp_get_wtime( );
        //totalVolume = 0;//totalVolume?
	// sum up the weighted heights into the variable "volume"
	// using an OpenMP for loop and a reduction:

	#pragma omp parallel for default(none), reduction(+:totalVolume), shared(fullTileArea,stderr)
    for( int i = 0; i < NUMNODES*NUMNODES; i++ )
    {
    	int iu = i % NUMNODES;
    	int iv = i / NUMNODES;
        float z = Height( iu, iv );
        //condition1 edge
        if(iu==0){
        	//(0,0)
        	if(iv==0){
        		totalVolume += .25*(z*fullTileArea);
        	}
        	//(0,NumNodes)
        	if(iv==NUMNODES){
        		totalVolume += .25*(z*fullTileArea);
        	}
        	//(0,Edge)
        	else{
        		totalVolume += .5*(z*fullTileArea);
        	}
        }//end of cond1
        //________________//
        //condition2 edge
        if(iu==NUMNODES){
        	//(0,0)
        	if(iv==0){
        		totalVolume += .25*(z*fullTileArea);
        	}
        	//(0,NumNodes)
        	if(iv==NUMNODES){
        		totalVolume += .25*(z*fullTileArea);
        	}
        	//(0,Edge)
        	else{
        		totalVolume += .5*(z*fullTileArea);
        	}
        }//end of cond2
        //________________//
        //condition3 edge
        if(iv==0){
        	//(0,Edge)
        	//we used ! to stop over counting corners
        	if((iu!=0)&&(iu!=NUMNODES))
        		totalVolume += .5*(z*fullTileArea);
        }//end of cond3
        //________________//

        //condition4 edge
        if(iv==NUMNODES){
        	//(NUMNODES,Edge)
        	//we used ! to stop over counting
        	if((iu!=0)&&(iu!=NUMNODES))
        		totalVolume += .5*(z*fullTileArea);
        }//end of cond4
        //________________//

        //condition5 middle nodes
        else{
        	//((!Edege&&!Corner),(!Edege&&!Corner))
        	totalVolume += (z*fullTileArea);
        }//end of cond5
       }//end of pragma for
    
    double time1 = omp_get_wtime( );
	double megaVolumePerSecond = (double)NUMNODES*NUMNODES / ( time1 - time0 ) / 1000000.;
		if( megaVolumePerSecond > maxPerformance )
			maxPerformance = megaVolumePerSecond;

	 }// for ( # of timing tries )
	fprintf(stderr, "Peak Performance = %8.2lf MegaHeights/Sec\n", maxPerformance);
	return 0;

}//end of main
