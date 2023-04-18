#pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"

#define GATE_TYPE_NAND 0
#define GATE_TYPE_AND 1
#define GATE_TYPE_NOR 2
#define GATE_TYPE_OR 3
#define GATE_TYPE_XNOR 4
#define GATE_TYPE_XOR 5
#define GATE_TYPE_BUF 6
class CGenericGate : public CIC
{
	int m_attr[2];
	int m_type; // 0-5: NAND,AND,NOR,OR,XNOR,XOR
public:
	// attr: size of pin.
	// e.g. for 7400 set 4(Quad),3(2-input+1-output = 3 pins/el.)
	CGenericGate()
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_type = GATE_TYPE_NAND;
	}
	CGenericGate(int type)
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_type = type;
	}
	virtual void Tick1()
	{
		int i, j;
		for(i=0; i<m_attr[0]; i++)
		{
			int res = 0;
			if(m_type == GATE_TYPE_NAND || m_type == GATE_TYPE_AND) // AND
			{
				res = 1;
				for(j=0; j<m_attr[1]-1; j++)
				{
					res &= m_pinList[i*m_attr[1] + j]->Get();
				}
				if(m_type == GATE_TYPE_NAND) res ^= 1;
			}else if(m_type == GATE_TYPE_NOR || m_type == GATE_TYPE_OR) // OR
			{
				for(j=0; j<m_attr[1]-1; j++)
				{
					res |= m_pinList[i*m_attr[1] + j]->Get();
				}
				if(m_type == GATE_TYPE_NOR) res ^= 1;
			}
			else if(m_type == GATE_TYPE_XNOR || m_type == GATE_TYPE_XOR) // XOR
			{
				for(j=0; j<m_attr[1]-1; j++)
				{
					res ^= m_pinList[i*m_attr[1] + j]->Get();
				}
				if(m_type == GATE_TYPE_XNOR) res ^= 1;
			}
			else
			{
				res = m_pinList[i*m_attr[1]]->Get();
				j=1;
			}
			m_pinList[i*m_attr[1] + j]->Set(res);
		}
	}
protected:
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		m_pinList = pinList; // Copy.
		m_attr[0] = attr[0];
		m_attr[1] = attr[1];
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CGenericBuffer : public CIC
{
	int m_attr[3]; // [0]: circuit number, [1]: bitwise buffer invert, [2]: enable logic level (1:positive)
	int m_type; // 3S, OC
public:
	// attr: size of pin.
	CGenericBuffer()
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_attr[2] = 0;
		m_type = 0;
	}
	CGenericBuffer(int type)
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_attr[2] = 0;
		m_type = type;
	}
	virtual void Tick1()
	{
		int i, j;
		for(i=0; i<m_attr[0]; i++)
		{
			// [0]: EN
			// [1]: Input
			// [2]: Output
			int en = m_pinList[i*3 + 0]->Get();
			if(en == ((m_attr[2] >> i) & 1))
			{
				int r = ((m_attr[1] >> i) & 1) ^ m_pinList[i*3 + 1]->Get();
				m_pinList[i*3 + 2]->Set(r&1);
			}
		}
	}
protected:
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		m_pinList = pinList; // Copy.
		m_attr[0] = attr[0];
		m_attr[1] = attr[1];
		m_attr[2] = attr[2];
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
class CGenericTransceiver : public CIC
{
	int m_attr[3]; // [0]: circuit number, [1]: buffer invert(A->B), [2]: buffer invert (B->A)
	int m_type; // 3S, OC
public:
	// attr: size of pin.
	CGenericTransceiver()
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_attr[2] = 0;
		m_type = 0;
	}
	CGenericTransceiver(int type)
	{
		m_attr[0] = 0;
		m_attr[1] = 0;
		m_attr[2] = 0;
		m_type = type;
	}
	virtual void Tick1()
	{
		if(m_pinList.size()<2) return;
		// pin[0]: EN, pin[1]: DIR (1: A->B)
		int i;
		int en  = m_pinList[0]->Get(); // /EN 
		int dir = m_pinList[1]->Get(); // DIR
		if(en == 0) // /EN=0
		{
			for(i=0; i<m_attr[0]; i++)
			{
				if(dir) // DIR=1
				{
					int l = m_pinList[2+i*2 + 0]->Get();
					m_pinList[2+i*2+1]->Set((l^m_attr[1])&1);
				}
				else // DIR=0
				{
					int l = m_pinList[2+i*2 + 1]->Get();
					m_pinList[2+i*2+0]->Set((l^m_attr[2])&1);
				}
			}
		}
	}
