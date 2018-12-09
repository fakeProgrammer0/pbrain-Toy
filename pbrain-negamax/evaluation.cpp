#include "evaluation.h"
#include "historytable.h"

int FakeForbiddenCheck(int player, ChessAnalyzeData *checkData, Psquare p0);
int KeyPointFakeForbiddenCheck(int player, Psquare p0, int direction, int shift);

extern int width,height;
/*
*评价相关初始化
*
*/
void evaluationInit(int width,int height){
	//分配记录
	if(m_nRecord) delete m_nRecord;
	m_nRecord = new int** [width];
	for(int i =0;i<width;i++)
	{
		m_nRecord[i] = new int* [height];
		for(int j=0; j<height;j++)
		{
			m_nRecord[i][j] = new int [4];
		}
	}
}

// @green 假禁手
//关键点禁手检测
int KeyPointFakeForbiddenCheck(int player, Psquare p0, int direction, int shift) {
	int result = NO_FORBIDDEN;
	ChessAnalyzeData checkData[4];

	int s = direction;
	Psquare p = p0;

	nxtP(p, shift);
	p->z = 1 + player; // 先假设有该玩家的棋子

	result = FakeForbiddenCheck(player, checkData, p);
	p->z = 0;

	return result;
}

//假禁手检测
int FakeForbiddenCheck(int player, ChessAnalyzeData *checkData, Psquare p0)
{
	//禁手类型统计
	int CountThree = 0;
	int CountFour = 0;
	int adjsame[4];
	for (int i = 0; i<4; i++) {
		AnalysisLine(p0, diroff[i], &checkData[i]);
		adjsame[i] = checkData[i].adjsamePre + checkData[i].adjsameNxt - 1;
		if (adjsame[i] == 5)
		{
			return NO_FORBIDDEN; // @green 黑方如果下这一步直接五连赢棋，那么禁手规则无效，黑方胜利
		}
	}

	for (int i = 0; i < 4; i++)
	{

		int direction = diroff[i];
		//开始禁手分析

		if (adjsame[i] > 5)//长链禁手
			return LONGFBD;

		if (adjsame[i] == 4) //四子相连
		{
			//活四冲四判断
			bool isFour = false;
			if (checkData[i].adjemptyNxt > 0)
			{
				//递归判断关键点是否可下
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					isFour = true;
			}

			if (checkData[i].adjemptyPre > 0)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					isFour = true;
			}

			if (isFour)
				CountFour++;
		}
		else if (adjsame[i] == 3)  //三子相连
		{
			//冲四检查
			if (checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 1)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					CountFour++;
			}

			if (checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 1)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					CountFour++;
			}

			//活三检查
			bool isThree = false;

			if ((checkData[i].adjemptyNxt > 2 || checkData[i].adjemptyNxt == 2 && checkData[i].jumpsameNxt == 0) &&
				(checkData[i].adjemptyPre > 1 || checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					isThree = true;
			}

			if ((checkData[i].adjemptyPre > 2 || checkData[i].adjemptyPre == 2 && checkData[i].jumpsamePre == 0) &&
				(checkData[i].adjemptyNxt > 1 || checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					isThree = true;
			}

			if (isThree)
				CountThree++;

		}
		else if (adjsame[i] == 2) //两子相连
		{
			//冲四判断
			if (checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 2)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					CountFour++;
			}

			if (checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 2)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					CountFour++;
			}

			//活三判断
			if (checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 1 && (checkData[i].jumpemptyNxt > 1
				|| checkData[i].jumpemptyNxt == 1 && checkData[i].jumpjumpsameNxt == 0) && (checkData[i].adjemptyPre > 1
					|| checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					CountThree++;
			}

			if (checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 1 && (checkData[i].jumpemptyPre > 1
				|| checkData[i].jumpemptyPre == 1 && checkData[i].jumpjumpsamePre == 0) && (checkData[i].adjemptyNxt > 1
					|| checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					CountThree++;
			}
		}
		else if (adjsame[i] == 1) //单独一子
		{
			//冲四判断
			if (checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 3)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					CountFour++;
			}

			if (checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 3)
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					CountFour++;
			}

			//活三判断
			if (checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 2 && (checkData[i].jumpemptyNxt >1 ||
				checkData[i].jumpemptyNxt == 1 && checkData[i].jumpjumpsameNxt == 0) &&
				(checkData[i].adjemptyPre > 1 || checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, direction, checkData[i].adjsameNxt) == NO_FORBIDDEN)
					CountThree++;
			}

			if (checkData[i].adjemptyPre == 1 && checkData[i].jumpsamePre == 2 && (checkData[i].jumpemptyPre >1 ||
				checkData[i].jumpemptyPre == 1 && checkData[i].jumpjumpsamePre == 0) &&
				(checkData[i].adjemptyNxt > 1 || checkData[i].adjemptyNxt == 1 && checkData[i].jumpsameNxt == 0))
			{
				if (KeyPointFakeForbiddenCheck(player, p0, -direction, checkData[i].adjsamePre) == NO_FORBIDDEN)
					CountThree++;
			}
		}

		//检测禁手分析结果，适时返回结果，减少计算
		if (CountFour > 1)//四四禁手
		{
			return FOUR_FOUR_FBD;
		}
		if (CountThree > 1)//三三禁手
		{
			return THREE_THREE_FBD;
		}
	}

	if (CountFour == 1 && CountThree == 1)
	{
		clog << "DEBUG FOUR_THREE_NO_FBD: [" << p0->x << "," << p0->y << "]" << endl;
		logEvalBoard(player);
		return FOUR_THREE_NO_FBD;
	}

	return NO_FORBIDDEN;
}


//@green
//判断一步棋是否是引起禁手关键点，但这个函数的逻辑是用来引导[非禁手方]下棋的，所以要说成“假禁手”
//因为三三、四四、三四这些假禁手棋型，对棋手越有利，一般启发评价值都很高
//没办法，源代码中设计的启发函数太烂了，不然我也不想用这种套路
//bool causeBan(Psquare p0, int player)
//{
//	//检查非先手方
//	if (player != firstPlayer)
//	{
//		ChessAnalyzeData checkData[4];
//		p0->z = player + 1;
//		int fb_type = ForbiddenCheck(checkData, p0);
//		p0->z = EMPTY_MOVE;
//
//		if (fb_type != NO_FORBIDDEN)
//		{
//			return true;
//		}
//	}
//	return false;
//}

