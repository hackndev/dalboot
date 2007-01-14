# Simple PDB file format parser
# Author: Alex Osborne <alex@mugofgrog.com>
# Created: 27 Nov 2006
#
import struct
import sys

def main():
	if len(sys.argv) > 1:
		fn = sys.argv[1]
	else:
		fn = 'brahma-spl.pdb'
	out = file('test.pdb','wb')
	pdb = Pdb()
	pdb.read(file(fn,'rb'))
	print str(pdb)
	pdb.write(out)

def trunc(x):
	return x[:x.index('\0')]

class Pdb:
	def __init__(self):
		pass
	
	attr_resdb = 1
	def isprc(self):
		return self.attr & Pdb.attr_resdb == Pdb.attr_resdb
	
	def read(self, f):
		self.read_header(f)
		self.read_rlheader(f)
		if self.isprc():
			self.read_resourcelist(f)
		else:
			self.read_recordlist(f)
		self.read_resources(f)
	def write(self, f):
		self.write_header(f)
		self.write_rlheader(f)
		if self.isprc():
			self.write_resourcelist(f)
			self.write_resources(f)
		else:
			print 'TODO: pdb record list'

	header_fmt 	= ('>'	## Database header
			+ '32s'	# name
			+ 'H'	# attributes
			+ 'H'	# version
			+ 'I'	# creation date
			+ 'I'	# modification date
			+ 'I'	# backup date
			+ 'I'	# modification number
			+ 'I'	# app info id
			+ 'I'	# sort info id
			+ '4s'	# type
			+ '4s'	# creator
			+ 'I')	# unique id seed
	header_sz = struct.calcsize(header_fmt)

	def read_header(self, f):
		( self.name, self.attr, self.ver, self.created, self.modified,
		self.backupdate, self.modnumber,
		self.appinfoid, self.sortinfoid, self.type, self.creator,
		self.seed ) = struct.unpack(Pdb.header_fmt, 
							f.read(Pdb.header_sz))
		self.name = trunc(self.name)
	
	def write_header(self, f):
		f.write(struct.pack(Pdb.header_fmt, 
		self.name, self.attr, self.ver, self.created, self.modified,
		self.backupdate, self.modnumber, 
		self.appinfoid, self.sortinfoid, self.type, self.creator,
		self.seed))

	rlheader_fmt	= ('>'	## Record list header
			+ 'I'	# next record list pointer (deprecated)
			+ 'H')	# number of records
	rlheader_sz = struct.calcsize(rlheader_fmt)

	def read_rlheader(self, f):
		(self.nextrl, self.numrecs) = struct.unpack(Pdb.rlheader_fmt,
						f.read(Pdb.rlheader_sz))
		if self.nextrl != 0:
			print 'Warning: Chained record lists found.'
			print 'Ignoring secondary lists.'
	
	def write_rlheader(self, f):
		f.write(struct.pack(Pdb.rlheader_fmt, 0, len(self.resources)))

	def read_recordlist(self, f):
		def read_record(i):
			rec = PdbRecord()
			rec.read_header(f)
			return rec
		self.resources = map(read_record, range(self.numrecs))
		if f.read(2) != '\0\0':
			print 'Warning: non-zero record list padding'

	def read_resourcelist(self, f):
		def read_resource(i):
			rsrc = PrcResource()
			rsrc.read_header(f)
			return rsrc
		self.resources = map(read_resource, range(self.numrecs))
		if f.read(2) != '\0\0':
			print 'Warning: non-zero resource list padding'

	def write_resourcelist(self, f):
		offset = 2+f.tell()+PrcResource.header_sz * len(self.resources)
		for rsrc in self.resources:
			if rsrc.offset != offset:
				print rsrc.offset, offset
			rsrc.offset = offset
			rsrc.write_header(f)
			offset += len(rsrc.data)
		f.write('\0\0') # padding

	def read_resources(self, f):
		pos = f.tell()
		for i, rsrc in enumerate(self.resources):
			if pos != rsrc.offset:
				print 'Warning: out of sync', pos, rsrc.offset
				pos = rsrc.offset
				f.seek(pos)

			if i+1 < len(self.resources):
				rsrc.data = f.read(self.resources[i+1].offset 
									- pos)
			else:
				rsrc.data = f.read()
			pos += len(rsrc.data)

	def write_resources(self, f):
		for rsrc in self.resources:
			f.write(rsrc.data)

	def __str__(self):
		return (str((self.name, self.attr, self.ver, self.created,
		self.modified, self.backupdate, self.modnumber, self.appinfoid,
		self.sortinfoid, self.type,
		self.creator, self.seed )) +
		'\n\n%d records:\n' % self.numrecs +
		'\n'.join([repr(x) for x in self.resources]))

class PrcResource:
	header_fmt	= ('>'	## Record header
			+ '4s'	# resource type
			+ 'H'	# id
			+ 'I')	# data offset
	header_sz = struct.calcsize(header_fmt)

	def read_header(self, f):
		(self.type, self.id, self.offset) = struct.unpack(
							PrcResource.header_fmt,
						f.read(PrcResource.header_sz))
	def write_header(self, f):
		f.write(struct.pack(PrcResource.header_fmt, self.type, self.id,
								self.offset))

	def __repr__(self):
		return '<PrcResource %s[%d] %d bytes>' % (self.type, self.id,
								len(self.data))
class PdbRecord:
	header_fmt	= ('>'	## Record header
			+ 'I'	# data offset
			+ 'B'	# attributes
			+ '3s')	# uniqueID
	header_sz = struct.calcsize(header_fmt)

	def read_header(self, f):
		(self.offset, self.attrib, self.id) = struct.unpack(
							PdbRecord.header_fmt,
						f.read(PdbRecord.header_sz))
	def write_header(self, f):
		f.write(struct.pack(PdbRecord.header_fmt, self.offset, self.attrib,
								self.id))

	def __repr__(self):
		return '<PdbRecord %s[%d] %d bytes>' % (repr(self.id), self.attrib,
								len(self.data))


if __name__ == '__main__': main()
