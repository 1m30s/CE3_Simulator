#pragma once
// (c) 2023 Y.L.E. / Eleken
// 

#include "Core.h"
#include "Gate.h"
#include "Counter.h"
#include "FlipFlop.h"
#include "Decoder.h"
#include "Memory.h"
#include "Special.h"

CIC* CreateIC(const char* name, const vector<Wire*>& pinList);