/* @green 抽取原来统计棋型的代码 */
inline void countBoardTypes(int turn)
{
	int i, j, k;
	//初始化 位置分析记录表
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			m_nRecord[i][j][0] = TOBEANALYZE;
			m_nRecord[i][j][1] = TOBEANALYZE;
			m_nRecord[i][j][2] = TOBEANALYZE;
			m_nRecord[i][j][3] = TOBEANALYZE;
		}
	}
	//初始化 各棋型 总数记录表
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j< CHESSTYPESCNT; j++) // @green TODO: 7需要被修改
		{
			m_nRecordCount[i][j] = 0;
		}
	}

	//pipeOut("DEBUG before analyze");
	//分析统计横、竖、左斜、右斜的棋型
	for (i = 0; i< width; i++)
	{
		for (j = 0; j<height; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				int s = diroff[k];
				Psquare p0 = Square(i, j);
				if (m_nRecord[i][j][k] == TOBEANALYZE && p0->z != EMPTY_MOVE)
				{
					ChessAnalyzeData* data = new ChessAnalyzeData();//记录棋行相关统计信息的结构

					AnalysisLine(p0, s, data);//统计棋型信息

					AnalysisBoardType(data, k, m_nRecord, i, j);//根据统计的信息分析包含的棋型并更新位置分析记录表
					delete[] data;
				}
			}
		}
	}

	//pipeOut("DEBUG before count");
	//统计各棋型的总数
	for (i = 0; i< width; i++)
	{
		for (j = 0; j<height; j++)
		{
			Psquare p0 = Square(i, j);

			if (p0->z != EMPTY_MOVE)
			{
				for (k = 0; k<4; k++)
				{
					int nPlayTurn = p0->z - 1; //该位置代表的玩家
					switch (m_nRecord[i][j][k])
					{
					case FIVE://五连
						m_nRecordCount[nPlayTurn][FIVE]++;
						break;
					case FOUR://活四
						m_nRecordCount[nPlayTurn][FOUR]++;
						break;
					case SFOUR://冲四
						m_nRecordCount[nPlayTurn][SFOUR]++;
						break;
					case THREE://活三
						m_nRecordCount[nPlayTurn][THREE]++;
						break;
					case STHREE://眠三
						m_nRecordCount[nPlayTurn][STHREE]++;
						break;
					case TWO://活二
						m_nRecordCount[nPlayTurn][TWO]++;
						break;
					case TWO * 2://两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
						m_nRecordCount[nPlayTurn][TWO] += 2;
						break;
					default:
						break;
					}

				}
			}
		}
	}

	if (info_fb_check)
	{
		if (m_nRecordCount[firstPlayer][FOUR]) // 检查四三非禁手 firstPlayer
		{
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					Psquare p0 = Square(x, y);
					if (p0->z == firstPlayer + 1)
					{
						ChessAnalyzeData checkData[4];
						int no_fb_type = FakeForbiddenCheck(firstPlayer, checkData, p0);
						if (no_fb_type == FOUR_THREE_NO_FBD)
						{
							int CountFOUR = 0, CountSFOUR = 0, CountTHREE = 0, CountSTHREE = 0;
							for (int k = 0; k < 4; k++)
							{
								int s = diroff[k];
								ChessAnalyzeData* data = new ChessAnalyzeData();//记录棋行相关统计信息的结构

								AnalysisLine(p0, s, data);//统计棋型信息

														  // 调用该函数会在原来的棋型分析上叠加，可能会有副作用
								AnalysisBoardType(data, k, m_nRecord, x, y);//根据统计的信息分析包含的棋型并更新位置分析记录表
								delete[] data;

								int nPlayTurn = p0->z - 1; //该位置代表的玩家
								switch (m_nRecord[x][y][k])
								{
								case FIVE://五连
										  //
									break;
								case FOUR://活四
									CountFOUR++;
									break;
								case SFOUR://冲四
									CountSFOUR++;
									break;
								case THREE://活三
									CountTHREE++;
									break;
								case STHREE://眠三
									CountSTHREE++;
									break;
								case TWO://活二
										 //;
									break;
								case TWO * 2://两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
											 //;
									break;
								default:
									break;
								}
							}


							if (CountFOUR == 1 && CountTHREE == 1)
								m_nRecordCount[firstPlayer][FOUR_THREE] = 1;
							if (CountFOUR == 1 && CountSTHREE == 1)
								m_nRecordCount[firstPlayer][FOUR_STHREE] = 1;
							if (CountSFOUR == 1 && CountTHREE == 1)
								m_nRecordCount[firstPlayer][SFOUR_THREE] = 1;
							if (CountSFOUR == 1 && CountSTHREE == 1)
								m_nRecordCount[firstPlayer][SFOUR_STHREE] = 1;
						}
					}
				}
			}
		}
	}
	else
	{
		//bool stopCheckFakeFBD = false;
		if (m_nRecordCount[firstPlayer][FOUR] || m_nRecordCount[firstPlayer][THREE]) // 只有含有三或四才检查假禁手
		{
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					Psquare p0 = Square(x, y);
					if (p0->z == firstPlayer + 1) // 是firstPlayer的棋子
					{
						ChessAnalyzeData checkData[4];
						int fake_fb_type = FakeForbiddenCheck(firstPlayer, checkData, p0);

						if (fake_fb_type != NO_FORBIDDEN)
						{
							if (fake_fb_type == FOUR_FOUR_FBD)
							{
								m_nRecordCount[firstPlayer][FOUR_FOUR_FAKE_FBD] = 1;
								//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD: [%d,%d]", x, y);
							}
							else if (fake_fb_type == THREE_THREE_FBD)
							{
								m_nRecordCount[firstPlayer][THREE_THREE_FAKE_FBD] = 1;
								//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD: [%d,%d]", x, y);
							}

							// 四三非禁手
							if (fake_fb_type == FOUR_THREE_NO_FBD)
							{
								int CountFOUR = 0, CountSFOUR = 0, CountTHREE = 0, CountSTHREE = 0;
								for (int k = 0; k < 4; k++)
								{
									int s = diroff[k];
									ChessAnalyzeData* data = new ChessAnalyzeData();//记录棋行相关统计信息的结构

									AnalysisLine(p0, s, data);//统计棋型信息

									// 调用该函数会在原来的棋型分析上叠加，可能会有副作用
									AnalysisBoardType(data, k, m_nRecord, x, y);//根据统计的信息分析包含的棋型并更新位置分析记录表
									delete[] data;

									int nPlayTurn = p0->z - 1; //该位置代表的玩家
									switch (m_nRecord[x][y][k])
									{
									case FIVE://五连
											  //
										break;
									case FOUR://活四
										CountFOUR++;
										break;
									case SFOUR://冲四
										CountSFOUR++;
										break;
									case THREE://活三
										CountTHREE++;
										break;
									case STHREE://眠三
										CountSTHREE++;
										break;
									case TWO://活二
											 //;
										break;
									case TWO * 2://两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
												 //;
										break;
									default:
										break;
									}
								}

								if (CountFOUR == 1 && CountTHREE == 1)
									m_nRecordCount[firstPlayer][FOUR_THREE] = 1;
								if (CountFOUR == 1 && CountSTHREE == 1)
									m_nRecordCount[firstPlayer][FOUR_STHREE] = 1;
								if (CountSFOUR == 1 && CountTHREE == 1)
									m_nRecordCount[firstPlayer][SFOUR_THREE] = 1;
								if (CountSFOUR == 1 && CountSTHREE == 1)
									m_nRecordCount[firstPlayer][SFOUR_STHREE] = 1;
							}
						}

					}
				}
			}
		}
	}
	
		

	// TODO: 对假禁手的检测做优化
	const int secondPlayer = 1 - firstPlayer;
	//bool stopCheckFakeFBD = false;
	if (m_nRecordCount[secondPlayer][FOUR] || m_nRecordCount[secondPlayer][THREE]) // 只有含有三或四才检查假禁手
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				Psquare p0 = Square(x, y);
				if (p0->z == secondPlayer + 1) // 是secondPlayer的棋子
				{
					ChessAnalyzeData checkData[4];
					int fake_fb_type = FakeForbiddenCheck(secondPlayer, checkData, p0);

					if (fake_fb_type != NO_FORBIDDEN)
					{
						if (fake_fb_type == FOUR_FOUR_FBD)
						{
							m_nRecordCount[secondPlayer][FOUR_FOUR_FAKE_FBD] = 1;
							//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD: [%d,%d]", x, y);
						}
						else if (fake_fb_type == THREE_THREE_FBD)
						{
							m_nRecordCount[secondPlayer][THREE_THREE_FAKE_FBD] = 1;
							//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD: [%d,%d]", x, y);
						}

						// 四三非禁手
						if (fake_fb_type == FOUR_THREE_NO_FBD)
						{
							int CountFOUR = 0, CountSFOUR = 0, CountTHREE = 0, CountSTHREE = 0;
							for (int k = 0; k < 4; k++)
							{
								int s = diroff[k];
								ChessAnalyzeData* data = new ChessAnalyzeData();//记录棋行相关统计信息的结构

								AnalysisLine(p0, s, data);//统计棋型信息

								// 调用该函数会在原来的棋型分析上叠加，可能会有副作用
								AnalysisBoardType(data, k, m_nRecord, x, y);//根据统计的信息分析包含的棋型并更新位置分析记录表
								delete[] data;

								int nPlayTurn = p0->z - 1; //该位置代表的玩家
								switch (m_nRecord[x][y][k])
								{
								case FIVE://五连
										  //
									break;
								case FOUR://活四
									CountFOUR++;
									break;
								case SFOUR://冲四
									CountSFOUR++;
									break;
								case THREE://活三
									CountTHREE++;
									break;
								case STHREE://眠三
									CountSTHREE++;
									break;
								case TWO://活二
										 //;
									break;
								case TWO * 2://两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
											 //;
									break;
								default:
									break;
								}
							}

							if (CountFOUR == 1 && CountTHREE == 1)
								m_nRecordCount[secondPlayer][FOUR_THREE] = 1;
							if (CountFOUR == 1 && CountSTHREE == 1)
								m_nRecordCount[secondPlayer][FOUR_STHREE] = 1;
							if (CountSFOUR == 1 && CountTHREE == 1)
								m_nRecordCount[secondPlayer][SFOUR_THREE] = 1;
							if (CountSFOUR == 1 && CountSTHREE == 1)
								m_nRecordCount[secondPlayer][SFOUR_STHREE] = 1;
						}
					}

					// 假禁手检测太耗时间，早停策略：找到就停
					/*if (m_nRecordCount[secondPlayer][FOUR_FOUR_FAKE_FBD] || 
						m_nRecordCount[secondPlayer][THREE_THREE_FAKE_FBD] && (
							m_nRecordCount[firstPlayer][FIVE] == 0  
							&& m_nRecordCount[firstPlayer][FOUR] == 0 
							&& m_nRecordCount[firstPlayer][SFOUR] == 0))
					{
						stopCheckFakeFBD = true;
						break;
					}*/
				}

				// 假禁手检测太耗时间，早停策略：找到就停
				/*if (stopCheckFakeFBD)
				{
					break;
				}*/
			}
		}
	}

}

