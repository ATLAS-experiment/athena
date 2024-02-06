#!/bin/env python                                                                                                                                              

import os, sys, math

def tshift_poly(dt, p0, p1, p2, p3):
    p0n = p0 + p1 * dt + p2 * dt * dt + p3 * dt * dt * dt
    p1n = p1 + 2 * p2 * dt + 3 * p3 * dt * dt
    p2n = p2 + 3 * p3 * dt
    p3n = p3
    return p0n, p1n, p2n, p3n


def p3root(a, b, c, d):

    pi = 3.14159265359
    p = (3*a*c-b*b)/(3*a*a)
    q = (2*b*b*b-9*a*b*c+27*a*a*d)/(27*a*a*a)
    r = 18.0
    offset = -b/(3*a)
    discriminant = 4*p*p*p+27*q*q
    print('in p3root3')
    print('p: ',p,' q: ',q,' disc: ', discriminant, ' offset: ', offset)
    if discriminant > 0:
        v = math.sqrt(q*q/4 + p*p*p/27 )
        r = pow(-q/2 + v,0.3333333)+pow(-q/2-v,0.3333333)+offset
    if discriminant < 0 and p < 0:
        cphi = (3.0/2.0)*(q/p)*math.sqrt(-3/p)
        phi  = math.acos(cphi)
        z1 = 2*math.sqrt(-p/3)*math.cos(phi/3) + offset
        z2 = 2*math.sqrt(-p/3)*math.cos((phi+2*pi)/3) + offset
        z3 = 2*math.sqrt(-p/3)*math.cos((phi+4*pi)/3) + offset
        if abs(z1)<abs(z2) and abs(z1)<abs(z3):
            r = z1
        elif abs(z2)<abs(z3):
            r = z2
        else:
            r = z3

    return r

a=-3.271641e-06
b=-5.354605e-04
c=8.187117e-02
d=-2.81115e-01

r_orig = p3root(a, b, c, d - 1.0)
t=r_orig
pvr = a*t*t*t + b*t*t + c*t + d -1.0
t=18.0
pv18 = a*t*t*t + b*t*t + c*t + d -1.0
print(' root is: ', r_orig,' value at root: ', pvr,' value at 18 ', pv18)
shiftval = ( r_orig - 18 ) # calculate first shift along t so that r(18)=1
p0n, p1n, p2n, p3n = tshift_poly(shiftval, d, c, b, a)
r_new = p3root(p3n, p2n, p1n, p0n)
t=r_new
pvr = p3n*t*t*t + p2n*t*t + p1n*t + p0n -1.0
t=18.0
pv18 = p3n*t*t*t + p2n*t*t + p1n*t + p0n -1.0
print(' new root is: ', r_new,' value at root: ', pvr,' value at 18 ', pv18)


 
