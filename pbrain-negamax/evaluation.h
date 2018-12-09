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

// �ٽ���
#define FOUR_FOUR_FAKE_FBD 7 // ���ļٽ���
#define THREE_THREE_FAKE_FBD 8 // �����ٽ���

// �ǽ���
#define FOUR_THREE 9 // ���Ļ���
#define FOUR_STHREE 10 // ��������
#define SFOUR_THREE 11 // ���Ļ���
#define SFOUR_STHREE 12 // ��������

#define CHESSTYPESCNT 13 // ��Ҫ���������ͣ������������е�9��

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