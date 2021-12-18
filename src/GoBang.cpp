#include <iostream>
#include <cmath>
#include <array>
#include <time.h>
#include <graphics.h>
#include "GoBang.h"

void GoBang::RenderPiece(int AxisX, int AxisY, PieceFlag Flag, short Radius) const
{
	switch (Flag)
	{
	case PieceWhite:
		setlinecolor(COLOR_PIECE_WHITE);
		setfillcolor(COLOR_PIECE_WHITE);
		fillcircle(AxisX, AxisY, Radius?Radius:this->PieceRadius);
		break;
	case PieceBlack:
		setlinecolor(COLOR_PIECE_BLACK);
		setfillcolor(COLOR_PIECE_BLACK);
		fillcircle(AxisX, AxisY, Radius ? Radius : this->PieceRadius);
		break;
	case PieceEmpty:
	default:
		break;
	}
}

void GoBang::RenderPiece(Piece piece) const
{
	this->RenderPiece(piece.Position.x, piece.Position.y, piece.CurrentFlag, 0);
	if (piece.CurrentFlag != PieceEmpty && this->State.CountPiecesShowed && piece.IdxShowed) {
		TCHAR idxPiece[4];
		swprintf_s(idxPiece, _T("%d"), piece.IdxShowed);
		settextstyle(14, 10, FONT_TEXT);
		RECT tmpRect;
		tmpRect = { piece.Position.x - this->PieceRadius, piece.Position.y - this->PieceRadius, piece.Position.x + this->PieceRadius, piece.Position.y + this->PieceRadius };
		setbkmode(TRANSPARENT);
		settextcolor((piece.CurrentFlag < PieceEmpty) ? WHITE : BLACK);
		drawtext(idxPiece, &tmpRect, TEXT_FORMAT);
		setbkmode(OPAQUE);
	}
}

void GoBang::RenderPiece(unsigned short IdxPiece, short Radius, PieceFlag Flag) const
{
	if (IdxPiece >= 0 && IdxPiece < NUM_PIECE) // 还是判断一下索引有效性
		this->RenderPiece(this->Pieces[IdxPiece].Position.x, this->Pieces[IdxPiece].Position.y, Flag, Radius);
}

void GoBang::InitState()
{
	this->State.Winner = PieceEmpty;
	this->State.IterationFlag = PieceBlack;
	this->State.FlagIsPlaying = 0;
	this->State.FlagIsRunning = 1;
	this->State.FlagIsExeced = 0;
	this->State.CountPiecesShowed = 0;
	this->State.CountPiecesShowedWhite = 0;
	this->State.TimePlayed = NULL;
	this->State.TimePieceWait = NULL;
	this->State.TimeFrequencyReductionStart = GetTickCount();
	this->State.TimeFrequencyReductionEnd = NULL;
}

void GoBang::InitPieces()
{
	for (char y = 0; y < this->LineCount; y++)
	{
		for (char x = 0; x < this->LineCount; x++)
		{
			this->Pieces[y * this->LineCount + x] = { { (x + 1) * this->GridSize, (y + 1) * this->GridSize},PieceEmpty, 0};
		}
	}
}

