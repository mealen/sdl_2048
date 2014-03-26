#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 40;
bool quit = false;

/**
* Log an sdl error.
*/
void logSDLError(std::ostream &os, const std::string &msg)
{
    os << msg << " error: " << SDL_GetError() << std::endl;
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

    SDL_Color color = {255,255,255};
    //SDL_Texture* text = renderText("Hello World", "./res/sample.ttf",color, 16, renderer);

    if(background == nullptr || numbers == nullptr)
    {
        logSDLError(std::cout, "LoadTexture");
        return 4;
    }

    //set up the clip points
    int iW=100, iH=100;

    SDL_Rect clips[12];
    for(int i=0; i<12; ++i)
    {
        clips[i].x= i * iW;
        clips[i].y= 0;
        clips[i].w = iW;
        clips[i].h = iH;
    }

    int moveDirection=0;//2 down, 8 up, 4 left, 6 right.

    SDL_Event e;
    while (!quit)
    {
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                quit=true;
            if(e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_UP:
                    moveDirection = 8;
                    break;
                case SDLK_DOWN:
                    moveDirection = 2;
                    break;

                case SDLK_LEFT:
                    moveDirection = 4;
                    break;

                case SDLK_RIGHT:
                    moveDirection = 6;
                    break;

                default:
                    break;

                }

            }
        }


        SDL_RenderClear(renderer);
        renderTexture(background,renderer,0,0, NULL);

        SDL_Rect dst;
        dst.x=25;
        dst.y=25;
        dst.w=iW;
        dst.h=iH;
        renderTexture(numbers, renderer,dst,&clips[0]);

        /*
                for(int i=0; i < SCREEN_WIDTH / iW; i++)
                {
                    for(int j=0; j < SCREEN_HEIGHT / iH; j++)
                    {
                        renderTexture(image,renderer,i*iW,j*iH, &clips[(i+j)%4]);
                        std::cout << "printed using" << useClip%4 << std::endl;

                    }
                }
        */
        //renderTexture(text,renderer,0,0);
        SDL_RenderPresent(renderer);
    }


    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
