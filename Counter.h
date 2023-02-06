//pragma once
// (c) 2023 Y.L.E. / Eleken
// 

//include "Core.h"
//ifndef IS_ALL_CLR
static int IsAllElementsClear(uint8_t* array, int sz)
{
	int i, cond=1; 
	for(i=0;i<sz;i++) {
		if(array[i]!=0) cond=0;
	}
	return cond;
}
//define IS_ALL_CLR(arr) (IsAllElementsClear(arr, sizeof(arr)))
//endif

//ifndef IS_ALL_SET
static int IsAllElementsSet(uint8_t* array, int sz)
{
	int i, cond=1; 
	for(i=0;i<sz;i++) {
		if(array[i]==0) cond=0;
	}
	return cond;
}

//define IS_ALL_SET(arr) (IsAllElementsSet(arr, sizeof(arr)))
//endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Synchronous Counter (74561)
///////////////////////////////////////////////////////////////////////////////////////////////////
class GenericSyncCounter : public CIC
{
protected:
	int m_modulo;
	uint8_t m_lastCLK;
	uint8_t m_intVal[4]; // Q0-3
	uint8_t m_en_polarity; // 0 -> negative (74169 type) / 1 -> positive (74161 type)
	
	void FF_Init()
	{
		m_lastCLK = 1;
		memset(m_intVal, 0, sizeof(m_intVal));
	}
	void Pin_Init(const vector<Wire*>& pinList)
	{
		/*
		// 1: /Async Load
		// 2: CLK
		// 3-6: In0-3
		// 7: P_IN
		// 8: /Async Cler
		// 9: /Sync Clear
		// 11: /Sync Load
		// 12: T_IN
		// 13-16: Q3-0
		// 17: /OE
		// 18: C_Carry
		// 19: R_Carry
		// 20: Count UP(1)/DOWN(0)
		// For other configurations, reorder m_pin to 74561 like numbering
		// such as, m_pin = [None, pin[1], pin[2], w_vcc, w_gnd, ...]
		*/
		m_pinList = pinList;
	}
public:
	GenericSyncCounter()
	{
		//m_pinList = pinList;
		m_modulo = 16;
		m_en_polarity = 1;
		
		FF_Init(); // Initialize internal value
	}
	virtual void Tick1() //// (for flip-flops,) output signal is not updated to avoid racing
	{
		//// Asynchronous Function
		if(m_pinList[8]->Get() == 0){ // Clear
			memset(m_intVal, 0, sizeof(m_intVal));
		}else if(m_pinList[9]->Get() == 0){ // Load
			m_intVal[0] = m_pinList[3]->Get();
			m_intVal[1] = m_pinList[4]->Get();
			m_intVal[2] = m_pinList[5]->Get();
			m_intVal[3] = m_pinList[6]->Get();
		}else{
			if(m_lastCLK == 0 && m_pinList[2]->Get() == 1) // Rising Edge
			{
				if(m_pinList[9]->Get() == 0){ // Clear
					memset(m_intVal, 0, sizeof(m_intVal));
				}else if(m_pinList[11]->Get() == 0){ //// Load
					m_intVal[0] = m_pinList[3]->Get();
					m_intVal[1] = m_pinList[4]->Get();
					m_intVal[2] = m_pinList[5]->Get();
					m_intVal[3] = m_pinList[6]->Get();
				}else if(m_pinList[7]->Get() == m_en_polarity && m_pinList[12]->Get() == m_en_polarity){ //// Count Up/Dn
					m_intVal[0] ^= 1;
					int cntDir = (m_pinList[20]->Get() ^ 1); // pin value 1: up, 0: down
					if(m_intVal[0] == cntDir ){
						m_intVal[1] ^= 1;
						if(m_intVal[1] == cntDir ){
							m_intVal[2] ^= 1;
							if(m_intVal[2] == cntDir ){
								m_intVal[3] ^= 1;
							}
						}
					}
					// For decade counter
					if(m_modulo == 10){
						if(cntDir == 0){ // up
							if(m_intVal[0] == 0 && m_intVal[1] == 1 && m_intVal[2] == 0 && m_intVal[3] == 1){
								memset(m_intVal, 0, sizeof(m_intVal));
							}
						}else{ // dn
							if(m_intVal[0] == 1 && m_intVal[1] == 1 && m_intVal[2] == 1 && m_intVal[3] == 1){
								m_intVal[0] = 1;
								m_intVal[1] = 0;
								m_intVal[2] = 0;
								m_intVal[3] = 1;
							}
						}
					}
				}
			}
		}
		m_lastCLK = m_pinList[2]->Get();
	}
	virtual void Tick2() // output signal will be updated
	{
		if(m_pinList[17]->Get() == 0) // /OE
		{
			m_pinList[13]->Set(m_intVal[3]); // Q[3]
			m_pinList[14]->Set(m_intVal[2]);
			m_pinList[15]->Set(m_intVal[1]);
			m_pinList[16]->Set(m_intVal[0]); // Q[0]
			
		}
		// Carry: Always applied
		int isFull = 0;
		if(m_modulo == 10){
			if(m_intVal[0] == 1 &&
				m_intVal[1] == 0 &&
				m_intVal[2] == 0 &&
				m_intVal[3] == 1) isFull = 1;
		}else{
			if(m_intVal[0] == 1 &&
				m_intVal[1] == 1 &&
				m_intVal[2] == 1 &&
				m_intVal[3] == 1) isFull = 1;
		}
		int RCO = 1 ^ m_en_polarity;
		if (isFull && m_pinList[12]->Get() == m_en_polarity){ // R Carry (Sync)
			RCO ^= 1;
		}
		m_pinList[19]->Set(RCO); // R Carry
		
		int CCO = 0;
		if ( isFull &&
			m_pinList[2]->Get() == 1 && // CLK = 1
			m_pinList[7]->Get() == m_en_polarity &&
			m_pinList[12]->Get() == m_en_polarity){ 
			CCO = 1; // C Carry (Async)
		}
		
		m_pinList[18]->Set(CCO); // C Carry 
	}
};