void GoBang::WinnerChecker()
{
	if (this->State.Winner != PieceEmpty || !this->State.FlagIsPlaying || this->State.FlagIsExeced) return;

	/* 横竖斜 五子同色，判赢 */
	// 横
	PieceFlag currentRowPieceFlag = PieceEmpty;
	unsigned char currentRowPieceFlagFreq = 0;
	// 列
	PieceFlag currentColPieceFlag = PieceEmpty;
	unsigned char currentColPieceFlagFreq = 0;

	std::array<PieceFlag, 30> currentForwardObliqueFlag = {}; // 正斜
	std::array<unsigned char, 30> currentForwardObliqueFreq = {};
	std::array<PieceFlag, 30> currentReverseObliqueFlag = {}; // 反斜
	std::array<unsigned char, 30> currentReverseObliqueFreq = {};

	for (char y = 0; y < this->LineCount; y++)
	{
		for (char x = 0; x < this->LineCount; x++)
		{
			/* 初始化 */
			if (x == 0 && y == 0) {
				currentRowPieceFlag = this->Pieces[y * this->LineCount + x].CurrentFlag;
				currentColPieceFlag = this->Pieces[y * this->LineCount + x].CurrentFlag;
				currentForwardObliqueFlag[0] = this->Pieces[y * this->LineCount + x].CurrentFlag;
				currentReverseObliqueFlag[0] = this->Pieces[y * this->LineCount + x].CurrentFlag;
				continue;
			}

			/* 横向迭代 */
			if (currentRowPieceFlag == this->Pieces[y * this->LineCount + x].CurrentFlag) {
				currentRowPieceFlagFreq++;
			} else {
				currentRowPieceFlag = this->Pieces[y * this->LineCount + x].CurrentFlag;
				currentRowPieceFlagFreq = 1;
			}

			/* 纵向迭代 */
			if (currentColPieceFlag == this->Pieces[x * this->LineCount + y].CurrentFlag) {
				currentColPieceFlagFreq++;
			} else {
				currentColPieceFlag = this->Pieces[x * this->LineCount + y].CurrentFlag;
				currentColPieceFlagFreq = 1;
			}

			{  
				// 正斜
				unsigned char diff = abs(y-x) + (y < x?this->LineCount:0);
				if (currentForwardObliqueFreq[diff] == 0) currentForwardObliqueFlag[diff] = this->Pieces[y * this->LineCount + x].CurrentFlag;
				if (currentForwardObliqueFlag[diff] == this->Pieces[y * this->LineCount + x].CurrentFlag) {
					currentForwardObliqueFreq[diff]++;
				} else {
					currentForwardObliqueFlag[diff] = this->Pieces[y * this->LineCount + x].CurrentFlag;
					currentForwardObliqueFreq[diff] = 1;
				}

				// 反斜
				diff = x + y;
				if (currentReverseObliqueFreq[diff] == this->LineCount) currentReverseObliqueFlag[diff] = this->Pieces[y * this->LineCount + x].CurrentFlag;
				if (currentReverseObliqueFlag[diff] == this->Pieces[y * this->LineCount + x].CurrentFlag) {
					currentReverseObliqueFreq[diff]++;
				} else {
					currentReverseObliqueFlag[diff] = this->Pieces[y * this->LineCount + x].CurrentFlag;
					currentReverseObliqueFreq[diff] = 1;
				}
			}

			/* 判断 */
			if (
				(currentRowPieceFlagFreq == 5 && currentRowPieceFlag != PieceEmpty) ||
				(currentColPieceFlagFreq == 5 && currentColPieceFlag != PieceEmpty)
			) {
				this->State.Winner = this->State.IterationFlag == PieceBlack ? PieceWhite : PieceBlack;
				return;
			}

			for (unsigned char i = 0; i < this->LineCount; i++)
			{
				if (
					(currentForwardObliqueFreq[i] == 5 && currentForwardObliqueFlag[i] != PieceEmpty) ||
					(currentReverseObliqueFreq[i] == 5 && currentReverseObliqueFlag[i] != PieceEmpty)
					) {
					this->State.Winner = this->State.IterationFlag == PieceBlack ? PieceWhite : PieceBlack;
					return;
				}
			}

			for (unsigned char i = this->LineCount; i < this->LineCount*2; i++)
			{
				if (
					(currentForwardObliqueFreq[i] == 5 && currentForwardObliqueFlag[i] != PieceEmpty) ||
					(currentReverseObliqueFreq[i] == 5 && currentReverseObliqueFlag[i] != PieceEmpty)
					) {
					this->State.Winner = this->State.IterationFlag == PieceBlack ? PieceWhite : PieceBlack;
					return;
				}
			}

		}
	}
}


