#include "search.h"
#include "KillerTable.h"

KillerTable killerTable;

 //初始化(棋盘等)              
void brain_init(){
	pipeOut("DEBUG brain_init");
	if(width<5 || width>MAX_BOARD || height<5 || height>MAX_BOARD){
    pipeOut("ERROR size of the board");
    return;
  }
  
  //初始化棋盘
  boardInit();
  evaluationInit(width,height);
  initHistoryTable(width,height);
  
  pipeOut("OK");
}

//开始
void brain_restart()
{
	pipeOut("DEBUG brain_restart");
	boardInit();
	evaluationInit(width,height);
	initHistoryTable(width,height);

	pipeOut("OK");
}


//int isFree(int x, int y)
//{
//	return x>=0 && y>=0 && x<width && y<height && Square(x,y)->z == EMPTY_MOVE;
//}
//AI 下子
void brain_my(int x,int y)
{
	pipeOut("DEBUG brain_my [%d,%d]",x,y);
  if(isFree(x,y)){
	  SetChessOnBoard(x,y,TURN_MOVE);
  }else{
    pipeOut("ERROR my move [%d,%d]",x,y);
  }

  /*char titleMsg[256] = "\0";
  sprintf(titleMsg, "MoveID: %d\nAfter My Move:[%d,%d]", moveID, x, y);
  logEvalBoard(titleMsg);*/

  /*clog << "MoveID: "<< moveID << "\nAfter My Move:[" << x << "," << y << "]\n";
  logEvalBoard(1);
  moveID++;*/
}

//对手下子
void brain_opponents(int x,int y) 
{
	pipeOut("DEBUG brain_opponents [%d , %d]",x,y);
  if(isFree(x,y)){
	  SetChessOnBoard(x,y,OPPONENT_MOVE);
  }else{
    pipeOut("ERROR opponents's move [%d,%d]",x,y);
  }

  /*char titleMsg[256] = "\0";
  sprintf(titleMsg, "MoveID: %d\nAfter Opponent's Move:[%d,%d]", moveID, x, y);
  logEvalBoard(titleMsg);*/

  /*clog << "MoveID: " << moveID << "\nAfter Opponent's Move:[" << x << "," << y << "]\n";
  logEvalBoard(0);
  moveID++;*/
}

//棋盘外
void brain_block(int x,int y)
{
  if(isFree(x,y)){
	  SetChessOnBoard(x,y,OUTSIDE_MOVE);
  }else{
    pipeOut("ERROR winning move [%d,%d]",x,y);
  }
}

//undo
int brain_takeback(int x,int y)
{
	pipeOut("DEBUG brain_takeback [%d , %d]" ,x, y);
	if(!isFree(x,y)){
	  SetChessOnBoard(x,y,EMPTY_MOVE);
    return 0;
  }
  return 2;
}



#define MATCH_SPARE 7      //how much is time spared for the rest of game

//算法停止时间
DWORD stopTime()
{
	return start_time + min(info_timeout_turn, info_time_left/MATCH_SPARE)-30;
}


//计算下棋
void brain_turn() 
{ 
	pipeOut("DEBUG brain_turn");

	//使用negamax+迭代加深
	IterDeeping();
	//使用简单搜索，固定深度
	//SimpleSearch();
}

void brain_end()
{
	freeResource();
}


//这里未实现
#ifdef DEBUG_EVAL
#include <windows.h>

void brain_eval(int x,int y)
{
  HDC dc;
  HWND wnd;
  RECT rc;
  char c;
  wnd=GetForegroundWindow();
  dc= GetDC(wnd);
  GetClientRect(wnd,&rc);
  c=(char)(board[x][y]+'0');
  TextOut(dc, rc.right-15, 3, &c, 1);
  ReleaseDC(wnd,dc);
}

#endif


//迭代加深搜索+NegaMax/alphaBeta搜索 
void IterDeeping()
{
	//分配搜索时间   
    int nMinTimer =  GetTickCount() + unsigned int((min(info_timeout_turn, info_time_left/MATCH_SPARE)-30)*0.618f);   
	reSetHistoryTable(width,height); // 每一步都要重新设置历史启发表的值
	killerTable.resetKillerMoves();

	Mov LastBestMove;

	//迭代加深搜索   
    //for(int depth=1; depth<20;(depth < 4 ? depth *= 2 : depth +=1))   
    for(int depth=2; depth<20;(depth < 4 ? depth *= 2 : depth +=1))   
    {   
		bestMove.val = -10000;
		pipeOut("DEBUG MAX depth:%d,",depth);

		isEarlyStopping = false;
           
        int score;
		//负极大值搜索 or // alpha-beta搜索
		//score = NegaMax(depth,0,depth);    
		//score = alphabeta(depth,-9999,9999,0,depth);  
		
		//score = NegaMaxAlphaBetaHistory(depth, -9999, 9999, 0, depth);
		//score = PVS(depth, -10000, 10000, 0, depth);
		score = PVS_TT(depth, -10000, 10000, 0, depth);
		//score = AB_TT(depth, -10000, 10000, 0, depth);
		//score = PVS_Killer(depth, -10000, 10000, 0, depth);

		// 把bestMove的历史启发分数提升至当前历史启发表的最高分数+1
		// 用于确保迭代加深过程中，下一次迭代最先搜索的分支是上一次得到的最优分支
		// 尽管不是很恰当，但是这个微小的变化，也不会对搜索产生太大偏向影响
		promoteHistoryScoreToMax(bestMove, 0);


		pipeOut("DEBUG BestScore:%d",score);
		pipeOut("DEBUG BestMove:[%d,%d],%d",bestMove.x,bestMove.y,bestMove.val);
		pipeOut("DEBUG Time use:%d,time left:%d",GetTickCount()- start_time,info_timeout_turn-(GetTickCount()-start_time));
		
		// 若强行终止思考，停止搜索   
		//if(terminate_v)   
		if(isEarlyStopping) // 用isEarlyStopping条件判断，不要用terminate_v
            break;   
   
        // 若时间已经达到规定时间的一半，再搜索一层的时间可能不够，停止搜索。   
        if( GetTickCount() > nMinTimer)   
            break;   
   
        // 在规定的深度内，遇到杀棋，停止思考。   
		if(score == 9999)   
              break;   

		if (isFree(bestMove.x, bestMove.y)) {
			LastBestMove = bestMove;
		}
		else
			pipeOut("DEBUG ERROR: BestMove is not valid");
			
    }   

	//if (terminate_v)
	if(isEarlyStopping)
	{
		pipeOut("DEBUG Early Stopping");

		if (!isFree(bestMove.x, bestMove.y))
		{
			if (isFree(LastBestMove.x, LastBestMove.y))
				bestMove = LastBestMove;
			else
				pipeOut("DEBUG ERROR: CANNOT FIND A Valid move!");
		}
			

		MakeMove(bestMove, 0);
		int earlyStoppingSearchScore = evaluate_turn(0, 1);
		UnmakeMove(bestMove);

		MakeMove(LastBestMove, 0);
		int LastBestSearchScore = evaluate_turn(0, 1);
		UnmakeMove(LastBestMove);

		if (earlyStoppingSearchScore < LastBestSearchScore) {
			bestMove = LastBestMove;
			pipeOut("DEBUG Change BestMove:[%d,%d],%d", bestMove.x, bestMove.y, bestMove.val);
		}
	}

	int beforeMoveEvalScore = evaluate_turn(0, 0);
	MakeMove(bestMove, 0);
	int afterMoveEvalScore = evaluate_turn(0, 1);
	UnmakeMove(bestMove);
	pipeOut("DEBUG board evaluation:");
	pipeOut("DEBUG before move : %d; after move : %d", beforeMoveEvalScore, afterMoveEvalScore);
	pipeOut("DEBUG TT insertCnt:%d;hitCnt:%d;hit rate:%.2f", TransTable::getInstance().getInsertCnt(), TransTable::getInstance().getSearchHitCnt(), TransTable::getInstance().getHitRate());

	do_mymove(bestMove.x,bestMove.y);
}

