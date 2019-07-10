import numpy as np
import matplotlib.pyplot as plt
from scipy import misc
import cahnhilliard as ch

def compute_dimensionless_ch_params_from_polymer_params( L_kuhn , m , X , N , L_omega ):
    m_scaled = (1 - m) / 2.
    eps_2    = L_kuhn**2 / ( 3 * m_scaled * (1 - m_scaled) * X * L_omega**2 )
    sigma    = 36 * L_omega**2 / ( m_scaled**2 * (1 - m_scaled)**2 * L_kuhn**2 * X * N**2 )
    return eps_2 , sigma

# ********* POLYMER PARAMETERS *********
X        =      np.mean([ 0.08 , 0.3 ])
N        =      np.mean([ 200  , 2000])
L_repeat =      (10**-9) * np.mean([ 20   , 80  ]) # meters
n_repeat = 15
L_omega  = n_repeat * L_repeat
L_kuhn   =      (10**-9) * np.mean([ 0.5 , 3.0  ]) # meters
m        = 0.0
# **************************************

eps2_base , sigma_base = compute_dimensionless_ch_params_from_polymer_params( L_kuhn , m , X , N , L_omega )
print( eps2_base**0.5 , sigma_base )

chparams          = ch.CHparamsVector();
info              = ch.SimInfo();

# *********** INPUTS ***********
info.t0       = 0.0
info.nx       = 192
info.ny       = 192
info.dx       = 1./info.nx
info.dy       = 1./info.ny
info.bc       = 'neumann'

eps_2        = eps2_base
sigma        = sigma_base
b            = 1.0
u            = 1.0
sigma_noise  = 0.0

# Set up grid for spatial-field quantities
nx                = int(info.nx)
xx,yy             = np.meshgrid( np.arange(0,1,1/info.nx), np.arange(0,1,1/info.nx) )

chparams.eps_2        = ch.DoubleVector(eps_2  * np.ones(nx**2))
chparams.b            = ch.DoubleVector(b      * np.ones(nx**2))
chparams.u            = ch.DoubleVector(u      * np.ones(nx**2))
chparams.sigma        = ch.DoubleVector(sigma  * np.ones(nx**2))
chparams.m            = ch.DoubleVector(m      * np.ones(nx**2))
chparams.sigma_noise  = sigma_noise

n_dt = 500
# ******************************

# Define timescales
biharm_dt         = (info.dx**4) / np.max(chparams.eps_2)
diff_dt           = (info.dx**2) / np.max( [np.max(chparams.u) , np.max(chparams.b)] )
lin_dt            = 1.0 / np.max(chparams.sigma)

# Reset from saved state
n_tsteps          = 25
info.t0           = 0
stiff_dt          = np.min([ biharm_dt , diff_dt , lin_dt ])
t                 = np.linspace(info.t0 , info.t0 + n_dt * stiff_dt , n_tsteps+1)

chparams.dt_check = t[1]-t[0]

# Run solver
print( 'Biharmonic timescale dt_biharm = ' , biharm_dt )
print( 'Diffusion timescale dt_diff = ' , diff_dt , ' = ' , diff_dt/biharm_dt , ' dt_biharm')
print( 'Linear timescale dt_lin = ' , lin_dt , ' = ' , lin_dt/biharm_dt , ' dt_biharm')
print( 'Sampling interval = ' , chparams.dt_check / stiff_dt , ' dt_stiff' )

for i in range(n_tsteps):
    info.t0        = t[i]
    info.tf        = t[i+1]
    rhs            = ch.CahnHilliard2DRHS(chparams , info)
    print( 't0 = ', t[i]/lin_dt, ' dt_lin , tf = ', t[i+1]/lin_dt, ' dt_lin')
    ch.run_ch_vector_nonthermal(chparams,info,rhs);