void GoBang::GameInit()
{
	this->InitState(); // 初始化状态集
	this->InitPieces(); // 初始化棋子

	/* 初始化窗口(强行补充右边面板宽度) */
	initgraph(this->WinSize + this->RightBoardWidth, this->WinSize);
	// 设置背景色为蓝色 & 背景色清空屏幕
	setbkcolor(COLOR_BG);
	cleardevice();

	/* 初始化棋盘网格线  */
	setlinecolor(COLOR_GRID_LINE);
	/* 计算四个顶点 */
	Axis PositionInitTopLeft = { this->GridSize, this->GridSize };
	Axis PositionInitTopRight = { this->GridSize * this->LineCount, this->GridSize };
	Axis PositionInitBottomLeft = { this->GridSize, this->GridSize * this->LineCount };
	Axis PositionInitBottomRight = { this->GridSize * this->LineCount, this->GridSize * this->LineCount };
	settextcolor(BLACK);
	for (char i = 0; i < this->LineCount; i++)
	{
		TCHAR s[3];
		swprintf_s(s, _T("%d"), i + 1);
		/* 渲染索引 x & y */
		outtextxy(PositionInitTopLeft.x - (this->PieceRadius << 1), PositionInitTopLeft.y + i * this->GridSize - (this->PieceRadius >> 1), s);
		outtextxy(PositionInitBottomLeft.x + i * this->GridSize - (this->PieceRadius >> 1), PositionInitTopLeft.y - (this->PieceRadius << 1), s);

		/* 渲染横线 */
		line(PositionInitTopLeft.x, PositionInitTopLeft.y + i * this->GridSize, PositionInitTopRight.x, PositionInitTopRight.y + i * this->GridSize);
		/* 渲染纵线 */
		line(PositionInitBottomLeft.x + i * this->GridSize, PositionInitTopLeft.y, PositionInitBottomLeft.x + i * this->GridSize, PositionInitBottomLeft.y);
	}

}