/* 跟据下一步是哪一方走棋对局面进行评估，修改原来的 evaluate 函数 */
int evaluate_turn(int player, int turn)
{
	//pipeOut("DEBUG evaluate");

	countBoardTypes(turn);

	//pipeOut("DEBUG before return score");
	int opponentPlayer = 1 - player; //对手玩家
	int Turn_Value = 0, Opponent_Turn_Value = 0;

	bool isSecondPlayer = player != firstPlayer;

	if (turn == player) // 己方下棋
	{
		/*
		五连局面 9999
		对手五连 -9999
		己方四四假禁手 9990
		己方活四活三  9980
		己方活四眠三 9970
		己方活四 9960
		己方冲四活三 9950
		己方冲四眠三 9940
		己方冲四 9930
		对方四四假禁手 -9990
		对方活四活三  -9980
		对方活四眠三 -9970
		对方活四 -9960
		己方冲四活三 -9950
		己方冲四眠三 -9940
		对方冲四 -9930 // 必防
		己方三三假禁手 9920
		己方活三 9910
		对方三三假禁手 且己方没有眠三 -9920
		对方活三 且己方没有眠三 -9910 // 必防。如果有眠三的话，可以采用顽强进攻的策略
		*/


		//如果有五连直接返回胜利代表的值
		if (m_nRecordCount[player][FIVE] > 0)
			return 9999;//成功
		else if (m_nRecordCount[opponentPlayer][FIVE] > 0)
			return -9999;//失败

		if (isSecondPlayer || !info_fb_check)
			if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
				//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
				//logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
				return 9990;
			}

		if (m_nRecordCount[player][FOUR_THREE])
		{
			//pipeOut("DEBUG Find my FOUR_THREE");
			//logEvalBoard(player, "DEBUG Find my FOUR_THREE");
			return 9980;
		}

		if (m_nRecordCount[player][FOUR_STHREE])
		{
			//pipeOut("DEBUG Find my FOUR_STHREE");
			//logEvalBoard(player, "DEBUG Find my FOUR_STHREE");
			return 9970;
		}

		//己手活四，必胜 		
		if (m_nRecordCount[player][FOUR])
			return  9960;//必胜

		if (m_nRecordCount[player][SFOUR_THREE])
		{
			//pipeOut("DEBUG Find my SFOUR_THREE");
			//logEvalBoard(player, "DEBUG Find my SFOUR_THREE");
			return 9950;
		}

		if (m_nRecordCount[player][SFOUR_STHREE])
		{
			//pipeOut("DEBUG Find my SFOUR_STHREE");
			//logEvalBoard(player, "DEBUG Find my SFOUR_STHREE");
			return 9940;
		}

		if (m_nRecordCount[player][SFOUR]) //己方冲四，必胜
			return 9930;

		if (!isSecondPlayer || !info_fb_check)
			if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
				return -9990;

		if (m_nRecordCount[opponentPlayer][FOUR_THREE])
		{
			//pipeOut("DEBUG Find opponent's FOUR_THREE");
			//logEvalBoard(1 - player, "DEBUG Find opponent's FOUR_THREE");
			return -9980;
		}

		if (m_nRecordCount[opponentPlayer][FOUR_STHREE])
		{
			//pipeOut("DEBUG Find opponent's FOUR_STHREE");
			//logEvalBoard(1 - player, "DEBUG Find opponent's FOUR_STHREE");
			return -9970;
		}

		//对手活四，必败
		if (m_nRecordCount[opponentPlayer][FOUR])
			return -9960;

		if (m_nRecordCount[opponentPlayer][SFOUR_THREE])
		{
			//pipeOut("DEBUG Find opponent's SFOUR_THREE");
			//logEvalBoard(1 - player, "DEBUG Find opponent's SFOUR_THREE");
			return -9950;
		}

		if (m_nRecordCount[opponentPlayer][SFOUR_STHREE])
		{
			//pipeOut("DEBUG Find opponent's SFOUR_STHREE");
			//logEvalBoard(1 - player, "DEBUG Find opponent's SFOUR_STHREE");
			return -9940;
		}

		//对手冲四，必防
		if (m_nRecordCount[opponentPlayer][SFOUR])
			return -9930;

		// 己方三三假禁手
		if (isSecondPlayer || !info_fb_check)
			if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
				//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
				//logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
				return 9920;
			}


		// 己方活三
		if (m_nRecordCount[player][THREE])
			return 9910;

		if (!isSecondPlayer || !info_fb_check) // 对方三三假禁手
			if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
				if (m_nRecordCount[player][STHREE] == 0) // 而己方没有眠三，基本凉了，因为是防不住的
					return -9920;
				else Opponent_Turn_Value += 2000;

		// 对方活三，必防，如果你没有[眠三]的话
		if (m_nRecordCount[opponentPlayer][THREE])
			if (m_nRecordCount[player][STHREE] == 0)
				return -9910;
		//else
		//Opponent_Turn_Value += 2000;

		if (m_nRecordCount[opponentPlayer][THREE] > 1)
			Opponent_Turn_Value += 2000;
		else
		{
			if (m_nRecordCount[opponentPlayer][THREE])
				Opponent_Turn_Value += 200;
		}

	}
	else // 轮到对方走棋
	{
		/*
		对方五连 -9999
		己方五连 9999
		对方四四假禁手 -9980
		对方活四 -9970
		对方冲四 -9960
		己方四四假禁手 9980
		己方活四 9970
		己方冲四 9960 // 对方必防
		对方三三假禁手 -9950
		对方活三 -9940
		己方三三假禁手 且对方没有眠三 9950
		己方活三 且对方没有眠三 9940
		*/

		if (m_nRecordCount[opponentPlayer][FIVE] > 0)
			return -9999;//失败
		if (m_nRecordCount[player][FIVE] > 0)
			return 9999;//成功

		if (!isSecondPlayer)
			if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
				return -9980;

		/*计算并返回评价值*/
		//对手活四 / 冲四，必败
		if (m_nRecordCount[opponentPlayer][FOUR])
			return -9970;

		if (m_nRecordCount[opponentPlayer][SFOUR])
			return -9960;

		// 己方四四假禁手
		if (isSecondPlayer)
			if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
				//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
				//logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
				return 9980;
			}

		//己手活四，必胜 		
		if (m_nRecordCount[player][FOUR])
			return  9970;//必胜
		if (m_nRecordCount[player][SFOUR]) //己方冲四，对方必防
			return 9960;

		if (!isSecondPlayer) // 对方三三假禁手
			if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
				return -9950;

		// 对方活三
		if (m_nRecordCount[opponentPlayer][THREE])
		{
			return -9940;
		}

		// 己方三三假禁手
		if (isSecondPlayer)
			if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
				if (m_nRecordCount[opponentPlayer][STHREE] == 0)
					//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
					//logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
					return 9950;
				else
					Turn_Value += 4000;
			}

		if (m_nRecordCount[player][THREE])
			if (m_nRecordCount[opponentPlayer][STHREE] == 0)
				return 9940;
		//else
		//Turn_Value += 4000;


		if (m_nRecordCount[player][THREE]>1)
		{
			Turn_Value += 500;
		}
		else
		{
			if (m_nRecordCount[player][THREE])
				Turn_Value += 100;
		}

	}

	//if (turn == player) // 己方下棋
	//{
	//	/*
	//	轮到自己下棋
	//	五连局面 9999
	//	对手五连 -9999
	//	己方四四假禁手 9980
	//	己方活四 9970
	//	己方冲四 9960
	//	对方四四假禁手 -9980
	//	对方活四 -9970
	//	对方冲四 -9960 // 必防
	//	己方三三假禁手 9950
	//	己方活三 9940
	//	对方三三假禁手 -9950
	//	对方活三 且己方没有眠三 -9940 // 必防。如果有眠三的话，可以采用顽强进攻的策略
	//	*/


	//	//如果有五连直接返回胜利代表的值
	//	if (m_nRecordCount[player][FIVE] > 0)
	//		return 9999;//成功
	//	else if (m_nRecordCount[opponentPlayer][FIVE] > 0)
	//		return -9999;//失败
	//	
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
	//			pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			return 9980;
	//		}
	//			
	//	
	//	//己手活四，必胜 		
	//	if (m_nRecordCount[player][FOUR]) 
	//		return  9970;//必胜
	//	if (m_nRecordCount[player][SFOUR]) //己方冲四，必胜
	//		return 9960;

	//	if (!isSecondPlayer)
	//		if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
	//			return -9980;

	//	//对手活四，必败
	//	if (m_nRecordCount[opponentPlayer][FOUR])
	//		return -9970;

	//	//对手冲四，必防
	//	if (m_nRecordCount[opponentPlayer][SFOUR])
	//		return -9960;

	//	// 己方三三假禁手
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
	//			pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
	//			logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
	//			return 9950;
	//		}
	//			

	//	// 己方活三
	//	if (m_nRecordCount[player][THREE])
	//		return 9940;

	//	if (!isSecondPlayer) // 对方三三假禁手
	//		if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
	//			if(m_nRecordCount[player][STHREE] == 0) // 而己方没有眠三，基本凉了，因为是防不住的
	//				return -9950;
	//			else Opponent_Turn_Value += 2000;

	//	// 对方活三，必防，如果你没有[眠三]的话
	//	if (m_nRecordCount[opponentPlayer][THREE])
	//		if(m_nRecordCount[player][STHREE] == 0)
	//			return -9950;
	//		//else
	//			//Opponent_Turn_Value += 2000;

	//	if (m_nRecordCount[opponentPlayer][THREE] > 1)
	//		Opponent_Turn_Value += 2000;
	//	else
	//	{
	//		if (m_nRecordCount[opponentPlayer][THREE])
	//			Opponent_Turn_Value += 200;
	//	}

	//}
	//else // 轮到对方走棋
	//{
	//	/*
	//	对方五连 -9999
	//	己方五连 9999
	//	对方四四假禁手 -9980
	//	对方活四 -9970
	//	对方冲四 -9960
	//	己方四四假禁手 9980
	//	己方活四 9970
	//	己方冲四 9960 // 对方必防
	//	对方三三假禁手 -9950
	//	对方活三 -9940 
	//	己方三三假禁手 且对方没有眠三 9950
	//	己方活三 且对方没有眠三 9940
	//	*/

	//	if (m_nRecordCount[opponentPlayer][FIVE] > 0)
	//		return -9999;//失败
	//	if (m_nRecordCount[player][FIVE] > 0)
	//		return 9999;//成功

	//	if (!isSecondPlayer)
	//		if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
	//			return -9980;

	//	/*计算并返回评价值*/
	//	//对手活四 / 冲四，必败
	//	if (m_nRecordCount[opponentPlayer][FOUR]) 
	//		return -9970;

	//	if (m_nRecordCount[opponentPlayer][SFOUR])
	//		return -9960;

	//	// 己方四四假禁手
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
	//			//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			//logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			return 9980;
	//		}

	//	//己手活四，必胜 		
	//	if (m_nRecordCount[player][FOUR])
	//		return  9970;//必胜
	//	if (m_nRecordCount[player][SFOUR]) //己方冲四，对方必防
	//		return 9960;

	//	if (!isSecondPlayer) // 对方三三假禁手
	//		if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
	//				return -9950;

	//	// 对方活三
	//	if (m_nRecordCount[opponentPlayer][THREE])
	//	{
	//		return -9940;
	//	}

	//	// 己方三三假禁手
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
	//			if (m_nRecordCount[opponentPlayer][STHREE] == 0)
	//				//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
	//				//logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
	//				return 9950;
	//			else
	//				Turn_Value += 4000;
	//		}

	//	if (m_nRecordCount[player][THREE])
	//		if (m_nRecordCount[opponentPlayer][STHREE] == 0)
	//			return 9940;
	//		//else
	//			//Turn_Value += 4000;
	//	

	//	if (m_nRecordCount[player][THREE]>1)
	//	{
	//		Turn_Value += 500;
	//	}
	//	else
	//	{
	//		if (m_nRecordCount[player][THREE])
	//			Turn_Value += 100;
	//	}

	//}


	// 卧槽 还真有打错的
	/*if (m_nRecordCount[opponentPlayer][THREE] && m_nRecordCount[player][SFOUR] == 0)
	{
		return -9950;
	}*/

	/*if (m_nRecordCount[player][THREE] > 1 &&
		m_nRecordCount[opponentPlayer][SFOUR] == 0 &&
		m_nRecordCount[opponentPlayer][THREE] == 0 &&
		m_nRecordCount[opponentPlayer][STHREE] == 0)
	{
		return 9940;
	}*/


	if (m_nRecordCount[player][STHREE])
		Turn_Value += m_nRecordCount[player][STHREE] * 10;
	if (m_nRecordCount[opponentPlayer][STHREE])
		Opponent_Turn_Value += m_nRecordCount[opponentPlayer][STHREE] * 10;
	if (m_nRecordCount[player][TWO])
		Turn_Value += m_nRecordCount[player][TWO] * 4;
	if (m_nRecordCount[opponentPlayer][TWO])
		Opponent_Turn_Value += m_nRecordCount[opponentPlayer][TWO] * 4;

	return Turn_Value - Opponent_Turn_Value;
}


