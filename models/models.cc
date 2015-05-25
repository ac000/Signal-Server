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
