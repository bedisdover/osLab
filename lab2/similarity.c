#include <stdio.h>
#include <string.h>
#include <math.h>
#include "similarity.h"

int min(int a, int b) {
    int result = a;
    result = result < b ? result : b;

    return result;
}

int CalculateStringDistance(const char *strA, int pABegin, int pAEnd, const char *strB,
                            int pBBegin, int pBEnd) {
    if (pABegin > pAEnd) {
        if (pBBegin > pBEnd)
            return 0;
        else
            return pBEnd - pBBegin + 1;
    }
    if (pBBegin > pBEnd) {
        if (pABegin > pAEnd)
            return 0;
        else
            return pAEnd - pABegin + 1;
    }
    if (strA[pABegin] == strB[pBBegin]) {
        return CalculateStringDistance(strA, pABegin + 1, pAEnd, strB, pBBegin + 1, pBEnd);
    }
    else {
        int t1 = CalculateStringDistance(strA, pABegin + 1, pAEnd, strB,
                                         pBBegin + 2, pBEnd);
        int t2 = CalculateStringDistance(strA, pABegin + 2, pAEnd, strB,
                                         pBBegin + 1, pBEnd);
        int t3 = CalculateStringDistance(strA, pABegin + 2, pAEnd, strB,
                                         pBBegin + 2, pBEnd);
        return min(t1, min(t2, t3)) + 1;
    }
}

//获得相似度
int getSimilarity(const char *source, const char *target) {
    return CalculateStringDistance(source, 0, strlen(source), target, 0, strlen(target));
}