/*
*评价函数
*/

int evaluate(int player)
{
	//pipeOut("DEBUG evaluate");

	int i,j,k;
	//初始化 位置分析记录表
	for(i = 0;i < width;i++)
	{
		for(j = 0; j < height; j++)
		{
			m_nRecord[i][j][0] = TOBEANALYZE;
			m_nRecord[i][j][1] = TOBEANALYZE;
			m_nRecord[i][j][2] = TOBEANALYZE;
			m_nRecord[i][j][3] = TOBEANALYZE;
  		}
	}
	//初始化 各棋型 总数记录表
	for(i = 0;i < 2;i++)
	{
		for(j = 0;j< 7;j++)
		{
			m_nRecordCount[i][j] = 0;
		}
	}

	//pipeOut("DEBUG before analyze");
	//分析统计横、竖、左斜、右斜的棋型
	for(i = 0; i< width;i++)
	{
		for(j = 0; j<height;j++)
		{
			for (int k = 0; k < 4; k++) 
			{
				int s = diroff[k];
				Psquare p0 = Square(i,j); 
				if(m_nRecord[i][j][k] == TOBEANALYZE && p0->z != EMPTY_MOVE)
				{
					ChessAnalyzeData* data = new ChessAnalyzeData();//记录棋行相关统计信息的结构
				
					AnalysisLine(p0,s,data);//统计棋型信息
			
					AnalysisBoardType(data,k,m_nRecord,i,j);//根据统计的信息分析包含的棋型并更新位置分析记录表
					delete[] data;
				}
			}
		}
	}

	//pipeOut("DEBUG before count");
	//统计各棋型的总数
	for(i = 0; i< width ;i++)
	{
		for(j = 0;j<height;j++)
		{
			Psquare p0 = Square(i,j);

			if(p0->z != EMPTY_MOVE)
			{
				for(k = 0;k<4;k++)
				{
					int nPlayTurn = p0->z -1; //该位置代表的玩家
					switch (m_nRecord[i][j][k])
					{
					case FIVE://五连
						m_nRecordCount[nPlayTurn][FIVE]++;
						break;
					case FOUR://活四
						m_nRecordCount[nPlayTurn][FOUR]++;
						break;
					case SFOUR://冲四
						m_nRecordCount[nPlayTurn][SFOUR]++;
						break;
					case THREE://活三
						m_nRecordCount[nPlayTurn][THREE]++;
						break;
					case STHREE://眠三
						m_nRecordCount[nPlayTurn][STHREE]++;
						break;
					case TWO://活二
						m_nRecordCount[nPlayTurn][TWO]++;
						break;
					case TWO * 2://两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
						m_nRecordCount[nPlayTurn][TWO] += 2;
						break;
					default:
						break;
					}

				}
			}
		}
	}

	//pipeOut("DEBUG before return score");
	int opponentPlayer = 1-player; //对手玩家

	//如果有五连直接返回胜利代表的值

	if(m_nRecordCount[player][FIVE] > 0)
		return 9999;//成功
	if(m_nRecordCount[opponentPlayer][FIVE] > 0)
		return -9999;//失败
	
	int Turn_Value =0,Opponent_Turn_Value = 0;

	/*计算并返回评价值*/
	//对手活四，必败
	if(m_nRecordCount[opponentPlayer][FOUR])
			return -9990;
	
	//对手冲四，必防
	if(m_nRecordCount[opponentPlayer][SFOUR])
			return -9980;

		//己手活四，必胜
	if(m_nRecordCount[player][FOUR])
			return  9970;//必胜

	
	if (m_nRecordCount[player][SFOUR] && m_nRecordCount[player][THREE])
	{
		return 9960;
	}

	if(m_nRecordCount[opponentPlayer][THREE] && m_nRecordCount[player][SFOUR] == 0)
	{
		return -9950;
	}

	if (m_nRecordCount[player][THREE] > 1 &&
		m_nRecordCount[opponentPlayer][SFOUR] ==0&&
		m_nRecordCount[opponentPlayer][THREE]==0&&
		m_nRecordCount[opponentPlayer][STHREE] == 0)
	{
		return 9940;
	}


	if(m_nRecordCount[player][SFOUR])
		Turn_Value += 300;


	if(m_nRecordCount[opponentPlayer][THREE] > 1 )
		Opponent_Turn_Value += 2000;
	else
	{
		if(m_nRecordCount[opponentPlayer][THREE])
		Opponent_Turn_Value += 200;
	}


		if (m_nRecordCount[player][THREE]>1)
		{
			Turn_Value += 500;
		}
		else
		{
			if(m_nRecordCount[player][THREE])
			Turn_Value += 100;
		}

	
		if(m_nRecordCount[player][STHREE])
			Turn_Value += m_nRecordCount[player][STHREE] * 10;
		if(m_nRecordCount[opponentPlayer][STHREE])
			Opponent_Turn_Value += m_nRecordCount[opponentPlayer][STHREE] * 10 ;
		if(m_nRecordCount[player][TWO])
			Turn_Value += m_nRecordCount[player][TWO] * 4;
		if(m_nRecordCount[opponentPlayer][TWO])
			Opponent_Turn_Value += m_nRecordCount[opponentPlayer][TWO] * 4;

		return Turn_Value - Opponent_Turn_Value;
}



