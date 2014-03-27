#include <iostream>
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

bool quit = false;

void logMessage(const std::string &msg)
{
    if(DEBUG == 1)
    {
        std::cout << msg << std::endl;
    }
}

/**
* Log an sdl error.
*/
void logSDLError(std::ostream &os, const std::string &msg)
{
    os << msg << " error: " << SDL_GetError() << std::endl;
}

void logMatrixState(int numbers[4][4])
{
    for (int i=0; i<4; i++)
    {
        for (int j=0; j<4; j++)
        {
            std::cout << numbers[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

SDL_Texture* renderText(const std::string &message, const std::string &fontFile,
                        SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
    //Open the font
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == nullptr)
    {
        logSDLError(std::cout, "TTF_OpenFont");
        return nullptr;
    }
    //We need to first render to a surface as that's what TTF_RenderText
    //returns, then load that surface into a texture
    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (surf == nullptr)
    {
        TTF_CloseFont(font);
        logSDLError(std::cout, "TTF_RenderText");
        return nullptr;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == nullptr)
    {
        logSDLError(std::cout, "CreateTexture");
    }
    //Clean up the surface and font
    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}



SDL_Texture* loadTexture(const std::string &file, SDL_Renderer *ren)
{
    SDL_Texture *texture = nullptr;

//     SDL_Surface *loadedImage = SDL_LoadBMP(file.c_str());
    texture = IMG_LoadTexture(ren,file.c_str());

    if (texture== nullptr)
    {
        logSDLError(std::cout, "IMG_LoadTexture");
    }
    return texture;
}

/**
* returns 1 if there were a change, returns 0 if no change
*
*/
int moveSingleArray2(int numbers[], int numberCount)
{
    int internalReturn = 0;
    if(numberCount == 1)
        return 0;
    internalReturn = moveSingleArray2(numbers +1, numberCount -1); //this ensures after current, it is ordered(all zero possible)
    if(numbers[0] == 0)
        return internalReturn;
    if(numbers[1] == 0)
    {
        numbers[1] = numbers[0];
        numbers[0] = 0;
        moveSingleArray2(numbers + 1, numberCount -1); //order after current, in case all zeros. what if we already done a merge?
        return 1;
    }
    else if (numbers[0]==numbers[1])
    {
        numbers[1] = numbers[1] * 2;
        numbers[0] = 0;
        return 1;
    }
    return internalReturn;
}

int moveNumbers(int numbers[4][4], int direction)
{
    int returnValue=0;
    logMessage("before");
    logMatrixState(numbers);
    switch(direction)
    {
    case 2:
        logMessage("moving down ");

        for(int i=0; i<4; i++)
        {
            int coloumn[4] = {numbers[i][0],numbers[i][1],numbers[i][2],numbers[i][3]};

            if(moveSingleArray2(coloumn,4) == 1)
                returnValue = 1;


            numbers[i][0] = coloumn[0];
            numbers[i][1] = coloumn[1];
            numbers[i][2] = coloumn[2];
            numbers[i][3] = coloumn[3];
        }
        break;
    case 4:
        logMessage("moving left ");
        for(int i=0; i<4; i++)
        {
            //new
            int coloumn[4] = {numbers[3][i],numbers[2][i],numbers[1][i],numbers[0][i]};
            if(moveSingleArray2(coloumn,4) == 1)
                returnValue = 1;
            numbers[3][i] = coloumn[0];
            numbers[2][i] = coloumn[1];
            numbers[1][i] = coloumn[2];
            numbers[0][i] = coloumn[3];
        }
        break;
    case 6:
        logMessage("moving right ");
        for(int i=0; i<4; i++)
        {
            //new
            int coloumn[4] = {numbers[0][i],numbers[1][i],numbers[2][i],numbers[3][i]};
            if(moveSingleArray2(coloumn,4) == 1)
                returnValue = 1;
            numbers[0][i] = coloumn[0];
            numbers[1][i] = coloumn[1];
            numbers[2][i] = coloumn[2];
            numbers[3][i] = coloumn[3];
        }
        break;
    case 8:
        logMessage("moving up ");
        for(int i=0; i<4; i++)
        {
            //new
            int coloumn[4] = {numbers[i][3],numbers[i][2],numbers[i][1],numbers[i][0]};
            if(moveSingleArray2(coloumn,4) == 1)
                returnValue = 1;
            numbers[i][3] = coloumn[0];
            numbers[i][2] = coloumn[1];
            numbers[i][1] = coloumn[2];
            numbers[i][0] = coloumn[3];
        }
        break;
    }
    logMessage("after");
    logMatrixState(numbers);
    return returnValue;
}

/**
* draw the texture to a SDL_Renderer, with given scale.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip=nullptr)
{
    SDL_RenderCopy(ren, tex, clip, &dst);
}

/**
* draw the texture to a SDL_Renderer withot scaling.
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip=nullptr)
{
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip!=nullptr)
    {
        dst.w = clip->w;
        dst.h = clip->h;
    }
    else
    {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    renderTexture(tex, ren, dst, clip);
}

int insertNumber(int numbers[4][4])
{
    logMessage("insert call");
    int newNumber;
    if(rand() % 5 == 4) // %20 chance
        newNumber = 4;
    else
        newNumber = 2;

    int emptyCellCount = 0;
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(numbers[i][j] == 0)
                emptyCellCount++;
        }
    }
    if (emptyCellCount == 0)
        return 1;

    int insertCell = rand() % emptyCellCount;

    emptyCellCount = 0; //now find the element and set
    for(int i=0; i<4; i++)
    {
        for(int j=0; j<4; j++)
        {
            if(numbers[i][j] == 0)
                if(emptyCellCount == insertCell)
                {
                    numbers[i][j]= newNumber;
                    return 0;
                }
                else
                    emptyCellCount++;
        }
    }


}


int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Lesson 2", 100, 100,
                                       SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(win == nullptr)
    {
        logSDLError(std::cout, "SDL_CreateWindow");
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        logSDLError(std::cout, "SDL_CreateRenderer");
        return 1;
    }

    //this is for performance.
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        logSDLError(std::cout, "IMG_Init");
        return 1;
    }

    if (TTF_Init() != 0)
    {
        logSDLError(std::cout, "TTF_Init");
        return 1;
    }

    SDL_Texture* background = loadTexture("./res/background.png",renderer);
    SDL_Texture* numbers = loadTexture("./res/numbers.png",renderer);

    SDL_Color color = {50,50,50};
    //SDL_Texture* text = renderText("Hello World", "./res/sample.ttf",color, 16, renderer);

    if(background == nullptr || numbers == nullptr)
    {
        logSDLError(std::cout, "LoadTexture");
        return 4;
    }

    //set up the clip points


    SDL_Rect clips[12];
    for(int i=0; i<12; ++i)
    {
        clips[i].x= i * TILE_WIDTH;
        clips[i].y= 0;
        clips[i].w = TILE_WIDTH;
        clips[i].h = TILE_HEIGHT;
    }
    int currentNumbersMatrix[4][4] = {0};
    currentNumbersMatrix[0][0] = 2;
    currentNumbersMatrix[0][3] = 2;
    int moveDirection=0;//2 down, 8 up, 4 left, 6 right.

    SDL_Event e;
    int isMoved=0;
    while (!quit)
    {
        SDL_WaitEvent(&e);
        if(e.type == SDL_QUIT)
            quit=true;
        if(e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_q:
                quit=true;
                break;
            case SDLK_UP:
                moveDirection = 8;
                isMoved = moveNumbers(currentNumbersMatrix,moveDirection);
                break;
            case SDLK_DOWN:
                moveDirection = 2;
                isMoved = moveNumbers(currentNumbersMatrix,moveDirection);
                break;

            case SDLK_LEFT:
                moveDirection = 4;
                isMoved = moveNumbers(currentNumbersMatrix,moveDirection);
                break;

            case SDLK_RIGHT:
                moveDirection = 6;
                isMoved = moveNumbers(currentNumbersMatrix,moveDirection);
                break;

            default:
                break;


            }
        }

        //move is done, so add a number to a empty cell
        if(isMoved == 1)
        {
            insertNumber(currentNumbersMatrix);
        }
        isMoved=0;
        SDL_RenderClear(renderer);
        renderTexture(background,renderer,0,0, NULL);

        SDL_Rect dst;
        dst.x=25;
        dst.y=25;
        dst.w=TILE_WIDTH;
        dst.h=TILE_HEIGHT;
        //renderTexture(numbers, renderer,dst,&clips[0]);


        for(int i=0; i < 4; i++)
        {
            for(int j=0; j < 4; j++)
            {
                if(currentNumbersMatrix[i][j] > 0)
                {
                    int xpos=TILE_BASE_X + TILE_BASE_MARGIN + i * (TILE_BASE_MARGIN + TILE_WIDTH);
                    int ypos=TILE_BASE_Y + TILE_BASE_MARGIN + j * (TILE_BASE_MARGIN + TILE_HEIGHT);
                    //std::cout << "xpos = " << xpos << std::endl;
                    //std::cout << "ypos = " << ypos << std::endl;
                    SDL_Texture* numberText = renderText(std::to_string(currentNumbersMatrix[i][j]),"./res/Gauge-Regular.ttf",color,72, renderer);
                    renderTexture(numbers,renderer,xpos,ypos, &clips[0]);
                    renderTexture(numberText,renderer,xpos + 20 ,ypos + 20, NULL);//the font is smaller than the tile
                    //std::cout << "printed using" << useClip%4 << std::endl;
//                    SDL_Delay(1000);
//                    SDL_RenderPresent(renderer);
                }
            }
        }

        //renderTexture(text,renderer,0,0);
        SDL_RenderPresent(renderer);
    }


    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
