#ifndef __GAMMALINEAR_H
#define	__GAMMALINEAR_H

#include "new_gamma_def.h"
#include "app_gamma.h"


int getRGB2XYZMatrix(double outMatrix[3][4], double RGBXYZ[][6], int n);
int getRGBFromPoints(double outRGB[3], double targetXYZ[3], double RGBXYZ[][6], int n, int logFlag,sky_comDriver *dev);


#endif