//分析所选取部分行的棋型
void AnalysisBoardType(ChessAnalyzeData* data,int direction,int*** m_nRecord,int x,int y)
{
	//pipeOut("DEBUG analysis %d,%d,%d,%d,%d||%d,%d,%d,%d,%d",data->jumpjumpsamePre,data->jumpemptyPre,data->jumpsamePre,data->adjemptyPre,data->adjsamePre, data->adjsameNxt,data->adjemptyNxt, data->jumpsameNxt  , data->jumpemptyNxt ,data->jumpjumpsameNxt);
	int adjsame = data->adjsamePre + data->adjsameNxt -1;
	int leftEdge,rightEdge;
	int leftRange = data->adjsamePre  + data->adjemptyPre  + data->jumpsamePre + data->jumpemptyPre  + data->jumpjumpsamePre;
	int rightRange =  data->adjsameNxt+ data->adjemptyNxt+ data->jumpsameNxt  + data->jumpemptyNxt +data->jumpjumpsameNxt;
	//肯定不能构成五连的直接返回
	if (leftRange +rightRange -1 <5)
	{
		//设置已分析位置信息
	//	pipeOut("DEBUG analysisBoardType <5");
		SetAnalyzed(direction,m_nRecord,x,y,leftRange,rightRange);
		return ;
	}

	if(adjsame > 5)//长连
	{
		//设置已分析位置信息
		SetAnalyzed(direction,m_nRecord,x,y,data->adjsamePre,data->adjsameNxt);
		if ( !info_exact5)//如果允许多余五子相连
		{
		//	pipeOut("DEBUG type:five1");
			SetBoardType(direction,m_nRecord,x,y,FIVE);
		}
	}
	else if(adjsame == 5)//五连
	{
	//	pipeOut("DEBUG type:five2");
		SetAnalyzed(direction,m_nRecord,x,y,data->adjsamePre,data->adjsameNxt);
		SetBoardType(direction,m_nRecord,x,y,FIVE);
	}
	else if (adjsame == 4)//四连
	{
		leftEdge = data->adjsamePre;
		rightEdge = data->adjsameNxt;
		SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
		//?****?
		//冲四
		if ((data->adjemptyPre == 0 && data->adjemptyNxt >= 1) || (data->adjemptyNxt == 0 && data->adjemptyPre >= 1))//o****_ ,  _****o
		{	
		//	pipeOut("DEBUG type:SFOUR o****_ ,  _****o");
			SetBoardType(direction,m_nRecord,x,y,SFOUR);
		}
		//活四
		else if (data->adjemptyNxt >=1 && data->adjemptyPre >= 1)//_****_
		{  
		//	pipeOut("DEBUG type:FOUR _****_");
			SetBoardType(direction,m_nRecord,x,y,FOUR);
		}
	}
	else if (adjsame == 3)//三连
	{
		//冲四
		//@green因为leftEdge和rightEdge不一样，所以要分情况处理
		if (data->adjemptyPre == 1 && data->jumpsamePre >= 1) //*_***
		{
		//	pipeOut("DEBUG type:SFOUR *_***");
			leftEdge = data->adjsamePre + data->adjemptyPre +1;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,SFOUR);
		}
		if (data->adjemptyNxt == 1 && data->jumpsameNxt >= 1)//***_*
		{
		//	pipeOut("DEBUG type:SFOUR ***_*");
			leftEdge = data->adjsamePre ;
			rightEdge = data->adjsameNxt+ data->adjemptyNxt + 1;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,SFOUR);
		}

		//活三
		//if ((data->adjemptyPre > 2 || data->adjemptyPre == 2 && data->jumpsamePre == 0) && (data->adjemptyNxt > 1 || data->adjemptyNxt == 1 && data->jumpsameNxt == 0))
		//___***__ , ___***_o ,o__***__ , o__***_o
		if (data->adjemptyPre >=2 &&data->adjemptyNxt >=1)
		{ //__***_
		//	pipeOut("DEBUG type:THREE __***_");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,THREE);
		}
		//if ((data->adjemptyNxt > 2 || data->adjemptyNxt == 2 && data->jumpsameNxt == 0) && data->adjemptyPre >1 || data->adjemptyPre == 1 && data->jumpsamePre == 0)
		//__***___ , o_***___ , __***__o , o_***__o
		if(data->adjemptyNxt >= 2 && data->adjemptyPre >= 1)
		{   //_***__
			//pipeOut("DEBUG type:THREE _***__");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,THREE);
		}

		//眠三
		if (data->adjemptyPre == 0 && data->adjemptyNxt >=2 || data->adjemptyNxt == 0 && data->adjemptyPre >= 2 || data->adjemptyPre == 1 && data->jumpsamePre == 0 && data->adjemptyNxt == 1 && data->jumpsameNxt == 0)
		{  // __***o , o***__ , o_***_o
			//pipeOut("DEBUG type:STHREE __***o , o***__ , o_***_o");
			leftEdge =  data->adjsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetBoardType(direction,m_nRecord,x,y,STHREE);
		}
	}
	else if (adjsame == 2)//二连
	{

		bool left = false;
		bool right = false;
		//冲四
		if ( data->adjemptyPre == 1 && data->jumpsamePre >= 2)
		{ //左**_**
		//	pipeOut("DEBUG type:SFOUR left**_**");
			leftEdge = data->adjsamePre + data->adjemptyPre + 2;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,SFOUR);
			left = true;
		}
		if (data->adjemptyNxt == 1 && data->jumpsameNxt >= 2)
		{//右**_**
		//	pipeOut("DEBUG type:SFOUR right**_**");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + 2;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,SFOUR);
			right = true;
		}
		//活三
		if (  data->adjemptyPre == 1 && data->jumpsamePre == 1 &&data->jumpemptyPre >=1 && data->adjemptyNxt >= 1)
		{ //_*_**_
		//	pipeOut("DEBUG type:THREE _*_**_");
			leftEdge = data->adjsamePre + data->adjemptyPre + data->jumpsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,THREE);
			left = true;
		}
		if (data->adjemptyNxt ==1 && data->jumpsameNxt == 1 && data->jumpemptyNxt >= 1 && data->adjemptyPre >= 1)
		{//_**_*_
		//	pipeOut("DEBUG type:THREE _**_*_");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + data->jumpsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,THREE);
			right = true;
		}
		//眠三
		if (data->adjemptyPre == 1 && data->jumpsamePre == 1 && data->jumpemptyPre >=1 && data->adjemptyNxt == 0)
		{  //_*_**o
		//	pipeOut("DEBUG type:STHREE _*_**o");
			leftEdge = data->adjsamePre + data->adjemptyPre + data->jumpsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,STHREE);
			left = true;
		}
		if (data->adjemptyNxt == 1 && data->jumpsameNxt == 1 && data->jumpemptyNxt >= 1 && data->adjemptyPre == 0) 
		{  //o**_*_
		//	pipeOut("DEBUG type:STHREE o**_*_");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + data->jumpsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,STHREE);
			right = true;
		}

		//活二
		if (data->adjemptyPre!=0 && data->adjemptyNxt != 0 && data->adjemptyPre + data->adjemptyNxt >= 4)
		{//_**___ , __**__ , ___**_
		//	pipeOut("DEBUG type:TWO _**___ , __**__ , ___**_");
			if(left && data->adjemptyPre == 1)
			{}
			if(right && data->adjemptyNxt == 1)
			{}
			else{
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetBoardType(direction,m_nRecord,x,y,TWO);
			}
		}
		//眠二不加入判断，因为价值不大
	}
	else if (adjsame == 1) //单独一子
	{
		//冲四、活三、眠三都直接忽略，因为可以通过以上的其他情况加入检查

		//活二
		if (data->adjemptyPre ==1 &&data->jumpsamePre == 1 && data->jumpemptyPre != 0 && data->adjemptyNxt != 0 && data->jumpemptyPre + data->adjemptyNxt >= 3)
		{  //__*_*_ , _*_*__
		//	pipeOut("DEBUG type:TWO __*_*_ , _*_*__");
			leftEdge = data->adjsamePre + data->adjemptyPre + data->jumpsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetBoardType(direction,m_nRecord,x,y,TWO);
		}
		if (data->adjemptyNxt == 1 && data->jumpsameNxt == 1 && data->jumpemptyNxt != 0 && data->adjemptyPre != 0 && data->jumpemptyNxt + data->adjemptyPre >= 3)
		{	//__*_*_ , _*_*__
	//		pipeOut("DEBUG type:TWO __*_*_ , _*_*__");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + data->jumpsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			if (isRecordedTwo(direction,m_nRecord,x,y))
			{
				SetBoardType(direction,m_nRecord,x,y,TWO * 2);//由于只有一子如果记录两个方向的棋型不够，所以翻倍
			}
			SetBoardType(direction,m_nRecord,x,y,TWO);
		}
		if (data->adjemptyPre == 2 && data->jumpsamePre == 1 && data->jumpemptyPre > 0 && data->adjemptyNxt > 0)
		{	//_*__*_
	//		pipeOut("DEBUG type:TWO _*__*_");
			leftEdge = data->adjsamePre + data->adjemptyPre + data->jumpsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetBoardType(direction,m_nRecord,x,y,TWO);
		}
		if (data->adjemptyNxt == 2 && data->jumpsameNxt == 1 && data->jumpemptyNxt >0 && data->adjemptyPre > 0 )
		{	// _*__*_
	//		pipeOut("DEBUG type:TWO  _*__*_");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + data->jumpsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			if (isRecordedTwo(direction,m_nRecord,x,y))
			{
				SetBoardType(direction,m_nRecord,x,y,TWO * 2);//由于只有一子如果记录两个方向的棋型不够，所以翻倍
			}
			SetBoardType(direction,m_nRecord,x,y,TWO);
		}
	}


}

