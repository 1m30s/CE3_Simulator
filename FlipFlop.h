//pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"

// Generic Class
// FlipFlop, Latch

// 7474,74273,74373,74374

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generic D-FF (Edge-trigger)
///////////////////////////////////////////////////////////////////////////////////////////////////
class GenericDFF: public CIC
{
protected:
	int m_circuitCount;
	int m_pinListCount;
	int m_negEdge;
	int m_masterSlave;
	int m_lastCLK[8];
	int m_intVal[8];
	Wire* m_oe;
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		//memset(m_attr, 0, sizeof(m_attr));
		m_circuitCount = attr[0];
		
		m_pinList = pinList; // list of pin list
		m_negEdge = 0;
		m_masterSlave = 0; // M-S-FF mode (input change is inhibited when clock is low)
	}
public:
	GenericDFF()
	{
		m_circuitCount = 0;
		m_pinListCount = 6;
		
		m_negEdge = 0;
		m_masterSlave = 0; // M-S-FF mode (input change is inhibited when clock is low)
		// 0:CLK, 1:D, 2:/CLR,3:/SET, 4:/OE, 4:/Q, 5:Q
		m_oe = g_wireManager.GetGND();
		
		FF_Init(); // Initialize internal value
	}
	void FF_Init()
	{
		memset(m_lastCLK, 1, sizeof(m_lastCLK));
		memset(m_intVal, 1, sizeof(m_intVal));
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		//printf("Tick...");
		//printf("CC=%d,PL=%d\n", m_circuitCount, m_pinListCount);
		int i;
		for(i=0; i<m_circuitCount; i++)
		{
			int clk = m_pinList[i*m_pinListCount + 0]->Get() ^ m_negEdge;
			// Asynchronous Function
			if(m_pinList[i*m_pinListCount + 2]->Get() == 0){ // CLR
				m_intVal[i] = 0x2;
			}else if(m_pinList[i*m_pinListCount + 3]->Get() == 0){ // SET
				m_intVal[i] = 0x1;
			}else{
				if(m_lastCLK[i] == 0 && clk){ // Rising Edge
					if(m_pinList[i*m_pinListCount + 1]->Get()){
						m_intVal[i] = 0x1; // H
					}else{
						m_intVal[i] = 0x2; // L
					}
				}
			}
			m_lastCLK[i] = clk;
		}
		//printf("OK...\n");
	}
	virtual void Tick2() // output signal will be updated
	{
		if(m_oe->Get() == 0){
			int i;
			for(i=0; i<m_circuitCount; i++)
			{
				m_pinList[i*m_pinListCount + (m_pinListCount-1)]->Set((m_intVal[i] & 1)? 1: 0);
				m_pinList[i*m_pinListCount + (m_pinListCount-2)]->Set((m_intVal[i] & 2)? 1: 0);
			}
		}
	}
};

