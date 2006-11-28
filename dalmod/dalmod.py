import sys
import pdb
import struct

def main():
	if len(sys.argv) < 3:
		print 'Usage: %s BigDAL.prc bootmenu.bin' % sys.argv[0]
		sys.exit(1)
	
	prc = pdb.Pdb()
	prc.read(file(sys.argv[1], 'rb'))

	boot = find_boot(prc)
	
	dal_start = unpack_branch(boot.data)
	print 'Palm OS start offset:', hex(dal_start)

	bm_start = len(boot.data)
	print 'Boot menu start offset:', hex(bm_start)

	bootmenu = file(sys.argv[2], 'rb').read()
	if struct.unpack('<I', bootmenu[4:8])[0] != 0xb007c0de:
		print 'Magic number missing.'
		print 'Are you sure this is bootmenu?'
		sys.exit(5)

	# insert dal_start address into bootmenu
	bootmenu = bootmenu[:8] + pack_branch(dal_start-bm_start-8) + bootmenu[12:]

	# now write bootmenu into DAL
	boot.data = pack_branch(bm_start) + boot.data[4:] + bootmenu

	prc.write(file('BigDAL-bm.prc','w'))
	


def unpack_branch(data):
	if data[3] != '\xea':
		print 'Initial branch not found', repr(data[:4])
		print 'Are you sure this is BigDAL?'
		sys.exit(4)

	# unpack the signed 24-bit branch offset
	if ord(data[2]) & 0x80:
		pad = '\xff' # negative
	else:
		pad = '\0' # positive
	return (struct.unpack('<i', data[:3] + pad)[0] << 2) + 8

def pack_branch(offset):
	return struct.pack('<i', (offset-8)>>2)[:3] + '\xea'


def find_boot(prc):
	boots = [x for x in prc.resources if x.type == 'boot']
	if len(boots) != 1:
		print 'Found %d boot resources.' % len(boots)
		print 'Are you sure this is BigDAL?'
		sys.exit(2)
	return boots[0]


if __name__ == '__main__': main()