//设置已分析的棋位记录
void SetAnalyzed(int direction,int*** m_nRecord,int x,int y,int leftEdge,int rightEdge)
{
	m_nRecord[x][y][direction] = ANALYZED;
	switch (direction)
	{
	case UP_DOWN:
		for (int i = 1; i < leftEdge; i++)
		{
			m_nRecord[x][y-i][UP_DOWN] = ANALYZED;
		}
		for (int i = 1; i < rightEdge; i++)
		{
			m_nRecord[x][y+i][UP_DOWN] = ANALYZED;
		}
		break;
	case LEFTUP_RIGHTDOWN:
		for (int i = 1; i < leftEdge; i++)
		{
			m_nRecord[x-i][y-i][LEFTUP_RIGHTDOWN] = ANALYZED;
		}
		for (int i = 1; i < rightEdge; i++)
		{
			m_nRecord[x+i][y+i][LEFTUP_RIGHTDOWN] = ANALYZED;
		}
		break;
	case LEFT_RIGHT:
		for (int i = 1; i < leftEdge; i++)
		{
			m_nRecord[x-i][y][LEFT_RIGHT] = ANALYZED;
		}
		for (int i = 1; i < rightEdge; i++)
		{
			m_nRecord[x+i][y][LEFT_RIGHT] = ANALYZED;
		}
		break;
	case RIGHTUP_LEFTDOWN:
		for (int i = 1; i < leftEdge; i++)
		{
			m_nRecord[x-i][y+i][RIGHTUP_LEFTDOWN] = ANALYZED; 
		}
		for (int i = 1; i < rightEdge; i++)
		{
			m_nRecord[x+i][y-i][RIGHTUP_LEFTDOWN] = ANALYZED;
		}

		break;
	}
}

