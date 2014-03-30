#include <iostream>
#include <string>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "TileData.h"
#include "Board.h"

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int DEBUG = 1;

//These parts depends on the graphics

const int TILE_WIDTH = 100;
const int TILE_HEIGHT = 100;
const int TILE_BASE_X = 15;
const int TILE_BASE_Y = 15;
const int TILE_BASE_WIDTH = 440;
const int TILE_BASE_HEIGHT = 440;
const int TILE_BASE_MARGIN = 10;
const int ANIM_SPEED = 10;


typedef struct {
	SDL_Renderer* renderer;
	SDL_Texture* background;
	SDL_Texture* numberTiles;
} RenderSystem;

/**
* Log an sdl error.
*/
void logSDLError(std::ostream &os, const std::string &msg) {
	os << msg << " error: " << SDL_GetError() << std::endl;
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
	SDL_Color color, int fontSize, SDL_Renderer *renderer) {
	//Open the font
	TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
	if (font == NULL) {
		logSDLError(std::cout, "TTF_OpenFont");
		return NULL;
	}
	//We need to first render to a surface as that's what TTF_RenderText
	//returns, then load that surface into a texture
	SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
	if (surf == NULL) {
		TTF_CloseFont(font);
		logSDLError(std::cout, "TTF_RenderText");
		return NULL;
	}
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
	if (texture == NULL) {
		logSDLError(std::cout, "CreateTexture");
	}
	//Clean up the surface and font
	SDL_FreeSurface(surf);
	TTF_CloseFont(font);
	return texture;
}

SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren) {
	SDL_Texture *texture = NULL;

	//     SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
	texture = IMG_LoadTexture(ren, file.c_str());

	if (texture == NULL) {
		logSDLError(std::cout, "IMG_LoadTexture");
	}
	return texture;
}


/**
* draw the texture to a SDL_Renderer, with given scale.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst,
	SDL_Rect *clip = NULL) {
	SDL_RenderCopy(ren, tex, clip, &dst);
}

/**
* draw the texture to a SDL_Renderer withot scaling.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y,
	SDL_Rect *clip = NULL) {
	SDL_Rect dst;
	dst.x = x;
	dst.y = y;
	if (clip != NULL) {
		dst.w = clip->w;
		dst.h = clip->h;
	}
	else {
		SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
	}
	renderTexture(tex, ren, dst, clip);
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
						clipValue = int(log2(tiles[i2][j2].newValue)) - 1; //since 2 is the first one but it is 2^^1 not 0.
					}
					//std::cout << "clip value " << clipValue << std::endl;
				}
				else {
					std::cout << "startvalue " << tiles[i][j].startValue << " newValue " << tiles[i][j].newValue << std::endl;
					int tileStartValue = abs(tiles[i][j].startValue) - abs(tiles[i][j].startValue) % 2; //this calculates if an insert is done to that point.
					clipValue = int(log2(tileStartValue)) - 1;
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

		int clipValue = int(log2(tiles[insertedTileX][insertedTileY].newValue)) - 1;
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

	std::ostringstream scoreString;
	scoreString << score;
	SDL_Texture* scoreText = renderText(scoreString.str(),
		"./res/Gauge-Regular.ttf", color, 48, renderSystem.renderer);
	renderTexture(scoreText, renderSystem.renderer, 500, 170, NULL); //the font is smaller than the tile

	if (isGameOver == 1) {
		//color = { 150, 50, 50 };
		color.r = 150;
		color.g = 50;
		color.b = 50;
		SDL_Texture* gameOverText = renderText("Game Over!", "./res/Gauge-Regular.ttf", color, 72, renderSystem.renderer);
		renderTexture(gameOverText, renderSystem.renderer, 40, SCREEN_HEIGHT / 2 - 20, NULL); //the font is smaller than the tile
	}

	SDL_RenderPresent(renderSystem.renderer);
}


int main(int argc, char **argv) {
	RenderSystem currentRS;
	Board *board = new Board();

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}

	SDL_Window *win = SDL_CreateWindow("2048-ng", 100, 100, SCREEN_WIDTH,
		SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (win == NULL) {
		logSDLError(std::cout, "SDL_CreateWindow");
		return 1;
	}

	currentRS.renderer = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (currentRS.renderer == NULL) {
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

	if (currentRS.background == NULL || currentRS.numberTiles == NULL) {
		logSDLError(std::cout, "LoadTexture");
		return 4;
	}

	SDL_Event e;
	//initial render
	renderGame(currentRS, board->tiles, board->isGameOver, board->score);
	while (!board->quit) {
		//        while(SDL_PollEvent(&e)) {
		SDL_WaitEvent(&e);
		{
			if (e.type == SDL_MOUSEMOTION) {
				continue;
			}
			if (e.type == SDL_QUIT)
				board->quit = true;
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
				case SDLK_ESCAPE:
					board->quit = true;
					break;
				case SDLK_q:
					board->quit = true;
					break;
				case SDLK_UP:
					if (!board->isGameOver) {
						board->moveDirection = 8;
						board->backupNumbersMatrix();
						board->isMoved = board->moveNumbers();
					}
					break;
				case SDLK_DOWN:
					if (!board->isGameOver) {
						board->moveDirection = 2;
						board->backupNumbersMatrix();
						board->isMoved = board->moveNumbers();
					}
					break;

				case SDLK_LEFT:
					if (!board->isGameOver) {
						board->moveDirection = 4;
						board->backupNumbersMatrix();
						board->isMoved = board->moveNumbers();
					}
					break;

				case SDLK_RIGHT:
					if (!board->isGameOver) {
						board->moveDirection = 6;
						board->backupNumbersMatrix();
						board->isMoved = board->moveNumbers();
					}
					break;

				default:
					break;
				}
			}
		}

		//if move is done, add a number to a empty cell
		if (board->isMoved == 1) {
			//render before insert, or inserted element overwrites the tile before animation.
			//renderGame(currentRS, board.tiles, board.isGameOver, board.score);
			if (!board->insertNumber()) {
				board->endGame();
				board->isGameOver = 1;
			}
			//logMessage("after insert");
			board->logMatrixState();

			renderGame(currentRS, board->tiles, board->isGameOver, board->score);
			board->isMoved = 0;
		}
		else {

			if (board->checkGameOver()) {
				board->isGameOver = 1;
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