//负极大值搜索+历史表启发
int NegaMax(int depth,int player,int MaxDepth)
{
	if (depth <=0)//预判层上的伪叶子结点，计算其启发评价值
	{
		return evaluate(player);
	}

	int bestMoveIndex = -1;
	int bestVal = -10000;
	int moveListLen  =0;
	Mov* moveList=GenerateMoves(moveListLen,player);

	if(moveListLen == 0)
	{
		pipeOut("gen movelist is empty");
		delete[] moveList;
		moveList = NULL;
		return evaluate(player);
	}
	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i],player);
	}

	moveList = MergeSort(moveList,moveListLen); //历史启发排序，alpha-beta时可用

	for(int i = 0;i<moveListLen;i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			pipeOut("DEBUG It's time to terminate");
			break;
		}

		MakeMove(moveList[i],player);

		if (isGameOver(moveList[i]))//儿子结点为胜负已分状态（真正的叶子结点），表明player方走这一步走法后会获胜
		{
			if (depth == MaxDepth)
			{
				bestMove = moveList[i];
			}
			UnmakeMove(moveList[i]);
			delete[] moveList;
			moveList = NULL;
			return 9999;
		}

		moveList[i].val = -NegaMax(depth - 1,1 - player,MaxDepth);

		UnmakeMove(moveList[i]);

		if(bestVal < moveList[i].val)
		{

			bestVal = moveList[i].val;
			bestMoveIndex = i;
			if(depth == MaxDepth)
			{	
				bestMove = moveList[i];
			}
		}

	}

	if(bestMoveIndex != -1)
	{
		enterHistoryScore(moveList[bestMoveIndex],depth,player);
	}

	if(depth == MaxDepth)
	{
		bestMove = moveList[bestMoveIndex];
	}
	delete[] moveList;
	moveList = NULL;

	return bestVal;
}

//固定深度+NegaMax搜索 
void SimpleSearch()
{
	// DEBUG
	//printBoard();

	Mov resultMov;
	int depth = 4;
	
	bestMove.val = -10000;
	int score;
	//负极大值搜索
	score = NegaMax00(depth, 0, depth); // score只是用于返回启发分数，NegaMax00函数会设置bestMove全局变量，作为brain的下一步行动
	//score = MinMax(depth, 0, depth);

	// 其实在这个简单搜索里面，AlphaBeta搜索并没有起到很明显的效果，因为没有历史启发
	// 局面一大了，搜索时间不够，往往只会在左上角的地方落子s
	//score = AlphaBeta(depth, depth, 0, -10000, 10000);
	//score = NegaMaxAlphaBeta(depth, -10000, 10000, 0, depth);

	pipeOut("DEBUG Depth value:%d", score);
	pipeOut("DEBUG BestMove:[%d,%d],%d", bestMove.x, bestMove.y, bestMove.val);
	do_mymove(bestMove.x, bestMove.y);
}

//负极大值搜索
int NegaMax00(int depth, int player, int MaxDepth)
{
	if (depth <= 0)//伪叶子结点
	{
		return evaluate(player);
	}

	int bestMoveIndex = -1;
	int bestVal = -10000;
	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);

	if (moveListLen == 0)
	{
		pipeOut("gen movelist is empty");
		delete[] moveList;
		moveList = NULL;
		return evaluate(player);
	}

	for (int i = 0; i<moveListLen; i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			pipeOut("DEBUG It's time to terminate");
			break;
		}

		MakeMove(moveList[i], player);

		if (isGameOver(moveList[i]))//儿子结点为胜负已分状态，真正的叶子结点
		{
			if (depth == MaxDepth)
			{
				bestMove = moveList[i];
			}
			UnmakeMove(moveList[i]);
			delete[] moveList;
			moveList = NULL;
			return 9999;
		}

		//moveList[i].val = -NegaMax(depth - 1, 1 - player, MaxDepth);
		moveList[i].val = -NegaMax00(depth - 1, 1 - player, MaxDepth);

		UnmakeMove(moveList[i]);

		if (bestVal < moveList[i].val)
		{
			bestVal = moveList[i].val;
			bestMoveIndex = i;
			if (depth == MaxDepth) // 设置根节点的可选分支，防止【早停】时bestMove还没有被设置
			{
				bestMove = moveList[i];
			}
		}

	}

	if (depth == MaxDepth)
	{
		bestMove = moveList[bestMoveIndex];
	}
	delete[] moveList;
	moveList = NULL;

	return bestVal;
}