void GoBang::GameRender()
{
	BeginBatchDraw();
	settextcolor(BLACK);
	/* 天元 */
	short IdxCenter = (this->LineCount >> 1) * this->LineCount + (this->LineCount >> 1);
	this->RenderPiece(IdxCenter, this->PieceRadius >> 2, PieceBlack);
	/* 四星(四角 4*4 位置) */
	unsigned short IdxStartTopLeft = (this->LineCount) * 3 + 3;
	unsigned short IdxStartTopRight = (this->LineCount) * 4 - 4;
	unsigned short IdxStartBottomLeft = (this->LineCount - 4) * this->LineCount + 3;
	unsigned short IdxStartBottomRight = (this->LineCount - 3) * this->LineCount - 4;
	this->RenderPiece(IdxStartTopLeft, this->PieceRadius >> 2, PieceBlack);
	this->RenderPiece(IdxStartTopRight, this->PieceRadius >> 2, PieceBlack);
	this->RenderPiece(IdxStartBottomLeft, this->PieceRadius >> 2, PieceBlack);
	this->RenderPiece(IdxStartBottomRight, this->PieceRadius >> 2, PieceBlack);

	RECT tmpRect;
	/* 椭圆ellipse 左上角 右下角 */
	settextstyle(40, 20, FONT_TEXT);
	tmpRect = { this->WinSize + this->PieceRadius, this->GridSize, this->WinSize + (this->GridSize << 2), (this->GridSize << 1) + this->PieceRadius };
	drawtext(T_TEXT_TITLE, &tmpRect, TEXT_FORMAT);

	settextstyle(14, 10, FONT_TEXT);
	tmpRect = { this->WinSize + this->PieceRadius, (this->GridSize << 1) + (this->PieceRadius << 1), this->WinSize + (this->PieceRadius << 2), (this->GridSize << 1) + (this->PieceRadius << 2) };
	drawtext(_T("黑方:"), &tmpRect, TEXT_FORMAT);
	TCHAR tmpNum[4];
	unsigned short space = this->PieceRadius * 2 + (this->PieceRadius >> 1);
	if (this->State.CountPiecesShowed) {
		swprintf_s(tmpNum, _T("%d"), this->State.CountPiecesShowed - this->State.CountPiecesShowedWhite);
		tmpRect = { this->WinSize + this->PieceRadius + space, (this->GridSize << 1) + (this->PieceRadius << 1), this->WinSize + (this->PieceRadius << 2) + space, (this->GridSize << 1) + (this->PieceRadius << 2) };
		drawtext(tmpNum, &tmpRect, TEXT_FORMAT);
	}
	setcolor(WHITE);
	tmpRect = { this->WinSize + this->PieceRadius, (this->GridSize << 1) + (this->PieceRadius << 2), this->WinSize + (this->PieceRadius << 2), (this->GridSize << 1) + 5 * this->PieceRadius };
	drawtext(_T("白方:"), &tmpRect, TEXT_FORMAT);
	if (this->State.CountPiecesShowed) {
		swprintf_s(tmpNum, _T("%d"), this->State.CountPiecesShowedWhite);
		tmpRect = { this->WinSize + this->PieceRadius + space, (this->GridSize << 1) + (this->PieceRadius << 2), this->WinSize + (this->PieceRadius << 2) + space, (this->GridSize << 1) + 5 * this->PieceRadius};
		drawtext(tmpNum, &tmpRect, TEXT_FORMAT);
	}
	setcolor(BLACK);

	/* 当前游戏状态 */
	settextstyle(20, 8, FONT_TEXT);
	tmpRect = { this->WinSize, this->GridSize * (this->LineCount - 10), this->WinSize + this->RightBoardWidth, this->GridSize * (this->LineCount - 8) };
	TCHAR tmpText[50];
	if (!this->State.FlagIsPlaying) {
		swprintf_s(tmpText, _T("游戏尚未开始"));
	} else {
		if (this->State.Winner != PieceEmpty) {
			swprintf_s(tmpText, _T("恭喜%s获得胜利"), this->State.Winner == PieceWhite?_T("白方") : _T("黑方"));
		} else {
			swprintf_s(tmpText, _T("等待%s落子%3ds"), this->State.IterationFlag == PieceWhite ? _T("白方") : _T("黑方"), TIME_DIFF_PIECE_WAIT - (GetTickCount() - this->State.TimePieceWait)/1000);
		}
	}
	
	drawtext(tmpText, &tmpRect, TEXT_FORMAT);

	/* 按钮(开始|重置 退出) */
	settextstyle(18, 15, FONT_TEXT);
	roundrect(this->WinSize + this->PieceRadius, this->GridSize * (this->LineCount - 5), this->WinSize + (this->GridSize << 2), this->GridSize * (this->LineCount - 4), BTN_RADIUS, BTN_RADIUS);
	tmpRect = { this->WinSize + this->PieceRadius, this->GridSize * (this->LineCount - 5) , this->WinSize + (this->GridSize << 2), this->GridSize * (this->LineCount - 4) };
	drawtext(this->State.FlagIsPlaying? T_TEXT_BTN_RESTART : T_TEXT_BTN_START, &tmpRect, TEXT_FORMAT);

	roundrect(this->WinSize + this->PieceRadius, this->GridSize * (this->LineCount - 4) + (this->GridSize >> 1), this->WinSize + (this->GridSize << 2), this->GridSize * (this->LineCount - 2) - (this->GridSize >> 1), BTN_RADIUS, BTN_RADIUS);
	tmpRect = { this->WinSize + this->PieceRadius, this->GridSize * (this->LineCount - 4) + (this->GridSize >> 1), this->WinSize + (this->GridSize << 2), this->GridSize * (this->LineCount - 2) - (this->GridSize >> 1) };
	drawtext(T_TEXT_BTN_EXEC, &tmpRect, TEXT_FORMAT);

	/* 版权 */
	settextstyle(10, 5, FONT_TEXT);
	tmpRect = { this->WinSize, this->GridSize * this->LineCount, this->WinSize + this->RightBoardWidth, this->WinSize };
	drawtext(T_TEXT_COPYRIGHT, &tmpRect, TEXT_FORMAT);

	for (unsigned char idx = 0; idx < NUM_PIECE; idx++)
	{
		if (this->Pieces[idx].CurrentFlag != PieceEmpty) {
			this->RenderPiece(this->Pieces[idx]);
		}
	}
	
	EndBatchDraw();
}