class C74561: public GenericSyncCounter
{
	
};
class C74169: public GenericSyncCounter
{
public:
	C74169(const vector<Wire*>& pinList) : GenericSyncCounter()
	{
		vector<Wire*> p2(21, g_wireManager.GetVCC());
		p2[20]  = pinList[1]; // U/D
		p2[2]  = pinList[2]; // clock
		p2[3]  = pinList[3]; // IN
		p2[4]  = pinList[4];
		p2[5]  = pinList[5];
		p2[6]  = pinList[6];
		p2[7]  = pinList[7]; // P_IN
		p2[11] = pinList[9]; // sync load
		p2[12] = pinList[10]; // T_IN
		p2[13] = pinList[11]; // Out
		p2[14] = pinList[12]; // Out
		p2[15] = pinList[13]; // Out
		p2[16] = pinList[14]; // Out
		p2[17] = g_wireManager.GetGND(); // /OE
		p2[18] = g_wireManager.GetNC();
		p2[19] = pinList[15]; // Ripple Carry
		
		GenericSyncCounter::Pin_Init(p2);
		m_en_polarity = 0; // negative enable
	}
};
class C74161: public GenericSyncCounter
{
public:
	C74161(const vector<Wire*>& pinList) : GenericSyncCounter()
	{
		vector<Wire*> p2(21, g_wireManager.GetVCC());
		p2[8]  = pinList[1]; // async clear
		p2[2]  = pinList[2]; // clock
		p2[3]  = pinList[3]; // IN
		p2[4]  = pinList[4];
		p2[5]  = pinList[5];
		p2[6]  = pinList[6];
		p2[7]  = pinList[7]; // P_IN
		p2[11] = pinList[9]; // sync load
		p2[12] = pinList[10]; // T_IN
		p2[13] = pinList[11]; // Out
		p2[14] = pinList[12]; // Out
		p2[15] = pinList[13]; // Out
		p2[16] = pinList[14]; // Out
		p2[17] = g_wireManager.GetGND(); // /OE
		p2[18] = g_wireManager.GetNC();
		p2[19] = pinList[15]; // Ripple Carry
		
		GenericSyncCounter::Pin_Init(p2);
	}
};
class C74160: public C74161
{
public:
	C74160(const vector<Wire*>& pinList) : C74161(pinList)
	{
		m_modulo = 10;
	}
};
class C74163: public GenericSyncCounter
{
public:
	C74163(const vector<Wire*>& pinList) : GenericSyncCounter()
	{
		vector<Wire*> p2(21, g_wireManager.GetVCC());
		p2[2]  = pinList[2]; // clock
		p2[3]  = pinList[3]; // IN
		p2[4]  = pinList[4];
		p2[5]  = pinList[5];
		p2[6]  = pinList[6];
		p2[7]  = pinList[7]; // P_IN
		p2[9]  = pinList[1]; // sync clear
		p2[11] = pinList[9]; // sync load
		p2[12] = pinList[10]; // T_IN
		p2[13] = pinList[11]; // Out
		p2[14] = pinList[12]; // Out
		p2[15] = pinList[13]; // Out
		p2[16] = pinList[14]; // Out
		p2[17] = g_wireManager.GetGND(); // /OE
		p2[18] = g_wireManager.GetNC();
		p2[19] = pinList[15]; // Ripple Carry
		
		GenericSyncCounter::Pin_Init(p2);
	}
};
class C74162: public C74163
{
public:
	C74162(const vector<Wire*>& pinList) : C74163(pinList)
	{
		m_modulo = 10;
	}
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Synchronous Counter (74190/191)
///////////////////////////////////////////////////////////////////////////////////////////////////
class C74191: public CIC
{
protected:
	int m_modulo;
	uint8_t m_lastCLK;
	uint8_t m_intVal[6]; // Q0-3, C_Carry, R_Carry
	