/*@green
固定层数极大极小过程 等价于NegMax00 
对提升算法没有任何帮助，就当熟悉一下逻辑吧*/
//int MinMax(int depth, int player, int MaxDepth)
//{
//	if (depth <= 0)
//	{
//		return evaluate(0); // 因为是MinMax过程，所以局面的评价都是从Max节点出发的
//	}
//
//	int moveListLen = 0;
//	Mov* moveList = GenerateMoves(moveListLen, player);
//
//	if (moveListLen == 0)
//	{
//		//pipeOut("gen movelist is empty");
//		delete[] moveList;
//		moveList = NULL;
//		return evaluate(0);
//	}
//
//	int bestMoveIndex = -1;
//	int bestVal = player == 0 ? -9999 : 9999; // player == 0 当前为Max节点？？
//	for (int i = 0;i < moveListLen;i++)
//	{
//		// 如果超时或被强制结束，停止搜索
//		if (terminate_v || GetTickCount() >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//
//		// 下完这步棋后，棋局结束，player获得胜利
//		// 有个坑在于：isGameOver函数没有在棋盘上放置棋子，所以isGameOver函数要在MakeMove函数之后执行
//		// 写在for循环里判断，而不是放在MinMax的开始位置，可以起到剪枝的效果，这样剩下到[兄弟节点]就没必要搜索了
//		if (isGameOver(moveList[i]))
//		{
//			bestVal = player == 0 ? 9999 : -9999;
//			// 处理depth==MaxDepth的情况
//			bestMoveIndex = i;
//			UnmakeMove(moveList[i]);
//			break;
//		}
//
//		moveList[i].val = MinMax(depth - 1, 1 - player, MaxDepth); // 更新儿子节点的启发评价分数
//		UnmakeMove(moveList[i]);
//
//		if (player == 0)
//		{
//			if (moveList[i].val > bestVal)
//			{
//				bestVal = moveList[i].val;
//				bestMoveIndex = i;
//			}
//		}
//		else {
//			if (moveList[i].val < bestVal)
//			{
//				bestVal = moveList[i].val;
//				bestMoveIndex = i;
//			}
//		}
//	}
//	
//	// 返回到MinMax最开始到父节点，设置该节点的下一步选择 bestMove
//	if (depth == MaxDepth)
//	{
//		bestMove = moveList[bestMoveIndex];
//	}
//	delete[] moveList;
//	moveList = NULL;
//
//	return bestVal;
//}
//
//int purge = 0;
//
//// @green
//// 简单的ApphaBeta过程，局面的评价以max方为准：evalutate(0)
//int AlphaBeta(int MaxDepth, int depth, int player, int alpha, int beta)
//{
//	if (depth <= 0)
//	{
//		return evaluate(0);
//	}
//
//	int moveListLen = 0;
//	Mov* moveList = GenerateMoves(moveListLen, player);
//
//	if (moveListLen == 0)
//	{
//		delete[] moveList;
//		moveList = NULL;
//		return evaluate(0);
//	}
//
//	int bestVal = player == 0 ? -9999 : 9999;
//	int bestMoveIndex = -1;
//	for (int i = 0; i < moveListLen; i++)
//	{
//		if (terminate_v || GetTickCount() + 1 >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//
//		// debug @green
//		/*if (purge)
//		{
//			if (purge == 1) pipeOut("Alpha Purge");
//			else pipeOut("Beta Purge");
//			purge = 0;
//		}
//		printBoard();*/
//		
//
//		if (isGameOver(moveList[i]))
//		{
//			if (depth == MaxDepth)
//			{
//				bestMove = moveList[bestMoveIndex];
//			}
//			UnmakeMove(moveList[i]);
//			delete[] moveList;
//			moveList = NULL;
//			bestVal = player == 0 ? 9999 : -9999;
//			return bestVal;
//		}
//
//		moveList[i].val = AlphaBeta(MaxDepth, depth - 1, 1 - player, alpha, beta);
//		UnmakeMove(moveList[i]);
//
//		if (player == 0)
//		{
//			if (moveList[i].val > bestVal)
//			{
//				bestVal = moveList[i].val;
//				bestMoveIndex = i;
//			}
//
//			if (moveList[i].val > alpha) // 更新max节点的alpha值
//			{
//				alpha = moveList[i].val;
//			}
//
//			if (moveList[i].val >= beta)
//			{
//				purge = 2;
//				break;
//			}
//			
//		}
//		else
//		{
//			if (moveList[i].val < bestVal)
//			{
//				bestVal = moveList[i].val;
//				bestMoveIndex = i;
//			}
//
//			if (moveList[i].val < beta)
//			{
//				beta = moveList[i].val;
//			}
//
//			// 在Min节点发生alpha剪枝，剪去该Min节点的其它分支
//			if (moveList[i].val <= alpha)
//			{
//				purge = 1;
//				break;
//			}
//		}
//	}
//
//	if (depth == MaxDepth)
//	{
//		bestMove = moveList[bestMoveIndex];
//	}
//
//	delete[] moveList;
//	moveList = NULL;
//	return bestVal;
//}
//
//
//// alphaBeta搜索
//int alphabeta(int depth,int alpha,int beta,int player,int MaxDepth)
//{
//	if (depth == 0)
//	{
//		return evaluate(0);
//	}
//
//	int moveListLen = 0;
//	Mov* moveList = GenerateMoves(moveListLen, player);
//
//	if (moveListLen == 0)
//	{
//		delete[] moveList;
//		moveList = NULL;
//		return evaluate(0);
//	}
//
//	int bestMoveIndex = -1;
//	for (int i = 0; i < moveListLen; i++)
//	{
//		if (terminate_v | GetTickCount() >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//		if (isGameOver(moveList[i]))
//		{
//			if (depth == MaxDepth)
//			{
//				bestMove = moveList[bestMoveIndex];
//			}
//			UnmakeMove(moveList[i]);
//			delete[]moveList;
//			moveList = NULL;
//			return player == 0 ? 9999 : -9999;
//		}
//
//		moveList[i].val = alphabeta(depth - 1, alpha, beta, 1 - player, MaxDepth);
//		UnmakeMove(moveList[i]);
//
//		if (player == 0)
//		{
//			if (moveList[i].val > alpha)
//			{
//				alpha = moveList[i].val;
//				if (alpha >= beta)
//					break;
//			}
//		}
//		else
//		{
//			if (moveList[i].val < beta)
//			{
//				beta = moveList[i].val;
//				if (beta <= alpha)
//					break;
//			}
//		}
//	}
//
//	if (depth == MaxDepth)
//	{
//		bestMove = moveList[bestMoveIndex];
//	}
//
//	delete[]moveList;
//	moveList = NULL;
//	return player == 0 ? alpha : beta;
//}
//
//int NegaMaxAlphaBeta(int depth, int alpha, int beta, int player, int MaxDepth)
//{
//	if (depth == 0)
//	{
//		return evaluate(player);
//	}
//
//	int moveListLen = 0;
//	Mov* moveList = GenerateMoves(moveListLen, player);
//
//	if (moveListLen == 0)
//	{
//		delete[] moveList;
//		moveList = NULL;
//		return evaluate(player);
//	}
//
//	int bestMoveIndex = -1;
//	for (int i = 0; i < moveListLen; i++)
//	{
//		/*pipeOut("startTime: %ul", start_time);
//		pipeOut(" stopTime: %ul", stopTime());
//		pipeOut(" currTime: %ul", GetTickCount());
//		pipeOut(" use_Time: %ul\n", GetTickCount() - start_time);*/
//
//		if (terminate_v | GetTickCount() >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//		if (isGameOver(moveList[i]))
//		{
//			UnmakeMove(moveList[i]);
//			bestMoveIndex = i;
//			alpha = 9999;
//			break;
//		}
//
//		moveList[i].val = -NegaMaxAlphaBeta(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
//		UnmakeMove(moveList[i]);
//		
//		if (moveList[i].val > alpha)
//		{
//			alpha = moveList[i].val;
//			//if (depth == MaxDepth)
//				bestMoveIndex = i;
//		}
//		if (alpha >= beta)
//			break;
//	}
//
//	if (depth == MaxDepth)
//		bestMove = moveList[bestMoveIndex];
//
//	delete[] moveList;
//	moveList = NULL;
//
//	return alpha;
//}


