/** Library to read MOPS MITI files

$Id: miti.c 1718 2007-07-27 02:34:35Z denneau $
*/

#include <stdlib.h>
#include <string.h>
#include "miti.h"


/** parse_miti - parse a line of MITI
return zero if no errors occured.
*/
int parse_miti(char *buf, MITI_BLK *blk) {
    char *tok;
    char *last;
    char *tokens[7] = {0, 0, 0, 0, 0, 0, 0};
    int n = 0;

    if (NULL == blk)
        return EMITIBADINPUT;

    tok = strtok_r(buf, " 	", &last);
    while (tok != NULL && n < 7) {
        tokens[n] = tok;
        tok = strtok_r(NULL, " 	", &last);
        n++;
    }

    if (tokens[0]) 
        strncpy(blk->id, tokens[0], MITI_MAX_ID_LEN);
    else
        return EMITIINSUFFARGS;

    if (tokens[1]) 
        blk->epoch_mjd = atof(tokens[1]);
    else
        return EMITIINSUFFARGS;

    if (tokens[2]) 
        blk->ra_deg = atof(tokens[2]);
    else
        return EMITIINSUFFARGS;

    if (tokens[3]) 
        blk->dec_deg = atof(tokens[3]);
    else
        return EMITIINSUFFARGS;

    if (tokens[4]) 
        blk->mag = atof(tokens[4]);
    else
        return EMITIINSUFFARGS;

    if (tokens[5]) 
        strncpy(blk->obscode, tokens[5], 9);
    else
        return EMITIINSUFFARGS;

    if (tokens[6]) 
        strncpy(blk->ground, tokens[6], 19);
    else
        blk->ground[0] = 0;     ///< empty string

    return 0;
}
