#pragma once
// (c) 2023 Y.L.E. / Eleken
// 
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <vector>
using namespace std;


class Wire;
class CWireManager;
class CICManager;
class CSignalLogger;

// To get pre-difined wires from IC module, declare wireman here
extern CWireManager g_wireManager;

class Wire
{
public:
	Wire(uint8_t value = 1, uint8_t readOnly = 0, const char* name = 0, uint8_t type = 0)
	{
		this->value = value & 1;
		this->readOnly = readOnly;
		this->type = type;
		this->updateFlag = 0;
		if(name){
			strcpy(this->name, name);
		}else{
			this->name[0] = 0;
		}
	}
	void Show(){
		printf("%s: %d\n", this->name, this->value);
	}
	void Toggle(){
		if(readOnly) return;
		this->value ^= 1;
	}
	void Set(uint8_t value){
		if(readOnly) return;
		this->value = value & 1;
	}
	uint8_t Get(){
		return this->value;
	}
	
	uint8_t value;
	uint8_t readOnly;
	uint8_t type;
	uint8_t updateFlag;
	char name[32];
};
/*
class Wire_NC: public Wire
{
public:
	Wire_NC()
	{
		Wire::Wire(1, 1, "NC", 0); // Read only
	}
};*/

class CWireManager
{
public:
	CWireManager()
	{
		// Pre-defined wire
		w_nc = new Wire(1,1,"NC");
		//Append(w_nc);
		w_vcc = new Wire(1,1,"VCC");
		//Append(w_vcc);
		w_gnd = new Wire(0,1,"GND");
		//Append(w_gnd);
		//w_clk = new Wire(1,0,"CLK"); // pre-defined clk. 
	}
	
	~CWireManager()
	{
		int i;
		for(i=0; i<m_wireList.size(); i++) delete m_wireList[i];
		delete w_nc;
		delete w_vcc;
		delete w_gnd;
		//delete w_clk;
	}
	Wire* NewWire(const char* name, int value = 0)
	{
		Wire* p = new Wire(value, 0, name, 0);
		m_wireList.push_back(p);
		return p;
	}
	
	Wire* Append(Wire* p)
	{
		m_wireList.push_back(p);
		return p;
	}
	Wire* Get(const char* name)
	{
		int i;
		for(i=0; i<m_wireList.size(); i++)
		{
			if(strcmp(m_wireList[i]->name, name) == 0) return m_wireList[i];
		}
		return NULL;
	}
	Wire* Get(unsigned int i)
	{
		if(i >= m_wireList.size()) return NULL;
		return m_wireList[i];
	}
	unsigned int GetWireSize()
	{
		m_wireList.size();
	}
	Wire* GetNC(){
		return w_nc;
	}
	Wire* GetVCC(){
		return w_vcc;
	}
	Wire* GetGND(){
		return w_gnd;
	}
	//Wire* GetCLK(){
	//	return w_clk;
	//}
private:
	Wire* w_nc, *w_vcc, *w_gnd, *w_clk;
	vector<Wire*> m_wireList;
};

#define VCC -2
#define GND -1
#define NC  -3
class CIC
{
public:
	CIC(){}
	CIC(const vector<Wire*>& pinList){}
	virtual ~CIC(){}
	virtual void Tick1() {}
	virtual void Tick2() {}
protected:
	vector<Wire*> m_pinList;
	void ConvertFromPinNumberList(const vector<Wire*> pinList, vector<Wire*> orderedPinList, 
		const int* pinNumberList, int sz)
	{
		orderedPinList.resize(sz);
		
		int i;
		for(i=0; i<sz; i++) {
			if(pinNumberList[i]>=0){
				orderedPinList[i] = pinList[pinNumberList[i]];
			}else if(pinNumberList[i] == -2){
				orderedPinList[i] = g_wireManager.GetVCC();
			}else if(pinNumberList[i] == -1){
				orderedPinList[i] = g_wireManager.GetGND();
			}else{
				orderedPinList[i] = g_wireManager.GetNC();
			}
		}
	}
};

// 実行時に Instantiate する CIC の継承クラスをスイッチしたい
// 要するにテキストファイルにリストを用意しておいて、
// その内容を基に new する
// if(strcmp()==0) return new ... みたいな感じはできるけど
// もしくは、すべての CIC の継承先を列挙できれば...
// 回路図から C++ のソースを出すような仕様だと、毎回 Build が必要になってしまう
// ptr = new classList[i]; のようにできればいいけど

// どれを new するか選択する関数を用意するしかなさそう
// CIC* CreateIC(const char* typename){
// 	if()
// }


class CICManager
{
public:
	CICManager(int tickCnt = 8, int outputDetect = 0)
	{
		m_tickCnt = tickCnt;
		m_outputDetect = outputDetect;
		
		
	}
	~CICManager()
	{
		int i;
		for(i=0; i<m_ICList.size(); i++) delete m_ICList[i];
	}
	// Append to manager. IC Instance must be instantiated outside.
	// e.g. 
	// CIC* pIC = new C7400(pinList);
	// man.Append(pIC);
	void Append(CIC* pIC)
	{
		m_ICList.push_back(pIC);
	}
	// Call Tick over all ICs
	void Tick1()
	{
		int i, tick;
		for(tick=0; tick<m_tickCnt; tick++)
			for(i=0; i<m_ICList.size(); i++) m_ICList[i]->Tick1();
	}
	void Tick2()
	{
		int i, tick;
		for(tick=0; tick<m_tickCnt; tick++)
			for(i=0; i<m_ICList.size(); i++) m_ICList[i]->Tick2();
	}
	void Tick()
	{
		Tick1(); // Gate Update
		Tick2(); // FF Update
		Tick1(); // Gate Update
	}
private:
	vector<CIC*> m_ICList;
	int m_tickCnt;
	int m_outputDetect;
	