// AlphaBeta负极大值+历史启发值
int NegaMaxAlphaBetaHistory(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	// 在历史置换表中查找到该节点，并且深度大于当前节点往下搜索的深度
	if (TransTable::getInstance().searchCurrBoardNode())
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth)
		{
			TransTable::getInstance().incrementSearchHitCnt();

			//pipeOut("DEBUG Get current board node from TransTable. Depth: %d;Node Depth:%d", depth, currBoardNode->getDepth());
			//clog << "DEBUG Get current board node from TransTable\nNode Depth:" << currBoardNode->getDepth() << "; Current Depth : " << depth << endl;
			//logEvalBoard(player);

			if (depth <= 0) pipeOut("DEBUG Get a leaf node from TransTable");

			if (depth == MaxDepth) {
				bestMove = currBoardNode->getMove();
				if (!isFree(bestMove.x, bestMove.y))
				{
					pipeOut("DEBUG ERROR Get an invalid Move Node From TT!");
					pipeOut("DEBUG Invalid bestMove:[%d,%d]", bestMove.x, bestMove.y);
				}
			}

			if (turn == currBoardNode->getTurn()) return currBoardNode->getScore();
			pipeOut("DEBUG BUT turn != player");
			
			//clog << "DEBUG BUT turn != player" << endl;
			//logFile("DEBUG BUT turn != player");

			return -currBoardNode->getScore();
		}
	}

	//clog << "Depth:" << depth << endl;
	//logEvalBoard(player);

	if (depth <= 0)
	{
		//return evaluate(player);
		return evaluate_turn(player, turn);
	}
	

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);

	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		//return evaluate(player);
		//int turn = (MaxDepth - depth) % 2;
		return evaluate_turn(player, turn);
	}

	// 从历史启发表获取初始值
	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen); // 根据历史启发值对待搜索分支进行排序，提高AlphaBeta剪枝的概率

	if (depth == MaxDepth && depth > 2 && !(moveList[0].x == bestMove.x && moveList[0].y == bestMove.y))
	{
		/*for (int idx = 0; idx < moveListLen; idx++)
		{
			if (moveList[idx].x == bestMove.x && moveList[idx].y == bestMove.y)
			{
				pipeOut("ERROR: bestMove.val: %d", m_HistoryTable[0][bestMove.x][bestMove.y]);
			}
			pipeOut("Move: [%d,%d], val:%d", moveList[idx].x, moveList[idx].y, moveList[idx].val);
		}*/

		pipeOut("DEBUG ERROR BestMove is not the last best search branch!! BestMove:[%d, %d];moveList[0]:[%d,%d]", bestMove.x, bestMove.y, moveList[0].x, moveList[0].y);
	}

	//logFile("Go Down to search Children Nodes\n");
	//clog << "Go Down to search Children Nodes\n";

	int bestMoveIndex = -1;
	for (int i = 0; i < moveListLen; i++)
	{
		
		/*pipeOut("startTime: %ul", start_time);
		pipeOut(" stopTime: %ul", stopTime());
		pipeOut(" currTime: %ul", GetTickCount());
		pipeOut(" use_Time: %ul\n", GetTickCount() - start_time);*/

		if (terminate_v | GetTickCount() >= stopTime())
		{
			pipeOut("DEBUG It's time to terminate.");
			isEarlyStopping = true;
			break;
		}

		MakeMove(moveList[i], player);
		if (isGameOver(moveList[i]))
		{
			UnmakeMove(moveList[i]);
			bestMoveIndex = i; // 设置bestMoveIndex还是必要的，这样能够处理depth == MaxDepth的情况
			alpha = 9999;
			break; // 直接剪枝
		}

		moveList[i].val = -NegaMaxAlphaBetaHistory(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
		UnmakeMove(moveList[i]);

		if (moveList[i].val > alpha)
		{
			alpha = moveList[i].val;
			/*if (depth == MaxDepth) 
			{
				bestMoveIndex = i;
				bestMove = moveList[bestMoveIndex];
			}*/

			// 直接更新吧，不要写冗余代码
			// 更为重要的是，在AlphaBeta搜索中，每一个节点的bestMoveIndex这个信息都是有用的，不仅仅是需要一个bestVal分数而已
			bestMoveIndex = i; 
		}
		if (alpha >= beta)
		{
			//logEvalBoard(player, "AlphaBeta Purge");
			break;
		}
	}

	// 离开一个分支返回上一层时，用该分支的最优走法，更新历史启发表的值
	if (bestMoveIndex != -1 && !isEarlyStopping)
	{
		enterHistoryScore(moveList[bestMoveIndex], depth, player);

		// 更新置换表：更新深度更深的节点
		if (!TransTable::getInstance().searchCurrBoardNode() 
			|| TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
				pipeOut("DEBUG ERROR: insert an invalid Move!");
				pipeOut("DEBUG Invalid insert Move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}

			TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex]);
			//pipeOut("DEBUG Insert current board node in TransTable");
			//logFile("DEBUG Insert current board node in TransTable");
			//clog << "DEBUG Insert current board node in TransTable" << endl;
		}
	}

	//logEvalBoard(player, "Go Back to Parent Node");

	// 搜索最终都会返回到根节点，从根节点退出，不管是不是时间不够没搜索完就早停了
	if (depth == MaxDepth) {
		if (isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
		{
			bestMove = moveList[bestMoveIndex];
		}
		else {
			pipeOut("DEBUG When Return from Root Node, bestMove is invalid!");
			pipeOut("DEBUG Invalid return Move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
		}
		//enterHistoryScore(moveList[bestMoveIndex], depth, player); // 继续加一层，效果好一点
	}


	// 统一处理资源销毁
	delete[] moveList;
	moveList = NULL;

	return alpha;
}

// Principal Variation Search 主要变例搜索
// 因为没有找到合适的伪代码，所以暂时采用维基百科给出的伪代码形式
int PVS(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // 在置换表中找到了当前节点
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // 并且深度大于等于当前节点往下搜索的深度
		{
			TransTable::getInstance().incrementSearchHitCnt();

			if (depth == MaxDepth)
			{
				if (!isFree(currBoardNode->getMove().x, currBoardNode->getMove().y))
				{
					pipeOut("DEBUG ERROR: get an invalid move from TT");
					pipeOut("DEBUG invalid move:[%d,%d]", currBoardNode->getMove().x, currBoardNode->getMove().y);
				}else
					bestMove = currBoardNode->getMove();
			}

			if (turn == currBoardNode->getTurn()) return currBoardNode->getScore();
			pipeOut("DEBUG BUT TURN is opponent's turn!");
			return -currBoardNode->getScore();
		}
	}

	if (depth <= 0)
	{
		return evaluate_turn(player, turn);
	}

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);
	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		return evaluate_turn(player, turn);
	}

	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen);

	if (depth == MaxDepth && depth > 2 && !(moveList[0].x == bestMove.x && moveList[0].y == bestMove.y))
	{
		pipeOut("DEBUG ERROR: current best Search branch is not last best Search bestMove!");
	}

	int bestMoveIndex = -1;
	int bestScore = -10000;
	for (int i = 0; i < moveListLen; i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			isEarlyStopping = true;
			break;
		}

		MakeMove(moveList[i], player);

		if (isGameOver(moveList[i]))
		{
			UnmakeMove(moveList[i]);
			bestMoveIndex = i;
			alpha = bestScore = 9999;
			break;
		}

		if (i == 0)
		{
			moveList[i].val = -PVS(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
		}
		else {
			moveList[i].val = -PVS(depth - 1, -alpha - 1, -alpha, 1 - player, MaxDepth);
			if (alpha < moveList[i].val && moveList[i].val < beta) // 落在区间之间，预测失败，需要进行一次完整的AlphaBeta搜索
			{
				moveList[i].val = -PVS(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // 更新当前节点的子分支上限
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore >= alpha) alpha = bestScore; // 更新[当前节点的父结点]的最优子分支上限
			if (alpha >= beta) break; // 剪枝
		}
	}

	if (bestMoveIndex != -1)
	{
		// 如果是早停的话，搜到到的分数不是当前节点的最优分数，只是一个下限
		if (!isEarlyStopping) {
			enterHistoryScore(moveList[bestMoveIndex], depth, player);

			if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
			{
				if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
				{
					pipeOut("DEBUG ERROR: insert an invalid move into TT");
					pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
				}else
					TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex]);
			}
		}
		
		if (depth == MaxDepth)
		{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
				pipeOut("DEBUG ERROR: get an invalid move from TT");
				pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}else
				bestMove = moveList[bestMoveIndex];
		}
	}

	delete[] moveList;
	moveList = NULL;
	return bestScore;
}

