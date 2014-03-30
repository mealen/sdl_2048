#ifndef BOARD_H
#define BOARD_H

#include "TileData.h"

class Board
{
public:
	Board();
	~Board();
	TileData tiles[4][4];
	TileData oldNumbers[4][4]; // = {0};
	int moveDirection; //2 down, 8 up, 4 left, 6 right.
	int isGameOver;
	int score;
	int isMoved;
	bool quit;
	void fillTileMove();
	void logMatrixState();
	void logMoveData();
	int moveZerosInSingleArray(TileData [], int);
	int moveNumbers();
	int insertNumber();
	void setTilesForMove();
	int moveSingleArray(TileData [], int);
	int mergeSingleArray(TileData [], int);
	void endGame();
	int checkGameOver();
	int backupNumbersMatrix();
private:

};
#else
#endif