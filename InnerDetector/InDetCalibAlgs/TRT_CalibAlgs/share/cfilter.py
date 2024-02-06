#!/bin/env python

import os,sys, math

def tshift_poly(dt,p0,p1,p2,p3):
    p0n = p0 + p1*dt + p2*dt*dt + p3*dt*dt*dt
    p1n = p1 + 2*p2*dt + 3*p3*dt*dt
    p2n = p2 + 3*p3*dt
    p3n = p3
    return p0n,p1n,p2n,p3n

def p3root(a, b, c, d):              # this is to replace the numpy root finder, should numpy not be avilable

    r = 18.0   
    pi = 3.14159265359
    if a*a < 1.0e-48 :
        return r
    p = (3*a*c-b*b)/(3*a*a)
    q = (2*b*b*b-9*a*b*c+27*a*a*d)/(27*a*a*a)

    offset = -b/(3*a)
    discriminant = 4*p*p*p+27*q*q
    if discriminant > 0:
        v = math.sqrt(q*q/4 + p*p*p/27 )
        if (-q/2 + v) >0 and (-q/2-v) > 0 :
            r = pow(-q/2 + v,0.3333333)+pow(-q/2-v,0.3333333)+offset
    if discriminant < 0 and p < 0:
        cphi = (3.0/2.0)*(q/p)*math.sqrt(-3/p)
        phi  = math.acos(cphi)
        z1 = 2*math.sqrt(-p/3)*math.cos(phi/3) + offset
        z2 = 2*math.sqrt(-p/3)*math.cos((phi+2*pi)/3) + offset
        z3 = 2*math.sqrt(-p/3)*math.cos((phi+4*pi)/3) + offset
        if abs(z1-18)<abs(z2-18) and abs(z1-18)<abs(z3-18):
            r = z1
        elif abs(z2-18)<abs(z3-18):
            r = z2
        else:
            r = z3

    return r

keeprt=False
keept0=False
shiftrt=False
shiftt0=False
do_global=False

if sys.argv[1]=="t0shift":
    do_global=True
    keeprt=True
    keept0=True
    shiftt0=True
    Dt0=float(sys.argv[2])
    olddbs=open(sys.argv[3]).readlines()
    newdbs=open(sys.argv[3]).readlines()
else:
    olddbs=open(sys.argv[1]).readlines()
    newdbs=open(sys.argv[2]).readlines()

    if len(sys.argv)>4:
        olddicts=open(sys.argv[3]).readlines()
        keeprt=sys.argv[4].find("keeprt")>=0
        keept0=sys.argv[4].find("keept0")>=0
        shiftrt=sys.argv[4].find("shiftrt")>=0
        shiftt0=sys.argv[4].find("shiftt0")>=0
        if keeprt:
            print ("cfilter called with 4 arguments. Option keeprt")
        if keept0:
            print ("cfilter called with 4 arguments. Option keept0")
        if shiftrt:
            print ("cfilter called with 4 arguments. Option shiftrt")
        if shiftt0:
            print ("cfilter called with 4 arguments. Option shiftt0")
    else:
        print ("cfilter WARNING: No oldt0s given. The two first arguments must be calibout.txt calibout.txt.")
        keeprt=sys.argv[3].find("keeprt")>=0
        keept0=sys.argv[3].find("keept0")>=0
        shiftrt=sys.argv[3].find("shiftrt")>=0
        shiftt0=sys.argv[3].find("shiftt0")>=0



oldrts=[]
oldt0s=[]
olderrors=[]
for olddb in olddbs:
    if not olddb.find('#')==0:
        if len(olddb.split(":")[-1].split())>10: 
            olderrors.append(olddb.strip())
        elif len(olddb.split(":")[-1].split())>4: 
            oldrts.append(olddb.strip())
        else:
            oldt0s.append(olddb.strip())

newrts		=[]
newt0s		=[]
newerrors	=[]
for newdb in newdbs:
    if not newdb.find('#')==0:
        if len(newdb.split(":")[-1].split())>10: 
            newerrors.append(newdb.strip())
        elif len(newdb.split(":")[-1].split())>4: 
            newrts.append(newdb.strip())
        else:
            newt0s.append(newdb.strip())

rts=newrts
t0s=newt0s
if keeprt: rts=oldrts
if keept0: t0s=oldt0s

#for rt in rts:
#    print (rt)

