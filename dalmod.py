import sys
import pdb

def main():
	if len(sys.argv) < 2:
		print 'Usage: %s BigDAL.prc' % sys.argv[0]
		sys.exit(1)
	
	prc = pdb.Pdb()
	prc.read(file(sys.argv[1], 'rb'))

	boot = find_boot(prc)
	print repr(boot.data[:4])

def find_boot(prc):
	boots = [x for x in prc.resources if x.type == 'boot']
	if len(boots) != 1:
		print 'Found %d boot resources.' % len(boots)
		print 'Are you sure this is BigDAL?'
		sys.exit(2)
	return boots[0]


if __name__ == '__main__': main()