	void FF_Init()
	{
		m_lastCLK = 1;
		memset(m_intVal, 0, sizeof(m_intVal));
	}
public:
	C74191(const vector<Wire*>& pinList)
	{
		//1: B
		//2: QB
		//3: QA
		//4: /EN
		//5: DN(H)/UP(L)
		//6: QC
		//7: QD
		//9: D
		//10: C
		//11: /LD
		//12: MAX/MIN
		//13: /RCO
		//14 CLK
		//15: DA
		//Async Preset, No clear
		
		m_pinList = pinList;
		m_modulo = 16;
		FF_Init();
	}
	virtual void Tick1() //// (for flip-flops,) output signal is not updated to avoid racing
	{
		int clk = m_pinList[14]->Get();
		// Asynchronous Function
		if(this->m_pinList[11]->Get() == 0) // Load
		{
			this->m_intVal[0] = this->m_pinList[15]->Get();
			this->m_intVal[1] = this->m_pinList[1]->Get();
			this->m_intVal[2] = this->m_pinList[10]->Get();
			this->m_intVal[3] = this->m_pinList[9]->Get();
		}
		else if(this->m_pinList[4]->Get() == 0) // EN=0
		{
			if(this->m_lastCLK == 0 && clk == 1) // Rising Edge
			{
				this->m_intVal[0] ^= 1;
				int cntDir = this->m_pinList[5]->Get(); // Count Up([5]=0) or Down ([5]=1)
				if(this->m_intVal[0] == cntDir) // 0: up
				{
					this->m_intVal[1] ^= 1;
					if(this->m_intVal[1] == cntDir ){
						this->m_intVal[2] ^= 1;
						if(this->m_intVal[2] == cntDir ){
							this->m_intVal[3] ^= 1;
						}
					}
				}
				// For decade counter
				if(this->m_modulo == 10){
					if(cntDir == 0){ // up
						if(this->m_intVal[0] == 0 && 
							this->m_intVal[1] == 1 && 
							this->m_intVal[2] == 0 && 
							this->m_intVal[3] == 1)
						{
							this->m_intVal[0] = 0;
							this->m_intVal[1] = 0;
							this->m_intVal[2] = 0;
							this->m_intVal[3] = 0;
						}
					}
					else{ // dn
						if(this->m_intVal[0] == 1 && 
							this->m_intVal[1] == 1 && 
							this->m_intVal[2] == 1 && 
							this->m_intVal[3] == 1)
						{
							this->m_intVal[0] = 1;
							this->m_intVal[1] = 0;
							this->m_intVal[2] = 0;
							this->m_intVal[3] = 1;
						}
					}
				}
			}
		}
		this->m_lastCLK = clk;
	}
	
