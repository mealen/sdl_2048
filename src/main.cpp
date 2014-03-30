#include <iostream>
#include <cassert>
#include <string>
#include <ctime>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int DEBUG = 1;

//These parts depends on the graphichs

const int TILE_WIDTH = 100;
const int TILE_HEIGHT = 100;
const int TILE_BASE_X = 15;
const int TILE_BASE_Y = 15;
const int TILE_BASE_WIDTH = 440;
const int TILE_BASE_HEIGHT = 440;
const int TILE_BASE_MARGIN = 10;
const int ANIM_SPEED = 10;

enum possibleMerges {
	noMerge, mergeOn, mergeOff
};

typedef struct {
	SDL_Renderer* renderer;
	SDL_Texture* background;
	SDL_Texture* numberTiles;
} RenderSystem;

typedef struct {

	int startX;
	int startY;
	int moveX;
	int moveY;
	int totalMove;
	int startValue;
	int newValue;
	enum possibleMerges mergeStatus;
} TileData;

typedef struct {
	TileData tiles[4][4];
	TileData oldNumbers[4][4]; // = {0};
	int moveDirection; //2 down, 8 up, 4 left, 6 right.
	int isGameOver;
	int score;
	int isMoved;
	bool quit;
} BoardStatus;

void logMessage(const std::string &msg) {
	if (DEBUG == 1) {
		std::cout << msg << std::endl;
	}
}

/**
* Log an sdl error.
*/
void logSDLError(std::ostream &os, const std::string &msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}

void logMatrixState(TileData tiles[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << tiles[i][j].newValue << " ";
		}
		std::cout << std::endl;
	}
}