//在当前位置设置棋型
void SetBoardType(int direction,int*** m_nRecord,int x,int y,int type)
{
	m_nRecord[x][y][direction] = type;
}

//判断是否会成两个活二（这种情况只会出现在分析棋型时为单独一子的情况）
bool isRecordedTwo(int direction,int*** m_nRecord,int x,int y)
{
	if (m_nRecord[x][y][direction] == TWO)
	{
		return true;
	}
	return false;
}

//设置左向棋型
void SetLeftBoardType(int direction,int*** m_nRecord,int x,int y,int leftEdge,int type)
{
	switch (direction)
	{
		case UP_DOWN:
			m_nRecord[x][y-(leftEdge-1)][UP_DOWN] = type;
		break;
	case LEFTUP_RIGHTDOWN:
			m_nRecord[x-(leftEdge-1)][y-(leftEdge-1)][LEFTUP_RIGHTDOWN] = type;
		break;
	case LEFT_RIGHT:
		m_nRecord[x-(leftEdge-1)][y][LEFT_RIGHT] = type;
		break;
	case RIGHTUP_LEFTDOWN:
		m_nRecord[x-(leftEdge-1)][y+(leftEdge-1)][RIGHTUP_LEFTDOWN] = type; 
		break;
	}
}
//设置右向棋型
void SetRightBoardType(int direction,int*** m_nRecord,int x,int y,int rightEdge,int type)
{
	switch (direction)
	{
		case UP_DOWN:
			m_nRecord[x][y + (rightEdge-1)][UP_DOWN] = type;
		break;
	case LEFTUP_RIGHTDOWN:
			m_nRecord[x+(rightEdge-1)][y+(rightEdge-1)][LEFTUP_RIGHTDOWN] = type;
		break;
	case LEFT_RIGHT:
		m_nRecord[x+(rightEdge-1)][y][LEFT_RIGHT] = type;
		break;
	case RIGHTUP_LEFTDOWN:
		m_nRecord[x+(rightEdge-1)][y-(rightEdge-1)][RIGHTUP_LEFTDOWN] = type; 
		break;
	}
}



