#include "KillerTable.h"



KillerTable::KillerTable()
{
	killerMoves = new Mov*[MaxPlyDepth];
	for (int i = 0; i < MaxPlyDepth; i++)
	{
		killerMoves[i] = new Mov[KillerMovesPerPly];
		for (int j = 0; j < KillerMovesPerPly; j++)
		{
			killerMoves[i][j] = NULLMove;
		}
	}
}

KillerTable::~KillerTable()
{
	for (int i = 0; i < MaxPlyDepth; i++)
	{
		delete[] killerMoves[i];
		killerMoves[i] = NULL;
	}
	delete[] killerMoves;
	killerMoves = NULL;
}

void KillerTable::resetKillerMoves()
{
	for (int i = 0; i < MaxPlyDepth; i++)
	{
		for (int j = 0; j < KillerMovesPerPly; j++)
		{
			if(!isFree(killerMoves[i][j].x, killerMoves[i][j].y))
				killerMoves[i][j] = NULLMove;
		}
	}
}


void KillerTable::promoteKillerMoves(Mov* moveList, int moveListLen, int depth)
{
	if (depth > MaxPlyDepth) return;

	int searchKillerMovesCnt = 0;
	for (int i = 0; i < moveListLen; i++)
	{
		for (int j = 0; j < KillerMovesPerPly; j++)
		{
			if (moveList[i] == killerMoves[depth - 1][j]) // ��moveList���ҵ�killerMove�������ƶ���������ǰ
			{
				//pipeOut("DEBUG promote A KILLER MOVE");

				for (int k = i; k > searchKillerMovesCnt; k--)
				{
					moveList[k] = moveList[k - 1];
				}
				moveList[searchKillerMovesCnt] = killerMoves[depth - 1][j];
				searchKillerMovesCnt++;

				if (searchKillerMovesCnt >= KillerMovesPerPly)
					break;
			}
		}
		if (searchKillerMovesCnt >= KillerMovesPerPly)
			break;
	}
}

void KillerTable::updateKillerMoves(const Mov& move, int depth)
{
	if (depth > MaxPlyDepth) return;

	// 1.�򵥵��滻���ԣ�ֻ��FIFO
	/*for (int j = KillerMovesPerPly - 1; j > 0; j--)
		killerMoves[depth - 1][j] = killerMoves[depth - 1][j - 1];
	killerMoves[depth - 1][0] = move;*/

	// 2.ά�ֱ��е�killerMove��valҪ������
	for (int i = 0; i < KillerMovesPerPly; i++)
	{
		//if (move == killerMoves[depth - 1][i] && move.val >= killerMoves[depth - 1][i].val)
		if (move.val >= killerMoves[depth - 1][i].val)
		{
			for (int j = i; j > 0; j--)
				killerMoves[depth - 1][j] = killerMoves[depth - 1][j - 1];
			killerMoves[depth - 1][0] = move;
			return;
		}
	}
	
}