class C7474: public GenericDFF
{
public:
	C7474(const vector<Wire*>& pinList)
	{
		const int attr[3] = {2,6,0}; // 2 circuits
		const int pl[2*6] = {3,2,1,4,6,5,11,12,13,10,8,9};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74174: public GenericDFF
{
public:
	C74174(const vector<Wire*>& pinList)
	{
		const int attr[3] = {6,6,0};
		const int pl[6*6] = {
			9,3,1,VCC,NC,2, 9,4,1,VCC,NC,5,
			9,6,1,VCC,NC,7, 9,11,1,VCC,NC,10,
			9,13,1,VCC,NC,12, 9,14,1,VCC,NC,15};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74175: public GenericDFF
{
public:
	C74175(const vector<Wire*>& pinList)
	{
		const int attr[3] = {4,6,0};
		const int pl[4*6] = {
			9,4,1,VCC,3,2, 9,5,1,VCC,6,7,
			9,12,1,VCC,11,10, 9,13,1,VCC,14,15};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74273: public GenericDFF
{
public:
	C74273(const vector<Wire*>& pinList)
	{
		const int attr[3] = {8,6,0};
		const int pl[8*6] = {
			11,3,1,VCC,NC,2, 11,4,1,VCC,NC,5,
			11,7,1,VCC,NC,6, 11,8,1,VCC,NC,9,
			11,13,1,VCC,NC,12, 11,14,1,VCC,NC,15,
			11,17,1,VCC,NC,16, 11,18,1,VCC,NC,19};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74374: public GenericDFF
{
public:
	C74374(const vector<Wire*>& pinList)
	{
		const int attr[3] = {8,6,0};
		const int pl[8*6] = {
			11,3,VCC,VCC,NC,2, 
			11,4,VCC,VCC,NC,5, 
			11,7,VCC,VCC,NC,6, 
			11,8,VCC,VCC,NC,9, 
			11,13,VCC,VCC,NC,12, 
			11,14,VCC,VCC,NC,15, 
			11,17,VCC,VCC,NC,16, 
			11,18,VCC,VCC,NC,19};
		m_oe = pinList[1];
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74534: public GenericDFF
{
public:
	C74534(const vector<Wire*>& pinList)
	{
		const int attr[3] = {8,6,0};
		const int pl[8*6] = {
			11,3,VCC,VCC,2, NC,
			11,4,VCC,VCC,5, NC,
			11,7,VCC,VCC,6, NC,
			11,8,VCC,VCC,9, NC,
			11,13,VCC,VCC,12, NC,
			11,14,VCC,VCC,15, NC,
			11,17,VCC,VCC,16, NC,
			11,18,VCC,VCC,19,NC};
		m_oe = pinList[1];
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74574: public GenericDFF
{
public:
	C74574(const vector<Wire*>& pinList)
	{
		const int attr[3] = {8,6,0};
		const int pl[8*6] = {
			11,2,VCC,VCC,NC,19, 
			11,3,VCC,VCC,NC,18, 
			11,4,VCC,VCC,NC,17, 
			11,5,VCC,VCC,NC,16, 
			11,6,VCC,VCC,NC,15, 
			11,7,VCC,VCC,NC,14, 
			11,8,VCC,VCC,NC,13, 
			11,9,VCC,VCC,NC,12};
		m_oe = pinList[1];
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};
class C74564: public GenericDFF
{
public:
	C74564(const vector<Wire*>& pinList)
	{
		const int attr[3] = {8,6,0};
		const int pl[8*6] = {
			11,2,VCC,VCC,19, NC,
			11,3,VCC,VCC,18, NC,
			11,4,VCC,VCC,17, NC,
			11,5,VCC,VCC,16, NC,
			11,6,VCC,VCC,15, NC,
			11,7,VCC,VCC,14, NC,
			11,8,VCC,VCC,13, NC,
			11,9,VCC,VCC,12,NC};
		m_oe = pinList[1];
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generic JK-FF (Edge-trigger)
///////////////////////////////////////////////////////////////////////////////////////////////////

class GenericJKFF: public CIC
{
protected:
	int m_circuitCount;
	int m_pinListCount;
	int m_negEdge;
	int m_masterSlave;
	int m_lastCLK[8];
	int m_intVal[8];
	int m_lastInput;
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		//memset(m_attr, 0, sizeof(m_attr));
		m_circuitCount = attr[0];
		
		m_pinList = pinList; // list of pin list
		m_negEdge = 0;
		m_masterSlave = 0; // M-S-FF mode (input change is inhibited when clock is low)
	}
public:
	GenericJKFF()
	{
		m_circuitCount = 0;
		m_pinListCount = 7;
		
		m_lastInput = 0;
		m_negEdge = 0;
		m_masterSlave = 0; // M-S-FF mode (input change is inhibited when clock is low)
		// 0:CLK, 1:J, 2: K, 3:/CLR,4:/SET, 5:/Q, 6:Q
		
		FF_Init(); // Initialize internal value
	}
	void FF_Init()
	{
		memset(m_lastCLK, 1, sizeof(m_lastCLK));
		memset(m_intVal, 1, sizeof(m_intVal));
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		for(i=0; i<m_circuitCount; i++)
		{
			int clk = m_pinList[i*m_pinListCount + 0]->Get() ^ m_negEdge;
			// Asynchronous Function
			if(m_pinList[i*m_pinListCount + 3]->Get() == 0){ // CLR
				m_intVal[i] = 0x2;
			}else if(m_pinList[i*m_pinListCount + 4]->Get() == 0){ // SET
				m_intVal[i] = 0x1;
			}else{
				if(m_lastCLK[i] == 0 && clk){ // Rising Edge
					int curInput = (m_pinList[i*m_pinListCount + 1]->Get() << 1) | m_pinList[i*m_pinListCount + 2]->Get();
					if(curInput == 3){ // J=1, K=1
						m_intVal[i] ^= 0x3;
					}else if(curInput == 2){ // J=1, K=0
						m_intVal[i] = 1; // H
					}else if(curInput == 1){ // J=0, K=1
						m_intVal[i] = 2; // L
					}
				}
				m_lastCLK[i] = clk;
			}
			
			// Error Check (for 74N)
			// Data change from H to L while CLK=H is inhibited.
			if(m_series == SERIES_74N){
				int curInput = (m_pinList[i*m_pinListCount + 1]->Get() << 1) | m_pinList[i*m_pinListCount + 2]->Get();
				if(clk == 1 && (m_lastInput != 0) && curInput != m_lastInput) // mode falling edge while CLK=H
				{
					printf("Warning: inhibited data change detected while CLK=H. (JK-FF)\n");
				}
				m_lastInput = curInput;
			}
		}
	}
	virtual void Tick2() // output signal will be updated
	{
		int i;
		for(i=0; i<m_circuitCount; i++)
		{
			m_pinList[i*m_pinListCount + (m_pinListCount-1)]->Set((m_intVal[i] & 1)? 1: 0);
			m_pinList[i*m_pinListCount + (m_pinListCount-2)]->Set((m_intVal[i] & 2)? 1: 0);
		}
	}
};

class C7473: public GenericJKFF
{
public:
	C7473(const vector<Wire*>& pinList)
	{
		const int attr[3] = {2,7,0};
		const int pl[2*7] = {
			1,14,3,2,VCC,13,12,
			5,7,10,6,VCC,8,9};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
		m_negEdge = 1;
	}
};
class C74107: public GenericJKFF
{
public:
	C74107(const vector<Wire*>& pinList)
	{
		const int attr[3] = {2,7,0};
		const int pl[2*7] = {
			12,1,4,13,VCC,2,3,
			9,8,11,10,VCC,6,5};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
		m_negEdge = 1;
	}
};
class C74113: public GenericJKFF
{
public:
	C74113(const vector<Wire*>& pinList)
	{
		const int attr[3] = {2,7,0};
		const int pl[2*7] = {
			1,3,2,VCC,4,6,5,
			13,11,12,VCC,10,8,9};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
		m_negEdge = 1;
	}
};
class C74112: public GenericJKFF
{
public:
	C74112(const vector<Wire*>& pinList)
	{
		const int attr[3] = {2,7,0};
		const int pl[2*7] = {
			1,3,2,15,4,6,5,
			13,11,12,14,10,7,9};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		
		Init(pll, attr);
		m_negEdge = 1;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Transparent Latch
///////////////////////////////////////////////////////////////////////////////////////////////////
class GenericLatch: public CIC
{
protected:
	int m_circuitCount;
	int m_pinListCount;
	int m_intVal[8];
//	Wire* m_oe;
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		//memset(m_attr, 0, sizeof(m_attr));
		m_circuitCount = attr[0];
		
		m_pinList = pinList; // list of pin list
	}
public:
	GenericLatch()
	{
		m_circuitCount = 0;
		m_pinListCount = 5;
		// 0:LE, 1:D, 2: /OE, 3:/Q, 4:Q
		
		//m_oe = g_wireManager.GetGND();
		
		FF_Init(); // Initialize internal value
	}
	void FF_Init()
	{
		memset(m_intVal, 0, sizeof(m_intVal));
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		for(i=0; i<m_circuitCount; i++)
		{
			int latch = m_pinList[i*m_pinListCount + 0]->Get();
			// Asynchronous Function
			if(latch == 1){
				m_intVal[i] = m_pinList[1]->Get();
			}
			int oe = m_pinList[i*m_pinListCount + 2]->Get();
			if(oe == 0){
				m_pinList[i*m_pinListCount + (m_pinListCount-1)]->Set(m_intVal[i]);
				m_pinList[i*m_pinListCount + (m_pinListCount-2)]->Set(m_intVal[i] ^ 1);
			}
			
		}
	}
};
/*
class C7475(GenericLatch):
	def __init__(self,pin):
		pl = [[pin[13],pin[2],w_gnd, pin[1],pin[16]], [pin[13],pin[3],w_gnd, pin[14],pin[15]], 
			[pin[4],pin[6],w_gnd,pin[11],pin[10]], [pin[4],pin[7],w_gnd,pin[8],pin[9]]]
		super.__init__(pl)
class C74375(GenericLatch):
	def __init__(self,pin):
		pl = [[pin[4],pin[1],w_gnd, pin[2],pin[3]], [pin[4],pin[7],w_gnd, pin[6],pin[5]], 
			[pin[12],pin[9],w_gnd,pin[10],pin[11]], [pin[12],pin[15],w_gnd,pin[14],pin[13]]]
		super.__init__(pl)
class C74373(GenericLatch):
	def __init__(self,pin):
		pl = [[pin[11],pin[3],pin[1], w_nc,pin[2]],
			[pin[11],pin[4],pin[1], w_nc,pin[5]],
			[pin[11],pin[7],pin[1], w_nc,pin[6]],
			[pin[11],pin[8],pin[1], w_nc,pin[9]],
			[pin[11],pin[13],pin[1], w_nc,pin[12]],
			[pin[11],pin[14],pin[1], w_nc,pin[15]],
			[pin[11],pin[17],pin[1], w_nc,pin[16]],
			[pin[11],pin[18],pin[1], w_nc,pin[19]],]
		super.__init__(pl)
class C74573(GenericLatch):
	def __init__(self,pin):
		pl = [[pin[11],pin[2],pin[1], w_nc,pin[19]],
			[pin[11],pin[3],pin[1], w_nc,pin[18]],
			[pin[11],pin[4],pin[1], w_nc,pin[17]],
			[pin[11],pin[5],pin[1], w_nc,pin[16]],
			[pin[11],pin[6],pin[1], w_nc,pin[15]],
			[pin[11],pin[7],pin[1], w_nc,pin[14]],
			[pin[11],pin[8],pin[1], w_nc,pin[13]],
			[pin[11],pin[9],pin[1], w_nc,pin[12]],]
		super.__init__(pl)
class C74563(GenericLatch):
	def __init__(self,pin):
		pl = [[pin[11],pin[2],pin[1], pin[19],w_nc],
			[pin[11],pin[3],pin[1], pin[18],w_nc],
			[pin[11],pin[4],pin[1], pin[17],w_nc],
			[pin[11],pin[5],pin[1], pin[16],w_nc],
			[pin[11],pin[6],pin[1], pin[15],w_nc],
			[pin[11],pin[7],pin[1], pin[14],w_nc],
			[pin[11],pin[8],pin[1], pin[13],w_nc],
			[pin[11],pin[9],pin[1], pin[12],w_nc],]
		super.__init__(pl)
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
// Shift Register
///////////////////////////////////////////////////////////////////////////////////////////////////
class C74164: public CIC
{
protected:
	int m_intVal, m_lastCLK;
public:
	C74164(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_intVal = 0x00;
		m_lastCLK = 1;
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		int clk = m_pinList[8]->Get();
		// async
		if(m_pinList[9]->Get() == 0){ // /CLR=0
			m_intVal = 0;
		}
		// sync
		else if(m_lastCLK == 0 && clk){ // Rising Edge
			m_intVal = (m_intVal<<1) | ((m_pinList[1]->Get() & m_pinList[2]->Get())& 1);
		}
		m_lastCLK = clk;
		
	}
	virtual void Tick2() // output signal will be updated
	{
		m_pinList[3]->Set(m_intVal&0x01? 1: 0);
		m_pinList[4]->Set(m_intVal&0x02? 1: 0);
		m_pinList[5]->Set(m_intVal&0x04? 1: 0);
		m_pinList[6]->Set(m_intVal&0x08? 1: 0);
		m_pinList[10]->Set(m_intVal&0x10? 1: 0);
		m_pinList[11]->Set(m_intVal&0x20? 1: 0);
		m_pinList[12]->Set(m_intVal&0x40? 1: 0);
		m_pinList[13]->Set(m_intVal&0x80? 1: 0);
	}
};

class C74194: public CIC
{
protected:
	int m_intVal, m_lastCLK;
	int m_lastMode;
public:
	C74194(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_intVal = 0x00;
		m_lastCLK = 1;
		m_lastMode = 0;
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		int clk = m_pinList[11]->Get();
		// async
		if(m_pinList[1]->Get() == 0){ // /CLR=0
			m_intVal = 0;
		}
		// sync
		else if(m_lastCLK == 0 && clk){ // Rising Edge
			int ctl = m_pinList[9]->Get() | (m_pinList[10]->Get()<<1);
			if(ctl == 1){ // Shift Right
				m_intVal = ((m_intVal << 1) | (m_pinList[2]->Get())) & 0x0F; // Load to A
			}
			else if(ctl == 2){ // Shift Left
				m_intVal = (m_intVal >> 1) | (m_pinList[7]->Get()<<3); // Load to D
			}
			else if(ctl == 3){ // Load
				m_intVal = (m_pinList[3]->Get()<<0);  // A
				m_intVal |= (m_pinList[4]->Get()<<1);
				m_intVal |= (m_pinList[5]->Get()<<2);
				m_intVal |= (m_pinList[6]->Get()<<3); // D
			}
			
		}
		// Error Check (for 74N)
		if(m_series == SERIES_74N){
			int mode = m_pinList[9]->Get() | (m_pinList[10]->Get()<<1);
			if(clk == 0 && (m_lastMode != 0 && mode == 0)) // mode falling edge while CLK=L
			{
				printf("Warning: explicit clock edge detected by mode change while CLK=L. (74194)\n");
			}
			m_lastMode = mode;
		}
		m_lastCLK = clk;
		
	}
	virtual void Tick2() // output signal will be updated
	{
		m_pinList[15]->Set(m_intVal&0x1? 1: 0); // A
		m_pinList[14]->Set(m_intVal&0x2? 1: 0);
		m_pinList[13]->Set(m_intVal&0x4? 1: 0);
		m_pinList[12]->Set(m_intVal&0x8? 1: 0); // D
	}
};

class C74195: public CIC
{
protected:
	int m_intVal, m_lastCLK;
public:
	C74195(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_intVal = 0x00;
		m_lastCLK = 1;
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		int clk = m_pinList[10]->Get();
		// async
		if(m_pinList[1]->Get() == 0){ // /CLR=0
			m_intVal = 0;
		}
		// sync
		else if(m_lastCLK == 0 && clk){ // Rising Edge
			int ctl = (m_pinList[9]->Get()); // Shift/Load
			if(ctl == 1){ // Shift Right
				
				m_intVal = (m_intVal << 1) & 0x0F; // Shift
				if(m_pinList[2]->Get() == 1 && m_pinList[3]->Get() == 1){ // J=1, /K=1
					m_intVal |= m_pinList[2]->Get();
				}
				else if(m_pinList[2]->Get() == 0 && m_pinList[3]->Get() == 1){ // J=0, /K=1 (Hold)
					m_intVal |= (m_intVal & 2) >> 1;
				}
				else if(m_pinList[2]->Get() == 1 && m_pinList[3]->Get() == 0){ // J=1, /K=0 (Invert)
					m_intVal |= ((m_intVal & 2) >> 1) ^ 1;
				}
				
			}
			else{ // Load
				m_intVal = (m_pinList[4]->Get()<<0);  // A
				m_intVal |= (m_pinList[5]->Get()<<1);
				m_intVal |= (m_pinList[6]->Get()<<2);
				m_intVal |= (m_pinList[7]->Get()<<3); // D
			}
			
		}
		m_lastCLK = clk;
		
	}
	virtual void Tick2() // output signal will be updated
	{
		m_pinList[15]->Set(m_intVal&0x1? 1: 0); // A
		m_pinList[14]->Set(m_intVal&0x2? 1: 0);
		m_pinList[13]->Set(m_intVal&0x4? 1: 0);
		m_pinList[12]->Set(m_intVal&0x8? 1: 0); // D
		m_pinList[11]->Set(m_intVal&0x8? 0: 1); // /D
	}
};

