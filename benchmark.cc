#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <benchmark/benchmark.h>

extern "C" {
#include "radiointerferometryc99.h"
}

#define RADIANS(deg) ((deg) * M_PI / 180.0)

static void STANDARD(benchmark::State& state) {
    double lla[3];
    double xyz[3]; 
    double uvw[3];

    lla[0] = RADIANS(-121.47073316425822);  // Longitude
    lla[1] = RADIANS(+40.815986729567);     // Latitude
    lla[2] = +1020.8551;                    // Altitude

    // Antenna LLA to ECEF
    calc_position_to_xyz_frame_from_ecef((double*)&xyz, 1, lla[0], lla[1], lla[2]);

    for (auto _ : state) {
        // Copy Antenna XYZ to UVW buffer.
        memcpy(&uvw, &xyz, sizeof(double) * 3);

        // Rotate Antenna XYZ towards the Source. 
        calc_position_to_uvw_frame_from_xyz((double*)&uvw, 1, 6.0, 2.0, lla[0]);
    }
}
BENCHMARK(STANDARD)->Iterations(2 << 29);

BENCHMARK_MAIN();
