#include <iostream>
#include <string>
#include <cassert>
#include <ctime>

#include "Board.h"

Board::Board(){
	srand(time(NULL));
	//memset(tiles, 0, 16 * sizeof(TileData));
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			tiles[i][j] = *(new TileData());
		}
	}

	int values[4] = { rand() % 4, rand() % 4, rand() % 4, rand() % 4 };
	tiles[values[0]][values[1]].newValue = 2;
	tiles[values[0]][values[1]].startValue = 2;

	tiles[values[2]][values[3]].newValue += 2;
	tiles[values[2]][values[3]].startValue += 2;


	isGameOver = 0;
	moveDirection = 0;
	score = 0;
	isMoved = 0;
	quit = false;


	
}

void logMessage(const std::string &msg) {
	std::cout << msg << std::endl;
}

void Board::logMatrixState() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << tiles[i][j].newValue << " ";
		}
		std::cout << std::endl;
	}
}

void Board::logMoveData() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << "[" << tiles[i][j].moveX << ","
				<< tiles[i][j].moveY << "]";
			std::cout << "[" << tiles[i][j].startValue << "] ";
			//std::cout << "[" << tileData[j][i].startX << ","
			//		<< tileData[j][i].startY << "," << tileData[j][i].totalMove
			//		<< "] ";
		}
		std::cout << std::endl;
	}
}


void Board::fillTileMove() {
	logMessage("filling  move");
	int baseX = 0;
	int baseY = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; ++j) {
			switch (moveDirection) {
			case 6:
				baseX = 1;
				break;
			case 8:
				baseY = -1;
				break;
			case 2:
				baseY = 1;
				break;
			case 4:
				baseX = -1;
				break;
			default:
				break;
			}

			tiles[tiles[i][j].startX][tiles[i][j].startY].moveX =
				tiles[i][j].totalMove * baseX;
			tiles[tiles[i][j].startX][tiles[i][j].startY].moveY =
				tiles[i][j].totalMove * baseY;
			//tiles[i][j].moveX = i -tiles[i][j].startX;
			//tiles[i][j].moveY = j -tiles[i][j].startY;

		}
	}
}

//cleaned of zeros.
int Board::moveZerosInSingleArray(TileData tiles[], int numberCount) {
	int returnValue = 0;
	for (int i = numberCount - 1; i > 0; i--) {
		if (tiles[i].newValue == 0) {
			for (int j = i - 1; j >= 0; j--) {
				if (tiles[j].newValue != 0) {
					TileData temp = tiles[i];
					//the start values should not interchange

					tiles[i] = tiles[j];
					tiles[i].startValue = temp.startValue;
					temp.startValue = tiles[j].startValue;
					tiles[j] = temp;
					tiles[i].totalMove += i - j;

					returnValue = 1;
					break;
				}
			}
		}
	}
	return returnValue;
}

int Board::moveNumbers() {
	int returnValue = 0;
	logMessage("before");
	logMatrixState();
	setTilesForMove();
	switch (moveDirection) {
	case 6:
		logMessage("moving down ");
		for (int i = 0; i < 4; i++) {
			TileData column[4] = { tiles[i][0], tiles[i][1],
				tiles[i][2], tiles[i][3] };
			if (moveSingleArray(column, 4) == 1)
				returnValue = 1;
			tiles[i][0] = column[0];
			tiles[i][1] = column[1];
			tiles[i][2] = column[2];
			tiles[i][3] = column[3];

		}
		break;
	case 8:
		logMessage("moving left ");
		for (int i = 0; i < 4; i++) {
			TileData row[4] = { tiles[3][i], tiles[2][i], tiles[1][i],
				tiles[0][i] };
			if (moveSingleArray(row, 4) == 1)
				returnValue = 1;
			tiles[3][i] = row[0];
			tiles[2][i] = row[1];
			tiles[1][i] = row[2];
			tiles[0][i] = row[3];

		}
		break;
	case 2:
		logMessage("moving right ");
		for (int i = 0; i < 4; i++) {
			TileData row[4] = { tiles[0][i], tiles[1][i], tiles[2][i],
				tiles[3][i] };
			if (moveSingleArray(row, 4) == 1)
				returnValue = 1;
			tiles[0][i] = row[0];
			tiles[1][i] = row[1];
			tiles[2][i] = row[2];
			tiles[3][i] = row[3];

		}
		break;
	case 4:
		logMessage("moving up ");
		for (int i = 0; i < 4; i++) {
			TileData column[4] = { tiles[i][3], tiles[i][2],
				tiles[i][1], tiles[i][0] };
			if (moveSingleArray(column, 4) == 1)
				returnValue = 1;
			tiles[i][3] = column[0];
			tiles[i][2] = column[1];
			tiles[i][1] = column[2];
			tiles[i][0] = column[3];

		}
		break;
	}
	logMessage("after");
	logMatrixState();
	fillTileMove();
	logMessage("change");
	logMoveData();
	return returnValue;
}

