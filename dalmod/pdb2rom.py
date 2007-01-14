#!/usr/bin/env python
#
# Generate a boot ROM image out of the PDB files included
# in Palm's ROM updates.
#

from pdb import *
import sys, struct

FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])

def dump(src, length=16):
    N=0; result=''
    while src:
       s,src = src[:length],src[length:]
       hexa = ' '.join(["%02X"%ord(x) for x in s])
       s = s.translate(FILTER)
       result += "%04X   %-*s   %s\n" % (N, length*3, hexa, s)
       N+=length
    return result

def readblock(pdb, block):
	typ = block[:4]
	if typ != 'DBLK':
		print 'Warning: block type is "%s", expected "DBLK"' % (typ)
	
	size = struct.unpack('>I', block[4:8])[0]
	data = block[8:]
	if len(data) != size: print 'Warning: Size mismatch.'

	return data
	

def main():
	if len(sys.argv) < 3:
		print 'Usage: %s outfile pdb1 [pdb2 [pdb3...]]' % sys.argv[0]
		sys.exit(1)

	out = file(sys.argv[1], 'wb')
	sect = put = 0
	for fn in sys.argv[2:]:
		pdb = Pdb()
		pdb.read(file(fn, 'rb'))

		for rsrc in pdb.resources:
			data = readblock(pdb, rsrc.data)
			out.write(data)
			put += len(data)

		# make sure each PDB is 0x10000 aligned
		if put % 0x10000:
			pad = 0x10000 - (put % 0x10000)
			out.write('\xff' * pad)
			put += pad
			

if __name__ == '__main__': main()
