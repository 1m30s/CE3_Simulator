//pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Decoders
///////////////////////////////////////////////////////////////////////////////////////////////////

class C74138: public CIC
{
public:
	C74138(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int i;
		int g1 = m_pinList[6]->Get();
		int g2 = m_pinList[4]->Get() | m_pinList[5]->Get();
		int sel = 0;
		if(m_pinList[1]->Get()) sel |= 1;
		if(m_pinList[2]->Get()) sel |= 2;
		if(m_pinList[3]->Get()) sel |= 4;
		int y = 0xFF;
		if(g1 == 1 and g2 == 0){
			y &= ~(1<<sel);
		}
		
		const int pl[] = {15,14,13,12,11,10,9,7};
		for(i=0; i<8; i++){
			m_pinList[pl[i]]->Set(y & (1<<i)? 1: 0);
		}
	}
};

class C74238: public CIC
{
public:
	C74238(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int i;
		int g1 = m_pinList[6]->Get();
		int g2 = m_pinList[4]->Get() | m_pinList[5]->Get();
		int sel = 0;
		if(m_pinList[1]->Get()) sel |= 1;
		if(m_pinList[2]->Get()) sel |= 2;
		if(m_pinList[3]->Get()) sel |= 4;
		int y = 0xFF;
		if(g1 == 1 and g2 == 0){
			y &= ~(1<<sel);
		}
		y ^= 0xFF;
		
		const int pl[] = {15,14,13,12,11,10,9,7};
		for(i=0; i<8; i++){
			m_pinList[pl[i]]->Set(y & (1<<i)? 1: 0);
		}
	}
};

class C74139: public CIC
{
public:
	C74139(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int i;
		int g = m_pinList[1]->Get();
		int sel = 0;
		if(m_pinList[2]->Get()) sel |= 1;
		if(m_pinList[3]->Get()) sel |= 2;
		int y = 0x0F;
		if(g == 0) y &= ~(1<<sel);
		m_pinList[4]->Set(y & 0x01? 1: 0);
		m_pinList[5]->Set(y & 0x02? 1: 0);
		m_pinList[6]->Set(y & 0x04? 1: 0);
		m_pinList[7]->Set(y & 0x08? 1: 0);
		
		g = m_pinList[15]->Get();
		sel = 0;
		if(m_pinList[14]->Get()) sel |= 1;
		if(m_pinList[13]->Get()) sel |= 2;
		y = 0x0F;
		if(g == 0) y &= ~(1<<sel);
		m_pinList[12]->Set(y & 0x01? 1: 0);
		m_pinList[11]->Set(y & 0x02? 1: 0);
		m_pinList[10]->Set(y & 0x04? 1: 0);
		m_pinList[9]->Set(y & 0x08? 1: 0);
	}
};


class C7442: public CIC
{
public:
	C7442(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int i;
		int sel = 0;
		if(m_pinList[15]->Get()) sel |= 1;
		if(m_pinList[14]->Get()) sel |= 2;
		if(m_pinList[13]->Get()) sel |= 4;
		if(m_pinList[12]->Get()) sel |= 8;
		int y = ~(1<<sel);
		
		const int pl[] = {1,2,3,4,5,6,7,9,10,11};
		for(i=0; i<10; i++){
			m_pinList[pl[i]]->Set(y & (1<<i)? 1: 0);
		}
	}
};

class C74154: public CIC
{
public:
	C74154(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int g2 = m_pinList[18]->Get() | m_pinList[19]->Get();
		int sel = 0;
		int i;
		if(m_pinList[23]->Get()) sel |= 1;
		if(m_pinList[22]->Get()) sel |= 2;
		if(m_pinList[21]->Get()) sel |= 4;
		if(m_pinList[20]->Get()) sel |= 8;
		int y = 0xFFFF;
		if(g2 == 0){
			y &= ~(1<<sel);
		}
		
		const int pl[] = {1,2,3,4,5,6,7,8,9,10,11,13,14,15,16,17};
		for(i=0; i<16; i++){
			m_pinList[pl[i]]->Set(y & (1<<i)? 1: 0);
		}
	}
};

// Selector

// 8-1 Selector
class C74151: public CIC
{
public:
	C74151(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int sel = 0;
		if(m_pinList[11]->Get()) sel |= 1;
		if(m_pinList[12]->Get()) sel |= 2;
		if(m_pinList[13]->Get()) sel |= 4;
		
		int stb = m_pinList[7]->Get();
		const int pl[] = {4,3,2,1,15,14,13,12};
		if(stb == 1){
			m_pinList[5]->Set(0); // Y
			m_pinList[6]->Set(1); // W
		}else{
			m_pinList[5]->Set(m_pinList[pl[sel]]->Get()); // Y
			m_pinList[6]->Set(m_pinList[pl[sel]]->Get() ^ 1); // W
		}
	}
};


