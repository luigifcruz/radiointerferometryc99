import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import math

import astropy.constants as const
from astropy.coordinates import ITRS, SkyCoord, AltAz, EarthLocation
from astropy.time import Time,TimeDelta

from phasing import compute_uvw

from guppi import guppi

import toml

ITRF = "/opt/mnt/share/telinfo_ata.toml"
REFANT = "1C"
OBSFREQ = 6500.125*1e6 #Hz
BW = 1.024e9 #Hz

exec(open("/home/sonata/utils/rcparams.py").read())

#Offset_position: RA, Dec = 2.434000 hours, 61.6230 degrees
#W3OH_position:   RA, Dec = 2.451075 hours, 61.8735 degrees

def parse_toml(toml_dict):
    """
    Parse a toml file as a pandas dataframe
    with columns of [x,y,z]
    """
    df = pd.DataFrame()
    df = df.from_dict(toml_dict['antennas'])[['name','position']]
    df.index = np.char.upper(list(df['name']))
    df = df.drop(columns=['name'])

    pos = np.array([i for i in df['position'].values])
    df = df.drop(columns=['position'])
    df['x'] = pos[:,0]
    df['y'] = pos[:,1]
    df['z'] = pos[:,2]
    return df


def create_phasors(delay, nchans=2048, bw=1.024e9):
    chanwidth = bw/nchans
    freqs = np.linspace(0, bw-chanwidth, nchans)

    phasors = np.exp(-1j*2*np.pi*delay*freqs)
    return phasors


def get_fringe_rate(delay, freq=OBSFREQ, bw=BW):
    return np.exp(-1j*2*np.pi*delay * (freq - bw/2.))

def PointsInCircum(r,n=100):
    pi = math.pi
    return [(math.cos(2*pi/n*x)*r,math.sin(2*pi/n*x)*r) 
            for x in range(0,n+1)]


# load in antenna positions
telinfo = toml.load(ITRF)
itrf = parse_toml(telinfo)


# Where I want to phase up, i.e. W3OH
ra  = 2.451075 #hours
dec = 61.87350 #degrees

# Convert to degrees
ra = ra * 360 / 24.

source = SkyCoord(ra, dec, unit='deg')


antnames = ["1C", "1E", "1G", "1H", "1K", "2A", "2B", "2C",
            "2E", "2H", "2J", "2L", "2K", "2M", "4E", "3D",
            "3L", "4J", "5B", "4G"]

itrf_sub = itrf.loc[antnames]
irefant = itrf_sub.index.values.tolist().index(REFANT)

#spectra_n = (hdr['PKTSTART'] + hdr['PKTSTOP']) / 2. 
#t = hdr['SYNCTIME'] + spectra_n * hdr['TBIN']

t = 1649366473 #unix time

ts = Time(t, format='unix')

uvw = compute_uvw(ts, source, 
        itrf_sub[['x','y','z']], itrf_sub[['x','y','z']].values[irefant])

delays = uvw[:,2] / const.c.value
print("Delays [s]:")
for ant, delay in zip(antnames, delays):
    print(ant, delay)


for i, delay in enumerate(delays):
    phasors = create_phasors(delay)
    phasors *= get_fringe_rate(delay)

    plt.plot(np.angle(phasors), ".")
plt.xlabel("Frequency channel number")
plt.ylabel("Phase [rad]")
#also to show the phasors
#plt.show()
