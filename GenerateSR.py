import zipfile
import os
import sys
import configparser

signalList = [[1, "DATA1"],[2,"DATA2"],[3,"DATA3"],[4,"DATA4"],
	[5,"DATA5"],[6,"DATA6"],[7,"DATA7"],[8,"DATA8"],
	[9, "DATA1"],[10,"DATA2"],[11,"DATA3"],[12,"DATA4"],
	[13,"DATA5"],[14,"DATA6"],[15,"DATA7"],[16,"DATA8"],
	]

def Generate(filename, data, signalList, freq=1):
	
	print("compressing...")
	zip = zipfile.ZipFile(filename, "w", compression=zipfile.ZIP_DEFLATED)
	
	metadata = "[global]\n"\
		"sigrok version=0.5.0-git-07d4e8631\n\n"\
		"[device 1]\n"\
		"capturefile=logic-1\n"
	metadata += "samplerate={} MHz\n".format(freq)
	metadata += "total probes={}\n".format(len(signalList))
#	metadata += "total probes=16\n"
	metadata += "total analog=0\n"
	
	for d in signalList:
		metadata += "probe{}={}\n".format(d[0], d[1])
	unitsize = ((len(signalList)-1)//16)*2+1
	metadata += "unitsize={}\n".format(unitsize)
	#	"analog1=SCL analog\n"\
	#	"analog2=SDA analog\n"
	
	version = "2"
	zip.writestr("metadata", metadata)
	zip.writestr("version", version)
	
	# output files per 4MB
	chunk = 0
	fsize = len(data)
	while(chunk*4*1024*1024 < fsize):
		zip.writestr("logic-1-{}".format(chunk+1), 
			data[chunk*4*1024*1024:(chunk+1)*4*1024*1024])
		chunk += 1

#buf = bytearray(1024*1024*1)
#i=0
#print("generating byte array")
#for d in range(len(buf)):
#	buf[d] = i & 0xFF
#	i += 3

#Generate("test.sr", buf, signalList)

# Generate from data file and metadata file
def Generate2(output_fn, data_fn, metadata_fn):
	print("compressing...")
	zip = zipfile.ZipFile(output_fn, "w", compression=zipfile.ZIP_DEFLATED)
	
	version = "2"
	zip.write(metadata_fn, "metadata")
	zip.writestr("version", version)
	
	f = open(data_fn, "rb")
	
	# read metadata
	meta = configparser.ConfigParser()
	meta.read(metadata_fn)
	analogch = meta["device 1"]["total analog"]
	
	# Digital
	# output files per 4MB
	chunk = 0
	#fsize = len(data)
	d = f.read(4*1024*1024)
	while(d):
		zip.writestr("logic-1-{}".format(chunk+1), d )
		chunk += 1
		d = f.read(4*1024*1024)
	
	# Analog
	if(int(analogch) > 0):
		for chn in range(1,64):
			if(meta["device 1"].get("analog{}".format(chn)) != None): break
		try:
			chunk = 0
			f2 = open(data_fn + "_analog", "rb")
			d = f2.read(4*1024*1024)
			while(d):
				zip.writestr("analog-1-{}-{}".format(chn, chunk+1), d )
				chunk += 1
				d = f2.read(4*1024*1024)
		except:
			pass

if __name__ == '__main__':
	# argv[1]: input, argv[2]: output
	sys.exit(Generate2(sys.argv[2], sys.argv[1], "metadata"))


