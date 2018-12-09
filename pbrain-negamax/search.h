#ifndef ALPHABETA_H
#define ALPHABETA_H
#include "skelet\pisqpipe.h"
#include <windows.h>
#include "evaluation.h"
#include "board.h" 
#include "historytable.h"

//#include "TT.h"
#include "TransTable.h"

const char *infotext="name=\"mysearch\", author=\"  xyh  \", version=\"1.0\", country=\"China\", www=\"#####\"";

extern int firstPlayer;
 int** m_HistoryTable[2];

int** PosValue;
int m_nRecordCount[2][CHESSTYPESCNT];
int*** m_nRecord;

DWORD stopTime();

Psquare board,boardb,boardk;

Mov bestMove;

bool isEarlyStopping;

Mov* GenerateMoves(int& moveLen,int player);
void MakeMove(Mov m,int player);
void UnmakeMove(Mov);
int distance(Psquare p0,Psquare p1);


//固定深度搜索
void SimpleSearch();
//迭代加深搜索+alphaBeta搜索 
void IterDeeping();
//极小极大搜索
int NegaMax00(int depth,int player,int MaxDepth);
//极小极大搜索+历史表启发
int NegaMax(int depth, int player, int MaxDepth);

//alphaBeta搜索
//int alphabeta(int depth,int alpha,int beta,int player,int MaxDepth);
//int NegaMaxAlphaBeta(int depth, int alpha, int beta, int player, int MaxDepth);
int NegaMaxAlphaBetaHistory(int depth, int alpha, int beta, int player, int MaxDepth);
//int AlphaBetaNewVal(int depth, int alpha, int beta, int player, int MaxDepth, int turn);

int PVS(int depth, int alpha, int beta, int player, int MaxDepth);
int PVS_TT(int depth, int alpha, int beta, int player, int MaxDepth);
int PVS_Killer(int depth, int alpha, int beta, int player, int MaxDepth);

int AB_TT(int depth, int alpha, int beta, int player, int MaxDepth);

//int MinMax(int depth, int player, int MaxDepth);
//int AlphaBeta(int MaxDepth, int depth, int player, int alpha, int beta);

//判断是否是禁手
bool isBan(Psquare p0,int player);
//任意选择一步
Mov mov_rd(Mov* moveList,int count);
//判断执行move后棋局是否结束
bool isGameOver(Mov move);

void freeResource();


#endif