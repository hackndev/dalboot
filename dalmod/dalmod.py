import sys
import pdb
import struct

def main():
	if len(sys.argv) < 2:
		print 'Usage: %s BigDAL.prc' % sys.argv[0]
		sys.exit(1)
	
	prc = pdb.Pdb()
	prc.read(file(sys.argv[1], 'rb'))

	boot = find_boot(prc)
	
	if boot.data[3] != '\xea':
		print 'Initial branch not found', repr(boot.data[:4])
		print 'Are you sure this is BigDAL?'
		sys.exit(4)

	# unpack the signed 24-bit branch offset
	if ord(boot.data[2]) & 0x80:
		pad = '\xff' # negative
	else:
		pad = '\0' # positive
	palmosstart, = struct.unpack('<i', boot.data[:3] + pad)
	palmosstart = (palmosstart << 2) + 8
	print 'Palm OS start offset:', hex(palmosstart)


def find_boot(prc):
	boots = [x for x in prc.resources if x.type == 'boot']
	if len(boots) != 1:
		print 'Found %d boot resources.' % len(boots)
		print 'Are you sure this is BigDAL?'
		sys.exit(2)
	return boots[0]


if __name__ == '__main__': main()
