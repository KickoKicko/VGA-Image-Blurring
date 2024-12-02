#include "gaussianMatrices.h"

int staticGaussianKernel0(int *filterMatrix)
{
    filterMatrix[0] = 1;
    return 1;
}
int staticGaussianKernel1(int *filterMatrix)
{
    filterMatrix[0] = 1;
    filterMatrix[1] = 2;
    filterMatrix[2] = 1;
    filterMatrix[3] = 2;
    filterMatrix[4] = 4;
    filterMatrix[5] = 2;
    filterMatrix[6] = 1;
    filterMatrix[7] = 2;
    filterMatrix[8] = 1;
    return 16;
}

int staticGaussianKernel2(int *filterMatrix)
{
    filterMatrix[0] = 1;
    filterMatrix[1] = 8;
    filterMatrix[2] = 17;
    filterMatrix[3] = 8;
    filterMatrix[4] = 1;
    filterMatrix[5] = 8;
    filterMatrix[6] = 70;
    filterMatrix[7] = 141;
    filterMatrix[8] = 70;
    filterMatrix[9] = 8;
    filterMatrix[10] = 17;
    filterMatrix[11] = 141;
    filterMatrix[12] = 286;
    filterMatrix[13] = 141;
    filterMatrix[14] = 17;
    filterMatrix[15] = 8;
    filterMatrix[16] = 70;
    filterMatrix[17] = 141;
    filterMatrix[18] = 70;
    filterMatrix[19] = 8;
    filterMatrix[20] = 1;
    filterMatrix[21] = 8;
    filterMatrix[22] = 17;
    filterMatrix[23] = 8;
    filterMatrix[24] = 1;
    return 1266;
}

int staticGaussianKernel3(int *filterMatrix)
{
    filterMatrix[0] = 1;
    filterMatrix[1] = 34;
    filterMatrix[2] = 286;
    filterMatrix[3] = 581;
    filterMatrix[4] = 286;
    filterMatrix[5] = 34;
    filterMatrix[6] = 1;
    filterMatrix[7] = 34;
    filterMatrix[8] = 1177;
    filterMatrix[9] = 9822;
    filterMatrix[10] = 19920;
    filterMatrix[11] = 9822;
    filterMatrix[12] = 1177;
    filterMatrix[13] = 34;
    filterMatrix[14] = 286;
    filterMatrix[15] = 9822;
    filterMatrix[16] = 81937;
    filterMatrix[17] = 166178;
    filterMatrix[18] = 81937;
    filterMatrix[19] = 9822;
    filterMatrix[20] = 286;
    filterMatrix[21] = 581;
    filterMatrix[22] = 19920;
    filterMatrix[23] = 166178;
    filterMatrix[24] = 337028;
    filterMatrix[25] = 166178;
    filterMatrix[26] = 19920;
    filterMatrix[27] = 581;
    filterMatrix[28] = 286;
    filterMatrix[29] = 9822;
    filterMatrix[30] = 81937;
    filterMatrix[31] = 166178;
    filterMatrix[32] = 81937;
    filterMatrix[33] = 9822;
    filterMatrix[34] = 286;
    filterMatrix[35] = 34;
    filterMatrix[36] = 1177;
    filterMatrix[37] = 9822;
    filterMatrix[38] = 19920;
    filterMatrix[39] = 9822;
    filterMatrix[40] = 1177;
    filterMatrix[41] = 34;
    filterMatrix[42] = 1;
    filterMatrix[43] = 34;
    filterMatrix[44] = 286;
    filterMatrix[45] = 581;
    filterMatrix[46] = 286;
    filterMatrix[47] = 34;
    filterMatrix[48] = 1;
    return 1497340;
}