int Board::insertNumber() {
	logMessage("insert call");
	int newNumber;
	if (rand() % 5 == 4) // %20 chance
		newNumber = 4;
	else
		newNumber = 2;

	int emptyCellCount = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (tiles[i][j].newValue == 0)
				emptyCellCount++;
		}
	}
	if (emptyCellCount == 0)
		return 0;

	int insertCell = rand() % emptyCellCount;

	emptyCellCount = 0; //now find the element and set
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (tiles[i][j].newValue == 0) {
				if (emptyCellCount == insertCell) {
					tiles[i][j].newValue = newNumber;
					tiles[i][j].startValue = -1 * tiles[i][j].startValue - 1; //for new entry it is negated, and +1 is incase of 0
					logMoveData();
					return 1;
				}
				else {
					emptyCellCount++;
				}
			}
		}
	}
	//this should newer happen, but gcc complains
	assert(0);
	return 0;
}

void Board::setTilesForMove() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; ++j) {
			tiles[i][j].startX = i;
			tiles[i][j].startY = j;
			tiles[i][j].startValue = tiles[i][j].newValue;
			tiles[i][j].totalMove = 0;
			tiles[i][j].moveX = 0;
			tiles[i][j].moveY = 0;
		}
	}
}


/**
* moves the zeros to the end, then merge part is called.
* moves zeros after that again.
*/
int Board::moveSingleArray(TileData tiles[], int numberCount) {
	int returnValue = 0;
	returnValue = moveZerosInSingleArray(tiles, numberCount);
	returnValue += mergeSingleArray(tiles, numberCount);
	returnValue += moveZerosInSingleArray(tiles, numberCount);
	return (returnValue > 0);
}


/**
* returns 1 if there were a change, returns 0 if no change
*
*/
int Board::mergeSingleArray(TileData tiles[], int numberCount) {
	int internalReturn = 0;
	if (numberCount == 1)
		return 0;
	internalReturn = mergeSingleArray(tiles + 1, numberCount - 1); //this ensures after current, it is ordered(all zero possible)
	if (tiles[0].newValue == 0)
		return internalReturn;
	if (tiles[0].newValue == tiles[1].newValue) {
		//the switching is because we need a move data.
		//TileData temp = tiles[0];
		//tiles[0] = tiles[1];
		//tiles[1] = temp;

		//now do the merge
		tiles[1].newValue = tiles[1].newValue * 2;
		tiles[1].mergeStatus = mergeOn;
		tiles[0].mergeStatus = mergeOff;
		tiles[0].newValue = 0;
		tiles[0].totalMove++;
		score = score + tiles[1].newValue;

		return 1;
	}
	return internalReturn;
}



void Board::endGame() {

}
/**
* it checks if the game is over.
*
*/
int Board::checkGameOver() {
	//TODO optimise this part

	//if there are 2 same adjent cell, it is not over. This for
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (tiles[i][j].newValue == tiles[i][j + 1].newValue
				|| tiles[i][j].newValue == tiles[i + 1][j].newValue
				|| tiles[i][j].newValue == 0) {
				return 0;
			}
		}
		if (tiles[i][3].newValue == tiles[i + 1][3].newValue
			|| tiles[i][3].newValue == 0) // this is the last coloumn check.
			return 0;
	}
	for (int j = 0; j < 3; j++) { //this is the last row check.
		if (tiles[3][j].newValue == tiles[3][j + 1].newValue
			|| tiles[3][j].newValue == 0)
			return 0;
	}
	return 1;

}

/**
* This function overwrites old matrix.
*
*/
int Board::backupNumbersMatrix() {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			oldNumbers[i][j] = tiles[i][j];
		}
	}
	return 0;
}