	virtual void Tick2()
	{
		this->m_pinList[3]->Set(this->m_intVal[0]); // Q[0]
		this->m_pinList[2]->Set(this->m_intVal[1]);
		this->m_pinList[6]->Set(this->m_intVal[2]);
		this->m_pinList[7]->Set(this->m_intVal[3]); // Q[3]
		// Carry
		int maxmin = 0;
		int rco = 1;
		int isMax = 0;
		
		if(m_modulo == 10)
		{
			if(this->m_pinList[5]->Get() == 0 && 
				this->m_intVal[0] == 1 && 
				this->m_intVal[1] == 0 && 
				this->m_intVal[2] == 0 && 
				this->m_intVal[3] == 1) isMax = 1; // Up
		}else
		{
			if(this->m_pinList[5]->Get() == 0 && 
				this->m_intVal[0] == 1 && 
				this->m_intVal[1] == 1 && 
				this->m_intVal[2] == 1 && 
				this->m_intVal[3] == 1) isMax = 1; // Up
		}
		if(this->m_pinList[5]->Get() == 1 && 
			this->m_intVal[0] == 0 && 
			this->m_intVal[1] == 0 && 
			this->m_intVal[2] == 0 && 
			this->m_intVal[3] == 0) isMax = 1; // Down
		
		if(isMax){
			maxmin = 1;
			if(this->m_pinList[14]->Get() == 0 && this->m_pinList[4]->Get() == 0) // CLK=0 and /EN=0
			{
				rco = 0;
			}
		}
		this->m_pinList[12]->Set(maxmin); // MAX/MIN
		this->m_pinList[13]->Set(rco); // RCO
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Synchronous Counter (74190/191)
///////////////////////////////////////////////////////////////////////////////////////////////////
class C74193: public CIC
{
protected:
	int m_modulo;
	uint8_t m_lastCLK;
	uint8_t m_intVal[6]; // Q0-3, C_Carry, R_Carry
	
	void FF_Init()
	{
		m_lastCLK = 3;
		memset(m_intVal, 0, sizeof(m_intVal));
	}
public:
	C74193(const vector<Wire*>& pinList){
		//1: B
		//2: QB
		//3: QA
		//4: CLKDN
		//5: CLK_UP
		//6: QC
		//7: QD
		//9: D
		//10: C
		//11: /LD
		//12: /CARRY
		//13: /BORROW
		//14 CLR
		//15: DA
		//Async Preset, No clear
		
		m_pinList = pinList;
		FF_Init();
		m_modulo = 16;
	}
	virtual void Tick1()
	{
		int clk1 = this->m_pinList[4]->Get();
		int clk2 = this->m_pinList[5]->Get();
		if(this->m_pinList[14]->Get() == 1) // CLR
		{
			this->m_intVal[0] = 0;
			this->m_intVal[1] = 0;
			this->m_intVal[2] = 0;
			this->m_intVal[3] = 0;
		}else if(this->m_pinList[11]->Get() == 0) // /Load
		{
			this->m_intVal[0] = this->m_pinList[15]->Get();
			this->m_intVal[1] = this->m_pinList[1]->Get();
			this->m_intVal[2] = this->m_pinList[10]->Get();
			this->m_intVal[3] = this->m_pinList[9]->Get();
		}else{
			if(clk2 == 1 && ((this->m_lastCLK&1) == 0 && clk1 == 1)) // Down
			{
				this->m_intVal[0] ^= 1;
				if(this->m_intVal[0] == 1 ){
					this->m_intVal[1] ^= 1;
					if(this->m_intVal[1] == 1 ){
						this->m_intVal[2] ^= 1;
						if(this->m_intVal[2] == 1 ){
							this->m_intVal[3] ^= 1;
						}
					}
				}
				// For decade counter
				if(this->m_modulo == 10){
					if(this->m_intVal[0] == 1 && 
						this->m_intVal[1] == 1 && 
						this->m_intVal[2] == 1 && 
						this->m_intVal[3] == 1)
					{
						this->m_intVal[0] = 1;
						this->m_intVal[1] = 0;
						this->m_intVal[2] = 0;
						this->m_intVal[3] = 1;
					}
				}
			}
			if(clk1 == 1 && ((this->m_lastCLK&2) == 0 && clk2 == 1)) // Up
			{
				this->m_intVal[0] ^= 1;
				if(this->m_intVal[0] == 0 ){
					this->m_intVal[1] ^= 1;
					if(this->m_intVal[1] == 0 ){
						this->m_intVal[2] ^= 1;
						if(this->m_intVal[2] == 0 ){
							this->m_intVal[3] ^= 1;
						}
					}
				}
				// For decade counter
				// For decade counter
				if(this->m_modulo == 10){
					if(this->m_intVal[0] == 0 && 
						this->m_intVal[1] == 1 && 
						this->m_intVal[2] == 0 && 
						this->m_intVal[3] == 1)
					{
						this->m_intVal[0] = 0;
						this->m_intVal[1] = 0;
						this->m_intVal[2] = 0;
						this->m_intVal[3] = 0;
					}
				}
			}
		}
		this->m_lastCLK = clk1 | (clk2<<1); //DN + UP<<1
	}
	virtual void Tick2()
	{
		this->m_pinList[3]->Set(this->m_intVal[0]); // Q[0]
		this->m_pinList[2]->Set(this->m_intVal[1]);
		this->m_pinList[6]->Set(this->m_intVal[2]);
		this->m_pinList[7]->Set(this->m_intVal[3]); // Q[3]
		// Carry
		int co = 1;
		int bo = 1;
		if(this->m_pinList[5]->Get() == 0 && 
			m_intVal[0] == 1 && 
			m_intVal[1] == 1 && 
			m_intVal[2] == 1 && 
			m_intVal[3] == 1)
		{
			co = 0;
		}
		if(this->m_pinList[4]->Get() == 0 && 
			m_intVal[0] == 0 && 
			m_intVal[1] == 0 && 
			m_intVal[2] == 0 && 
			m_intVal[3] == 0)
		{
			bo = 0;
		}
		this->m_pinList[12]->Set(co); // /Carry Out
		this->m_pinList[13]->Set(bo); // /Borrow Out
	}
};

class C74190: public C74191
{
public:
	C74190(const vector<Wire*>& pinList) : C74191(pinList)
	{
		m_modulo = 10;
	}
};
class C74192: public C74193
{
public:
	C74192(const vector<Wire*>& pinList) : C74193(pinList)
	{
		m_modulo = 10;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// 8-Bit Synchronous Counter (74461)
///////////////////////////////////////////////////////////////////////////////////////////////////
class C74461 : public CIC
{
protected:
	uint8_t m_lastCLK;
	uint16_t m_intVal;
	
	void FF_Init()
	{
		m_lastCLK = 1;
		m_intVal = 0;
	}
public:
	C74461(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		FF_Init(); // Initialize internal value
	}
	virtual void Tick1() //// (for flip-flops,) output signal is not updated to avoid racing
	{
		//// Asynchronous Function
		if(m_lastCLK == 0 && m_pinList[1]->Get() == 1) // Rising Edge
		{
			int ctl = (m_pinList[2]->Get()) | (m_pinList[11]->Get()<<1) | (m_pinList[23]->Get() << 2);
			
			if((ctl & 3) == 0){ // Clear
				m_intVal = 0;
			}else if((ctl & 3) == 2){ //// Load
				m_intVal = 0;
				int i;
				for(i=0; i<8; i++){
					m_intVal |= (m_pinList[3+i]->Get()) << i;
				}
			}else if(ctl == 3){ //// Count Up
				m_intVal ++;
			}
		}
		m_lastCLK = m_pinList[1]->Get();
	}
	virtual void Tick2() // output signal will be updated
	{
		// Carry out (always applied)
		if((m_pinList[23]->Get() == 0) &&  (m_intVal & 0xFF) == 0xFF){
			m_pinList[14]->Set(0);
		}else{
			m_pinList[14]->Set(1);
		}
		
		
		if(m_pinList[13]->Get() == 0) // /OE
		{
			int i;
			for(i=0; i<8; i++){
				if(m_intVal & (1<<i)) m_pinList[22-i]->Set(1);
				else m_pinList[22-i]->Set(0);
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Async counter
///////////////////////////////////////////////////////////////////////////////////////////////////
// Supports Multiple Circuits (74390,74393)
class GenericAsyncCounter : public CIC
{
protected:
	int m_modulo;
	uint8_t m_lastCLK[4];
	uint8_t m_intVal[4];
	int m_attr[3]; // m_attr[0]: circuit number
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		memset(m_attr, 0, sizeof(m_attr));
		
		// 0:CLK1, 1: CLK2, 2-3:R0, 4-5:R9, 6-9: Output
		m_pinList = pinList;
		m_attr[0] = attr[0];
	}
	void FF_Init()
	{
		memset(m_lastCLK, 1, sizeof(m_lastCLK));
		memset(m_intVal, 1, sizeof(m_intVal));
	}
public:
	GenericAsyncCounter()
	{
		m_modulo = 16;
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_attr[2] = 0;
		FF_Init();
	}
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		for(i=0; i<m_attr[0]; i++){
			int lastIntVal = m_intVal[i];
			
			// Asynchronous Function
			if(this->m_pinList[10*i + 2]->Get() == 1 && this->m_pinList[10*i + 3]->Get() == 1){ // R0
				this->m_intVal[i] = 0x00;
			}
			else if(this->m_pinList[10*i + 4]->Get() == 1 && this->m_pinList[10*i + 5]->Get() == 1){ // R9
				this->m_intVal[i] = 0x09;
			}
			else
			{
				if((this->m_lastCLK[i] & 1) && this->m_pinList[10*i + 0]->Get() == 0) // Falling Edge
					// bit 0
					this->m_intVal[i] ^= 1;
					
				if((this->m_lastCLK[i] & 2) && this->m_pinList[10*i + 1]->Get() == 0) // Falling Edge
					// bit 1
					this->m_intVal[i] ^= 2;//this->m_pinList[i][-3]->Get() ^ 1
				
				
				if((this->m_lastCLK[i] & 4) && (lastIntVal&2) == 0) // Falling Edge
					// bit 2
					this->m_intVal[i] ^= 4;//this->m_pinList[i][-3]->Get() ^ 1
				
				if((this->m_lastCLK[i] & 8) && (lastIntVal&4) == 0) // Falling Edge
					// bit 3
					this->m_intVal[i] ^= 8;//this->m_pinList[i][-3]->Get() ^ 1
			}
				
			// For decade counter
			if(this->m_intVal[i] == this->m_modulo){
				this->m_intVal[i] = 0;
			}
			
			this->m_lastCLK[i] = this->m_pinList[10*i + 0]->Get()? 0x1: 0x0;
			this->m_lastCLK[i] |= this->m_pinList[10*i + 1]->Get()? 0x2: 0x0;
			this->m_lastCLK[i] |= (lastIntVal & 0x6) << 1;
		}
	}
	virtual void Tick2() // update
	{
		int i;
		for(i=0; i<m_attr[0]; i++){
			m_pinList[10*i + 6]->Set((m_intVal[i] & 1)? 1: 0);
			m_pinList[10*i + 7]->Set((m_intVal[i] & 2)? 1: 0);
			m_pinList[10*i + 8]->Set((m_intVal[i] & 4)? 1: 0);
			m_pinList[10*i + 9]->Set((m_intVal[i] & 8)? 1: 0);
		}
	}
};

class C7493 : public GenericAsyncCounter
{
public:
	C7493(const vector<Wire*>& pinList): GenericAsyncCounter()
	{
		int attr[3] = {1,10,0}; // 1 circuit
		int pl[10] = {14,1,2,3,-1,-1,12,9,8,11};
		vector<Wire*> pll;
		ConvertFromPinNumberList(pinList, pll, pl, sizeof(pl)/sizeof(int));
		Init(pll, attr);
		m_modulo = 16;
	}
};
class C7492 : public GenericAsyncCounter
{
public:
	C7492(const vector<Wire*>& pinList): GenericAsyncCounter()
	{
		int attr[3] = {1,10,0}; // 1 circuit
		vector<Wire*> pll(1*10);
		int pl[10] = {14,1,6,7,-1,-1,12,9,8,11};
		int i;
		for(i=0; i<pll.size(); i++) {
			if(pl[i]>=0){
				pll[i] = pinList[pl[i]];
			}else if(pl[i] == -1){
				pll[i] = g_wireManager.GetGND();
			}
		}
		Init(pll, attr);
		m_modulo = 12;
	}
};
class C7490 : public GenericAsyncCounter
{
public:
	C7490(const vector<Wire*>& pinList): GenericAsyncCounter()
	{
		int attr[3] = {1,10,0}; // 1 circuit
		vector<Wire*> pll(1*10);
		int pl[10] = {14,1,2,3,6,7,12,9,8,11};
		int i;
		for(i=0; i<pll.size(); i++) {
			if(pl[i]>=0){
				pll[i] = pinList[pl[i]];
			}else if(pl[i] == -1){
				pll[i] = g_wireManager.GetGND();
			}
		}
		Init(pll, attr);
		m_modulo = 10;
	}
};

class C74393 : public GenericAsyncCounter
{
public:
	C74393(const vector<Wire*>& pinList): GenericAsyncCounter()
	{
		int attr[3] = {2,10,0}; // 2 circuits
		vector<Wire*> pll(2*10);
		
		Wire* p3 = pinList[3];
		if(p3 == g_wireManager.GetNC())
		{
			p3 = g_wireManager.NewWire("");
		}
		Wire* p11 = pinList[11];
		if(p11 == g_wireManager.GetNC())
		{
			p11 = g_wireManager.NewWire("");
		}
		
		int pl[20] = {1,3,2,2,-1,-1,3,4,5,6, 13,11,12,12,-1,-1,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) {
			if(pl[i] == 3){
				pll[i] = p3;
			}
			else if(pl[i] == 11){
				pll[i] = p11;
			}
			else if(pl[i]>=0){
				pll[i] = pinList[pl[i]];
			}else if(pl[i] == -1){
				pll[i] = g_wireManager.GetGND();
			}
		}
		Init(pll, attr);
		m_modulo = 16;
	}
};

