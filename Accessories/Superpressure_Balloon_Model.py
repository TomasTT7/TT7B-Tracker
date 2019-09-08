# -*- coding: cp1250 -*-

import numpy as np


# Input
V_b = 0.434              # [m^3]
m_b = 0.116              # [kg]
m_p = 0.011              # [kg]
p_a0 = 101325            # [Pa]
t_a0 = 15                # [°C]
t_g0in = 15              # [°C]
t_g0up = 15              # [°C]
m_fl = 0.0042            # [kg]
st = 16                  # [°C]

# Constants
Rair = 287.05            # J/(kg*K)
Rh = 4124                # J/(kg*K)
Rhe = 2077               # J/(kg*K)
Rg = Rh

P0 = 101325.0            # Pa
T0 = 15.0                # °C
A0 = 340.294             # m/s
G0 = 9.80665             # m/s^2

Ttp = -56.5              # °C
Tsp = -2.5               # °C
Tmp = -86.28             # °C
a = -0.0065              # °C/m
b = 0.001                # °C/m
c = 0.0028               # °C/m
d = -0.0028              # °C/m
e = -0.002               # °C/m
n = 5.2561

maxAlt = 91000
H = np.arange(0,maxAlt,1) # m
T = np.zeros(maxAlt)      # °C
P = np.zeros(maxAlt)      # Pa
Da = np.zeros(maxAlt)     # kg/m^3
Dh = np.zeros(maxAlt)     # kg/m^3
Dhe = np.zeros(maxAlt)    # kg/m^3
G = np.zeros(maxAlt)      # m/s^2


# Standard Atmosphere model
for g in range(0,maxAlt):
    if g == 0:
        G[g] = G0
    else:
        G[g] = G0 * (6371008.8/(6371008.8 + H[g]))**2

for i in range(11000):
    if i == 0:
        T[i] = T0
        P[i] = P0
    else:
        T[i] = T[i-1] + a
        P[i] = P0 * ((T[i] + 273.15) / (T0 + 273.15))**(-G[0] / a / Rair)

for i in range(11000,20000):
    T[i] = Ttp
    P[i] = P[10999] * np.exp(-G[0] / Rair / (Ttp + 273.15) * (H[i] - 10999))

for i in range(20000,32000):
    T[i] = T[i-1] + b
    P[i] = P[19999] * ((T[i] + 273.15) / (Ttp + 273.15))**(-G[0] / b / Rair)

for i in range(32000,47000):
    T[i] = T[i-1] + c
    P[i] = P[31999] * ((T[i] + 273.15) / (T[31999] + 273.15))**(-G[0] / c / Rair)

for i in range(47000,51000):
    T[i] = Tsp
    P[i] = P[46999] * np.exp(-G[0] / Rair / (Tsp + 273.15) * (H[i] - 46999))

for i in range(51000,71000):
    T[i] = T[i-1] + d
    P[i] = P[50999] * ((T[i] + 273.15) / (T[50999] + 273.15))**(-G[0] / d / Rair)

for i in range(71000,84852):
    T[i] = T[i-1] + e
    P[i] = P[70999] * ((T[i] + 273.15) / (T[70999] + 273.15))**(-G[0] / e / Rair)

for i in range(84852,91000):
    T[i] = Tmp
    P[i] = P[84851] * np.exp(-G[0] / Rair / (Tmp + 273.15) * (H[i] - 84851))

for d in range(0,maxAlt):
    Da[d] = P[d] / (Rair * (T[d] + 273.15))
    Dh[d] = P[d] / (Rh * (T[d] + 273.15))
    Dhe[d] = P[d] / (Rhe * (T[d] + 273.15))


# Calculations
rho_a0 = p_a0 / (Rair * (t_a0 + 273.15)) # [kg/m^3]
m_g = ((m_fl + m_b + m_p) * p_a0) / (Rg * (t_g0in + 273.15) * rho_a0 - p_a0) # [kg]
V_g0 = m_g * Rg * (t_g0up + 273.15) / p_a0 # [m^3]
A_0 = (((3 * V_g0) / (4 * np.pi))**(1. / 3))**2 * np.pi # [m^2]
F_lf0 = V_g0 * rho_a0 * G0 # [N]
F_g0 = (m_b + m_p + m_g) * G0 # [N]
dF_0 = F_lf0 - F_g0 # [N]
v_0 = ((2 * dF_0) / (rho_a0 * 0.47 * A_0))**(1. / 2) # [m/s]
rho_s = (m_b + m_p + m_g) / V_b # [kg/m^3]
h = 0 # [m]

for i in range(len(Da)):
    if Da[i] <= rho_s:
        h = i
        break

t_g = T[h] + st
p_g = m_g * Rg *(t_g + 273.15) / V_b # [Pa]
sp = p_g - P[h] # [Pa]


# Output
print("Input")
print("V_b:\t\t{0:.3f} m^3".format(V_b))
print("m_b:\t\t{0:.5f} kg".format(m_b))
print("m_p:\t\t{0:.5f} kg".format(m_p))
print("p_a0:\t\t{0:.0f} Pa".format(p_a0))
print("t_a0:\t\t{0:.1f} °C".format(t_a0))
print("t_g0in:\t\t{0:.1f} °C".format(t_g0in))
print("t_g0up:\t\t{0:.1f} °C".format(t_g0up))
print("m_fl:\t\t{0:.5f} kg".format(m_fl))
print("st:\t\t{0:.1f} °C".format(st))
print("")

print("Output")
print("rho_a0:\t\t{0:.5f} kg/m^3".format(rho_a0))
print("m_g:\t\t{0:.5f} kg".format(m_g))
print("V_g0:\t\t{0:.5f} m^3".format(V_g0))
print("A_0:\t\t{0:.5f} m^2".format(A_0))
print("F_lf0:\t\t{0:.5f} N".format(F_lf0))
print("F_g0:\t\t{0:.5f} N".format(F_g0))
print("dF_0:\t\t{0:.5f} N".format(dF_0))
print("v_0:\t\t{0:.5f} m/s".format(v_0))
print("rho_s:\t\t{0:.5f} kg/m^3".format(rho_s))
print("t_g:\t\t{0:.1f} °C".format(t_g))
print("p_g:\t\t{0:.0f} Pa".format(p_g))
print("sp:\t\t{0:.0f} Pa".format(sp))
print("")

print("Standard Atmosphere model")
print("Altitude:\t\t{} m".format(H[h]))
print("Temperature:\t\t{0:.2f} °C".format(T[h]))
print("Pressure:\t\t{0:.2f} Pa".format(P[h]))
print("Air Density:\t\t{0:.5f} kg/m^3".format(Da[h]))
print("Helium Density:\t\t{0:.5f} kg/m^3".format(Dhe[h]))
print("Hydrogen Density:\t{0:.5f} kg/m^3".format(Dh[h]))
print("Gravity:\t\t{0:.5f} m/s^2".format(G[h]))