// Principal Variation Search 主要变例搜索 + 置换表 TT
int PVS_TT(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // 在置换表中找到了当前节点
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode == NULL) pipeOut("DEBUG ERROR: Get NULL BoardNode From TT"); // 应该不会出现吧
		if (currBoardNode->getDepth() >= depth) // 并且深度大于等于当前节点往下搜索的深度
		{
			TransTable::getInstance().incrementSearchHitCnt();

			bool hitFlag = false;
			switch (currBoardNode->getScoreType())
			{
			case EXACT:
			{
				hitFlag = true;
				break;
			}
			case LOWER_BOUND:
			{
				if (currBoardNode->getScore() >= beta) // score的值可以引发当前节点的儿子分支剪枝
				{
					hitFlag = true;
				}
				else break; // 窗口值beta大于score，有可能是之前搜索到进行一半时间不够过早停止
			}
			case UPPER_BOUND:
			{
				if (currBoardNode->getScore() <= alpha) // score值小于alpha，表明
				{
					hitFlag = true;
				}
				else break;
			}
			}
			
			if (hitFlag)
			{
				if (depth == MaxDepth)
				{
					if (!isFree(currBoardNode->getMove().x, currBoardNode->getMove().y))
					{
						pipeOut("DEBUG ERROR: get an invalid move from TT");
						pipeOut("DEBUG invalid move:[%d,%d]", currBoardNode->getMove().x, currBoardNode->getMove().y);
					}
					else
						bestMove = currBoardNode->getMove();
				}

				if (turn == currBoardNode->getTurn()) return currBoardNode->getScore();
				pipeOut("DEBUG ERROR: BUT TURN is opponent's turn!"); // 应该不会发生，对于五子棋来说，只要确定了先手方，两个局面上棋子数目相同，接下来轮到谁走棋是固定的
				return -currBoardNode->getScore();
			}
		}
	}

	if (depth <= 0)
	{
		int score = evaluate_turn(player, turn);
		//TransTable::getInstance().insertCurrBoardNode(score, depth, turn, move)
		return score;
	}

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);
	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		return evaluate_turn(player, turn);
	}

	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen);

	if (depth == MaxDepth && depth > 2 && moveList[0] != bestMove)
	{
		pipeOut("DEBUG ERROR: current best Search branch is not last best Search bestMove!");
	}

	ScoreType bestScoreType = UPPER_BOUND;
	int bestMoveIndex = -1;
	int bestScore = -10000;
	for (int i = 0; i < moveListLen; i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			isEarlyStopping = true;
			break;
		}

		MakeMove(moveList[i], player);

		if (isGameOver(moveList[i]))
		{
			UnmakeMove(moveList[i]);
			bestMoveIndex = i;
			alpha = bestScore = 9999;
			break;
		}

		if (i == 0) // 对第一个分支进行完整的AlphaBeta搜索
		{
			moveList[i].val = -PVS_TT(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
		}
		else {
			// 假设之前搜的分支是最优分支（主要变例），得到分数是最好的，因此用一个窄窗(alpha, alpha+1)进行搜索，返回的分数应该会小于alpha
			// 由于搜索的窗口值较小，所以时间也比AlphaBeta搜索少很多
			moveList[i].val = -PVS_TT(depth - 1, -alpha - 1, -alpha, 1 - player, MaxDepth);
			if (alpha < moveList[i].val && moveList[i].val < beta) // 落在区间之间，预测失败，需要进行一次完整的AlphaBeta搜索
			{
				moveList[i].val = -PVS_TT(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // 更新当前节点的子分支上限
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore > alpha) // 超过了alpha值的上限，更新alpha值
			{ 
				alpha = bestScore;
				bestScoreType = EXACT; // 精确值
			}; 
			if (alpha >= beta) 
			{ 
				bestScoreType = LOWER_BOUND; // 发生剪枝，当前节点的返回值只是一个下限，后续有可能搜索到更好的分数
				break; // 剪枝
			} 
		}
	}

	if (bestMoveIndex != -1) // bestMoveIndex == -1 说明当前节点还没往下搜索，就因为时间不够被停止
	{
		// 没有找到节点，或者找到节点的深度小于当前深度
		if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			if (bestScoreType == EXACT)
			{
				TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
			else // UPPER_BOUND 或 LOWER_BOUND
			{
				TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
		}

		// 如果是早停的话，搜到的分数不是当前节点的最优分数，只是一个下限
		if (!isEarlyStopping) {
			enterHistoryScore(moveList[bestMoveIndex], depth, player);
		}

		if (depth == MaxDepth)
		{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
				pipeOut("DEBUG ERROR: return an invalid move at root node");
				pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}
			else
				bestMove = moveList[bestMoveIndex];
		}
	}

	delete[] moveList;
	moveList = NULL;
	return bestScore;
}

