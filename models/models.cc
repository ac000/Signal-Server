/*****************************************************************************
*  RF propagation models for Signal Server by Alex Farrant, CloudRF.com      *
*                   							     *
*  This program is free software; you can redistribute it and/or modify it   *
*  under the terms of the GNU General Public License as published by the     *
*  Free Software Foundation; either version 2 of the License or any later    *
*  version.								     *
* 									     *
*  This program is distributed in the hope that it will useful, but WITHOUT  *
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     *
*  for more details.							     *
*****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <stdio.h>

using namespace std;

/*
Whilst every effort has been made to ensure the accuracy of the models, their accuracy is not guaranteed.
Finding a reputable paper to source these models from took a while. There was lots of bad copy-pasta out there.
A good paper: http://www.cl.cam.ac.uk/research/dtg/lce-pub/public/vsa23/VTC05_Empirical.pdf
*/

double SUIpathLoss(float f, float TxH, float RxH, float d, int mode)
{
	/*
	   f = Frequency (MHz)
	   TxH =  Transmitter height (m) 
	   RxH = Receiver height (m)
	   d = distance (km)
	   mode 1 = Hilly + trees
	   mode 2 = Flat + trees OR hilly + light foliage
	   mode 3 = Flat + light foliage
	   http://www.cl.cam.ac.uk/research/dtg/lce-pub/public/vsa23/VTC05_Empirical.pdf
	 */
	d = d * 1000;		// km to m
	if (f < 1900 || f > 11000) {
		printf("Error: SUI model frequency range 1.9-11GHz\n");
		exit(EXIT_FAILURE);
	}
	// Terrain mode A is default
	double a = 4.6;
	double b = 0.0075;
	double c = 12.6;
	double s = 10.6;	// Optional fading value
	int XhCF = -10.8;

	if (mode == 2) {
		a = 4.0;
		b = 0.0065;
		c = 17.1;
		s = 6;		// average
	}
	if (mode == 3) {
		a = 3.6;
		b = 0.005;
		c = 20;
		s = 3;		// Optimistic
		XhCF = -20;
	}
	double d0 = 100;
	double A = 20 * log10((4 * M_PI * d0) / (300 / f));
	double y = (a - b * TxH) + c / TxH;
	double Xf = 6 * log10(f / 2000);
	double Xh = XhCF * log10(RxH / 2);
	return A + (10 * y * log10(d / d0)) + Xf + Xh + s;
}

double ECC33pathLoss(float f, float TxH, float RxH, float d, int mode)
{

	if (f < 700 || f > 3500) {
		printf("Error: ECC33 model frequency range 700-3500MHz\n");
		exit(EXIT_FAILURE);
	}
	// MHz to GHz
	f = f / 1000;

	double Gr = 0.759 * RxH - 1.862;	// Big city with tall buildings (1)
	// PL = Afs + Abm - Gb - Gr
	double Afs = 92.4 + 20 * log10(d) + 20 * log10(f);
	double Abm =
	    20.41 + 9.83 * log10(d) + 7.894 * log10(f) +
	    9.56 * (log10(f) * log10(f));
	double Gb = log10(TxH / 200) * (13.958 + 5.8 * (log10(d) * log10(d)));
	if (mode > 1) {		// Medium city (Europe)
		Gr = (42.57 + 13.7 * log10(f)) * (log10(RxH) - 0.585);
	}
	return Afs + Abm - Gb - Gr;
}

double EricssonpathLoss(float f, float TxH, float RxH, float d, int mode)
{
	/*
	   http://research.ijcaonline.org/volume84/number7/pxc3892830.pdf
	   AKA Ericsson 9999 model
	 */
	// Default is Urban which bizarrely has lowest loss
	double a0 = 36.2, a1 = 30.2, a2 = -12, a3 = 0.1;

	if (f < 150 || f > 3500) {
		printf
		    ("Error: Ericsson9999 model frequency range 150-3500MHz\n");
		exit(EXIT_FAILURE);
	}

	if (mode == 2) {	// Suburban / Med loss
		a0 = 43.2;
		a1 = 68.93;
	}
	if (mode == 1) {	// "Rural" but has highest loss according to Ericsson.
		a0 = 45.95;
		a1 = 100.6;
	}
	double g1 = (11.75 * RxH) * (11.75 * RxH);
	double g2 = (44.49 * log10(f)) - 4.78 * ((log10(f) * log10(f)));
	return a0 + a1 * log10(d) + a2 * log10(TxH) +
	    a3 * log10(TxH) * log10(d) - (3.2 * log10(g1)) + g2;
}

/*
int main(int argc, char* argv[]){
	if(argc<5){
		printf("Need freq,TxH,RxH,dist,terr\n");
		return 0;
	}
	int dis, ter;
	double frq, TxH, RxH;
	
	sscanf(argv[1],"%lf",&frq);
	sscanf(argv[2],"%lf",&TxH);
	sscanf(argv[3],"%lf",&RxH);
	sscanf(argv[4],"%d",&dis);
	sscanf(argv[5],"%d",&ter);
	// ALL are freq in MHz and distances in metres
	printf("FSPL: %.2f dB\n",FSPLpathLoss(frq,dis));
	printf("HATA (%d): %.2f dB\n",ter,HATApathLoss(frq,TxH,RxH,dis,ter));
	printf("COST-HATA (%d): %.2f dB\n",ter,COST231pathLoss(frq,TxH,RxH,dis,ter));
	printf("SUI (%d): %.2f dB\n",ter,SUIpathLoss(frq,TxH,RxH,dis,ter));
	printf("ECC33 (%d): %.2f dB\n",ter,ECC33pathLoss(frq,TxH,RxH,dis,ter));
	printf("Ericsson (%d): %.2f dB\n",ter,EricssonpathLoss(frq,TxH,RxH,dis,ter));
}
*/
