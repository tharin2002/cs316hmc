#!/bin/bash
#-- sample small test

PHYSRAND=../physrand

BANKS=8
CAPACITY=2
LINKS=4
BSIZE=64
QDEPTH=64
XDEPTH=128
VAULTS=16
NRQSTS=100
READ=50
WRITE=50
DRAMS=20

echo "Executing : $PHYSRAND -b $BANKS -c $CAPACITY -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -x $XDEPTH\
	-d $DRAMS -v $VAULTS -N $NRQSTS -R $READ -W $WRITE -S 65656"

$PHYSRAND -b $BANKS -c $CAPACITY -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -x $XDEPTH\
	-d $DRAMS -v $VAULTS -N $NRQSTS -R $READ -W $WRITE -S 65656
