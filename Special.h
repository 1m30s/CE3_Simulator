//pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Special Function ICs
///////////////////////////////////////////////////////////////////////////////////////////////////

class C74181: public CIC
{
protected:
	int m_intVal, m_lastCLK;
public:
	C74181(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_intVal = 0x00;
		m_lastCLK = 1;
	}
	
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		
		// Decode inputs
		int s=0, a=0, b=0;
		if(m_pinList[6]->Get()) s |= 0x01;
		if(m_pinList[5]->Get()) s |= 0x02;
		if(m_pinList[4]->Get()) s |= 0x04;
		if(m_pinList[3]->Get()) s |= 0x08;
		
		if(m_pinList[2 ]->Get()) a |= 0x01;
		if(m_pinList[23]->Get()) a |= 0x02;
		if(m_pinList[21]->Get()) a |= 0x04;
		if(m_pinList[19]->Get()) a |= 0x08;
		
		if(m_pinList[1 ]->Get()) b |= 0x01;
		if(m_pinList[22]->Get()) b |= 0x02;
		if(m_pinList[20]->Get()) b |= 0x04;
		if(m_pinList[18]->Get()) b |= 0x08;
		
		// Other input: /Cn, M
		int cin = m_pinList[7]->Get() ^ 1;
		int m   = m_pinList[8]->Get();
		
		int f = 0; // Result
		// Based on table 2 of SN74181N datasheet
		// (should rewrite by logic)
		if(m == 1) // Logic
		{
			int res[16] = {
				~a, ~(a|b), (~a)&b, 0, ~(a&b), ~b, a^b, a&(~b),
				(~a)|b, ~(a^b), b, a&b, 0xF, a|(~b), a|b, a};
			f = res[s] & 0x0F;
		}
		else // Arithmetic
		{
			int res[16] = {
				a, a|b, a|(~b), -1, a+(a&~b),
				(a|b)+(a&~b), a-b-1, (a&~b)-1,
				a+(a&b), a+b, (a|~b)+(a&b), (a&b)-1, 
				a+a, (a|b)+a, (a|~b)+a, a-1};
			f = (res[s] + cin ) & 0x1F; // with carry
		}
		
		m_pinList[9]->Set( (f & 0x01)? 1: 0);
		m_pinList[10]->Set((f & 0x02)? 1: 0);
		m_pinList[11]->Set((f & 0x04)? 1: 0);
		m_pinList[13]->Set((f & 0x08)? 1: 0);
		
		m_pinList[16]->Set((f & 0x10)? 0: 1); // /Cn
		m_pinList[14]->Set((a == b)? 1: 0); // A=N
		
		// Other output: A=N(14), /Cn+4(16), X(15),Y(17)
		// X,Y output (for Carry Look-ahead ICs) is not emulated yet.
	}
};

// 4x4 Register File. Output is OC but implemented as 3-State
class C74170: public CIC
{
	int m_intVal[4];
public:
	C74170(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	virtual void Tick1() // (for flip-flops,) output signal is not updated to avoid racing
	{
		int i;
		
		// Decode inputs
		int rs=0, ws=0, input=0;
		if(m_pinList[5]->Get()) rs |= 0x01; // Read Select
		if(m_pinList[4]->Get()) rs |= 0x02;
		if(m_pinList[14]->Get()) ws |= 0x01; // Write Select
		if(m_pinList[13]->Get()) ws |= 0x02;
		
		if(m_pinList[15]->Get()) input |= 0x01;
		if(m_pinList[14]->Get()) input |= 0x02;
		if(m_pinList[13]->Get()) input |= 0x04;
		if(m_pinList[12]->Get()) input |= 0x08;
		
		int output = m_intVal[rs];
		if(m_pinList[11]->Get() == 0) // /GR=0
		{
			if(output & 0x01) m_pinList[10]->Set(0);
			else m_pinList[10]->Set(1);
			if(output & 0x02) m_pinList[9]->Set(0);
			else m_pinList[9]->Set(1);
			if(output & 0x04) m_pinList[7]->Set(0);
			else m_pinList[7]->Set(1);
			if(output & 0x08) m_pinList[6]->Set(0);
			else m_pinList[6]->Set(1);
		}
		
		if(m_pinList[12]->Get()==0) // /GW=0
		{
			m_intVal[ws] = input;
		}
	}
	
};