#rtfix=[0,0]
#rtfix=[1,17.7]
rtfix=[1,18]
rtconsts={}
rtshifts={}
nprint = 0
for rt in rts:
    #print (rt.strip())
    rtkeytokens=rt.strip().split(':')[0].strip().split()
    rtdbkey="%s %s %s %s %s"%(rtkeytokens[0],rtkeytokens[1],rtkeytokens[2],rtkeytokens[3],rtkeytokens[4])
    rtdictkey="_%s_%s_%s_%s_%s"%(rtkeytokens[0],rtkeytokens[1],rtkeytokens[2],rtkeytokens[3],rtkeytokens[4])

    p0=float(rt.strip().split(':')[-1].split()[1].strip())
    p1=float(rt.strip().split(':')[-1].split()[2].strip())
    p2=float(rt.strip().split(':')[-1].split()[3].strip())
    p3=float(rt.strip().split(':')[-1].split()[4].strip())

    # create polynomial shifted to get the t where r(t)=1
    # find root (replacing the numpy root finder)
    poly_orig = [p3, p2, p1, p0 - rtfix[0]]
    r_orig = p3root(p3, p2, p1, p0 - rtfix[0])
    shiftval = ( r_orig - rtfix[1] ) # calculate first shift along t so that r(18)=1

    p0n, p1n, p2n, p3n = tshift_poly(shiftval, p0, p1, p2, p3)
    poly_new = [p3n, p2n, p1n, p0n]
    r_new = p3root(p3n, p2n, p1n, p0n)

    shiftvaln = 0.0 #poly_new.r[2] #calculate second shift along t so that r(0)=0 
    p0nn, p1nn, p2nn, p3nn = tshift_poly(shiftvaln, p0n, p1n, p2n, p3n)
    poly_newnew = [p3nn, p2nn, p1nn, p0nn]
    # end replace numpy

    rtconsts[rtdbkey]=[]
    if shiftrt:
        if nprint<10 :
            print ("%16s ... original polynomial: r=%.1f mm @ %f ns, shifting %f ns in t, new polynomial: r(%.1f ns) = %f mm, dt0 = %f" % (
                rtdbkey,
                rtfix[0],
                r_orig,
                shiftval,
                rtfix[1],
                p3nn*rtfix[1]*rtfix[1]*rtfix[1] + p2nn*rtfix[1]*rtfix[1] + p1nn*rtfix[1] + p0nn,
                shiftvaln
                ))
            print ("original polynomial: ",p0,p1,p2,p3)
            print ("new polynomial:      ",p0nn,p1nn,p2nn,p3nn)
            nprint = nprint +1
        #rtconsts[rtdbkey].append(0.0)
        rtconsts[rtdbkey].append(p0nn)
        rtconsts[rtdbkey].append(p1nn)
        rtconsts[rtdbkey].append(p2nn)
        rtconsts[rtdbkey].append(p3nn)
        rtshifts[rtkeytokens[0]]=shiftvaln #only compensate t0s for the second shift
    else:
        rtconsts[rtdbkey].append(p0)
        rtconsts[rtdbkey].append(p1)
        rtconsts[rtdbkey].append(p2)
        rtconsts[rtdbkey].append(p3)
        rtshifts[rtkeytokens[0]]=p0
        
t0consts={}
for t0 in t0s:
    t0keytokens=t0.strip().split(':')[0].strip().split()
    t0dbkey="%s %s %s %s %s"%(t0keytokens[0],t0keytokens[1],t0keytokens[2],t0keytokens[3],t0keytokens[4])
    t0consts[t0dbkey]=[]
    t0consts[t0dbkey].append(float(t0.strip().split(':')[-1].split()[0].strip()))
    t0consts[t0dbkey].append(float(t0.strip().split(':')[-1].split()[1].strip()))


#shift rts
dbrtouts=[]
for part in rtconsts:
    dbrtouts.append("%s : 0 %e %e %e %e"%(part,rtconsts[part][0],rtconsts[part][1],rtconsts[part][2],rtconsts[part][3]))

#shift t0s
dbt0outs=[]
for straw in t0consts:
    if not do_global:
        if shiftt0 and rtshifts.has_key(straw.split()[0]):
            dbt0outs.append("%s : %f %f"%(straw,t0consts[straw][0]+rtshifts[straw.split()[0]],t0consts[straw][1]))
            #dbt0outs.append("%s : %f %f SHIFTED %f"%(straw,t0consts[straw][0]+rtshifts[straw.split()[0]],t0consts[straw][1], rtshifts[straw.split()[0]]))
        else:
            dbt0outs.append("%s : %f %f"%(straw,t0consts[straw][0],t0consts[straw][1]))
    else:
            dbt0outs.append("%s : %f %f"%(straw,t0consts[straw][0]+Dt0,t0consts[straw][1]))
        

dbrtouts.sort()
dbt0outs.sort()

dbfile=open("dbconst.txt","w")
# In case there are not errors:
if len (newerrors)==0:
	dbfile.write("# Fileformat=1\n")
	dbfile.write("# RtRelation\n")
	for dbrtout in dbrtouts:
	    dbfile.write(dbrtout + '\n')
	dbfile.write("# StrawT0\n")
	for dbt0out in dbt0outs:
	    dbfile.write(dbt0out + '\n')
	dbfile.write("#GLOBALOFFSET 0.0000\n")
elif len (newerrors)>1:
        dbfile.write("# Fileformat=2\n")
        dbfile.write("# RtRelation\n")
        for dbrtout in dbrtouts:
            dbfile.write(dbrtout + '\n')
        dbfile.write("# errors\n")
        for errorout in newerrors:
            dbfile.write(errorout + '\n')
        dbfile.write("# StrawT0\n")
        for dbt0out in dbt0outs:
            dbfile.write(dbt0out + '\n')
        dbfile.write("#GLOBALOFFSET 0.0000\n")


if not do_global:
#if do_global:
    t0dicts={}
    for olddict in olddicts:
        t0dicts[olddict.split()[0]]=float(olddict.split()[1])

    dictouts=[]
    for straw in t0dicts:
        keytokens=straw.split('_')
        if keytokens[1]=="all": detector='-3'
        else: detector=keytokens[1].strip()
        if shiftt0 and rtshifts.has_key(detector):
            dictouts.append('%s %f'%(straw,t0dicts[straw]+rtshifts[detector]))
            #dictouts.append('%s %f SHIFTED %f'%(straw,t0dicts[straw]+rtshifts[detector],rtshifts[detector]))
        else:
            dictouts.append('%s %f'%(straw,t0dicts[straw]))

    dictouts.sort()

    dictfile=open("dictconst.txt","w")
    for dictout in dictouts:
        dictfile.write(dictout + '\n')

