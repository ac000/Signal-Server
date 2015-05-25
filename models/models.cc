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
