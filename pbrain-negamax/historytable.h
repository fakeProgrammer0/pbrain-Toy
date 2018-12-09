#pragma once
#include "board.h"
#include <math.h>

extern int** m_HistoryTable[2];
extern int m_nHistoryScoreMax;

void initHistoryTable(int width,int height);
void reSetHistoryTable(int width,int height);
int getHistoryScore(Mov move,int player);
void enterHistoryScore(Mov move,int depth,int player);
void freeHistoryTable();

void promoteHistoryScoreToMax(const Mov& move, int player);

