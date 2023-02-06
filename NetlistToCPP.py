import sys
import re

CE3Path = 'C:\electronics\bs3vp101120_a'

class ICList:
	def __init__(self):
		self.name = ""
		self.type = "C"
		self.pinList = []
		self.attr = ""
		self.series = "0"

def GetSeriesName(type):
	if(type.find("LS") >= 0):
		return "SERIES_74LS"
	if(type.find("ALS") >= 0):
		return "SERIES_74ALS"
	if(type.find("HCT") >= 0):
		return "SERIES_74HCT"
	if(type.find("HC") >= 0):
		return "SERIES_74HC"
	if(type.find("74S") >= 0):
		return "SERIES_74S"
	if(type.find("74N") >= 0):
		return "SERIES_74N"
	return ""

def main(argv):
	print ("// opening: "+ argv[1])
	
	# IC Instance
	icInstance = {} # Name, type (class name), Pin list
	wireInstance = [] # Name
	
	# 抵抗の扱い：無視する
	# 単純でないロジックの扱い (74121、遅れ素子など): 
	
	# Analyze
	curLineBlock = 0
	curWireName  = ""
	fr = open(argv[1], "rt")
	for line in fr: # For each lines
		if(line.startswith("$PACKAGES")):
			curLineBlock = 1
		elif (line.startswith ("$NETS")):
			curLineBlock = 2
		elif (line.startswith ("$END")):
			break
		else:
			if(line[0] == '!'): # component
				s = line.replace("\n", "").split("; ")
				#s = re.split(r'[! ;\n]+', line[2:])
				type = s[0].split("! ")[1]
				name = s[1]
				if(name.startswith("U") == False): continue
				# Modify type name...
				# SN.. -> .., LS,HC,HCT -> remove, 40H->74, 
				attr = ""
				series = GetSeriesName(type)
				
				s_type = type.split("_", 2)
				if(len(s_type) >= 2): attr = s_type[1]
				type = type.replace("SN","")
				type = type.replace("TC","")
				type = type.replace("LS","")
				type = type.replace("ALS","")
				type = type.replace("HCT","")
				type = type.replace("HC","")
				type = type.replace("S","")
				type = type.replace("74N","74")# 明示的に 74N を指定している場合
				type = type.replace("40H","74")
				type = re.findall(r"\d+", type) # 先頭の数字だけ
				if(len(type)):
					#print(line)
					#print(type)
					
					ic = ICList()
					ic.type = "C"+type[0]
					ic.name = name
					ic.attr = attr
					ic.series = series
					ic.pinList = ["w_nc" for i in range(24)] # dummy
					icInstance[name] = ic
			elif(curLineBlock == 2):
				# Split to pin
				s = re.split("[ ;,\n]+", line)
				s[0] = s[0].replace("/", "n")
				if(s[0] != ""):
					if(s[0].startswith("VCC")):
						curWireName = "w_vcc"
					elif(s[0].startswith("VDD")):
						curWireName = "w_vcc"
					elif(s[0].startswith("GND")):
						curWireName = "w_gnd"
					else:
						curWireName = "w_"+s[0]
						wireInstance.append(curWireName)
				# Split to component + number
				for pin in s[1:]:
					t = pin.split(".", 2)
					if(len(t) == 2):
						try:
							pn = int(t[1])
							while(len(icInstance[t[0]].pinList) <= pn): icInstance[t[0]].pinList.append("w_nc")
							icInstance[t[0]].pinList[pn] = curWireName
						except:
							pass
	#f = argv[1]
	
	# Output
	print("#include \"Core.h\"")
	print("#include \"ICCatalog.h\"")
	print("void CreateSchematics(CWireManager* pWireManager, CICManager* pICManager)")
	print("{")
	print("\t// Wire Instance-----------------------------------------")
	print('\tWire* w_gnd = pWireManager->GetGND();')
	print('\tWire* w_vcc = pWireManager->GetVCC();')
	print('\tWire* w_nc = pWireManager->GetNC();')
	for d in wireInstance:
		print('\tWire* {0} = pWireManager->NewWire(\"{0}\", 1);'.format(d))
	
	print("")
	print("\t// IC Instance-------------------------------------------")
	#print(icInstance)
	for key in list(icInstance):
		pinDefText = "" # pin[0]: dummy
		d = icInstance[key]
		for pins in d.pinList:
			pinDefText += pins + ","
		# for old compiler this is not supported. implicitly add -std=c++0x option if you use gcc 4.6.
		print("\tvector<Wire*> pin_{} = {{\n\t\t{}}};".format(d.name, pinDefText))
		print("\t{1}* {0} = new {1}(pin_{0});".format(d.name, d.type))
		if(len(d.attr)):
			print("\t{0}->SetAttribute(\"{1}\");".format(d.name, d.attr))
		if(len(d.series)):
			print("\t{0}->SetSeries({1});".format(d.name, d.series))
	
	print("\t// IC Manager-------------------------------------------")
	for key in list(icInstance):
		d = icInstance[key]
		print("\tpICManager->Append({});".format(d.name))
	print("\tpICManager->Tick();")
	
	print("}")
	
	return 0


if __name__ == '__main__':
	sys.exit(main(sys.argv))
