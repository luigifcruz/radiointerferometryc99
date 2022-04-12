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

#pragma pack(1)
typedef struct {
    double ha;
    double dec;
} HA_DEC;

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
    {0.64169, 1.079896295},
};

char* names[] = {"1C", "1E", "1G", "1H", "1K", "2A", "2B", "2C", "2E", "2H", "2J", "2L", "2K", "2M", "3D", "3L", "4E", "4G", "4J", "5B"};

static void STANDARD(benchmark::State& state) {
    // Cache metadata.
    size_t n_antennas = sizeof(antennas) / sizeof(XYZ);
    size_t n_beams = sizeof(beams) / sizeof(RA_DEC);

    // Reference Position (Array Center, LLA).
    LLA reference_lla = {
        calc_deg2rad(-121.470733),  // Longitude
        calc_deg2rad(40.815987),    // Latitude
        1020.86                     // Altitude
    };

    // Allocate memory.
    XYZ* receiver_xyz = (XYZ*)malloc(sizeof(UVW) * n_antennas);
    if (receiver_xyz == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    UVW* boresight_uvw = (UVW*)malloc(sizeof(UVW) * n_antennas);
    if (receiver_xyz == NULL) {
        printf("Error allocating memory.\n");
        exit(0);
    }

    UVW* source_uvw = (UVW*)malloc(sizeof(UVW) * n_antennas);
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

    // [Documentation - Phasors Processor] 
    //
    // [Legend]:
    //      - A: Number of Antennas. 
    //      - B: Number of Beams.
    //      - N: Number of Blocks.
    //
    // [Pipeline]:
    // 1. Start with Earth Centered Antenna Positions (ECEF).
    // 2. Translate Earth Centered Antenna Positions (ECEF) to Array Centered Antenna Positions (XYZ).
    //      - Runs on initialization for each antenna (A).
    //      - Based on "calc_position_to_xyz_frame_from_ecef" method.
    //      - Depends on the Array Center Reference Longitude, Latitude, and Altitude values.
    // 3. Rotate Array Centered Antenna Position (XYZ) towards Boresight (UVW).
    //      - Runs on each block for each antenna (A*N). 
    //      - Based on "calc_position_to_uvw_frame_from_xyz" method.
    //      - Depends on the Hour Angle & Declination values of the Boresight. 
    // 4. Calculate time delay on Boresight.
    //      - Runs on each block for each antenna (A*N). 
    //      - Defined by Ti = (Wi - Wr) / C.
    //          - Ti = Time Delay (s) of the signal from Reference Antenna.
    //          - Wi = Distance (m) of the current antenna to the boresight.
    //          - Wr = Distance (m) of the reference antenna to the boresight.
    //          - C  = Speed of Light (m/s).
    //      - Depends on the Array Centered Antenna Position (XYZ) and Hour Angle & Declination of the Boresight.
    // 5. Generate Hour Angle & Declination from RA & Declination according to time.
    //      - Part A runs on each block (N), and Part B runs on each block for every beam (B*N).
    //      - Based on "calc_ha_dec_rad_a" (Part A) and "calc_ha_dec_rad_b" (Part B) methods. 
    //      - Depends on the RA & Declination values of the Source.  
    // 6. Rotate Array Centered Antenna Position (XYZ) towards Source (UVW).
    //      - Runs on each block for each antenna for every beam (A*B*N). 
    //      - Based on "calc_position_to_uvw_frame_from_xyz" method.
    //      - Depends on the Hour Angle & Declination values of the Source. 
    // 7. Calculate time delay on Source.
    //      - Runs on each block for each antenna for every beam (A*B*N).
    //      - Defined by TPi = Ti - ((WPi - WPr) / C).
    //          - TPi = Time Delay (s) from Boresight to Source.
    //          - Ti = Time Delay (s) of the signal from Reference Antenna.
    //          - WPi = Distance (m) of the current antenna to the signal source.
    //          - WPr = Distance (m) of the reference antenna to the signal source.
    //          - C  = Speed of Light (m/s).

    // Translate Antenna Position (LLA) to Reference Position (XYZ).
    for (size_t i = 0; i < n_antennas; i++) {
        receiver_xyz[i].X = antennas[i].X - antennas[0].X;
        receiver_xyz[i].Y = antennas[i].Y - antennas[0].Y;
        receiver_xyz[i].Z = antennas[i].Z - antennas[0].Z;
    }

    for (auto _ : state) {
        // Boresight Position.
        HA_DEC boresight_ha_dec = {0.0, 0.0};

        calc_ha_dec_rad(
            beams[0].ra,
            beams[0].dec,
            reference_lla.lon,
            reference_lla.lat,
            reference_lla.alt,
            (1649366473.0/ 86400) + 2440587.5,
            0.0,
            &boresight_ha_dec.ha,
            &boresight_ha_dec.dec
        );

        // Copy Reference Position (XYZ) to Boresight Position (UVW).
        memcpy(
            (double*)boresight_uvw,
            receiver_xyz,
            sizeof(antennas)
        );

        // Rotate Reference Position (UVW) towards Boresight (HA, Dec).
        calc_position_to_uvw_frame_from_xyz(
            (double*)boresight_uvw,
            n_antennas,
            boresight_ha_dec.ha,
            boresight_ha_dec.dec,
            reference_lla.lon
        );

        // Calculate delay for boresight (Ti = (Wi - Wr) / C).
        printf("HA: %lf, DEC: %lf\n", boresight_ha_dec.ha, boresight_ha_dec.dec);
        for (size_t i = 0; i < n_antennas; i++) {
            printf("%s: U: %lf V: %lf W: %lf\n", names[i], boresight_uvw[i].U, boresight_uvw[i].V, boresight_uvw[i].W);
        }

        // Convert RA to Hour Angle (Part A).
        eraASTROM astrom;

        calc_ha_dec_rad_a(
            reference_lla.lon,
            reference_lla.lat,
            reference_lla.alt,
            2400000.5,
            0.3,
            &astrom
        );

        for (size_t j = 0; j < n_beams; j++) {
            // Copy Reference Position (XYZ) to Target Position (UVW).
            memcpy(
                source_uvw,
                receiver_xyz,
                sizeof(antennas)
            );

            // Convert RA to Hour Angle (Part B).
            HA_DEC target_ha_dec = {0.0, 0.0};

            calc_ha_dec_rad_b(
                beams[j].ra,
                beams[j].dec,
                &astrom, 
                &target_ha_dec.ha,
                &target_ha_dec.dec
            ); 

            // Rotate Reference Position (UVW) towards Target (HA, Dec).
            calc_position_to_uvw_frame_from_xyz(
                (double*)source_uvw,
                n_antennas,
                target_ha_dec.ha, 
                target_ha_dec.dec,
                reference_lla.lon
            );

            // Calculate delay for off-center source and subtract from boresight (TPi = Ti - ((WPi - WPr) / C)).
            for (size_t i = 0; i < n_antennas; i++) {
                dt[(j * n_beams) + i] = t[i] - ((source_uvw[i].W - source_uvw[0].W) / C); 
            }
        }
    }

    free(receiver_xyz);
    free(boresight_uvw);
    free(source_uvw);
    free(t);
    free(dt);
}

//BENCHMARK(STANDARD)->Unit(benchmark::TimeUnit::kMillisecond)->Iterations(10000);
BENCHMARK(STANDARD)->Unit(benchmark::TimeUnit::kMillisecond)->Iterations(1);

BENCHMARK_MAIN();
