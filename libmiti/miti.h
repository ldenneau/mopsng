/** Library for reading MOPS MITI files
*/

#ifndef __MOPS_MITI__
#define __MOPS_MITI__

#ifdef __cplusplus
extern "C" {
#endif


#define MITI_OBSCODE_LEN 9
#define MITI_GROUND_LEN  19
#define MITI_MAX_ID_LEN 99

typedef struct {
    char id[MITI_MAX_ID_LEN + 1];       ///< MITI observation/detection/orbit ID
    double epoch_mjd;   ///< epoch of observation, MJD
    double ra_deg;      ///< RA of observation, in degrees
    double dec_deg;     ///< declination of observation, in degrees
    double mag;         ///< apparent magnitude of observation
    char obscode[MITI_OBSCODE_LEN + 1];   ///< MPC observatory code
    char ground[MITI_GROUND_LEN + 1];    ///< "ground-truth" identifier
} MITI_BLK;


int parse_miti(char *buf, MITI_BLK *blk);     ///< parse line into MITI parts

enum {
    EMITIBADINPUT = 1,
    EMITIINSUFFARGS
};

#ifdef __cplusplus
}
#endif

#endif