void GoBang::GameUpdate()
{
	ExMessage msg = { 0 };
	if (peekmessage(&msg, EM_MOUSE)) {
		
		if (msg.message == WM_LBUTTONDOWN) {
			/* 在棋盘区域 */
			if (
				this->State.FlagIsPlaying && this->State.Winner == PieceEmpty && 
				msg.x >= this->GridSize && msg.x <= (this->GridSize * this->LineCount) &&
				msg.y >= this->GridSize && msg.y <= (this->GridSize * this->LineCount)
			) {
				for (unsigned char idx = 0; idx < NUM_PIECE; idx++)
				{
					if (this->Pieces[idx].CurrentFlag != PieceEmpty) continue;
					if (
						msg.x >= (this->Pieces[idx].Position.x - this->PieceRadius) && msg.x <= (this->Pieces[idx].Position.x + this->PieceRadius) &&
						msg.y >= (this->Pieces[idx].Position.y - this->PieceRadius) && msg.y <= (this->Pieces[idx].Position.y + this->PieceRadius)
					) {
						this->Pieces[idx].CurrentFlag = this->State.IterationFlag;
						/* 黑白子交换 */
						this->State.IterationFlag = this->State.IterationFlag > 0 ? PieceBlack : PieceWhite;
						/* 更新落子数 */
						this->State.CountPiecesShowed++;
						if (this->Pieces[idx].CurrentFlag == PieceWhite)this->State.CountPiecesShowedWhite++;
						/* 更新棋子关联出现索引 */
						this->Pieces[idx].IdxShowed = this->State.CountPiecesShowed;
						/* 更新等待时间 */
						this->State.TimePieceWait = GetTickCount();
					}
				}
			} else if ( /* 在开始游戏按钮区域 */
				msg.x >= (this->WinSize + this->PieceRadius) && msg.x <= (this->WinSize + (this->GridSize << 2)) &&
				msg.y >= (this->GridSize * (this->LineCount - 5)) && msg.y <= this->GridSize * (this->LineCount - 4)
			) {
				if (this->State.FlagIsPlaying) {
					this->GameInit();
				} else {
					this->State.FlagIsPlaying = 1;
					this->State.TimePieceWait = GetTickCount();
					this->State.TimePlayed = GetTickCount();
					this->InitPieces();
				}
			} else if ( /* 在退出游戏按钮区域 */
				msg.x >= (this->WinSize + this->PieceRadius) && msg.x <= (this->WinSize + (this->GridSize << 2)) &&
				msg.y >= (this->GridSize * (this->LineCount - 4) + (this->GridSize >> 1)) && msg.y <= (this->GridSize * (this->LineCount - 2) - (this->GridSize >> 1))
			) {
				this->State.FlagIsExeced = 1;
			}
		}
	}

	this->State.TimeFrequencyReductionEnd = GetTickCount();
	if ((this->State.TimeFrequencyReductionEnd - this->State.TimeFrequencyReductionStart) > TIME_DIFF_FREQUEBCY_REDUCTION) {
		if (this->State.Winner == PieceEmpty && this->State.TimePieceWait && (TIME_DIFF_PIECE_WAIT - (GetTickCount() - this->State.TimePieceWait) / 1000) <= 0) {
			this->State.Winner = this->State.IterationFlag == PieceBlack ? PieceWhite: PieceBlack;
		}
	}

	this->WinnerChecker();
}

bool GoBang::GameExec() const
{
	return this->State.FlagIsExeced;
}