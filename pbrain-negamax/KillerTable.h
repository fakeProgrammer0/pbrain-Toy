
#ifndef KILLERTABLE
#define KILLERTABLE

#include "board.h"
//#include "search.h"



class KillerTable
{
private:
	static const int MaxPlyDepth = 20;
	static const int KillerMovesPerPly = 2;
	Mov** killerMoves;
	const Mov NULLMove = { -1, -1, -10000, NULL};

public:
	KillerTable();
	~KillerTable();

	void resetKillerMoves();

	void promoteKillerMoves(Mov* moveList, int moveListLen, int depth);
	void updateKillerMoves(const Mov& move, int depth);
};

#endif // !KILLERTABLE