	vector<uint8_t> m_initialState;
};

// type: Tick(0), Clock(1)
#define LOG_TYPE_TICK 0
#define LOG_TYPE_CLOCK 1
class CSignalLogger
{
public:
	CSignalLogger(int type=1, int divider=1)
	{
		m_type = type;
		m_divider = divider;
		m_prescaler = 0;
	}
	~CSignalLogger()
	{
	}
	// busIdx: set 0-3 when you show hex number in log file.
	// if busIdx used LSb signal has to be appended first.
	void Append(Wire* pSignal, int busIdx = -1){
		if(pSignal){
			m_signalList.push_back(pSignal);
			if(busIdx >= 0)
			{
				m_busDecode[busIdx].push_back(m_signalList.size()-1);
			}
		}
	}
	void Log()
	{
		int i;
		uint8_t d=0;
		for(i=0; i<m_signalList.size(); i++)
		{
			if(i && (i%8) == 0)
			{
				m_log.push_back(d);
				d = 0;
			}
			if(m_signalList[i]->Get()) d |= 1<<(i%8);
		}
		m_log.push_back(d);
	}
	void OnTick()
	{
		if(m_type == 0) Log();
	}
	void OnClock()
	{
		if(m_type == 1) Log();
	}
	int OutputText(const char* filename)
	{
		int lpos;
		int ls2 = (m_signalList.size()+7)/8; // log data byte size per tick
		if(ls2 == 0) return -1;
		int llen = m_log.size() / ls2;

		// Label
		int i;
		FILE* fp = fopen(filename, "wt");
		if(fp == NULL) return -1;
		
		// Bus Decode
		for(i=0; i<4; i++)
		{
			fprintf(fp, "BUS%d,", i);
		}
		
		// Signal
		//for(i=0; i<m_signalList.size(); i++)
		for(i=m_signalList.size()-1; i>=0; i--)
		{
			if(strncmp(m_signalList[i]->name, "w_", 2)){
				fprintf(fp, "%s,", &m_signalList[i]->name[2]);
			}
			else{
				fprintf(fp, "%s,", m_signalList[i]->name);
			}
		}
		fprintf(fp, "\n");

		// Log body
		for(lpos=0; lpos<llen; lpos++)
		{
			// Bus Decode
			for(i=0; i<4; i++)
			{
				int d = 0;
				int j;
				for(j=0; j<m_busDecode[i].size(); j++)
				{
					//d = d<<1;
					int bp = m_busDecode[i][j];
					if(m_log[lpos*ls2 + (bp/8) ] & (1<<(bp%8))) d |= (1<<j);
				}
				if(m_busDecode[i].size() >= 9){
					fprintf(fp,"%04X,", d);
				}else if(m_busDecode[i].size() >= 1){
					fprintf(fp,"%02X,", d);
				}else{
					fprintf(fp,",");
				}
			}
			// Signal
			for(i=m_signalList.size()-1; i>=0; i--)
			{
				fprintf(fp, "%d,", (m_log[lpos * ls2 + (i/8) ] & (1<<(i%8)) )? 1: 0);
			}
			fprintf(fp, "\n");
		}
		
		fclose(fp);
		return 0;
	}
	int OutputSR(const char* filename, const char* tempfile, const char* metafile, float freq = 1.0f)
	{
		FILE* fp = fopen(tempfile, "wb");
		if(fp == NULL) return -1;
		int i, sz = m_log.size();
		for(i=0; i<sz; i++){
			fputc(m_log[i], fp);
		}
		fclose(fp);
		
		FILE* fp2 = fopen(metafile, "wt");
		if(fp2 == NULL) return -2;
		fprintf(fp2, "[global]\n"
				"sigrok version=0.5.0-git-07d4e8631\n\n"
				"[device 1]\n"
				"capturefile=logic-1\n");
		fprintf(fp2, "samplerate=%.2f MHz\n", freq);
		fprintf(fp2, "total probes=%d\n", (m_signalList.size()));
		fprintf(fp2, "total analog = 0\n");
		for(i=0; i<m_signalList.size(); i++){
			fprintf(fp2, "probe%d = %s\n", i+1, m_signalList[i]->name);
		}
		fprintf(fp2, "unitsize=%d\n", (m_signalList.size()-1)/8+1 );
		fclose(fp2);
			/*
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
			metadata += "unitsize={}\n".format(int((len(signalList)-1)/8)+1)
			#	"analog1=SCL analog\n"\
			#	"analog2=SDA analog\n"
			*/
		
		// GenerateSR.Generate(filename, tempfile); // これは Python の関数
		
		return 0;
	}
	
private:
	int m_type, m_divider;
	int m_prescaler;
	vector<Wire*> m_signalList;
	vector<int> m_busDecode[4];
	vector<uint8_t> m_log;
};

