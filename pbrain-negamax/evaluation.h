#ifndef EVALUATION_H
#define EVALUATION_H
#include "board.h"



#define TOBEANALYZE 0
#define FIVE	1
#define FOUR	2
#define SFOUR	3
#define THREE	4
#define STHREE	5
#define TWO		6

// 假禁手
#define FOUR_FOUR_FAKE_FBD 7 // 四四假禁手
#define THREE_THREE_FAKE_FBD 8 // 三三假禁手

// 非禁手
#define FOUR_THREE 9 // 活四活三
#define FOUR_STHREE 10 // 活四眠三
#define SFOUR_THREE 11 // 冲四活三
#define SFOUR_STHREE 12 // 冲四眠三

#define CHESSTYPESCNT 13 // 需要分析的棋型，就是上面罗列的9种

#define ANALYZED 9





extern int** PosValue;
extern int m_nRecordCount[2][CHESSTYPESCNT];
extern int*** m_nRecord;



void evaluationInit(int width,int height);

int evaluate(int player);
int evaluate_turn(int player, int turn);

//void AnalysisLine(Psquare p0,int direction,ChessAnalyzeData* data);

void AnalysisBoardType(ChessAnalyzeData* data,int direction,int*** m_nRecord,int x,int y);

void SetAnalyzed(int direction,int*** m_nRecord,int x,int y,int leftEdge,int rightEdge);

void SetBoardType(int direction,int*** m_nRecord,int x,int y,int type);

bool isRecordedTwo(int direction,int*** m_nRecord,int x,int y);

void SetLeftBoardType(int direction,int*** m_nRecord,int x,int y,int leftEdge,int type);

void SetRightBoardType(int direction,int*** m_nRecord,int x,int y,int rightEdge,int type);

#endif