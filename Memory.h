//pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// RAMs & ROMs
///////////////////////////////////////////////////////////////////////////////////////////////////

class GenericROM: public CIC
{
protected:
	int m_bitWidth, m_addrWidth;
	unsigned char* m_memoryArray;
	int m_memorySize;
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		m_bitWidth = attr[0]; // 1-8
		m_addrWidth = attr[1];
		
		m_pinList = pinList; // list of pin list
		// [0-1]: /OE, [1-m_bitWidth]: Data bus, [m_bitWidth+1-(m_bitWidth+m_addrWidth)]: Address Bus
	}
public:
	GenericROM()
	{
		m_bitWidth  = 0;
		m_addrWidth = 0;
		m_memoryArray = NULL;
		m_memorySize = 0;
	}
	virtual ~GenericROM()
	{
		if(m_memoryArray) delete [] m_memoryArray;
	}
	void SetAttribute(const char* attrText)// 
	{
		CIC::SetAttribute(attrText);
		Memory_Init(attrText, 0x00); // Load Initial Value here
	}
	void Memory_Init(const char* fn, int initVal)
	{
		int memorySize = 1 << m_addrWidth;
		m_memorySize = memorySize;
		//printf("memorySize = %d\n", memorySize);
		int i;
		// Allocate
		if(m_memoryArray) delete [] m_memoryArray;
		m_memoryArray = new unsigned char[memorySize];
		memset(m_memoryArray, initVal, memorySize);
		if(fn[0])
		{
			FILE* fp = fopen(fn, "rb");
			if(fp){
				for(i=0; i<memorySize; i++)
				{
					int a = fgetc(fp);
					if(a == EOF) break;
					m_memoryArray[i] = a;
				}
				fclose(fp);
			}else{
				printf("Error: can't read %s.\n", fn);
			}
		}
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		if(m_pinList[0]->Get() == 0 && m_pinList[1]->Get() == 0) // /OE = 0
		{
			int i;
			int addr = 0;
			for(i=0; i<m_addrWidth; i++)
			{
				if(m_pinList[m_bitWidth+2 + i]->Get()) addr |= (1<<i);
			}
			if(addr >= m_memorySize) addr = 0;
			int d = m_memoryArray[addr];
			// store
			for(i=0; i<m_bitWidth; i++)
			{
				if(d & (1<<i)) m_pinList[2+i]->Set(1);
				else m_pinList[2+i]->Set(0);
			}
		}
	}
};

class C2764: public GenericROM
{
public:
	C2764(const vector<Wire*>& pinList): GenericROM()
	{
		vector<Wire*> newPinList;
		newPinList.push_back(pinList[20]); // CE
		newPinList.push_back(pinList[22]); // OE
		// Data
		newPinList.push_back(pinList[11]);
		newPinList.push_back(pinList[12]);
		newPinList.push_back(pinList[13]);
		newPinList.push_back(pinList[15]);
		newPinList.push_back(pinList[16]);
		newPinList.push_back(pinList[17]);
		newPinList.push_back(pinList[18]);
		newPinList.push_back(pinList[19]);
		// Addr
		newPinList.push_back(pinList[10]);//0
		newPinList.push_back(pinList[9]);
		newPinList.push_back(pinList[8]);
		newPinList.push_back(pinList[7]);
		newPinList.push_back(pinList[6]); //4
		newPinList.push_back(pinList[5]);
		newPinList.push_back(pinList[4]);
		newPinList.push_back(pinList[3]);
		newPinList.push_back(pinList[25]);//8
		newPinList.push_back(pinList[24]);
		newPinList.push_back(pinList[21]);
		newPinList.push_back(pinList[23]);
		newPinList.push_back(pinList[2]); //12
		
		int attr[4] = {8,13,0,0};
		Init(newPinList, attr);
	}
};

// 74LS189 4x16 RAM
class C74189: public CIC
{
protected:
	int m_bitWidth, m_addrWidth;
	unsigned char m_memoryArray[16];
public:
	C74189(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_bitWidth  = 4;
		m_addrWidth = 4;
	}
	virtual ~C74189()
	{
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		// CS:2, WE:3
		int cs = m_pinList[2]->Get();
		int we = m_pinList[3]->Get();
		
		int i;
		int addr = 0;
		if(m_pinList[1]->Get()) addr |= 1;
		if(m_pinList[15]->Get()) addr |= 2;
		if(m_pinList[14]->Get()) addr |= 4;
		if(m_pinList[13]->Get()) addr |= 8;
		
		if(cs == 0)
		{
			if(we == 0) // Writing (output is hi-z)
			{
				int d = 0;
				if(m_pinList[4]->Get()) d |= 1;
				if(m_pinList[6]->Get()) d |= 2;
				if(m_pinList[10]->Get()) d |= 4;
				if(m_pinList[12]->Get()) d |= 8;
				m_memoryArray[addr] = d;
			}
			else // reading
			{
				int d = m_memoryArray[addr];
				// update output
				m_pinList[5]->Set( d&1? 1: 0);
				m_pinList[7]->Set( d&2? 1: 0);
				m_pinList[9]->Set( d&4? 1: 0);
				m_pinList[11]->Set(d&8? 1: 0);
			}
		}
	}
};

