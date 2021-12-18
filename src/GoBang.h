#pragma once
#include <array>
#include <graphics.h>

#define COLOR_BG RGB(240, 176, 96)
#define COLOR_GRID_LINE RGB(80, 80, 80)
#define COLOR_PIECE_WHITE RGB(243, 243, 243)
#define COLOR_PIECE_BLACK RGB(40, 40, 40)
#define NUM_PIECE 225
#define BTN_RADIUS 15
#define FONT_TEXT _T("宋体")
#define TEXT_FORMAT (DT_CENTER | DT_VCENTER | DT_SINGLELINE)
#define T_TEXT_TITLE _T("五子棋")
#define T_TEXT_BTN_START _T("开始游戏")
#define T_TEXT_BTN_RESTART _T("重置游戏")
#define T_TEXT_BTN_EXEC _T("退出游戏")
#define T_TEXT_COPYRIGHT _T("© 2021 Speauty, All rights reserved.")
#define TIME_DIFF_FREQUEBCY_REDUCTION 300 // ms
#define TIME_DIFF_PIECE_WAIT 120 // s

enum PieceFlag {
	PieceEmpty=0, PieceWhite=1, PieceBlack=-1
};

struct Axis {
	int x, y;
};

struct Piece 
{
	Axis Position;
	PieceFlag CurrentFlag;
	unsigned char IdxShowed;
};

struct State
{
	PieceFlag Winner; // 获胜者 白|黑
	PieceFlag IterationFlag ; // 当前落子方 默认黑子先行(初始化时, 指定)
	bool FlagIsPlaying; // 是否正在对战
	bool FlagIsRunning; // 是否正在运行
	bool FlagIsExeced; // 是否退出
	unsigned char CountPiecesShowed; // 总共落子数
	unsigned char CountPiecesShowedWhite; // 白方落子数
	time_t TimePlayed; // 开始对战时间
	time_t TimePieceWait; // 等待落子时间
	unsigned long TimeFrequencyReductionStart; // 降频-开始时间
	unsigned long TimeFrequencyReductionEnd; // 降频-结束时间
};

class GoBang{
private:
	const int WinSize; // 窗口大小
	const int LineCount; // 网格线数量(单向)
	const int RightBoardWidth;
	const int GridSize;
	const int PieceRadius;
	State State = {};
	std::array<Piece, NUM_PIECE> Pieces = {0};
private:
	void InitState();
	void InitPieces();
	void RenderPiece(int AxisX, int AxisY, PieceFlag Flag, short Radius) const;
	void RenderPiece(Piece piece) const;
	/* 独立渲染特殊点(天元&星) */
	void RenderPiece(unsigned short IdxPiece, short Radius, PieceFlag Flag) const;
	void WinnerChecker();
public:
	GoBang() : WinSize(630), LineCount(15), RightBoardWidth(200), GridSize(40), PieceRadius(15) {} /* 固定参数 */
	~GoBang() { closegraph(); }

	void GameInit();
	void GameRender();
	void GameUpdate();
	bool GameExec() const;
};