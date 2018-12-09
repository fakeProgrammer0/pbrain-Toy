#include "evaluation.h"
#include "historytable.h"

int FakeForbiddenCheck(int player, ChessAnalyzeData *checkData, Psquare p0);
int KeyPointFakeForbiddenCheck(int player, Psquare p0, int direction, int shift);

extern int width,height;
/*
*������س�ʼ��
*
*/
void evaluationInit(int width,int height){
	//�����¼
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

// @green �ٽ���
//�ؼ�����ּ��
int KeyPointFakeForbiddenCheck(int player, Psquare p0, int direction, int shift) {
	int result = NO_FORBIDDEN;
	ChessAnalyzeData checkData[4];

	int s = direction;
	Psquare p = p0;

	nxtP(p, shift);
	p->z = 1 + player; // �ȼ����и���ҵ�����

	result = FakeForbiddenCheck(player, checkData, p);
	p->z = 0;

	return result;
}

//�ٽ��ּ��
int FakeForbiddenCheck(int player, ChessAnalyzeData *checkData, Psquare p0)
{
	//��������ͳ��
	int CountThree = 0;
	int CountFour = 0;
	int adjsame[4];
	for (int i = 0; i<4; i++) {
		AnalysisLine(p0, diroff[i], &checkData[i]);
		adjsame[i] = checkData[i].adjsamePre + checkData[i].adjsameNxt - 1;
		if (adjsame[i] == 5)
		{
			return NO_FORBIDDEN; // @green �ڷ��������һ��ֱ������Ӯ�壬��ô���ֹ�����Ч���ڷ�ʤ��
		}
	}

	for (int i = 0; i < 4; i++)
	{

		int direction = diroff[i];
		//��ʼ���ַ���

		if (adjsame[i] > 5)//��������
			return LONGFBD;

		if (adjsame[i] == 4) //��������
		{
			//���ĳ����ж�
			bool isFour = false;
			if (checkData[i].adjemptyNxt > 0)
			{
				//�ݹ��жϹؼ����Ƿ����
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
		else if (adjsame[i] == 3)  //��������
		{
			//���ļ��
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

			//�������
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
		else if (adjsame[i] == 2) //��������
		{
			//�����ж�
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

			//�����ж�
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
		else if (adjsame[i] == 1) //����һ��
		{
			//�����ж�
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

			//�����ж�
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

		//�����ַ����������ʱ���ؽ�������ټ���
		if (CountFour > 1)//���Ľ���
		{
			return FOUR_FOUR_FBD;
		}
		if (CountThree > 1)//��������
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
//�ж�һ�����Ƿ���������ֹؼ��㣬������������߼�����������[�ǽ��ַ�]����ģ�����Ҫ˵�ɡ��ٽ��֡�
//��Ϊ���������ġ�������Щ�ٽ������ͣ�������Խ������һ����������ֵ���ܸ�
//û�취��Դ��������Ƶ���������̫���ˣ���Ȼ��Ҳ������������·
//bool causeBan(Psquare p0, int player)
//{
//	//�������ַ�
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

/* @green ��ȡԭ��ͳ�����͵Ĵ��� */
inline void countBoardTypes(int turn)
{
	int i, j, k;
	//��ʼ�� λ�÷�����¼��
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
	//��ʼ�� ������ ������¼��
	for (i = 0; i < 2; i++)
	{
		for (j = 0; j< CHESSTYPESCNT; j++) // @green TODO: 7��Ҫ���޸�
		{
			m_nRecordCount[i][j] = 0;
		}
	}

	//pipeOut("DEBUG before analyze");
	//����ͳ�ƺᡢ������б����б������
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
					ChessAnalyzeData* data = new ChessAnalyzeData();//��¼�������ͳ����Ϣ�Ľṹ

					AnalysisLine(p0, s, data);//ͳ��������Ϣ

					AnalysisBoardType(data, k, m_nRecord, i, j);//����ͳ�Ƶ���Ϣ�������������Ͳ�����λ�÷�����¼��
					delete[] data;
				}
			}
		}
	}

	//pipeOut("DEBUG before count");
	//ͳ�Ƹ����͵�����
	for (i = 0; i< width; i++)
	{
		for (j = 0; j<height; j++)
		{
			Psquare p0 = Square(i, j);

			if (p0->z != EMPTY_MOVE)
			{
				for (k = 0; k<4; k++)
				{
					int nPlayTurn = p0->z - 1; //��λ�ô�������
					switch (m_nRecord[i][j][k])
					{
					case FIVE://����
						m_nRecordCount[nPlayTurn][FIVE]++;
						break;
					case FOUR://����
						m_nRecordCount[nPlayTurn][FOUR]++;
						break;
					case SFOUR://����
						m_nRecordCount[nPlayTurn][SFOUR]++;
						break;
					case THREE://����
						m_nRecordCount[nPlayTurn][THREE]++;
						break;
					case STHREE://����
						m_nRecordCount[nPlayTurn][STHREE]++;
						break;
					case TWO://���
						m_nRecordCount[nPlayTurn][TWO]++;
						break;
					case TWO * 2://����������������ֻ������ڷ�������ʱΪ����һ�ӵ������
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
		if (m_nRecordCount[firstPlayer][FOUR]) // ��������ǽ��� firstPlayer
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
								ChessAnalyzeData* data = new ChessAnalyzeData();//��¼�������ͳ����Ϣ�Ľṹ

								AnalysisLine(p0, s, data);//ͳ��������Ϣ

														  // ���øú�������ԭ�������ͷ����ϵ��ӣ����ܻ��и�����
								AnalysisBoardType(data, k, m_nRecord, x, y);//����ͳ�Ƶ���Ϣ�������������Ͳ�����λ�÷�����¼��
								delete[] data;

								int nPlayTurn = p0->z - 1; //��λ�ô�������
								switch (m_nRecord[x][y][k])
								{
								case FIVE://����
										  //
									break;
								case FOUR://����
									CountFOUR++;
									break;
								case SFOUR://����
									CountSFOUR++;
									break;
								case THREE://����
									CountTHREE++;
									break;
								case STHREE://����
									CountSTHREE++;
									break;
								case TWO://���
										 //;
									break;
								case TWO * 2://����������������ֻ������ڷ�������ʱΪ����һ�ӵ������
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
		if (m_nRecordCount[firstPlayer][FOUR] || m_nRecordCount[firstPlayer][THREE]) // ֻ�к��������Ĳż��ٽ���
		{
			for (int x = 0; x < width; x++)
			{
				for (int y = 0; y < height; y++)
				{
					Psquare p0 = Square(x, y);
					if (p0->z == firstPlayer + 1) // ��firstPlayer������
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

							// �����ǽ���
							if (fake_fb_type == FOUR_THREE_NO_FBD)
							{
								int CountFOUR = 0, CountSFOUR = 0, CountTHREE = 0, CountSTHREE = 0;
								for (int k = 0; k < 4; k++)
								{
									int s = diroff[k];
									ChessAnalyzeData* data = new ChessAnalyzeData();//��¼�������ͳ����Ϣ�Ľṹ

									AnalysisLine(p0, s, data);//ͳ��������Ϣ

									// ���øú�������ԭ�������ͷ����ϵ��ӣ����ܻ��и�����
									AnalysisBoardType(data, k, m_nRecord, x, y);//����ͳ�Ƶ���Ϣ�������������Ͳ�����λ�÷�����¼��
									delete[] data;

									int nPlayTurn = p0->z - 1; //��λ�ô�������
									switch (m_nRecord[x][y][k])
									{
									case FIVE://����
											  //
										break;
									case FOUR://����
										CountFOUR++;
										break;
									case SFOUR://����
										CountSFOUR++;
										break;
									case THREE://����
										CountTHREE++;
										break;
									case STHREE://����
										CountSTHREE++;
										break;
									case TWO://���
											 //;
										break;
									case TWO * 2://����������������ֻ������ڷ�������ʱΪ����һ�ӵ������
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
	
		

	// TODO: �Լٽ��ֵļ�����Ż�
	const int secondPlayer = 1 - firstPlayer;
	//bool stopCheckFakeFBD = false;
	if (m_nRecordCount[secondPlayer][FOUR] || m_nRecordCount[secondPlayer][THREE]) // ֻ�к��������Ĳż��ٽ���
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				Psquare p0 = Square(x, y);
				if (p0->z == secondPlayer + 1) // ��secondPlayer������
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

						// �����ǽ���
						if (fake_fb_type == FOUR_THREE_NO_FBD)
						{
							int CountFOUR = 0, CountSFOUR = 0, CountTHREE = 0, CountSTHREE = 0;
							for (int k = 0; k < 4; k++)
							{
								int s = diroff[k];
								ChessAnalyzeData* data = new ChessAnalyzeData();//��¼�������ͳ����Ϣ�Ľṹ

								AnalysisLine(p0, s, data);//ͳ��������Ϣ

								// ���øú�������ԭ�������ͷ����ϵ��ӣ����ܻ��и�����
								AnalysisBoardType(data, k, m_nRecord, x, y);//����ͳ�Ƶ���Ϣ�������������Ͳ�����λ�÷�����¼��
								delete[] data;

								int nPlayTurn = p0->z - 1; //��λ�ô�������
								switch (m_nRecord[x][y][k])
								{
								case FIVE://����
										  //
									break;
								case FOUR://����
									CountFOUR++;
									break;
								case SFOUR://����
									CountSFOUR++;
									break;
								case THREE://����
									CountTHREE++;
									break;
								case STHREE://����
									CountSTHREE++;
									break;
								case TWO://���
										 //;
									break;
								case TWO * 2://����������������ֻ������ڷ�������ʱΪ����һ�ӵ������
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

					// �ٽ��ּ��̫��ʱ�䣬��ͣ���ԣ��ҵ���ͣ
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

				// �ٽ��ּ��̫��ʱ�䣬��ͣ���ԣ��ҵ���ͣ
				/*if (stopCheckFakeFBD)
				{
					break;
				}*/
			}
		}
	}

}

/* ������һ������һ������Ծ�������������޸�ԭ���� evaluate ���� */
int evaluate_turn(int player, int turn)
{
	//pipeOut("DEBUG evaluate");

	countBoardTypes(turn);

	//pipeOut("DEBUG before return score");
	int opponentPlayer = 1 - player; //�������
	int Turn_Value = 0, Opponent_Turn_Value = 0;

	bool isSecondPlayer = player != firstPlayer;

	if (turn == player) // ��������
	{
		/*
		�������� 9999
		�������� -9999
		�������ļٽ��� 9990
		�������Ļ���  9980
		������������ 9970
		�������� 9960
		�������Ļ��� 9950
		������������ 9940
		�������� 9930
		�Է����ļٽ��� -9990
		�Է����Ļ���  -9980
		�Է��������� -9970
		�Է����� -9960
		�������Ļ��� -9950
		������������ -9940
		�Է����� -9930 // �ط�
		���������ٽ��� 9920
		�������� 9910
		�Է������ٽ��� �Ҽ���û������ -9920
		�Է����� �Ҽ���û������ -9910 // �ط�������������Ļ������Բ�����ǿ�����Ĳ���
		*/


		//���������ֱ�ӷ���ʤ�������ֵ
		if (m_nRecordCount[player][FIVE] > 0)
			return 9999;//�ɹ�
		else if (m_nRecordCount[opponentPlayer][FIVE] > 0)
			return -9999;//ʧ��

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

		//���ֻ��ģ���ʤ 		
		if (m_nRecordCount[player][FOUR])
			return  9960;//��ʤ

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

		if (m_nRecordCount[player][SFOUR]) //�������ģ���ʤ
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

		//���ֻ��ģ��ذ�
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

		//���ֳ��ģ��ط�
		if (m_nRecordCount[opponentPlayer][SFOUR])
			return -9930;

		// ���������ٽ���
		if (isSecondPlayer || !info_fb_check)
			if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
				//pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
				//logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
				return 9920;
			}


		// ��������
		if (m_nRecordCount[player][THREE])
			return 9910;

		if (!isSecondPlayer || !info_fb_check) // �Է������ٽ���
			if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
				if (m_nRecordCount[player][STHREE] == 0) // ������û���������������ˣ���Ϊ�Ƿ���ס��
					return -9920;
				else Opponent_Turn_Value += 2000;

		// �Է��������ط��������û��[����]�Ļ�
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
	else // �ֵ��Է�����
	{
		/*
		�Է����� -9999
		�������� 9999
		�Է����ļٽ��� -9980
		�Է����� -9970
		�Է����� -9960
		�������ļٽ��� 9980
		�������� 9970
		�������� 9960 // �Է��ط�
		�Է������ٽ��� -9950
		�Է����� -9940
		���������ٽ��� �ҶԷ�û������ 9950
		�������� �ҶԷ�û������ 9940
		*/

		if (m_nRecordCount[opponentPlayer][FIVE] > 0)
			return -9999;//ʧ��
		if (m_nRecordCount[player][FIVE] > 0)
			return 9999;//�ɹ�

		if (!isSecondPlayer)
			if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
				return -9980;

		/*���㲢��������ֵ*/
		//���ֻ��� / ���ģ��ذ�
		if (m_nRecordCount[opponentPlayer][FOUR])
			return -9970;

		if (m_nRecordCount[opponentPlayer][SFOUR])
			return -9960;

		// �������ļٽ���
		if (isSecondPlayer)
			if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
				//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
				//logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
				return 9980;
			}

		//���ֻ��ģ���ʤ 		
		if (m_nRecordCount[player][FOUR])
			return  9970;//��ʤ
		if (m_nRecordCount[player][SFOUR]) //�������ģ��Է��ط�
			return 9960;

		if (!isSecondPlayer) // �Է������ٽ���
			if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
				return -9950;

		// �Է�����
		if (m_nRecordCount[opponentPlayer][THREE])
		{
			return -9940;
		}

		// ���������ٽ���
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

	//if (turn == player) // ��������
	//{
	//	/*
	//	�ֵ��Լ�����
	//	�������� 9999
	//	�������� -9999
	//	�������ļٽ��� 9980
	//	�������� 9970
	//	�������� 9960
	//	�Է����ļٽ��� -9980
	//	�Է����� -9970
	//	�Է����� -9960 // �ط�
	//	���������ٽ��� 9950
	//	�������� 9940
	//	�Է������ٽ��� -9950
	//	�Է����� �Ҽ���û������ -9940 // �ط�������������Ļ������Բ�����ǿ�����Ĳ���
	//	*/


	//	//���������ֱ�ӷ���ʤ�������ֵ
	//	if (m_nRecordCount[player][FIVE] > 0)
	//		return 9999;//�ɹ�
	//	else if (m_nRecordCount[opponentPlayer][FIVE] > 0)
	//		return -9999;//ʧ��
	//	
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
	//			pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			return 9980;
	//		}
	//			
	//	
	//	//���ֻ��ģ���ʤ 		
	//	if (m_nRecordCount[player][FOUR]) 
	//		return  9970;//��ʤ
	//	if (m_nRecordCount[player][SFOUR]) //�������ģ���ʤ
	//		return 9960;

	//	if (!isSecondPlayer)
	//		if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
	//			return -9980;

	//	//���ֻ��ģ��ذ�
	//	if (m_nRecordCount[opponentPlayer][FOUR])
	//		return -9970;

	//	//���ֳ��ģ��ط�
	//	if (m_nRecordCount[opponentPlayer][SFOUR])
	//		return -9960;

	//	// ���������ٽ���
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][THREE_THREE_FAKE_FBD]) {
	//			pipeOut("DEBUG Find my THREE_THREE_FAKE_FBD");
	//			logEvalBoard(player, "DEBUG Find my THREE_THREE_FAKE_FBD");
	//			return 9950;
	//		}
	//			

	//	// ��������
	//	if (m_nRecordCount[player][THREE])
	//		return 9940;

	//	if (!isSecondPlayer) // �Է������ٽ���
	//		if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
	//			if(m_nRecordCount[player][STHREE] == 0) // ������û���������������ˣ���Ϊ�Ƿ���ס��
	//				return -9950;
	//			else Opponent_Turn_Value += 2000;

	//	// �Է��������ط��������û��[����]�Ļ�
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
	//else // �ֵ��Է�����
	//{
	//	/*
	//	�Է����� -9999
	//	�������� 9999
	//	�Է����ļٽ��� -9980
	//	�Է����� -9970
	//	�Է����� -9960
	//	�������ļٽ��� 9980
	//	�������� 9970
	//	�������� 9960 // �Է��ط�
	//	�Է������ٽ��� -9950
	//	�Է����� -9940 
	//	���������ٽ��� �ҶԷ�û������ 9950
	//	�������� �ҶԷ�û������ 9940
	//	*/

	//	if (m_nRecordCount[opponentPlayer][FIVE] > 0)
	//		return -9999;//ʧ��
	//	if (m_nRecordCount[player][FIVE] > 0)
	//		return 9999;//�ɹ�

	//	if (!isSecondPlayer)
	//		if (m_nRecordCount[opponentPlayer][FOUR_FOUR_FAKE_FBD])
	//			return -9980;

	//	/*���㲢��������ֵ*/
	//	//���ֻ��� / ���ģ��ذ�
	//	if (m_nRecordCount[opponentPlayer][FOUR]) 
	//		return -9970;

	//	if (m_nRecordCount[opponentPlayer][SFOUR])
	//		return -9960;

	//	// �������ļٽ���
	//	if (isSecondPlayer)
	//		if (m_nRecordCount[player][FOUR_FOUR_FAKE_FBD]) {
	//			//pipeOut("DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			//logEvalBoard(player, "DEBUG Find my FOUR_FOUR_FAKE_FBD");
	//			return 9980;
	//		}

	//	//���ֻ��ģ���ʤ 		
	//	if (m_nRecordCount[player][FOUR])
	//		return  9970;//��ʤ
	//	if (m_nRecordCount[player][SFOUR]) //�������ģ��Է��ط�
	//		return 9960;

	//	if (!isSecondPlayer) // �Է������ٽ���
	//		if (m_nRecordCount[opponentPlayer][THREE_THREE_FAKE_FBD])
	//				return -9950;

	//	// �Է�����
	//	if (m_nRecordCount[opponentPlayer][THREE])
	//	{
	//		return -9940;
	//	}

	//	// ���������ٽ���
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


	// �Բ� �����д���
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
*���ۺ���
*/

int evaluate(int player)
{
	//pipeOut("DEBUG evaluate");

	int i,j,k;
	//��ʼ�� λ�÷�����¼��
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
	//��ʼ�� ������ ������¼��
	for(i = 0;i < 2;i++)
	{
		for(j = 0;j< 7;j++)
		{
			m_nRecordCount[i][j] = 0;
		}
	}

	//pipeOut("DEBUG before analyze");
	//����ͳ�ƺᡢ������б����б������
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
					ChessAnalyzeData* data = new ChessAnalyzeData();//��¼�������ͳ����Ϣ�Ľṹ
				
					AnalysisLine(p0,s,data);//ͳ��������Ϣ
			
					AnalysisBoardType(data,k,m_nRecord,i,j);//����ͳ�Ƶ���Ϣ�������������Ͳ�����λ�÷�����¼��
					delete[] data;
				}
			}
		}
	}

	//pipeOut("DEBUG before count");
	//ͳ�Ƹ����͵�����
	for(i = 0; i< width ;i++)
	{
		for(j = 0;j<height;j++)
		{
			Psquare p0 = Square(i,j);

			if(p0->z != EMPTY_MOVE)
			{
				for(k = 0;k<4;k++)
				{
					int nPlayTurn = p0->z -1; //��λ�ô�������
					switch (m_nRecord[i][j][k])
					{
					case FIVE://����
						m_nRecordCount[nPlayTurn][FIVE]++;
						break;
					case FOUR://����
						m_nRecordCount[nPlayTurn][FOUR]++;
						break;
					case SFOUR://����
						m_nRecordCount[nPlayTurn][SFOUR]++;
						break;
					case THREE://����
						m_nRecordCount[nPlayTurn][THREE]++;
						break;
					case STHREE://����
						m_nRecordCount[nPlayTurn][STHREE]++;
						break;
					case TWO://���
						m_nRecordCount[nPlayTurn][TWO]++;
						break;
					case TWO * 2://����������������ֻ������ڷ�������ʱΪ����һ�ӵ������
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
	int opponentPlayer = 1-player; //�������

	//���������ֱ�ӷ���ʤ�������ֵ

	if(m_nRecordCount[player][FIVE] > 0)
		return 9999;//�ɹ�
	if(m_nRecordCount[opponentPlayer][FIVE] > 0)
		return -9999;//ʧ��
	
	int Turn_Value =0,Opponent_Turn_Value = 0;

	/*���㲢��������ֵ*/
	//���ֻ��ģ��ذ�
	if(m_nRecordCount[opponentPlayer][FOUR])
			return -9990;
	
	//���ֳ��ģ��ط�
	if(m_nRecordCount[opponentPlayer][SFOUR])
			return -9980;

		//���ֻ��ģ���ʤ
	if(m_nRecordCount[player][FOUR])
			return  9970;//��ʤ

	
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



//������ѡȡ�����е�����
void AnalysisBoardType(ChessAnalyzeData* data,int direction,int*** m_nRecord,int x,int y)
{
	//pipeOut("DEBUG analysis %d,%d,%d,%d,%d||%d,%d,%d,%d,%d",data->jumpjumpsamePre,data->jumpemptyPre,data->jumpsamePre,data->adjemptyPre,data->adjsamePre, data->adjsameNxt,data->adjemptyNxt, data->jumpsameNxt  , data->jumpemptyNxt ,data->jumpjumpsameNxt);
	int adjsame = data->adjsamePre + data->adjsameNxt -1;
	int leftEdge,rightEdge;
	int leftRange = data->adjsamePre  + data->adjemptyPre  + data->jumpsamePre + data->jumpemptyPre  + data->jumpjumpsamePre;
	int rightRange =  data->adjsameNxt+ data->adjemptyNxt+ data->jumpsameNxt  + data->jumpemptyNxt +data->jumpjumpsameNxt;
	//�϶����ܹ���������ֱ�ӷ���
	if (leftRange +rightRange -1 <5)
	{
		//�����ѷ���λ����Ϣ
	//	pipeOut("DEBUG analysisBoardType <5");
		SetAnalyzed(direction,m_nRecord,x,y,leftRange,rightRange);
		return ;
	}

	if(adjsame > 5)//����
	{
		//�����ѷ���λ����Ϣ
		SetAnalyzed(direction,m_nRecord,x,y,data->adjsamePre,data->adjsameNxt);
		if ( !info_exact5)//������������������
		{
		//	pipeOut("DEBUG type:five1");
			SetBoardType(direction,m_nRecord,x,y,FIVE);
		}
	}
	else if(adjsame == 5)//����
	{
	//	pipeOut("DEBUG type:five2");
		SetAnalyzed(direction,m_nRecord,x,y,data->adjsamePre,data->adjsameNxt);
		SetBoardType(direction,m_nRecord,x,y,FIVE);
	}
	else if (adjsame == 4)//����
	{
		leftEdge = data->adjsamePre;
		rightEdge = data->adjsameNxt;
		SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
		//?****?
		//����
		if ((data->adjemptyPre == 0 && data->adjemptyNxt >= 1) || (data->adjemptyNxt == 0 && data->adjemptyPre >= 1))//o****_ ,  _****o
		{	
		//	pipeOut("DEBUG type:SFOUR o****_ ,  _****o");
			SetBoardType(direction,m_nRecord,x,y,SFOUR);
		}
		//����
		else if (data->adjemptyNxt >=1 && data->adjemptyPre >= 1)//_****_
		{  
		//	pipeOut("DEBUG type:FOUR _****_");
			SetBoardType(direction,m_nRecord,x,y,FOUR);
		}
	}
	else if (adjsame == 3)//����
	{
		//����
		//@green��ΪleftEdge��rightEdge��һ��������Ҫ���������
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

		//����
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

		//����
		if (data->adjemptyPre == 0 && data->adjemptyNxt >=2 || data->adjemptyNxt == 0 && data->adjemptyPre >= 2 || data->adjemptyPre == 1 && data->jumpsamePre == 0 && data->adjemptyNxt == 1 && data->jumpsameNxt == 0)
		{  // __***o , o***__ , o_***_o
			//pipeOut("DEBUG type:STHREE __***o , o***__ , o_***_o");
			leftEdge =  data->adjsamePre;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetBoardType(direction,m_nRecord,x,y,STHREE);
		}
	}
	else if (adjsame == 2)//����
	{

		bool left = false;
		bool right = false;
		//����
		if ( data->adjemptyPre == 1 && data->jumpsamePre >= 2)
		{ //��**_**
		//	pipeOut("DEBUG type:SFOUR left**_**");
			leftEdge = data->adjsamePre + data->adjemptyPre + 2;
			rightEdge = data->adjsameNxt;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetLeftBoardType(direction,m_nRecord,x,y,data->adjsamePre,SFOUR);
			left = true;
		}
		if (data->adjemptyNxt == 1 && data->jumpsameNxt >= 2)
		{//��**_**
		//	pipeOut("DEBUG type:SFOUR right**_**");
			leftEdge = data->adjsamePre;
			rightEdge = data->adjsameNxt + data->adjemptyNxt + 2;
			SetAnalyzed(direction,m_nRecord,x,y,leftEdge,rightEdge);
			SetRightBoardType(direction,m_nRecord,x,y,data->adjsameNxt,SFOUR);
			right = true;
		}
		//����
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
		//����
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

		//���
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
		//�߶��������жϣ���Ϊ��ֵ����
	}
	else if (adjsame == 1) //����һ��
	{
		//���ġ�������������ֱ�Ӻ��ԣ���Ϊ����ͨ�����ϵ��������������

		//���
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
				SetBoardType(direction,m_nRecord,x,y,TWO * 2);//����ֻ��һ�������¼������������Ͳ��������Է���
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
				SetBoardType(direction,m_nRecord,x,y,TWO * 2);//����ֻ��һ�������¼������������Ͳ��������Է���
			}
			SetBoardType(direction,m_nRecord,x,y,TWO);
		}
	}


}

//�����ѷ�������λ��¼
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

//�ڵ�ǰλ����������
void SetBoardType(int direction,int*** m_nRecord,int x,int y,int type)
{
	m_nRecord[x][y][direction] = type;
}

//�ж��Ƿ�������������������ֻ������ڷ�������ʱΪ����һ�ӵ������
bool isRecordedTwo(int direction,int*** m_nRecord,int x,int y)
{
	if (m_nRecord[x][y][direction] == TWO)
	{
		return true;
	}
	return false;
}

//������������
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
//������������
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