protected:
	void Init(const vector<Wire*>& pinList, const int* attr)
	{
		m_pinList = pinList; // Copy.
		m_attr[0] = attr[0];
		m_attr[1] = attr[1];
		m_attr[2] = attr[2];
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffers
///////////////////////////////////////////////////////////////////////////////////////////////////
class C74125: public CGenericBuffer
{
public:
	C74125(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {4,0,0}; // Quad, Non-inv, Negative EN

		vector<Wire*> pll(4*3);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C74126: public CGenericBuffer
{
public:
	C74126(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {4,0,0xFF}; // Quad, Non-inv, Positive EN

		vector<Wire*> pll(4*3);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

class C74365: public CGenericBuffer
{
public:
	C74365(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		m_pinList = pinList;
	}
	virtual void Tick1()
	{
		if(m_pinList.size()<2) return;
		int en  = (m_pinList[1]->Get() == 0) || (m_pinList[15]->Get() == 0); // /EN 
		
		if(en) // /EN=0
		{
			m_pinList[3]->Set(m_pinList[2]->Get());
			m_pinList[5]->Set(m_pinList[4]->Get());
			m_pinList[7]->Set(m_pinList[6]->Get());
			m_pinList[9]->Set(m_pinList[10]->Get());
			m_pinList[11]->Set(m_pinList[12]->Get());
			m_pinList[13]->Set(m_pinList[14]->Get());
		}
	}
};
class C74366: public CGenericBuffer
{
public:
	C74366(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		m_pinList = pinList;
	}
	virtual void Tick1()
	{
		if(m_pinList.size()<2) return;
		int en  = (m_pinList[1]->Get() == 0) || (m_pinList[15]->Get() == 0); // /EN 
		
		if(en) // /EN=0
		{
			m_pinList[3]->Set(m_pinList[2]->Get() ^ 1);
			m_pinList[5]->Set(m_pinList[4]->Get() ^ 1);
			m_pinList[7]->Set(m_pinList[6]->Get() ^ 1);
			m_pinList[9]->Set(m_pinList[10]->Get() ^ 1);
			m_pinList[11]->Set(m_pinList[12]->Get() ^ 1);
			m_pinList[13]->Set(m_pinList[14]->Get() ^ 1);
		}
	}
};
class C74367: public CGenericBuffer
{
public:
	C74367(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {6,0,0}; // Quad, Non-inv, Negative EN
		// [0]: EN
		// [1]: Input
		// [2]: Output

		vector<Wire*> pll(6*3);
		int pl[18] = {1,2,3, 1,4,5, 1,6,7, 15,10,9, 15,12,11, 15,14,13};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C74368: public CGenericBuffer
{
public:
	C74368(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {6,0xFF,0}; // Quad, Inv, Negative EN
		// [0]: EN
		// [1]: Input
		// [2]: Output

		vector<Wire*> pll(6*3);
		int pl[18] = {1,2,3, 1,4,5, 1,6,7, 15,10,9, 15,12,11, 15,14,13};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// 
//# 74240 Gate: Inverting, N: Negative enable
class C74240: public CGenericBuffer
{
public:
	C74240(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {8,0xFF,0}; 

		vector<Wire*> pll(8*3);
		int pl[24] = {1,2,18, 1,4,16, 1,6,14, 1,8,12,
			19,17,3, 19,15,5, 19,13,7, 19,11,9};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
//# 74241 Gate: Non-Inverting, N: Negative/Positive enable
class C74241: public CGenericBuffer
{
public:
	C74241(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {8,0x00,0x0F}; 

		vector<Wire*> pll(8*3);
		int pl[24] = {1,2,18, 1,4,16, 1,6,14, 1,8,12,
			19,17,3, 19,15,5, 19,13,7, 19,11,9};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
//# 74244 Gate: Non-Inverting, N: Negative enable
class C74244: public CGenericBuffer
{
public:
	C74244(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {8,0x00,0}; 

		vector<Wire*> pll(8*3);
		int pl[24] = {1,2,18, 1,4,16, 1,6,14, 1,8,12,
			19,17,3, 19,15,5, 19,13,7, 19,11,9};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

//# 74242 Transceiver
//# Gate: Inverting, N: Negative/Positive enable
class C74242: public CGenericBuffer
{
public:
	C74242(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {8,0xFF,0xF0}; 

		vector<Wire*> pll(8*3);
		int pl[24] = {1,3,11, 1,4,10,1,5,9,1,6,8,
			13,11,3,13,10,4,13,9,5,13,8,6};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
//# 74243 Transceiver
//# Gate: Non-Inverting, N: Negative/Positive enable
class C74243: public CGenericBuffer
{
public:
	C74243(const vector<Wire*>& pinList) : CGenericBuffer()
	{
		int attr[3] = {8,0x00,0xF0}; 

		vector<Wire*> pll(8*3);
		int pl[24] = {1,3,11, 1,4,10,1,5,9,1,6,8,
			13,11,3,13,10,4,13,9,5,13,8,6};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////
// Transceivers
///////////////////////////////////////////////////////////////////////////////////////////////////

// 74245 Transceiver
// Gate: Non-Inverting
// 19: /EN, 1: DIR(1: A->B)
class C74245: public CGenericTransceiver
{
public:
	C74245(const vector<Wire*>& pinList) : CGenericTransceiver()
	{
		const int attr[3] = {8,0,0}; // 

		vector<Wire*> pll(8*2+2);
		const int pl[18] = {19/*EN*/,1/*DIR*/, 2,18,3,17,4,16,5,15,6,14,7,13,8,12,9,11};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C74645: public C74245{};

class C74640: public CGenericTransceiver
{
public:
	C74640(const vector<Wire*>& pinList) : CGenericTransceiver()
	{
		const int attr[3] = {8,0xFF,0xFF}; // 

		vector<Wire*> pll(8*2+2);
		const int pl[18] = {19/*EN*/,1/*DIR*/, 2,18,3,17,4,16,5,15,6,14,7,13,8,12,9,11};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C74643: public CGenericTransceiver
{
public:
	C74643(const vector<Wire*>& pinList) : CGenericTransceiver()
	{
		const int attr[3] = {8,0xFF,0}; // 

		vector<Wire*> pll(8*2+2);
		const int pl[18] = {19/*EN*/,1/*DIR*/, 2,18,3,17,4,16,5,15,6,14,7,13,8,12,9,11};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SSI Gates
///////////////////////////////////////////////////////////////////////////////////////////////////

class CGenericNAND : public CGenericGate
{
public:
	CGenericNAND() : CGenericGate(GATE_TYPE_NAND){}
};


class C7404: public CGenericGate
{
public:
	C7404(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {6,2};

		vector<Wire*> pll(12);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// NAND
class C7400: public CGenericGate
{
public:
	C7400(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {4,3}; // Quad, 3 pins per element

		vector<Wire*> pll(12);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};


class C7410: public CGenericGate
{
public:
	C7410(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {3,4};

		vector<Wire*> pll(12);
		int pl[12] = {1,2,13,12, 3,4,5,6, 11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

class C7420: public CGenericGate
{
public:
	C7420(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {2,5};

		vector<Wire*> pll(10);
		int pl[10] = {1,2,4,5,6, 13,12,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C7430: public CGenericGate
{
public:
	C7430(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {1,9};

		vector<Wire*> pll(9);
		int pl[9] = {1,2,3,4,5,6,12,11,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C7440: public C7420{};

class C74133: public CGenericGate
{
public:
	C74133(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NAND)
	{
		int attr[2] = {1,14};

		vector<Wire*> pll(14);
		int pl[14] = {1,2,3,4,5,6,7,10,11,12,13,14,15, 9};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// AND
class C7408: public CGenericGate
{
public:
	C7408(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_AND)
	{
		int attr[2] = {4,3}; // Quad, 3 pins per element

		vector<Wire*> pll(12);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};


class C7411: public CGenericGate
{
public:
	C7411(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_AND)
	{
		int attr[2] = {3,4};

		vector<Wire*> pll(12);
		int pl[12] = {1,2,13,12, 3,4,5,6, 11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

class C7421: public CGenericGate
{
public:
	C7421(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_AND)
	{
		int attr[2] = {2,5};

		vector<Wire*> pll(10);
		int pl[10] = {1,2,4,5,6, 13,12,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// NOR
class C7402: public CGenericGate
{
public:
	C7402(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NOR)
	{
		int attr[2] = {4,3}; // Quad, 3 pins per element

		vector<Wire*> pll(12);
		int pl[12] = {6,5,4,3,2,1,8,9,10,11,12,13};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
class C7427: public CGenericGate
{
public:
	C7427(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NOR)
	{
		int attr[2] = {3,4};

		vector<Wire*> pll(12);
		int pl[12] = {1,2,13,12, 3,4,5,6, 11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};

class C7425: public CGenericGate
{
public:
	C7425(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_NOR)
	{
		int attr[2] = {2,5};

		vector<Wire*> pll(12);
		int pl[10] = {1,2,4,5,6, 13,12,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// OR
class C7432: public CGenericGate
{
public:
	C7432(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_OR)
	{
		int attr[2] = {4,3}; // Quad, 3 pins per element

		vector<Wire*> pll(12);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};
// XOR
class C7486: public CGenericGate
{
public:
	C7486(const vector<Wire*>& pinList) : CGenericGate(GATE_TYPE_XOR)
	{
		int attr[2] = {4,3}; // Quad, 3 pins per element

		vector<Wire*> pll(12);
		int pl[12] = {1,2,3,4,5,6, 13,12,11,10,9,8};
		int i;
		for(i=0; i<pll.size(); i++) pll[i] = pinList[pl[i]];
		Init(pll, attr);
	}
};


// AND-OR-INV
class C7454: public CIC
{
public:
	C7454(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	virtual void Tick1()
	{
		int i, j;
		if((m_attrText[0] == 'L' && m_attrText[1] == 'S') || 
			(m_attrText[0] == 'A' && m_attrText[1] == 'L'))
		{
			int res = 0;
			if(m_pinList[1]->Get() && m_pinList[2]->Get() && m_pinList[3]->Get()) res |= 1;
			if(m_pinList[4]->Get() && m_pinList[5]->Get()) res |= 1;
			if(m_pinList[9]->Get() && m_pinList[10]->Get() && m_pinList[11]->Get()) res |= 1;
			if(m_pinList[12]->Get() && m_pinList[13]->Get()) res |= 1;
			m_pinList[6]->Set(res ^ 1);
		}
		else
		{
			int res = 0;
			if(m_pinList[1]->Get() && m_pinList[13]->Get()) res |= 1;
			if(m_pinList[2]->Get() && m_pinList[3]->Get()) res |= 1;
			if(m_pinList[4]->Get() && m_pinList[5]->Get()) res |= 1;
			if(m_pinList[9]->Get() && m_pinList[10]->Get()) res |= 1;
			m_pinList[8]->Set(res ^ 1);
		}
	}
};

// AND-OR-INV
class C7455: public CIC
{
public:
	C7455(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	virtual void Tick1()
	{
		int i, j;
		{
			int res = 0;
			if(m_pinList[1]->Get() && m_pinList[2]->Get() && m_pinList[3]->Get() && m_pinList[4]->Get()) res |= 1;
			if(m_pinList[10]->Get() && m_pinList[11]->Get() && m_pinList[12]->Get() && m_pinList[13]->Get()) res |= 1;
			m_pinList[8]->Set(res ^ 1);
		}
	}
};

// AND-OR-INV
class C7451: public CIC
{
public:
	C7451(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	virtual void Tick1()
	{
		int i, j;
		if((m_attrText[0] == 'L' && m_attrText[1] == 'S') || 
			(m_attrText[0] == 'A' && m_attrText[1] == 'L'))
		{
			int res = 0;
			if(m_pinList[2]->Get() && m_pinList[3]->Get()) res |= 1;
			if(m_pinList[4]->Get() && m_pinList[5]->Get()) res |= 1;
			m_pinList[6]->Set(res ^ 1);
			res = 0;
			if(m_pinList[1]->Get() && m_pinList[13]->Get() && m_pinList[12]->Get()) res |= 1;
			if(m_pinList[11]->Get() && m_pinList[10]->Get() && m_pinList[9]->Get()) res |= 1;
			m_pinList[8]->Set(res ^ 1);
		}
		else
		{
			int res = 0;
			if(m_pinList[2]->Get() && m_pinList[3]->Get()) res |= 1;
			if(m_pinList[4]->Get() && m_pinList[5]->Get()) res |= 1;
			m_pinList[6]->Set(res ^ 1);
			res = 0;
			if(m_pinList[1]->Get() && m_pinList[13]->Get()) res |= 1;
			if(m_pinList[10]->Get() && m_pinList[9]->Get()) res |= 1;
			m_pinList[8]->Set(res ^ 1);
		}
	}
};