int PVS_Killer(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // 在置换表中找到了当前节点
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // 并且深度大于等于当前节点往下搜索的深度
		{
			TransTable::getInstance().incrementSearchHitCnt();

			int returnVal = -10001;
			switch (currBoardNode->getScoreType())
			{
			case EXACT:
			{
				returnVal = currBoardNode->getScore();
				break;
			}
			case LOWER_BOUND:
			{
				if (currBoardNode->getScore() >= beta)
					returnVal = currBoardNode->getScore();
				else break;
			}
			case UPPER_BOUND:
			{
				if (currBoardNode->getScore() <= alpha)
					returnVal = currBoardNode->getScore();
				else break;
			}
			}

			if (returnVal != -10001)
			{
				if (depth == MaxDepth)
				{
					if (!isFree(currBoardNode->getMove().x, currBoardNode->getMove().y))
					{
						pipeOut("DEBUG ERROR: get an invalid move from TT");
						pipeOut("DEBUG invalid move:[%d,%d]", currBoardNode->getMove().x, currBoardNode->getMove().y);
					}
					else
						bestMove = currBoardNode->getMove();
				}

				if (turn == currBoardNode->getTurn()) return currBoardNode->getScore();
				pipeOut("DEBUG ERROR: BUT TURN is opponent's turn!");
				return -currBoardNode->getScore();
			}
		}
	}

	if (depth <= 0)
	{
		//int score = evaluate_turn(player, turn); // 较复杂的启发函数
		int score = evaluate(player); // 简单的启发函数
		//TransTable::getInstance().insertCurrBoardNode(score, depth, turn, move)
		return score;
	}

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);
	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		return evaluate_turn(player, turn);
	}

	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen);

	if (depth == MaxDepth && depth > 2 && !(moveList[0].x == bestMove.x && moveList[0].y == bestMove.y))
	{
		pipeOut("DEBUG ERROR: current best Search branch is not last best Search bestMove!");
	}

	if(!isEarlyStopping)
		killerTable.promoteKillerMoves(moveList, moveListLen, depth);

	//bool isBestScoreExact = false;
	ScoreType bestScoreType = UPPER_BOUND;
	int bestMoveIndex = -1;
	int bestScore = -10000;
	for (int i = 0; i < moveListLen; i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			isEarlyStopping = true;
			break;
		}

		MakeMove(moveList[i], player);

		if (isGameOver(moveList[i]))
		{
			UnmakeMove(moveList[i]);
			bestMoveIndex = i;
			alpha = bestScore = 9999;
			break;
		}

		if (i == 0)
		{
			moveList[i].val = -PVS_Killer(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
		}
		else {
			moveList[i].val = -PVS_Killer(depth - 1, -alpha - 1, -alpha, 1 - player, MaxDepth);
			if (alpha < moveList[i].val && moveList[i].val < beta) // 落在区间之间，预测失败，需要进行一次完整的AlphaBeta搜索
			{
				moveList[i].val = -PVS_Killer(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // 更新当前节点的子分支上限
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore > alpha) // 更新[当前节点的父结点]的最优子分支上限
			{
				alpha = bestScore;
				//isBestScoreExact = true;
				bestScoreType = EXACT;
			};
			if (alpha >= beta)
			{
				killerTable.updateKillerMoves(moveList[bestMoveIndex], depth);
				//TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], LOWER_BOUND);
				bestScoreType = LOWER_BOUND;
				break;
			} // 剪枝
		}
	}


	if (bestMoveIndex != -1)
	{
		if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			if (bestScoreType == EXACT)
			{
				TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
			else // UPPER_BOUND 或 LOWER_BOUND
			{
				TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
		}

		/*if (isBestScoreExact)
		{
		TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex], EXACT);
		}
		else {
		TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], UPPER_BOUND);
		}*/

		// 如果是早停的话，搜到到的分数不是当前节点的最优分数，只是一个下限
		if (!isEarlyStopping) {
			enterHistoryScore(moveList[bestMoveIndex], depth, player);

			/*if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
			{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
			pipeOut("DEBUG ERROR: insert an invalid move into TT");
			pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}
			else
			TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex]);
			}*/
		}

		if (depth == MaxDepth)
		{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
				pipeOut("DEBUG ERROR: get an invalid move from TT");
				pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}
			else
				bestMove = moveList[bestMoveIndex];
		}
	}

	delete[] moveList;
	moveList = NULL;
	return bestScore;
}

