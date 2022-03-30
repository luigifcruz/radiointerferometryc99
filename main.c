#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RADIANS(deg) ((deg) * M_PI / 180.0)

#include "radiointerferometryc99.h"

int main() {
    printf("Benchmark!\n");

    double lla[3];
    double xyz[3]; 
    double uvw[3];

    lla[0] = RADIANS(-121.47073316425822);  // Longitude
    lla[1] = RADIANS(+40.815986729567);     // Latitude
    lla[2] = +1020.8551;                    // Altitude

    // Antenna - ECEF to XYZ
    calc_position_to_xyz_frame_from_ecef((double*)&xyz, 1, lla[0], lla[1], lla[2]);

    // Copy Antenna XYZ to UVW buffer.
    memcpy(&uvw, &xyz, sizeof(double) * 3);

    // Rotate Antenna XYZ towards the Source. 
    calc_position_to_uvw_frame_from_xyz((double*)&uvw, 1, 0.0, 0.0, lla[0]);

    printf("XYZ\n");
    printf("X: %lf\n", xyz[0]);
    printf("Y: %lf\n", xyz[1]);
    printf("Z: %lf\n", xyz[2]);

    printf("UVW\n");
    printf("U: %lf\n", uvw[0]);
    printf("V: %lf\n", uvw[1]);
    printf("W: %lf\n", uvw[2]);

    printf("End\n");

    return 0;
}