void logMoveData(TileData tileData[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << "[" << tileData[i][j].moveX << ","
				<< tileData[i][j].moveY << "]";
			std::cout << "[" << tileData[i][j].startValue << "] ";
			//std::cout << "[" << tileData[j][i].startX << ","
			//		<< tileData[j][i].startY << "," << tileData[j][i].totalMove
			//		<< "] ";
		}
		std::cout << std::endl;
	}
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer) {
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == nullptr) {
		logSDLError(std::cout, "TTF_OpenFont");
		return nullptr;
	}
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == nullptr) {
		TTF_CloseFont(font);
		logSDLError(std::cout, "TTF_RenderText");
		return nullptr;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == nullptr) {
		logSDLError(std::cout, "CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = nullptr;

	//     SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
	texture = IMG_LoadTexture(ren, file.c_str());

	if (texture == nullptr) {
		logSDLError(std::cout, "IMG_LoadTexture");
	}
	return texture;
}

void setTilesForMove(TileData tiles[4][4]) {
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

void fillTileMove(TileData tiles[4][4], int moveDirection) {
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

int moveZerosInSingleArray(TileData tiles[], int numberCount) {
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

/**
* returns 1 if there were a change, returns 0 if no change
*
*/
int mergeSingleArray(TileData tiles[], int numberCount, int* score) {
	int internalReturn = 0;
	if (numberCount == 1)
		return 0;
	internalReturn = mergeSingleArray(tiles + 1, numberCount - 1, score); //this ensures after current, it is ordered(all zero possible)
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
		*score = *score + tiles[1].newValue;

		return 1;
	}
	return internalReturn;
}

/**
* moves the zeros to the end, then merge part is called.
* moves zeros after that again.
*/
int moveSingleArray(TileData tiles[], int numberCount, int* score) {
	int returnValue = 0;
	returnValue = moveZerosInSingleArray(tiles, numberCount);
	returnValue += mergeSingleArray(tiles, numberCount, score);
	returnValue += moveZerosInSingleArray(tiles, numberCount);
	return (returnValue > 0);
}
//cleaned of zeros.

int moveNumbers(BoardStatus &bs) {
	int returnValue = 0;
	logMessage("before");
	logMatrixState(bs.tiles);
	setTilesForMove(bs.tiles);
	switch (bs.moveDirection) {
	case 6:
		logMessage("moving down ");
		for (int i = 0; i < 4; i++) {
			TileData coloumn[4] = { bs.tiles[i][0], bs.tiles[i][1],
				bs.tiles[i][2], bs.tiles[i][3] };
			if (moveSingleArray(coloumn, 4, &(bs.score)) == 1)
				returnValue = 1;
			bs.tiles[i][0] = coloumn[0];
			bs.tiles[i][1] = coloumn[1];
			bs.tiles[i][2] = coloumn[2];
			bs.tiles[i][3] = coloumn[3];

		}
		break;
	case 8:
		logMessage("moving left ");
		for (int i = 0; i < 4; i++) {
			TileData row[4] = { bs.tiles[3][i], bs.tiles[2][i], bs.tiles[1][i],
				bs.tiles[0][i] };
			if (moveSingleArray(row, 4, &(bs.score)) == 1)
				returnValue = 1;
			bs.tiles[3][i] = row[0];
			bs.tiles[2][i] = row[1];
			bs.tiles[1][i] = row[2];
			bs.tiles[0][i] = row[3];

		}
		break;
	case 2:
		logMessage("moving right ");
		for (int i = 0; i < 4; i++) {
			TileData row[4] = { bs.tiles[0][i], bs.tiles[1][i], bs.tiles[2][i],
				bs.tiles[3][i] };
			if (moveSingleArray(row, 4, &(bs.score)) == 1)
				returnValue = 1;
			bs.tiles[0][i] = row[0];
			bs.tiles[1][i] = row[1];
			bs.tiles[2][i] = row[2];
			bs.tiles[3][i] = row[3];

		}
		break;
	case 4:
		logMessage("moving up ");
		for (int i = 0; i < 4; i++) {
			TileData coloumn[4] = { bs.tiles[i][3], bs.tiles[i][2],
				bs.tiles[i][1], bs.tiles[i][0] };
			if (moveSingleArray(coloumn, 4, &(bs.score)) == 1)
				returnValue = 1;
			bs.tiles[i][3] = coloumn[0];
			bs.tiles[i][2] = coloumn[1];
			bs.tiles[i][1] = coloumn[2];
			bs.tiles[i][0] = coloumn[3];

		}
		break;
	}
	logMessage("after");
	logMatrixState(bs.tiles);
	fillTileMove(bs.tiles, bs.moveDirection);
	logMessage("change");
	logMoveData(bs.tiles);
	return returnValue;
}

/**
* draw the texture to a SDL_Renderer, with given scale.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst,
	SDL_Rect *clip = nullptr) {
	SDL_RenderCopy(ren, tex, clip, &dst);
}

/**
* draw the texture to a SDL_Renderer withot scaling.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y,
	SDL_Rect *clip = nullptr) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != nullptr) {
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
}

int insertNumber(TileData numbers[4][4]) {
	logMessage("insert call");
	int newNumber;
	if (rand() % 5 == 4) // %20 chance
		newNumber = 4;
	else
		newNumber = 2;

	int emptyCellCount = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (numbers[i][j].newValue == 0)
				emptyCellCount++;
		}
	}
	if (emptyCellCount == 0)
		return 0;

	int insertCell = rand() % emptyCellCount;

	emptyCellCount = 0; //now find the element and set
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (numbers[i][j].newValue == 0) {
				if (emptyCellCount == insertCell) {
					numbers[i][j].newValue = newNumber;
					numbers[i][j].startValue = -1 * numbers[i][j].startValue - 1; //for new entry it is negated, and +1 is incase of 0
					logMoveData(numbers);
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

int renderTiles(RenderSystem rs, TileData tiles[4][4], int frame,
	int frameSpeed) {

	int isFrameLast = 1;
	int insertedTileX = -1;
	int insertedTileY = -1;

	//TODO these calculations are redudant
	SDL_Rect clips[12];
	for (int clipIndex = 0; clipIndex < 12; ++clipIndex) {
		clips[clipIndex].x = clipIndex * TILE_WIDTH;
		clips[clipIndex].y = 0;
		clips[clipIndex].w = TILE_WIDTH;
		clips[clipIndex].h = TILE_HEIGHT;
	}
	SDL_RenderClear(rs.renderer);
	renderTexture(rs.background, rs.renderer, 0, 0, NULL);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			//std::cout << "[" << i << "," << j << "]";
			if (tiles[i][j].startValue > 0 || tiles[i][j].startValue < -1) { // if <= -1 then the inserted gets rendered at begining
				int isInPlace = 0;
				int movement = frame * frameSpeed;

				int xpos = TILE_BASE_X + TILE_BASE_MARGIN
					+ j * (TILE_BASE_MARGIN + TILE_WIDTH);
				if (movement >=
					((TILE_BASE_MARGIN + TILE_WIDTH)* abs(tiles[i][j].moveX))) {
					//the tile is in final position on x;
					xpos += (TILE_BASE_MARGIN + TILE_WIDTH) * tiles[i][j].moveX;
					isInPlace = 1;
				}
				else {
					// tile is still moving
					if (tiles[i][j].moveX > 0)
						xpos += movement;
					else
						xpos -= movement;
					isFrameLast = 0;
					isInPlace = 0;
				}

				int ypos = TILE_BASE_Y + TILE_BASE_MARGIN
					+ i * (TILE_BASE_MARGIN + TILE_HEIGHT);
				if (movement
					>= ((TILE_BASE_MARGIN + TILE_WIDTH)
					* abs(tiles[i][j].moveY))) {
					//the tile is in final position on y;
					ypos += (TILE_BASE_MARGIN + TILE_WIDTH) * tiles[i][j].moveY;
					isInPlace = 1;
				}
				else {
					// tile is still moving
					if (tiles[i][j].moveY > 0)
						ypos += movement;
					else
						ypos -= movement;
					isFrameLast = 0;
					isInPlace = 0;
				}

				int clipValue;
				int i2, j2;
				if (isInPlace) {
					i2 = i + tiles[i][j].moveY;
					j2 = j + tiles[i][j].moveX;
					//std::cout << "i:" << i << " j:" << j << " inplace" << std::endl;
					//		<< tiles[i2][j2].newValue << std::endl;
					if (tiles[i2][j2].newValue == 0) {
						clipValue = 0;
					}
					else {
						clipValue = log2(tiles[i2][j2].newValue) - 1; //since 2 is the first one but it is 2^^1 not 0.
					}
					//std::cout << "clip value " << clipValue << std::endl;
				}
				else {
					std::cout << "startvalue " << tiles[i][j].startValue << " newValue " << tiles[i][j].newValue << std::endl;
					int tileStartValue = abs(tiles[i][j].startValue) - abs(tiles[i][j].startValue) % 2; //this calculates if an insert is done to that point.
					clipValue = log2(tileStartValue) - 1;
				}

				renderTexture(rs.numberTiles, rs.renderer, xpos, ypos,
					&clips[clipValue]);
			}
			if (tiles[i][j].newValue > 0 && tiles[i][j].startValue <= -1){
				//std::cout << "new value at [" << i << "," << j << "]"<< std::endl;
				insertedTileX = i;
				insertedTileY = j;
			}
		}
	}
	if (isFrameLast && (insertedTileX != -1 || insertedTileY != -1)){
		//render the inserted one, since this is the last frame
		std::cout << "new value at [" << insertedTileX << "," << insertedTileY << "]" << std::endl;
		int xpos = TILE_BASE_X + TILE_BASE_MARGIN
			+ insertedTileY * (TILE_BASE_MARGIN + TILE_WIDTH);
		int ypos = TILE_BASE_Y + TILE_BASE_MARGIN
			+ insertedTileX * (TILE_BASE_MARGIN + TILE_HEIGHT);

		int clipValue = log2(tiles[insertedTileX][insertedTileY].newValue) - 1;
		renderTexture(rs.numberTiles, rs.renderer, xpos, ypos,
			&clips[clipValue]);

	}
	SDL_RenderPresent(rs.renderer);

	return isFrameLast;
}

void renderGame(RenderSystem renderSystem, TileData numbers[4][4],
	int isGameOver, int score) {

	SDL_Color color = { 50, 50, 50 };

	int i = 0;
	while (!renderTiles(renderSystem, numbers, i++, 10)) {
		SDL_Delay(1);
	}
	//render score
	SDL_Texture* scoreText = renderText(std::to_string(score),
		"./res/Gauge-Regular.ttf", color, 48, renderSystem.renderer);
	renderTexture(scoreText, renderSystem.renderer, 500, 170, NULL); //the font is smaller than the tile

	if (isGameOver == 1) {
		color = { 150, 50, 50 };
		SDL_Texture* gameOverText = renderText("Game Over!", "./res/Gauge-Regular.ttf", color, 72, renderSystem.renderer);
		renderTexture(gameOverText, renderSystem.renderer, 40, SCREEN_HEIGHT / 2 - 20, NULL); //the font is smaller than the tile
	}

	SDL_RenderPresent(renderSystem.renderer);
}

void initBoard(TileData tiles[4][4]) {
	srand(time(NULL));
	memset(tiles, 0, 16 * sizeof(TileData));
	int values[4] = { rand() % 4, rand() % 4, rand() % 4, rand() % 4 };
	tiles[values[0]][values[1]].newValue = 2;
	tiles[values[0]][values[1]].startValue = 2;

	tiles[values[2]][values[3]].newValue += 2;
	tiles[values[2]][values[3]].startValue += 2;
}

void endGame(TileData numbers[4][4]) {

}
/**
* it checks if the game is over.
*
*/
int checkGameOver(TileData tiles[4][4]) {
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
int backupNumbersMatrix(TileData original[4][4], TileData backup[4][4]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			backup[i][j] = original[i][j];
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	RenderSystem currentRS;
	BoardStatus board;

	board.isGameOver = 0;
	board.moveDirection = 0;
	board.score = 0;
	board.isMoved = 0;
	board.quit = false;

	initBoard(board.tiles);

	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			board.tiles[i][j].totalMove = 0;
		}
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *win = SDL_CreateWindow("Lesson 2", 100, 100, SCREEN_WIDTH,
		SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == nullptr) {
		logSDLError(std::cout, "SDL_CreateWindow");
		return 1;
	}

	currentRS.renderer = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (currentRS.renderer == nullptr) {
		logSDLError(std::cout, "SDL_CreateRenderer");
		return 1;
	}

	//this is for performance.
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
		logSDLError(std::cout, "IMG_Init");
		return 1;
	}

	if (TTF_Init() != 0) {
		logSDLError(std::cout, "TTF_Init");
		return 1;
	}

	currentRS.background = loadTexture("./res/background.png",
		currentRS.renderer);
	currentRS.numberTiles = loadTexture("./res/numbers.png",
		currentRS.renderer);

	if (currentRS.background == nullptr || currentRS.numberTiles == nullptr) {
		logSDLError(std::cout, "LoadTexture");
		return 4;
	}

	SDL_Event e;
	//initial render
	renderGame(currentRS, board.tiles, board.isGameOver, board.score);
	while (!board.quit) {
		//        while(SDL_PollEvent(&e)) {
		SDL_WaitEvent(&e);
		{
			if (e.type == SDL_MOUSEMOTION) {
				continue;
			}
			if (e.type == SDL_QUIT)
				board.quit = true;
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					board.quit = true;
					break;
				case SDLK_q:
					board.quit = true;
					break;
				case SDLK_UP:
					if (!board.isGameOver) {
						board.moveDirection = 8;
						backupNumbersMatrix(board.tiles, board.oldNumbers);
						board.isMoved = moveNumbers(board);
					}
					break;
				case SDLK_DOWN:
					if (!board.isGameOver) {
						board.moveDirection = 2;
						backupNumbersMatrix(board.tiles, board.oldNumbers);
						board.isMoved = moveNumbers(board);
					}
					break;

				case SDLK_LEFT:
					if (!board.isGameOver) {
						board.moveDirection = 4;
						backupNumbersMatrix(board.tiles, board.oldNumbers);
						board.isMoved = moveNumbers(board);
					}
					break;

				case SDLK_RIGHT:
					if (!board.isGameOver) {
						board.moveDirection = 6;
						backupNumbersMatrix(board.tiles, board.oldNumbers);
						board.isMoved = moveNumbers(board);
					}
					break;

				default:
					break;
				}
			}
		}

		//if move is done, add a number to a empty cell
		if (board.isMoved == 1) {
			//render before insert, or inserted element overwrites the tile before animation.
			//renderGame(currentRS, board.tiles, board.isGameOver, board.score);
			if (!insertNumber(board.tiles)) {
				endGame(board.tiles);
				board.isGameOver = 1;
			}
			logMessage("after insert");
			logMatrixState(board.tiles);

			renderGame(currentRS, board.tiles, board.isGameOver, board.score);
			board.isMoved = 0;
		}
		else {

			if (checkGameOver(board.tiles)) {
				board.isGameOver = 1;
			}

		}

	}

	SDL_DestroyTexture(currentRS.numberTiles);
	SDL_DestroyTexture(currentRS.background);
	SDL_DestroyRenderer(currentRS.renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return 0;
}
