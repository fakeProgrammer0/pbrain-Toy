#include "search.h"
#include "KillerTable.h"

KillerTable killerTable;

 //��ʼ��(���̵�)              
void brain_init(){
	pipeOut("DEBUG brain_init");
	if(width<5 || width>MAX_BOARD || height<5 || height>MAX_BOARD){
    pipeOut("ERROR size of the board");
    return;
  }
  
  //��ʼ������
  boardInit();
  evaluationInit(width,height);
  initHistoryTable(width,height);
  
  pipeOut("OK");
}

//��ʼ
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
//AI ����
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

//��������
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

//������
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

//�㷨ֹͣʱ��
DWORD stopTime()
{
	return start_time + min(info_timeout_turn, info_time_left/MATCH_SPARE)-30;
}


//��������
void brain_turn() 
{ 
	pipeOut("DEBUG brain_turn");

	//ʹ��negamax+��������
	IterDeeping();
	//ʹ�ü��������̶����
	//SimpleSearch();
}

void brain_end()
{
	freeResource();
}


//����δʵ��
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


//������������+NegaMax/alphaBeta���� 
void IterDeeping()
{
	//��������ʱ��   
    int nMinTimer =  GetTickCount() + unsigned int((min(info_timeout_turn, info_time_left/MATCH_SPARE)-30)*0.618f);   
	reSetHistoryTable(width,height); // ÿһ����Ҫ����������ʷ�������ֵ
	killerTable.resetKillerMoves();

	Mov LastBestMove;

	//������������   
    //for(int depth=1; depth<20;(depth < 4 ? depth *= 2 : depth +=1))   
    for(int depth=2; depth<20;(depth < 4 ? depth *= 2 : depth +=1))   
    {   
		bestMove.val = -10000;
		pipeOut("DEBUG MAX depth:%d,",depth);

		isEarlyStopping = false;
           
        int score;
		//������ֵ���� or // alpha-beta����
		//score = NegaMax(depth,0,depth);    
		//score = alphabeta(depth,-9999,9999,0,depth);  
		
		//score = NegaMaxAlphaBetaHistory(depth, -9999, 9999, 0, depth);
		//score = PVS(depth, -10000, 10000, 0, depth);
		score = PVS_TT(depth, -10000, 10000, 0, depth);
		//score = AB_TT(depth, -10000, 10000, 0, depth);
		//score = PVS_Killer(depth, -10000, 10000, 0, depth);

		// ��bestMove����ʷ����������������ǰ��ʷ���������߷���+1
		// ����ȷ��������������У���һ�ε������������ķ�֧����һ�εõ������ŷ�֧
		// ���ܲ��Ǻ�ǡ�����������΢С�ı仯��Ҳ�������������̫��ƫ��Ӱ��
		promoteHistoryScoreToMax(bestMove, 0);


		pipeOut("DEBUG BestScore:%d",score);
		pipeOut("DEBUG BestMove:[%d,%d],%d",bestMove.x,bestMove.y,bestMove.val);
		pipeOut("DEBUG Time use:%d,time left:%d",GetTickCount()- start_time,info_timeout_turn-(GetTickCount()-start_time));
		
		// ��ǿ����ֹ˼����ֹͣ����   
		//if(terminate_v)   
		if(isEarlyStopping) // ��isEarlyStopping�����жϣ���Ҫ��terminate_v
            break;   
   
        // ��ʱ���Ѿ��ﵽ�涨ʱ���һ�룬������һ���ʱ����ܲ�����ֹͣ������   
        if( GetTickCount() > nMinTimer)   
            break;   
   
        // �ڹ涨������ڣ�����ɱ�壬ֹͣ˼����   
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

//������ֵ����+��ʷ������
int NegaMax(int depth,int player,int MaxDepth)
{
	if (depth <=0)//Ԥ�в��ϵ�αҶ�ӽ�㣬��������������ֵ
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

	moveList = MergeSort(moveList,moveListLen); //��ʷ��������alpha-betaʱ����

	for(int i = 0;i<moveListLen;i++)
	{
		if (terminate_v || GetTickCount() >= stopTime())
		{
			pipeOut("DEBUG It's time to terminate");
			break;
		}

		MakeMove(moveList[i],player);

		if (isGameOver(moveList[i]))//���ӽ��Ϊʤ���ѷ�״̬��������Ҷ�ӽ�㣩������player������һ���߷�����ʤ
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

//�̶����+NegaMax���� 
void SimpleSearch()
{
	// DEBUG
	//printBoard();

	Mov resultMov;
	int depth = 4;
	
	bestMove.val = -10000;
	int score;
	//������ֵ����
	score = NegaMax00(depth, 0, depth); // scoreֻ�����ڷ�������������NegaMax00����������bestMoveȫ�ֱ�������Ϊbrain����һ���ж�
	//score = MinMax(depth, 0, depth);

	// ��ʵ��������������棬AlphaBeta������û���𵽺����Ե�Ч������Ϊû����ʷ����
	// ����һ���ˣ�����ʱ�䲻��������ֻ�������Ͻǵĵط�����s
	//score = AlphaBeta(depth, depth, 0, -10000, 10000);
	//score = NegaMaxAlphaBeta(depth, -10000, 10000, 0, depth);

	pipeOut("DEBUG Depth value:%d", score);
	pipeOut("DEBUG BestMove:[%d,%d],%d", bestMove.x, bestMove.y, bestMove.val);
	do_mymove(bestMove.x, bestMove.y);
}

//������ֵ����
int NegaMax00(int depth, int player, int MaxDepth)
{
	if (depth <= 0)//αҶ�ӽ��
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

		if (isGameOver(moveList[i]))//���ӽ��Ϊʤ���ѷ�״̬��������Ҷ�ӽ��
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
			if (depth == MaxDepth) // ���ø��ڵ�Ŀ�ѡ��֧����ֹ����ͣ��ʱbestMove��û�б�����
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
�̶���������С���� �ȼ���NegMax00 
�������㷨û���κΰ������͵���Ϥһ���߼���*/
//int MinMax(int depth, int player, int MaxDepth)
//{
//	if (depth <= 0)
//	{
//		return evaluate(0); // ��Ϊ��MinMax���̣����Ծ�������۶��Ǵ�Max�ڵ������
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
//	int bestVal = player == 0 ? -9999 : 9999; // player == 0 ��ǰΪMax�ڵ㣿��
//	for (int i = 0;i < moveListLen;i++)
//	{
//		// �����ʱ��ǿ�ƽ�����ֹͣ����
//		if (terminate_v || GetTickCount() >= stopTime())
//		{
//			break;
//		}
//
//		MakeMove(moveList[i], player);
//
//		// �����ⲽ�����ֽ�����player���ʤ��
//		// �и������ڣ�isGameOver����û���������Ϸ������ӣ�����isGameOver����Ҫ��MakeMove����֮��ִ��
//		// д��forѭ�����жϣ������Ƿ���MinMax�Ŀ�ʼλ�ã������𵽼�֦��Ч��������ʣ�µ�[�ֵܽڵ�]��û��Ҫ������
//		if (isGameOver(moveList[i]))
//		{
//			bestVal = player == 0 ? 9999 : -9999;
//			// ����depth==MaxDepth�����
//			bestMoveIndex = i;
//			UnmakeMove(moveList[i]);
//			break;
//		}
//
//		moveList[i].val = MinMax(depth - 1, 1 - player, MaxDepth); // ���¶��ӽڵ���������۷���
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
//	// ���ص�MinMax�ʼ�����ڵ㣬���øýڵ����һ��ѡ�� bestMove
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
//// �򵥵�ApphaBeta���̣������������max��Ϊ׼��evalutate(0)
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
//			if (moveList[i].val > alpha) // ����max�ڵ��alphaֵ
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
//			// ��Min�ڵ㷢��alpha��֦����ȥ��Min�ڵ��������֧
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
//// alphaBeta����
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


// AlphaBeta������ֵ+��ʷ����ֵ
int NegaMaxAlphaBetaHistory(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	// ����ʷ�û����в��ҵ��ýڵ㣬������ȴ��ڵ�ǰ�ڵ��������������
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

	// ����ʷ�������ȡ��ʼֵ
	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen); // ������ʷ����ֵ�Դ�������֧�����������AlphaBeta��֦�ĸ���

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
			bestMoveIndex = i; // ����bestMoveIndex���Ǳ�Ҫ�ģ������ܹ�����depth == MaxDepth�����
			alpha = 9999;
			break; // ֱ�Ӽ�֦
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

			// ֱ�Ӹ��°ɣ���Ҫд�������
			// ��Ϊ��Ҫ���ǣ���AlphaBeta�����У�ÿһ���ڵ��bestMoveIndex�����Ϣ�������õģ�����������Ҫһ��bestVal��������
			bestMoveIndex = i; 
		}
		if (alpha >= beta)
		{
			//logEvalBoard(player, "AlphaBeta Purge");
			break;
		}
	}

	// �뿪һ����֧������һ��ʱ���ø÷�֧�������߷���������ʷ�������ֵ
	if (bestMoveIndex != -1 && !isEarlyStopping)
	{
		enterHistoryScore(moveList[bestMoveIndex], depth, player);

		// �����û���������ȸ���Ľڵ�
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

	// �������ն��᷵�ص����ڵ㣬�Ӹ��ڵ��˳��������ǲ���ʱ�䲻��û���������ͣ��
	if (depth == MaxDepth) {
		if (isFree(moveList[bestMoveIndex].x, moveList[bestMoveIndex].y))
		{
			bestMove = moveList[bestMoveIndex];
		}
		else {
			pipeOut("DEBUG When Return from Root Node, bestMove is invalid!");
			pipeOut("DEBUG Invalid return Move:[%d,%d]", moveList[bestMoveIndex].x, moveList[bestMoveIndex].y);
		}
		//enterHistoryScore(moveList[bestMoveIndex], depth, player); // ������һ�㣬Ч����һ��
	}


	// ͳһ������Դ����
	delete[] moveList;
	moveList = NULL;

	return alpha;
}

// Principal Variation Search ��Ҫ��������
// ��Ϊû���ҵ����ʵ�α���룬������ʱ����ά���ٿƸ�����α������ʽ
int PVS(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // ���û������ҵ��˵�ǰ�ڵ�
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // ������ȴ��ڵ��ڵ�ǰ�ڵ��������������
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
			if (alpha < moveList[i].val && moveList[i].val < beta) // ��������֮�䣬Ԥ��ʧ�ܣ���Ҫ����һ��������AlphaBeta����
			{
				moveList[i].val = -PVS(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // ���µ�ǰ�ڵ���ӷ�֧����
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore >= alpha) alpha = bestScore; // ����[��ǰ�ڵ�ĸ����]�������ӷ�֧����
			if (alpha >= beta) break; // ��֦
		}
	}

	if (bestMoveIndex != -1)
	{
		// �������ͣ�Ļ����ѵ����ķ������ǵ�ǰ�ڵ�����ŷ�����ֻ��һ������
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

// Principal Variation Search ��Ҫ�������� + �û��� TT
int PVS_TT(int depth, int alpha, int beta, int player, int MaxDepth)
{
	int turn = (MaxDepth - depth) % 2;

	if (TransTable::getInstance().searchCurrBoardNode()) // ���û������ҵ��˵�ǰ�ڵ�
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode == NULL) pipeOut("DEBUG ERROR: Get NULL BoardNode From TT"); // Ӧ�ò�����ְ�
		if (currBoardNode->getDepth() >= depth) // ������ȴ��ڵ��ڵ�ǰ�ڵ��������������
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
				if (currBoardNode->getScore() >= beta) // score��ֵ����������ǰ�ڵ�Ķ��ӷ�֧��֦
				{
					hitFlag = true;
				}
				else break; // ����ֵbeta����score���п�����֮ǰ����������һ��ʱ�䲻������ֹͣ
			}
			case UPPER_BOUND:
			{
				if (currBoardNode->getScore() <= alpha) // scoreֵС��alpha������
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
				pipeOut("DEBUG ERROR: BUT TURN is opponent's turn!"); // Ӧ�ò��ᷢ����������������˵��ֻҪȷ�������ַ�������������������Ŀ��ͬ���������ֵ�˭�����ǹ̶���
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

		if (i == 0) // �Ե�һ����֧����������AlphaBeta����
		{
			moveList[i].val = -PVS_TT(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
		}
		else {
			// ����֮ǰ�ѵķ�֧�����ŷ�֧����Ҫ���������õ���������õģ������һ��խ��(alpha, alpha+1)�������������صķ���Ӧ�û�С��alpha
			// ���������Ĵ���ֵ��С������ʱ��Ҳ��AlphaBeta�����ٺܶ�
			moveList[i].val = -PVS_TT(depth - 1, -alpha - 1, -alpha, 1 - player, MaxDepth);
			if (alpha < moveList[i].val && moveList[i].val < beta) // ��������֮�䣬Ԥ��ʧ�ܣ���Ҫ����һ��������AlphaBeta����
			{
				moveList[i].val = -PVS_TT(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // ���µ�ǰ�ڵ���ӷ�֧����
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore > alpha) // ������alphaֵ�����ޣ�����alphaֵ
			{ 
				alpha = bestScore;
				bestScoreType = EXACT; // ��ȷֵ
			}; 
			if (alpha >= beta) 
			{ 
				bestScoreType = LOWER_BOUND; // ������֦����ǰ�ڵ�ķ���ֵֻ��һ�����ޣ������п������������õķ���
				break; // ��֦
			} 
		}
	}

	if (bestMoveIndex != -1) // bestMoveIndex == -1 ˵����ǰ�ڵ㻹û��������������Ϊʱ�䲻����ֹͣ
	{
		// û���ҵ��ڵ㣬�����ҵ��ڵ�����С�ڵ�ǰ���
		if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			if (bestScoreType == EXACT)
			{
				TransTable::getInstance().insertCurrBoardNode(bestScore, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
			else // UPPER_BOUND �� LOWER_BOUND
			{
				TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], bestScoreType);
			}
		}

		// �������ͣ�Ļ����ѵ��ķ������ǵ�ǰ�ڵ�����ŷ�����ֻ��һ������
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

	if (TransTable::getInstance().searchCurrBoardNode()) // ���û������ҵ��˵�ǰ�ڵ�
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // ������ȴ��ڵ��ڵ�ǰ�ڵ��������������
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
		//int score = evaluate_turn(player, turn); // �ϸ��ӵ���������
		int score = evaluate(player); // �򵥵���������
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
			if (alpha < moveList[i].val && moveList[i].val < beta) // ��������֮�䣬Ԥ��ʧ�ܣ���Ҫ����һ��������AlphaBeta����
			{
				moveList[i].val = -PVS_Killer(depth - 1, -beta, -alpha, 1 - player, MaxDepth);
			}
		}

		UnmakeMove(moveList[i]);

		if (moveList[i].val > bestScore) // ���µ�ǰ�ڵ���ӷ�֧����
		{
			bestScore = moveList[i].val;
			bestMoveIndex = i;

			if (bestScore > alpha) // ����[��ǰ�ڵ�ĸ����]�������ӷ�֧����
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
			} // ��֦
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
			else // UPPER_BOUND �� LOWER_BOUND
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

		// �������ͣ�Ļ����ѵ����ķ������ǵ�ǰ�ڵ�����ŷ�����ֻ��һ������
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

	if (TransTable::getInstance().searchCurrBoardNode()) // ���û������ҵ��˵�ǰ�ڵ�
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
		if (currBoardNode != NULL && currBoardNode->getDepth() >= depth) // ������ȴ��ڵ��ڵ�ǰ�ڵ��������������
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

		// ����[��ǰ�ڵ�ĸ����]�������ӷ�֧����
		if (moveList[i].val > alpha) // ���µ�ǰ�ڵ���ӷ�֧����
		{
			alpha = moveList[i].val;
			bestMoveIndex = i;
			bestScoreType = EXACT;

			if (alpha >= beta)
			{
				bestScoreType = LOWER_BOUND;
				break;
			} // ��֦
		}
	}


	if (bestMoveIndex != -1)
	{
		if (!TransTable::getInstance().searchCurrBoardNode() || TransTable::getInstance().getCurrBoardNode()->getDepth() <= depth)
		{
			// EXACT, UPPER_BOUND �� LOWER_BOUND
			TransTable::getInstance().insertCurrBoardNode(alpha, depth, turn, moveList[bestMoveIndex], bestScoreType);
		}


		// �������ͣ�Ļ����ѵ����ķ������ǵ�ǰ�ڵ�����ŷ�����ֻ��һ������
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

// AlphaBeta������ֵ+��ʷ����ֵ
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
//	// ����ʷ�������ȡ��ʼֵ
//	for (int i = 0; i < moveListLen; i++)
//	{
//		moveList[i].val = getHistoryScore(moveList[i], player);
//	}
//
//	moveList = MergeSort(moveList, moveListLen); // ������ʷ����ֵ�Դ�������֧�����������AlphaBeta��֦�ĸ���
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
//			bestMoveIndex = i; // ����bestMoveIndex���Ǳ�Ҫ�ģ������ܹ�����depth == MaxDepth�����
//			alpha = 9999;
//			break; // ֱ�Ӽ�֦
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
//			// ֱ�Ӹ��°ɣ���Ҫд�������
//			// ��Ϊ��Ҫ���ǣ���AlphaBeta�����У�ÿһ���ڵ��bestMoveIndex�����Ϣ�������õģ�����������Ҫһ��bestVal��������
//			bestMoveIndex = i;
//		}
//		if (alpha >= beta)
//			break;
//	}
//
//	// �뿪һ����֧������һ��ʱ���ø÷�֧�������߷���������ʷ�������ֵ
//	if (bestMoveIndex != -1)
//	{
//		enterHistoryScore(moveList[bestMoveIndex], depth, player);
//	}
//
//	// �������ն��᷵�ص����ڵ㣬�Ӹ��ڵ��˳��������ǲ���ʱ�䲻��û���������ͣ��
//	if (depth == MaxDepth) {
//		bestMove = moveList[bestMoveIndex];
//	}
//
//	// ͳһ������Դ����
//	delete[] moveList;
//	moveList = NULL;
//
//	return alpha;
//}

//������ǰ���̵Ŀ����岽��ֻ��ȡ���������������������ڵĿ���λ�����ҿ����Ƿ����
Mov* GenerateMoves(int& moveLen,int player)
{
	int range = 3;
	int count = 0;
	Psquare p = boardb;

	Mov* result = new Mov[width*height];

	//��ʼ����־λ�������ظ����������� // @green �����ظ�������ֶ�������Ӧ����Set�����𣬴���ɶ������
	int* makeSign = new int[width*height];
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			makeSign[i*width+j] = 0;
		}
	}

	//��ʼ��ȡ���е���λ
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
						if (isBan(Square(i,j),player))//�����жϣ�������������жϸ�ѡ���Ƿ���Ҫ�����жϣ��ڼ�����ز���
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
	//����Ϊ�գ��������ȡһ��λ��
	if (count == 0 )//����Ϊ�ջ�����������
	{
		if (Square(0, 0)->z == EMPTY_MOVE) {//����Ϊ�գ������������ѡ��һ��λ��
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



//�ж��Ƿ��ǽ��ֹؼ���
bool isBan(Psquare p0,int player)
{
	//������
	if (info_fb_check && player == firstPlayer )
	{
		ChessAnalyzeData checkData[4];
		p0->z = player+1; // @green �ȼ��������ϵĿ�λ�������ӣ��������
		int fb_type = ForbiddenCheck(checkData,p0);
		p0->z =  EMPTY_MOVE;
		
		if(fb_type != NO_FORBIDDEN)
		{
			return true;
		}
	}
	return false;
}



//�ж�ִ��move������Ƿ����
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