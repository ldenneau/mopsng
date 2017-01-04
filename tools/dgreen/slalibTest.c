/* To build this file enter  gcc -Wall slalibTest.c ./libsla.a -o slalibTest */
/* while in the directory containing the slalib source code.                 */
#include "slalib.h"
#include "slamac.h"
#include <stdio.h>

void main(void) {

    double mjdArray[6] = {56109.0, 54832.0, 53736.0, 51179.0, 50630.0, 50083.0};
    float tai_utc[6] = {35, 34, 33, 32, 31, 30};
    float result = 0;
    int x;
    
    for (x=0; x < sizeof(mjdArray)/sizeof(*mjdArray); x++) {
        result = slaDat(mjdArray[x]);
        if (result != tai_utc[x]) {
            printf("Time returned for mjd %f is incorrect!\n", mjdArray[x]);
            printf("Expected %f seconds got %f seconds.\n\n", tai_utc[x], result);
        } else {
            printf("Time of %f seconds returned for mjd %f is correct.\n\n", result, mjdArray[x]);
        }
    }
    printf("Test of sladat complete.\n");
}