int AB_TT(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // 在置换表中找到了当前节点
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // 并且深度大于等于当前节点往下搜索的深度
		{
			TransTable::getInstance().incrementSearchHitCnt();

			int returnVal = -10001;
			switch (currBoardNode->getScoreType())
			{
			case EXACT:
			{
				returnVal = currBoardNode->getScore();
				break;
			}
			case LOWER_BOUND:
			{
				if (currBoardNode->getScore() >= beta)
					returnVal = currBoardNode->getScore();
				else break;
			}
			case UPPER_BOUND:
			{
				if (currBoardNode->getScore() <= alpha)
					returnVal = currBoardNode->getScore();
				else break;
			}
			}

			if (returnVal != -10001)
			{
				if (depth == MaxDepth)
				{
					if (!isFree(currBoardNode->getMove().x, currBoardNode->getMove().y))
					{
						pipeOut("DEBUG ERROR: get an invalid move from TT");
						pipeOut("DEBUG invalid move:[%d,%d]", currBoardNode->getMove().x, currBoardNode->getMove().y);
					}
					else
						bestMove = currBoardNode->getMove();
				}

				if (turn == currBoardNode->getTurn()) return currBoardNode->getScore();
				pipeOut("DEBUG ERROR: BUT TURN is opponent's turn!");
				return -currBoardNode->getScore();
			}
		}
	}

	if (depth <= 0)
	{
		int score = evaluate_turn(player, turn);
		//TransTable::getInstance().insertCurrBoardNode(score, depth, turn, move)
		return score;
	}

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);
	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		return evaluate_turn(player, turn);
	}

	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen);

	if (depth == MaxDepth && depth > 2 && !(moveList[0].x == bestMove.x && moveList[0].y == bestMove.y))
	{
		pipeOut("DEBUG ERROR: current best Search branch is not last best Search bestMove!");
	}

	//bool isBestScoreExact = false;
	ScoreType bestScoreType = UPPER_BOUND;
	int bestMoveIndex = -1;
	for (int i = 0; i < moveListLen; i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			isEarlyStopping = true;
			break;
		}

		MakeMove(moveList[i], player);

		if (isGameOver(moveList[i]))
		{
			UnmakeMove(moveList[i]);
			bestMoveIndex = i;
			alpha = 9999;
			break;
		}

		moveList[i].val = -AB_TT(depth - 1, -beta, -alpha, 1 - player, MaxDepth);

		UnmakeMove(moveList[i]);

		// 更新[当前节点的父结点]的最优子分支上限
		if (moveList[i].val > alpha) // 更新当前节点的子分支上限
		{
			alpha = moveList[i].val;
			bestMoveIndex = i;
			bestScoreType = EXACT;

			if (alpha >= beta)
			{
				bestScoreType = LOWER_BOUND;
				break;
			} // 剪枝
		}
	}


	if (bestMoveIndex != -1)
	{
		if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			// EXACT, UPPER_BOUND 或 LOWER_BOUND
			TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], bestScoreType);
		}


		// 如果是早停的话，搜到到的分数不是当前节点的最优分数，只是一个下限
		if (!isEarlyStopping) {
			enterHistoryScore(moveList[bestMoveIndex], depth, player);

		}

		if (depth == MaxDepth)
		{
			if (!isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
			{
				pipeOut("DEBUG ERROR: get an invalid move from TT");
				pipeOut("DEBUG invalid move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
			}
			else
				bestMove = moveList[bestMoveIndex];
		}
	}

	delete[] moveList;
	moveList = NULL;
	return alpha;
}

