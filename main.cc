#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <benchmark/benchmark.h>

extern "C" {
#include "radiointerferometryc99.h"
}

#define C (double)299792458.0 // Speed of Light (m/s)

#pragma pack(1)
typedef struct {
    double lon;
    double lat;
    double alt;
} LLA;

#pragma pack(1)
typedef struct {
    double X;
    double Y;
    double Z;
} XYZ;

#pragma pack(1)
typedef struct {
    double U;
    double V;
    double W;
} UVW;

#pragma pack(1)
typedef struct {
    double ra;
    double dec;
} RA_DEC;

static XYZ antennas[] = {
    {-2524041.5388905862, -4123587.965024342, 4147646.4222955606},    // 1c 
    {-2524068.187873109, -4123558.735413135, 4147656.21282186},       // 1e 
    {-2524087.2078100787, -4123532.397416349, 4147670.9866770394},    // 1g 
    {-2524103.384010733, -4123511.111598937, 4147682.4133068994},     // 1h 
    {-2524056.730228759, -4123515.287949227, 4147706.4850287656},     // 1k 
    {-2523986.279601761, -4123497.427940991, 4147766.732988923},      // 2a 
    {-2523970.301363642, -4123515.238502669, 4147758.790023165},      // 2b 
    {-2523983.5419911123, -4123528.1422073604, 4147737.872218138},    // 2c 
    {-2523941.5221860334, -4123568.125040547, 4147723.8292249846},    // 2e 
    {-2524074.096220788, -4123468.5182652213, 4147742.0422435375},    // 2h 
    {-2524058.6409591637, -4123466.5112451194, 4147753.4513993543},   // 2j 
    {-2524026.989692545, -4123480.9405167866, 4147758.2356800516},    // 2l 
    {-2524048.5254066754, -4123468.3463909747, 4147757.835369889},    // 2k 
    {-2524000.5641107005, -4123498.2984570004, 4147756.815976133},    // 2m 
    {-2523945.086670364, -4123480.3638816103, 4147808.127865142},     // 3d 
    {-2523950.6822576034, -4123444.7023326857, 4147839.7474427638},   // 3l 
    {-2523880.869769226, -4123514.3375464156, 4147813.413426994},     // 4e 
    {-2523930.3747946257, -4123454.3080821196, 4147842.6449955846},   // 4g 
    {-2523898.1150373477, -4123456.314794732, 4147860.3045849088},    // 4j 
    {-2523824.598229116, -4123527.93080514, 4147833.98936114}         // 5b
};

static RA_DEC beams[] = {
    {0.0, 1.0},
    {1.0, 0.0},
    {2.0, 1.0},
    {4.0, 0.0},
    {5.0, 1.0},
    {0.0, 1.0},
    {1.0, 0.0},
    {2.0, 1.0},
    {4.0, 0.0},
    {5.0, 1.0},
    {0.0, 1.0},
    {1.0, 0.0},
    {2.0, 1.0},
    {4.0, 0.0},
    {5.0, 1.0},
    {0.0, 1.0},
    {1.0, 0.0},
    {2.0, 1.0},
    {4.0, 0.0},
    {5.0, 1.0},
};

static void STANDARD(benchmark::State& state) {
    // Cache metadata.
    size_t n_antennas = sizeof(antennas) / sizeof(XYZ);
    size_t n_beams = sizeof(beams) / sizeof(RA_DEC);

    // Reference Position (Array Center, LLA).
    LLA reference_pos = {
        calc_deg2rad(-121.470736),  // Longitude
        calc_deg2rad(40.817431),    // Latitude
        1019.222                    // Altitude
    };

    // Allocate memory.
    double* receiver_xyz = (double*)malloc(sizeof(antennas));
    if (receiver_xyz == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }
    memcpy(receiver_xyz, antennas, sizeof(antennas));

    double* boresight_uvw = (double*)malloc(sizeof(antennas));
    if (receiver_xyz == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    double* source_uvw = (double*)malloc(sizeof(antennas));
    if (receiver_xyz == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    double* t = (double*)malloc(sizeof(double) * n_antennas);
    if (t == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    double* dt = (double*)malloc(sizeof(double) * (n_antennas * n_beams));
    if (dt == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    for (auto _ : state) {
        // Translate Antenna Position (LLA) to Reference Position (XYZ).
        calc_position_to_xyz_frame_from_ecef(
            receiver_xyz,
            n_antennas,
            reference_pos.lon,
            reference_pos.lat,
            reference_pos.alt
        );

        // Copy Reference Position (XYZ) to Boresight Position (UVW).
        memcpy(
            boresight_uvw,
            receiver_xyz,
            sizeof(antennas)
        );

        // Boresight Position.
        double boresight_hour_angle_rad = 0.0;
        double boresight_declination_rad = 0.0;

        // Rotate Reference Position (UVW) towards Boresight (HA, Dec).
        calc_position_to_uvw_frame_from_xyz(
            boresight_uvw,
            n_antennas,
            boresight_hour_angle_rad,
            boresight_declination_rad,
            reference_pos.lon
        );

        // Calculate delay for boresight (Ti = (Wi - Wr) / C).
        for (size_t i = 0; i < n_antennas; i++) {
            t[i] = (((UVW*)boresight_uvw)[i].W - ((UVW*)boresight_uvw)[0].W) / C; 
        }

        for (size_t j = 0; j < n_beams; j++) {
            // Copy Reference Position (XYZ) to Target Position (UVW).
            memcpy(
                source_uvw,
                receiver_xyz,
                sizeof(antennas)
            );

            // Convert RA to Hour Angle.
            double hour_angle_rad = 0.0;
            double declination_rad = 0.0;

            calc_ha_dec_rad(
                beams[j].ra,
                beams[j].dec,
                reference_pos.lon,
                reference_pos.lat,
                reference_pos.alt,
                2400000.5,
                0.3,
                &hour_angle_rad,
                &declination_rad
            );

            // Rotate Reference Position (UVW) towards Target (HA, Dec).
            calc_position_to_uvw_frame_from_xyz(
                source_uvw,
                n_antennas,
                hour_angle_rad, 
                declination_rad,
                reference_pos.lon
            );

            // Calculate delay for off-center source and subtract from boresight (TPi = Ti - ((WPi - WPr) / C)).
            for (size_t i = 0; i < n_antennas; i++) {
                dt[(j * n_beams) + i] = t[i] - ((((UVW*)source_uvw)[i].W - ((UVW*)source_uvw)[0].W) / C); 
            }
        }
    }
}
BENCHMARK(STANDARD)->Unit(benchmark::TimeUnit::kMillisecond);

BENCHMARK_MAIN();
