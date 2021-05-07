#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

// print debugging messages?
#ifndef DEBUG
#define DEBUG   true//false
#endif

// Function declarations
float Ranf( unsigned int *seedp,  float low, float high );
int Ranf( unsigned int *seedp, int ilow, int ihigh );
float SQR( float x );
void	InitBarrier( int );
void	WaitBarrier( );

// Global variables
omp_lock_t	Lock;
int		NumInThreadTeam;
int		NumAtBarrier;
int		NumGone;

int	NowYear;		// 2021 - 2026
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population

unsigned int seed = 0;  // a thread-private variable
float x = Ranf( &seed, -1.f, 1.f );

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =	1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

// Units of grain growth are inches.
// Units of temperature are degrees Fahrenheit (°F).
// Units of precipitation are inches.

int main() {
    #ifndef _OPENMP
		fprintf(stderr, "No OpenMP support!\n");
		return 1;
	#endif

    // starting date and time:
    NowMonth =    0;
    NowYear  = 2021;

    // starting state (feel free to change this if you want):
    NowNumDeer = 1;
    NowHeight =  1.;

    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
    	NowPrecip = 0.;

    float tempFactor = exp(   -SQR(  ( NowTemp - MIDTEMP ) / 10.  )   );

    float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

    
    omp_set_num_threads( 3 );	// same as # of sections
    #pragma omp parallel sections
    {
        InitBarrier(3);
    	#pragma omp section
    	{
    		// GrainDeer( );
            while( NowYear < 2027 )
            {
            	// compute a temporary next-value for this quantity
            	// based on the current state of the simulation:
            	int nextNumDeer = NowNumDeer;
                int carryingCapacity = (int)( NowHeight );
                if( nextNumDeer < carryingCapacity )
                    nextNumDeer++;
                else
                    if( nextNumDeer > carryingCapacity )
                        nextNumDeer--;
                if( nextNumDeer < 0 )
                    nextNumDeer = 0;

            	// DoneComputing barrier:
            	// #pragma omp barrier
                WaitBarrier();
                NowNumDeer = nextNumDeer;

            	// DoneAssigning barrier:
                WaitBarrier();

            	// DonePrinting barrier:
                WaitBarrier();
            }
    	}

    	#pragma omp section
    	{
    		// Grain( );
            while( NowYear < 2027 )
            {
                // compute a temporary next-value for this quantity
                // based on the current state of the simulation:
            	// you then use tempFactor like this
                float nextHeight = NowHeight;
                nextHeight += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
                nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

            	// DoneComputing barrier:
            	// #pragma omp barrier
                WaitBarrier();
                //Be sure to clampHeight against zero, that is
                if(nextHeight <0.){
                    nextHeight = 0.;
                }

                if(nextHeight > 0){
                    NowHeight = nextHeight;
                }
                // printf("NowHeight here: %lf\n", NowHeight);
                if (DEBUG)   fprintf(stderr, "%lf %s", NowHeight, "NowHeight\n");


            	// DoneAssigning barrier:
            	// #pragma omp barrier
                WaitBarrier();

            	// DonePrinting barrier:
            	// #pragma omp barrier
                WaitBarrier();
            }
    	}

    	#pragma omp section
    	{
    		// Watcher( );
            while( NowYear < 2027 )
            {
            	// compute a temporary next-value for this quantity
            	// based on the current state of the simulation:

            	// DoneComputing barrier:
            	// #pragma omp barrier
                WaitBarrier();

                // DoneAssigning barrier:
            	// #pragma omp barrier
                WaitBarrier();

                
                // Increment time
                if(NowMonth == 11){
                    NowYear++;
                    NowMonth = 0;
                }
                else {
                    NowMonth++;
                }


                // Calculate new Environmental Parameters
                ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );

                temp = AVG_TEMP - AMP_TEMP * cos( ang );
                NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

                precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
                NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
                if( NowPrecip < 0. )
                	NowPrecip = 0.;

                

            	// DonePrinting barrier:
            	// #pragma omp barrier
                WaitBarrier();
            }
    	}

    	// #pragma omp section
    	// {
    	// 	// MyAgent( );	// your own
    	// }
    }       // implied barrier -- all functions must return in order
	// to allow any of them to get past here
}

float
Ranf( unsigned int *seedp,  float low, float high )
{
    float r = (float) rand_r( seedp );              // 0 - RAND_MAX

    return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}


int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;

    return (int)(  Ranf(seedp, low,high) );
}

float
SQR( float x )
{
    return x*x;
}

// specify how many threads will be in the barrier:
//	(also init's the Lock)

void
InitBarrier( int n )
{
    NumInThreadTeam = n;
    NumAtBarrier = 0;
	omp_init_lock( &Lock );
}


// have the calling thread wait here until all the other threads catch up:

void
WaitBarrier( )
{
    omp_set_lock( &Lock );
    {
            NumAtBarrier++;
            if( NumAtBarrier == NumInThreadTeam )
            {
                    NumGone = 0;
                    NumAtBarrier = 0;
                    // let all other threads get back to what they were doing
		// before this one unlocks, knowing that they might immediately
		// call WaitBarrier( ) again:
                    while( NumGone != NumInThreadTeam-1 );
                    omp_unset_lock( &Lock );
                    return;
            }
    }
    omp_unset_lock( &Lock );

    while( NumAtBarrier != 0 );	// this waits for the nth thread to arrive

    #pragma omp atomic
    NumGone++;			// this flags how many threads have returned
}