// 8-1 Selector (3S)
class C74251: public CIC
{
public:
	C74251(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int sel = 0;
		if(m_pinList[11]->Get()) sel |= 1;
		if(m_pinList[12]->Get()) sel |= 2;
		if(m_pinList[13]->Get()) sel |= 4;
		
		int oe = m_pinList[7]->Get();
		const int pl[] = {4,3,2,1,15,14,13,12};
		if(oe == 0){
			m_pinList[5]->Set(m_pinList[pl[sel]]->Get()); // Y
			m_pinList[6]->Set(m_pinList[pl[sel]]->Get() ^ 1); // W
		}
	}
};

// Dual 4-1 Selector
class C74153: public CIC
{
public:
	C74153(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int sel = 0;
		if(m_pinList[14]->Get()) sel |= 1;
		if(m_pinList[2]->Get()) sel |= 2;
		
		int stb1 = m_pinList[1]->Get();
		int stb2 = m_pinList[15]->Get();
		const int pl[] = {6,5,4,3,  10,11,12,13};
		if(stb1 == 1){
			m_pinList[7]->Set(0); // Y
		}else{
			m_pinList[7]->Set(m_pinList[pl[sel]]->Get()); // Y
		}
		if(stb2 == 1){
			m_pinList[9]->Set(0); // Y
		}else{
			m_pinList[9]->Set(m_pinList[pl[4+sel]]->Get()); // Y
		}
	}
};

class C74253: public CIC
{
public:
	C74253(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		int sel = 0;
		if(m_pinList[14]->Get()) sel |= 1;
		if(m_pinList[2]->Get()) sel |= 2;
		
		int stb1 = m_pinList[1]->Get();
		int stb2 = m_pinList[15]->Get();
		const int pl[] = {6,5,4,3,  10,11,12,13};
		if(stb1 == 0){
			m_pinList[7]->Set(m_pinList[pl[sel]]->Get()); // Y
		}
		if(stb2 == 0){
			m_pinList[9]->Set(m_pinList[pl[4+sel]]->Get()); // Y
		}
	}
};

// Quad 2-1 Selector
class C74157: public CIC
{
protected:
	int m_inv;
public:
	C74157(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_inv = 0;
	}
	
	virtual void Tick1(){
		
		int sel = m_pinList[1]->Get() & 1;
		const int inputPin[] = {2,5,11,14 , 3,6,10,13};
		const int outputPin[] = {4,7,9,12};
		int stb = m_pinList[15]->Get();
		int i;
		for(i=0; i<4; i++){
			if(stb == 1){
				m_pinList[outputPin[i]]->Set(0); // Y
			}else{
				m_pinList[outputPin[i]]->Set(m_pinList[inputPin[4*sel+i]]->Get() ^ m_inv); // Y
			}
		}
	}
};
class C74158: public C74157
{
public:
	C74158(const vector<Wire*>& pinList): C74157(pinList)
	{
		m_inv = 1;
	}
};

// Quad 2-1 Selector (Tristate)
class C74257: public CIC
{
protected:
	int m_inv;
public:
	C74257(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
		m_inv = 0;
	}
	
	virtual void Tick1(){
		
		int sel = m_pinList[1]->Get() & 1;
		const int inputPin[] = {2,5,11,14 , 3,6,10,13};
		const int outputPin[] = {4,7,9,12};
		int stb = m_pinList[15]->Get();
		int i;
		for(i=0; i<4; i++){
			if(stb == 0){
				m_pinList[outputPin[i]]->Set(m_pinList[inputPin[4*sel+i]]->Get() ^ m_inv); // Y
			}
		}
	}
};
class C74258: public C74257
{
public:
	C74258(const vector<Wire*>& pinList): C74257(pinList)
	{
		m_inv = 1;
	}
};




// Comparator
class C74521: public CIC
{
public:
	C74521(const vector<Wire*>& pinList)
	{
		m_pinList = pinList;
	}
	
	virtual void Tick1(){
		
		const int a[] = {2,4,6,8,11,13,15,17};
		const int b[] = {3,5,7,9,12,13,16,18};
		int g = m_pinList[1]->Get();
		if(g == 0){
			int equal = 1;
			int i;
			for(i=0; i<8; i++){
				if(m_pinList[a[i]]->Get() != m_pinList[b[i]]->Get()){
					equal = 0;
				}
			}
			if(equal){ 
				m_pinList[19]->Set(0);
			}else{
				m_pinList[19]->Set(1);
			}
		}else{
			m_pinList[19]->Set(1);
		}

	}
};

