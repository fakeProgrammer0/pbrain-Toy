# PVS 主要变例搜索

## 1.Fail-soft AlphaBeta
Fail-soft AlphaBeta是AlphaBeta的变种。在AlphaBeta搜索中，返回值总是alpha，而实际上这个节点返回的启发评价值可能比alpha小，也就是说这个节点的启发评价值很差，但这是我们无法知道。如果像MinMax过程的负极大值形式一样，每个节点的初始启发评价值都是-INFINTY，随着对分支的搜索，启发评价值不断提升，最后返回这个分数，那么就能够获得更多的信息。Fail-soft AlphaBeta对剪枝的效率没有提升，但是却是其它搜索算法的基础。

## 2.Principal Variant Search
PVS（Principal Variant Search）是对Fail-soft AlphaBeta搜索的改进。它的思路是：对节点进行搜索时
1. 假设第一个分支是最优分支（Principal Variant），进行一次完整的搜索
2. 之后对其它分支进行搜索时，采用(alpha, alpha+1)的窗口值进行搜索（即把beta换成alpha+1）。因为使用窄窗探测，PVS搜索也被称为极小窗口搜索（Minimal Window Search)。
	1. 如果之前的最优分支假设是正确的话，那么对当前分支的搜索返回值value<=alpha。**由于使用的窗口值(alpha, alpha+1)比原来的(alpha, beta)更小，所以极大地节省了效率**。
	2. 如果alpha < value < beta，那么意味着假设错误，返回值甚至有可能比beta大，只是因为alpha+1的限制才使得搜索中发生剪枝，返回值小于实际值，需要重新进行一次完整的AlphaBeta搜索。并把当前节点当作主要变例，继续后续分支的搜索。
	3. 如果value >= beta，直接剪枝吧

### 以下是PVS和置换表结合的代码：
```c++
// Principal Variation Search 主要变例搜索 + 置换表 TT
int PVS_TT(int depth, int alpha, int beta, int player, int MaxDepth)
{
	if (TransTable::getInstance().searchCurrBoardNode()) // 在置换表中找到了当前节点
	{
		BoardNode* currBoardNode = TransTable::getInstance().getCurrBoardNode();
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
					bestMove = currBoardNode->getMove();
				}

				// 对于五子棋来说，只要确定了先手方，根据棋子数目是奇数还是偶数，接下来轮到谁走棋是固定的
				return currBoardNode->getScore();
			}
		}
	}

	if (depth <= 0)
	{
		int score = evaluate(player);
		//TransTable::getInstance().insertCurrBoardNode(score, depth); // 存储叶子节点的话，虽然提高了置换表命中率，但是太耗内存。
		return score;
	}

	int moveListLen = 0;
	Mov* moveList = GenerateMoves(moveListLen, player);
	if (moveListLen == 0)
	{
		delete[] moveList;
		moveList = NULL;
		return evaluate(player);
	}

	for (int i = 0; i < moveListLen; i++)
	{
		moveList[i].val = getHistoryScore(moveList[i], player);
	}

	moveList = MergeSort(moveList, moveListLen);

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
				TransTable::getInstance().insertCurrBoardNode(bestScore, depth, moveList[bestMoveIndex], bestScoreType);
			}
			else // UPPER_BOUND 或 LOWER_BOUND
			{
				TransTable::getInstance().insertCurrBoardNode(alpha, depth, moveList[bestMoveIndex], bestScoreType);
			}
		}

		// 如果是早停的话，搜到的分数不是当前节点的最优分数，只是一个下限
		if (!isEarlyStopping) {
			enterHistoryScore(moveList[bestMoveIndex], depth, player);
		}

		if (depth == MaxDepth)
		{
			bestMove = moveList[bestMoveIndex];
		}
	}

	delete[] moveList;
	moveList = NULL;
	return bestScore;
}
```















