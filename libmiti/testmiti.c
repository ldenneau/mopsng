#include <stdio.h>
#include "miti.h"

int main(void) {

    char s[] = "L000003 53377.645014    216.754518      -16.716154      22.525528       568     S1006id";
    MITI_BLK m;

    parse_miti(s, &m);

    printf("%s %f %f %f %f %s %s\n",
        m.id,

        m.epoch_mjd,
        m.ra_deg,
        m.dec_deg,
        m.mag,

        m.obscode,
        m.ground
    );

    return 0;
}
