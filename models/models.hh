#ifndef _MODELS_HH_
#define _MODELS_HH_

double HATApathLoss(float f, float TxH, float RxH, float d, int mode);
double COST231pathLoss(float f, float TxH, float RxH, float d, int mode);
double FSPLpathLoss(float f, float d);
double SUIpathLoss(float f, float txH, float rxH, float d, int terrain);
double ECC33pathLoss(float f, float TxH, float RxH, float d, int mode);
double EricssonpathLoss(float f, float TxH, float RxH, float d, int mode);

#endif /* _MODELS_HH_ */