// AlphaBeta负极大值+历史启发值
//int AlphaBetaNewVal(int depth, int alpha, int beta, int player, int MaxDepth, int turn)
//{
//	if (depth <= 0)
//	{
//		return evaluate(player);
//	}
//
//	int moveListLen = 0;
//	Mov* moveList = GenerateMoves(moveListLen, player);
//
//	if (moveListLen == 0)
//	{
//		delete[] moveList;
//		moveList = NULL;
//		return evaluate(player);
//	}
//
//	// 从历史启发表获取初始值
//	for (int i = 0; i < moveListLen; i++)
//	{
//		moveList[i].val = getHistoryScore(moveList[i], player);
//	}
//
//	moveList = MergeSort(moveList, moveListLen); // 根据历史启发值对待搜索分支进行排序，提高AlphaBeta剪枝的概率
//
//	int bestMoveIndex = -1;
//	for (int i = 0; i < moveListLen; i++)
//	{
//		/*pipeOut("startTime: %ul", start_time);
//		pipeOut(" stopTime: %ul", stopTime());
//		pipeOut(" currTime: %ul", GetTickCount());
//		pipeOut(" use_Time: %ul\n", GetTickCount() - start_time);*/
//
//		if (terminate_v | GetTickCount() >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//		if (isGameOver(moveList[i]))
//		{
//			UnmakeMove(moveList[i]);
//			bestMoveIndex = i; // 设置bestMoveIndex还是必要的，这样能够处理depth == MaxDepth的情况
//			alpha = 9999;
//			break; // 直接剪枝
//		}
//
//		moveList[i].val = -AlphaBetaNewVal(depth - 1, -beta, -alpha, 1 - player, MaxDepth, 1 - turn);
//		UnmakeMove(moveList[i]);
//
//		if (moveList[i].val > alpha)
//		{
//			alpha = moveList[i].val;
//			/*if (depth == MaxDepth)
//			{
//			bestMoveIndex = i;
//			bestMove = moveList[bestMoveIndex];
//			}*/
//
//			// 直接更新吧，不要写冗余代码
//			// 更为重要的是，在AlphaBeta搜索中，每一个节点的bestMoveIndex这个信息都是有用的，不仅仅是需要一个bestVal分数而已
//			bestMoveIndex = i;
//		}
//		if (alpha >= beta)
//			break;
//	}
//
//	// 离开一个分支返回上一层时，用该分支的最优走法，更新历史启发表的值
//	if (bestMoveIndex != -1)
//	{
//		enterHistoryScore(moveList[bestMoveIndex], depth, player);
//	}
//
//	// 搜索最终都会返回到根节点，从根节点退出，不管是不是时间不够没搜索完就早停了
//	if (depth == MaxDepth) {
//		bestMove = moveList[bestMoveIndex];
//	}
//
//	// 统一处理资源销毁
//	delete[] moveList;
//	moveList = NULL;
//
//	return alpha;
//}

//产生当前棋盘的可行棋步，只获取棋盘上已有棋子三字以内的空棋位，并且考虑是否禁手
Mov* GenerateMoves(int& moveLen,int player)
{
	int range = 3;
	int count = 0;
	Psquare p = boardb;

	Mov* result = new Mov[width*height];

	//初始化标志位，避免重复加入最后队列 // @green 避免重复添加这种东西，不应该用Set来做吗，代码可读性真低
	int* makeSign = new int[width*height];
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			makeSign[i*width+j] = 0;
		}
	}

	//开始获取可行的棋位
	while(p != boardk)
	{
		if (p->z == TURN_MOVE || p->z == OPPONENT_MOVE)
		{
			for(int i = ( p->x - range < 0 ? 0 : p->x - range);i <= ( p->x + range > width ? width : p->x + range); i++)
			{
				for (int j = ( p->y - range <0 ? 0: p->y - range); j <= (p->y + range > height ? height:p->y + range); j++)
				{
					
					if ( Square(i,j)->z == EMPTY_MOVE && makeSign[i*width+j] == 0)
					{
						makeSign[i*width+j]=1;
						if (isBan(Square(i,j),player))//禁手判断，函数里会首先判断该选手是否需要禁手判断，在继续相关操作
						{
							continue;
						}
						result[count].x = i;
						result[count].y = j;
						result[count].val = 0;
						count++;
 					}
				}
			}	
		}
		p = (Psquare)(((char*)p)+ sizeof(Tsquare));
	}
	
	delete[] makeSign;
	//棋盘为空，则随机获取一个位置
	if (count == 0 )//棋盘为空或者棋盘已满
	{
		if (Square(0, 0)->z == EMPTY_MOVE) {//棋盘为空，从棋盘上随机选择一个位置
			srand(GetTickCount());
			int rndX = rand() % width;
			int rndY = rand() % height;
			for (int i = (rndX - range < 0 ? 0 : rndX - range); i <= (rndX + range > width ? width : rndX + range); i++)
			{
				for (int j = (rndY - range < 0 ? 0 : rndY - range); j <= (rndY + range > height ? height : rndY + range); j++)
				{
					//makeSign[i*width + j] = 1; // @green
					result[count].x = i;
					result[count].y = j;
					result[count].val = 0;
					count++;
				}
			}
		}
	}

	moveLen = count;
	return result;
}


//
Mov mov_rd(Mov* moveList,int count)
{
	srand(GetTickCount());
	int num = rand()%count;
	//pipeOut("DEBUG rnd :%d",num);
	return moveList[num];
}



//判断是否是禁手关键点
bool isBan(Psquare p0,int player)
{
	//检查禁手
	if (info_fb_check && player == firstPlayer )
	{
		ChessAnalyzeData checkData[4];
		p0->z = player+1; // @green 先假设棋盘上的空位放有棋子，方便分析
		int fb_type = ForbiddenCheck(checkData,p0);
		p0->z =  EMPTY_MOVE;
		
		if(fb_type != NO_FORBIDDEN)
		{
			return true;
		}
	}
	return false;
}



//判断执行move后棋局是否结束
bool isGameOver(Mov move)
{
	Psquare p0 = Square(move.x,move.y);
	
	for(int i=0;i<4;i++)
	{
		int s = diroff[i];
		Psquare pPrv = p0;
		Psquare pNxt = p0;
		int count = -1;
		do
		{
			prvP(pPrv,1);
			count++;
		}while(pPrv->z == p0->z);
		do
		{
			nxtP(pNxt,1);
			count++;
		} while (pNxt->z == p0->z);
		
		if (count == 5)
			return true;
		else if(count > 5 && !info_exact5 )
			return true;
	}
	return false;
}



void freeResource()
{
	delete []PosValue;
	PosValue = NULL;
	
	delete []m_nRecord;
	m_nRecord = NULL;

	//freeHistoryTable();
	//freeKillerMoves();

	//TransTable::freeInstance();
	/*delete[] boardChars;*/
};