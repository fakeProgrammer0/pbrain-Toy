#include "historytable.h"

int m_nHistoryScoreMax = 0;

void initHistoryTable(int width,int height)
{
	for (int j = 0; j < 2; j++)
	{
		m_HistoryTable[j] = new int* [width];
		for (int i = 0; i < width; i++)
		{
			m_HistoryTable[j][i] = new int[height];
		}
	}
}

void reSetHistoryTable(int width,int height)
{
	int midW = width / 2;
	int midH = height / 2;
	int maxR = max(midW, midH) + 1;

	for (int k = 0; k < 2; k++)
	{
		for (int i = 0; i < width; i++)
		{
			for (int j = 0; j < height; j++)
			{
				m_HistoryTable[k][i][j] = maxR - min(abs(i - midW),abs(j - midH));
			}
		}
	}

	//记录最大值
	m_nHistoryScoreMax = maxR;
}

int getHistoryScore(Mov move,int player)
{
	return m_HistoryTable[player][move.x][move.y];
}
void enterHistoryScore(Mov move,int depth,int player)
{
	m_HistoryTable[player][move.x][move.y] += 2<<depth; 
	
	if (m_HistoryTable[player][move.x][move.y] > m_nHistoryScoreMax)
		m_nHistoryScoreMax = m_HistoryTable[player][move.x][move.y];
}

// 提升至历史启发表的最高分数
// 用于确保迭代加深过程中，下一次迭代最先搜索的分支是上一次得到的最优分支
void promoteHistoryScoreToMax(const Mov& move, int player)
{
	m_HistoryTable[player][move.x][move.y] = ++m_nHistoryScoreMax;
}

void freeHistoryTable()
{
	delete[] m_HistoryTable[0];
	delete[] m_HistoryTable[1];
	m_HistoryTable[0] =NULL;
	m_HistoryTable[1] = NULL;
}





