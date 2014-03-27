#include <iostream>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

bool quit = false;

typedef struct {
    SDL_Renderer* renderer;
    SDL_Texture* background;
    SDL_Texture* numberTiles;
} RenderSystem;


typedef struct {
    int startX;
    int startY;
    int endX;
    int endY;
    int endPos; //the end position on moving row
    int totalMove = 0;
} tileMoveData;


typedef struct {
    int numbers[4][4];
    int oldNumbers[4][4]; // = {0};
    int moveDirection = 0;//2 down, 8 up, 4 left, 6 right.
    int isGameOver = 0;
    int score = 0;
    int isMoved = 0;
    tileMoveData tileMoves[4][4];

} BoardStatus;

void logMessage(const std::string &msg) {
    if(DEBUG == 1) {
        std::cout << msg << std::endl;
    }
}

/**
* Log an sdl error.
*/
void logSDLError(std::ostream &os, const std::string &msg) {
    os << msg << " error: " << SDL_GetError() << std::endl;
}

void logMatrixState(int numbers[4][4]) {
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            std::cout << numbers[j][i] << " "; //FIXME how this is reverse?
        }
        std::cout << std::endl;
    }
}



void logMoveData(tileMoveData moveData[4][4]) {
    for (int i=0; i<4; i++) {
        for (int j=0; j<4; j++) {
            std::cout << moveData[j][i].endPos << " "; //FIXME how this is reverse?
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
    texture = IMG_LoadTexture(ren,file.c_str());

    if (texture== nullptr) {
        logSDLError(std::cout, "IMG_LoadTexture");
    }
    return texture;
}

void fillTmdEndPosValues(int numbers[], tileMoveData tmd[], int numberCount) {
    for (int i=0; i < numberCount; i++) {
        tmd[i].endPos = i;
    }
}


int moveZerosInSingleArray(int numbers[], int numberCount, tileMoveData tmd[]) {
    int returnValue = 0;
    for(int i =numberCount -1; i>0; i--) {
        if(numbers[i] == 0) {
            for(int j=i-1; j >= 0; j--) {
                if(numbers[j] != 0) {
                    numbers[i] = numbers[j];
                    numbers[j] =0;
                    returnValue = 1;
                    //since the endpos values are filled, it must match at some point
                    for(int k=0; k <= j; k++) {
                        if(tmd[k].endPos == j) {
                            //it was moved from k position
                            tmd[k].endPos = i; // update the last position data
                        }
                    }
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
int mergeSingleArray(int numbers[], int numberCount, int* score, tileMoveData tmd[]) {
    int internalReturn = 0;
    if(numberCount == 1)
        return 0;
    internalReturn = mergeSingleArray(numbers +1, numberCount -1, score, tmd + 1); //this ensures after current, it is ordered(all zero possible)
    if(numbers[0] == 0)
        return internalReturn;
    if (numbers[0]==numbers[1]) {
        numbers[1] = numbers[1] * 2;
        numbers[0] = 0;
        *score = *score + numbers[1];
        tmd[0].endPos++; // the element move one tile

        return 1;
    }
    return internalReturn;
}


/**
* moves the zeros to the end, then merge part is called.
* moves zeros after that again.
*/
int moveSingleArray(int numbers[], int numberCount,int* score, tileMoveData tmd[]) {
    int returnValue = 0;
    fillTmdEndPosValues(numbers,tmd, numberCount);
    returnValue = moveZerosInSingleArray(numbers, numberCount,tmd);
    returnValue += mergeSingleArray(numbers,numberCount, score,tmd);
    returnValue += moveZerosInSingleArray(numbers, numberCount,tmd);
    return (returnValue > 0);
}
//cleaned of zeros.


int moveNumbers(BoardStatus &bs) {
    int returnValue=0;
    logMessage("before");
    logMatrixState(bs.numbers);
    switch(bs.moveDirection) {
    case 2:
        logMessage("moving down ");
        for(int i=0; i<4; i++) {
            int coloumn[4] = {bs.numbers[i][0],bs.numbers[i][1],bs.numbers[i][2],bs.numbers[i][3]};
            tileMoveData tmd[4] = {bs.tileMoves[i][0],bs.tileMoves[i][1],bs.tileMoves[i][2],bs.tileMoves[i][3]};
            if(moveSingleArray(coloumn,4, &(bs.score),tmd) == 1)
                returnValue = 1;
            bs.numbers[i][0] = coloumn[0];
            bs.numbers[i][1] = coloumn[1];
            bs.numbers[i][2] = coloumn[2];
            bs.numbers[i][3] = coloumn[3];
            //reinsert move data
            //TODO these should be enriched
            bs.tileMoves[i][0] = tmd[0];
            bs.tileMoves[i][1] = tmd[1];
            bs.tileMoves[i][2] = tmd[2];
            bs.tileMoves[i][3] = tmd[3];



        }
        break;
    case 4:
        logMessage("moving left ");
        for(int i=0; i<4; i++) {
            int coloumn[4] = {bs.numbers[3][i],bs.numbers[2][i],bs.numbers[1][i],bs.numbers[0][i]};
            tileMoveData tmd[4] = {bs.tileMoves[3][i],bs.tileMoves[2][i],bs.tileMoves[1][i],bs.tileMoves[0][i]};
            if(moveSingleArray(coloumn,4, &(bs.score),tmd) == 1)
                returnValue = 1;
            bs.numbers[3][i] = coloumn[0];
            bs.numbers[2][i] = coloumn[1];
            bs.numbers[1][i] = coloumn[2];
            bs.numbers[0][i] = coloumn[3];

            //reinsert move data
            //TODO these should be enriched
            bs.tileMoves[3][i] = tmd[0];
            bs.tileMoves[2][i] = tmd[1];
            bs.tileMoves[1][i] = tmd[2];
            bs.tileMoves[0][i] = tmd[3];
        }
        break;
    case 6:
        logMessage("moving right ");
        for(int i=0; i<4; i++) {
            int coloumn[4] = {bs.numbers[0][i],bs.numbers[1][i],bs.numbers[2][i],bs.numbers[3][i]};
            tileMoveData tmd[4] = {bs.tileMoves[0][i],bs.tileMoves[1][i],bs.tileMoves[2][i],bs.tileMoves[3][i]};
            if(moveSingleArray(coloumn,4, &(bs.score),tmd) == 1)
                returnValue = 1;
            bs.numbers[0][i] = coloumn[0];
            bs.numbers[1][i] = coloumn[1];
            bs.numbers[2][i] = coloumn[2];
            bs.numbers[3][i] = coloumn[3];

            //reinsert move data
            //TODO these should be enriched
            bs.tileMoves[0][i] = tmd[0];
            bs.tileMoves[1][i] = tmd[1];
            bs.tileMoves[2][i] = tmd[2];
            bs.tileMoves[3][i] = tmd[3];

        }
        break;
    case 8:
        logMessage("moving up ");
        for(int i=0; i<4; i++) {
            int coloumn[4] = {bs.numbers[i][3],bs.numbers[i][2],bs.numbers[i][1],bs.numbers[i][0]};
            tileMoveData tmd[4] = {bs.tileMoves[i][3],bs.tileMoves[i][2],bs.tileMoves[i][1],bs.tileMoves[i][0]};
            if(moveSingleArray(coloumn,4, &(bs.score),tmd) == 1)
                returnValue = 1;
            bs.numbers[i][3] = coloumn[0];
            bs.numbers[i][2] = coloumn[1];
            bs.numbers[i][1] = coloumn[2];
            bs.numbers[i][0] = coloumn[3];

            //reinsert move data
            //TODO these should be enriched
            bs.tileMoves[i][3] = tmd[0];
            bs.tileMoves[i][2] = tmd[1];
            bs.tileMoves[i][1] = tmd[2];
            bs.tileMoves[i][0] = tmd[3];
        }
        break;
    }
    logMessage("after");
    logMatrixState(bs.numbers);
    logMessage("change");
    logMoveData(bs.tileMoves);
    return returnValue;
}

/**
* draw the texture to a SDL_Renderer, with given scale.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip=nullptr) {
    SDL_RenderCopy(ren, tex, clip, &dst);
}

/**
* draw the texture to a SDL_Renderer withot scaling.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip=nullptr) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip!=nullptr) {
        dst.w = clip->w;
        dst.h = clip->h;
    } else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    renderTexture(tex, ren, dst, clip);
}

int insertNumber(int numbers[4][4]) {
    logMessage("insert call");
    int newNumber;
    if(rand() % 5 == 4) // %20 chance
        newNumber = 4;
    else
        newNumber = 2;

    int emptyCellCount = 0;
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            if(numbers[i][j] == 0)
                emptyCellCount++;
        }
    }
    if (emptyCellCount == 0)
        return 0;

    int insertCell = rand() % emptyCellCount;

    emptyCellCount = 0; //now find the element and set
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            if(numbers[i][j] == 0) {
                if(emptyCellCount == insertCell) {
                    numbers[i][j]= newNumber;
                    return 1;
                } else {
                    emptyCellCount++;
                }
            }
        }
    }
    //this should newer happen, but gcc complains
    assert(0);
    return 0;
}

void renderGame(RenderSystem renderSystem,  int numbers[4][4],int isGameOver, int score) {
    //TODO these calculations are redudant
    SDL_Rect clips[12];
    for(int i=0; i<12; ++i) {
        clips[i].x= i * TILE_WIDTH;
        clips[i].y= 0;
        clips[i].w = TILE_WIDTH;
        clips[i].h = TILE_HEIGHT;
    }

    SDL_Color color = {50,50,50};

    SDL_RenderClear(renderSystem.renderer);
    renderTexture(renderSystem.background,renderSystem.renderer,0,0, NULL);

    for(int i=0; i < 4; i++) {
        for(int j=0; j < 4; j++) {
            if(numbers[i][j] > 0) {
                int xpos=TILE_BASE_X + TILE_BASE_MARGIN + i * (TILE_BASE_MARGIN + TILE_WIDTH);
                int ypos=TILE_BASE_Y + TILE_BASE_MARGIN + j * (TILE_BASE_MARGIN + TILE_HEIGHT);
                int clipValue = log2(numbers[i][j]) -1 ;//since 2 is the first one but it is 2^^1 not 0.
                renderTexture(renderSystem.numberTiles,renderSystem.renderer,xpos,ypos, &clips[clipValue]);
                //SDL_Texture* numberText = renderText(std::to_string(numbers[i][j]),"./res/Gauge-Regular.ttf",color,72, renderer);
                //renderTexture(numberText,renderer,xpos + 20 ,ypos + 20, NULL);//the font is smaller than the tile
            }
        }
    }
    //render score
    SDL_Texture* scoreText = renderText(std::to_string(score),"./res/Gauge-Regular.ttf",color,48, renderSystem.renderer);
    renderTexture(scoreText,renderSystem.renderer, 500 ,170, NULL);//the font is smaller than the tile

    if(isGameOver == 1) {
        color = {150,50,50};
        SDL_Texture* gameOverText = renderText("Game Over!","./res/Gauge-Regular.ttf",color,72, renderSystem.renderer);
        renderTexture(gameOverText,renderSystem.renderer, 40 ,SCREEN_HEIGHT / 2 - 20, NULL);//the font is smaller than the tile
    }

    SDL_RenderPresent(renderSystem.renderer);
}

void initBoard(int numbers[4][4]) {
    srand (time(NULL));
    memset(numbers, 0, 16 * sizeof(int));
    numbers[rand() % 4][rand() % 4] = 2;

    numbers[rand() % 4][rand() % 4] += 2;
}

void endGame(int numbers[4][4]) {

}
/**
* it checks if the game is over.
*
*/
int checkGameOver(int numbers[4][4]) {
    //TODO optimise this part

    //if there are 2 same adjent cell, it is not over. This for
    for(int i = 0; i< 3; i++) {
        for(int j = 0; j< 3; j++) {
            if(numbers[i][j] == numbers[i][j+1] || numbers[i][j] == numbers[i+1][j] || numbers[i][j] == 0) {
                return 0;
            }
        }
        if(numbers[i][3] == numbers[i+1][3] || numbers[i][3] == 0) // this is the last coloumn check.
            return 0;
    }
    for(int j=0; j<3; j++) { //this is the last row check.
        if(numbers[3][j] == numbers[3][j+1] || numbers[3][j] == 0)
            return 0;
    }
    return 1;




}

/**
* This function overwrites old matrix.
*
*/
int backupNumbersMatrix(int original[4][4], int backup[4][4]) {
    for(int i=0; i<4; i++) {
        for(int j=0; j<4; j++) {
            backup[i][j] = original[i][j];
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    RenderSystem currentRS;
    BoardStatus board;
    initBoard(board.numbers);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Lesson 2", 100, 100,
                                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(win == nullptr) {
        logSDLError(std::cout, "SDL_CreateWindow");
        return 1;
    }

    currentRS.renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

    currentRS.background = loadTexture("./res/background.png",currentRS.renderer);
    currentRS.numberTiles = loadTexture("./res/numbers.png",currentRS.renderer);


    if(currentRS.background == nullptr || currentRS.numberTiles == nullptr) {
        logSDLError(std::cout, "LoadTexture");
        return 4;
    }

    SDL_Event e;
    //initial render
    renderGame(currentRS, board.numbers, board.isGameOver, board.score);
    while (!quit) {
//        while(SDL_PollEvent(&e)) {
        SDL_WaitEvent(&e);
        {
            if(e.type == SDL_QUIT)
                quit=true;
            if(e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_q:
                    quit=true;
                    break;
                case SDLK_UP:
                    if(!board.isGameOver) {
                        board.moveDirection = 8;
                        backupNumbersMatrix(board.numbers,board.oldNumbers);
                        board.isMoved = moveNumbers(board);
                    }
                    break;
                case SDLK_DOWN:
                    if(!board.isGameOver) {
                        board.moveDirection = 2;
                        backupNumbersMatrix(board.numbers,board.oldNumbers);
                        board.isMoved = moveNumbers(board);
                    }
                    break;

                case SDLK_LEFT:
                    if(!board.isGameOver) {
                        board.moveDirection = 4;
                        backupNumbersMatrix(board.numbers,board.oldNumbers);
                        board.isMoved = moveNumbers(board);
                    }
                    break;

                case SDLK_RIGHT:
                    if(!board.isGameOver) {
                        board.moveDirection = 6;
                        backupNumbersMatrix(board.numbers,board.oldNumbers);
                        board.isMoved = moveNumbers(board);
                    }
                    break;

                default:
                    break;
                }
            }
        }

        //if move is done, add a number to a empty cell
        if(board.isMoved == 1) {
            if(!insertNumber(board.numbers)) {
                endGame(board.numbers);
                board.isGameOver=1;
            }
            //don't move this over insert number.

            board.isMoved=0;
        } else {

            if(checkGameOver(board.numbers)) {
                board.isGameOver = 1;
            }

        }
        renderGame(currentRS, board.numbers, board.isGameOver, board.score);

    }

    SDL_DestroyTexture(currentRS.numberTiles);
    SDL_DestroyTexture(currentRS.background);
    SDL_DestroyRenderer(currentRS.renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
