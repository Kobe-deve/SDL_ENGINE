/*
	Project Tear Engine
	
	Developed by Demi
	9/16/2021 - 12/16/2021

*/

// SDL2 libraries 
#define SDL_MAIN_HANDLED

// main SDL2 handler
#include <SDL2/SDL.h>

// handles pngs with SDL2
#include <SDL2/SDL_image.h>

// handles fonts with SDL2
#include <SDL2/SDL_ttf.h>

// handles music with SDL2
#include <SDL2/SDL_mixer.h>

// windows handler
#include <windows.h>
#include <iostream>
#include <map>
#include <vector>
#include <utility>
#include <fstream>
#include <math.h>
using namespace std;

//Screen dimension constants
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 720;

#define PI 3.14159256

// max FPS constant 
int SCREEN_FPS = 60; 
// calculates ticks per frame for timers
int SCREEN_TICK_PER_FRAME = 1000 / SCREEN_FPS; 

// Color constants 
#define WHITE 255,255,255
#define BLACK 0,0,0

class text; 
class system_handler;
class game_handler;

// ------------------------------------ Handler Base --------------------------------

// class for handling timers
class timer 
{
	public:
		timer()
		{
			started = false;
			paused = false;
			
			startTicks = 0;
			pauseTicks = 0;
		}
		// starting timer 
		void start()
		{
			started = true;
			paused = false;
			
			startTicks = SDL_GetTicks();
			pauseTicks = 0;
		}
		// stopping timer 
		void stop()
		{
			started = false;
			paused = false;
			
			startTicks = 0;
			pauseTicks = 0;	
		}
		// pausing timer to resume later 
		void pause()
		{
			if(started && !paused)
			{
				paused = true;
				
				pauseTicks = SDL_GetTicks() - startTicks;
				startTicks = 0;
			}
		}
		// resuming timer 
		void resume()
		{
			if(started && paused)
			{
				paused = false;
				
				startTicks = SDL_GetTicks() - pauseTicks;
				pauseTicks = 0;
			}	
		}
		// gets ticks passed by from timer 
		Uint32 getTicks()
		{
			Uint32 t = 0;
			if(started)
			{
				if(paused)
					t = pauseTicks;
				else
					t = SDL_GetTicks() - startTicks;
			}
			return t;
		}
		bool paused = false; // is the timer paused? 
		bool started = false; // has the timer started? 
	
	private:
		// ticks used for timer 
		Uint32 startTicks = 0;
		Uint32 pauseTicks = 0;
};

// starting render font size of text
#define START_SIZE 50

// class for text handling 
class text 
{
	public:
		// loading text letters with specific textures 
		void loadLetters()
		{
			SDL_Texture ** texture = new SDL_Texture * [127]; // holds texture of text 
			string character = " ";
			
			// loads characters from value 29 to 125 
			for(int i = 29;i<126;i++)
			{
				character[0] = char(i);
				
				SDL_Surface* textSurface = TTF_RenderText_Solid(font, character.c_str(), textColor );
				
				if(textSurface == NULL)
					printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
				else
				{
					//Create texture from surface pixels
					texture[i] = SDL_CreateTextureFromSurface(renderer, textSurface );
					
					if(texture[i] == NULL )
						printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
					else
					{
						width = textSurface->w;
						height = textSurface->h;
					}
					
					letters.insert(pair<char,SDL_Texture*>((char)i,texture[i]));
					
					//Get rid of old surface
					SDL_FreeSurface( textSurface );
				}
			}
		}
		text(){}
		text(SDL_Renderer * r,int type=0)
		{
			renderer = r;
			switch(type)
			{
				case 1:
				font = TTF_OpenFont("resources/fonts/Dotty.ttf",START_SIZE);
				break;
				default:
				font = TTF_OpenFont("resources/fonts/chiaro.ttf",START_SIZE);
				break;
			}
			if(font == NULL)
				cout << "Error with loading font!";
			renderQuad = new SDL_Rect;
			
			loadLetters();
		}
		
		// for displaying text 
		void display(string t, int x=0, int y=0, double angle=0)
		{
			SDL_Rect renderQuad;
			
			// char used for getting parts of the t string 
			char chara = ' ';
			
			for(int i=0;i<t.length();i++)
			{
				// render the text, changing the size if it's different from the default 
				renderQuad = {x+(i*width*abs(size-(START_SIZE-1))), y, width*abs(size-(START_SIZE-1)), height*abs(size-(START_SIZE-1))};
				
				// set current character to a part of the string 
				chara = t[i];
				
				// set text color 
				if(black)
					SDL_SetTextureColorMod(letters.at(chara), BLACK);
				else
					SDL_SetTextureColorMod(letters.at(chara), textColor.r, textColor.g, textColor.b);
				
				// present alpha and texture piece 
				SDL_SetTextureAlphaMod(letters.at(chara), textColor.a);
				SDL_RenderCopyEx(renderer, letters.at(chara), NULL, &renderQuad, angle,NULL,SDL_FLIP_NONE);
			
			}		
		}

		int size = START_SIZE; // size of text 
		SDL_Color textColor = {255,255,255,255}; // text color 
		bool black = false; // does the text color just default to black?
		
	private:
		int height; // size of each letter (heigth * width)
		int width;
		
		// map of textures for letters, probably going to use a different data structure later (I hope)
		map<char,SDL_Texture*> letters; 
		
		SDL_Renderer * renderer; // renderer being used 
		
		SDL_Surface ** textSurface;
		
		SDL_Rect * renderQuad;
		
		string words;
		TTF_Font *font = NULL; // font information 
}; 

// image class  
class image 
{
	public:
		image(){}
		
		// loads class by getting texture ready 
		image(string path, SDL_Renderer* gRenderer)
		{
			//Load image at specified path
			SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
			if( loadedSurface == NULL )
				printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
			else
			{
				//Create texture from surface pixels
				texture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
				
				if( texture == NULL )
					printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
				else
				{
					width = loadedSurface->w;
					height = loadedSurface->h;
				}
				
				//Get rid of old loaded surface
				SDL_FreeSurface( loadedSurface );
			}
		}
		
		// changing color of the image 
		void setColor( Uint8 red, Uint8 green, Uint8 blue ) // changes color value of image
		{
			//Modulate texture
			SDL_SetTextureColorMod(texture, red, green, blue );
		}
		
		// changing transparency of the image 
		void setAlpha(Uint8 alph) // changes alpha value of image
		{
			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
			SDL_SetTextureAlphaMod(texture, alph);
		}
		
		// for rendering images 
		void render(SDL_Renderer* gRenderer, int x, int y, SDL_Rect* clip=NULL, int extraA = 0) // displays an image, clip is a section of an image
		{
			int temp = angle;
			//Set rendering space and render to screen
			renderQuad = { x, y, width*scale, height*scale };
			
			if(extraA != 0)
				angle = extraA;
			
			if(clip!=NULL)
			{
				renderQuad.w = clip->w*scale;
				renderQuad.h = clip->h*scale;	
			}
			
			//Render to screen 
			SDL_RenderCopyEx(gRenderer, texture, clip, &renderQuad, angle, center, flip);
			angle = temp;
		}
		
		// get texture
		SDL_Texture * grabTexture()
		{
			return texture;
		}
		
		// point of rotation 
		SDL_Point * center=NULL;
		
		// flipping the image 
		SDL_RendererFlip flip  = SDL_FLIP_NONE;
		
		// angle of rotation for the image 
		double angle = 0;
		
		// scale of the image 
		int scale = 1; 
		
		int height; // height and width of an image
		int width;
	protected:
		SDL_Rect renderQuad;
		int x = 0; // coordinates
		int y = 0;

		SDL_Texture* texture; // holds image information
};


/*
	
*/

// animated objects class 
class animatedAsset : public image 
{
	public:
		animatedAsset(){};
		animatedAsset(int w, int h, int r, int maxWid, int f, string path, SDL_Renderer* gRenderer)
		{
			frames = f;
			maxW = maxWid;
			camera.w = w;
			camera.h = h;
			camera.x = 0;
			camera.y = 0;
			rate = r;	
			
			//Load image at specified path
			SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
			if( loadedSurface == NULL )
				printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
			else
			{
				//Create texture from surface pixels
				texture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
				
				if( texture == NULL )
					printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
				else
				{
					width = loadedSurface->w;
					height = loadedSurface->h;
				}
				
				//Get rid of old loaded surface
				SDL_FreeSurface( loadedSurface );
			}
		};
		
		// display animation 
		void display(SDL_Renderer * r, int x, int y)
		{
			//Set rendering space and render to screen
			renderQuad = { x, y, width*scale, height*scale };
			
			renderQuad.w = camera.w*scale;
			renderQuad.h = camera.h*scale;	
		
			//Render to screen 
			SDL_RenderCopyEx(r, texture, &camera, &renderQuad, 0, center, flip);
		}
		
		// update animation 
		void update()
		{
			if(animate)
			{
				if(!down && step % rate == 0)
					counter++;
				else if(down && step % rate == 0)
					counter--;
				
				counter %= frames;
				
				if(counter > 0 && counter == frames-1)
					down = true;
				else if(camera.x == 0)
					down = false;
				
			
				step+=(1*SCREEN_FPS/60);
			}
			else if(counter == frames-1)
				done = true;
			camera.x = counter*camera.w;
		}
		
		// should the image move?
		bool animate = true;
		
		bool down = false;
		
		// pause on the first frame 
		bool pause_first = false;
		
		// the current frame that we're on in the image 
		int counter = 0;
		
		bool done = false;
		
	private:
	
		// max width of the image
		int maxW;
		
		// the section of the image that we're looking at 
		SDL_Rect camera;
		
		// increment step for counting 
		int step = 0;
		
		// number of frames in image 
		int frames = 0;
		
		// the rate of each image frame per 
		int rate;
};

// input enumeration
enum input_type {	
					NONE = 0,
					EXIT = -1,
					
					SELECT = -2,
					CANCEL = -3,
					
					MOUSECLICK_R = -4,
					MOUSECLICK_L = -5,
					
					UP = 1,
					DOWN = 2,
					LEFT = 3,	
					RIGHT = 4
				};

// input handler				
class input_handler
{
	public:
		input_handler()
		{
			e = new SDL_Event;
			state = NONE;
		}
		void update() // polls events 
		{
			input_type x = NONE;
			
			// get mouse info 
			SDL_GetMouseState( &mouseX, &mouseY );
			 
			const Uint8* keyStates = SDL_GetKeyboardState(NULL);
			while(SDL_PollEvent(e) != 0) // grabs input events 
			{			
				switch(e->type)
				{
					case SDL_MOUSEBUTTONDOWN:
						switch(e->button.button)
						{
							case SDL_BUTTON_RIGHT:
								x=MOUSECLICK_R;
							break;
							case SDL_BUTTON_LEFT:
								x=MOUSECLICK_L;
							break;
						}
					break;
					
					case SDL_JOYBUTTONDOWN: // for controller input
					
						switch(e->jbutton.button)
						{
							case SDL_CONTROLLER_BUTTON_DPAD_UP:
								x=UP;
							break;
							case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
								x=DOWN;
							break;
							case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
								x=LEFT;
							break;
							case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
								x=RIGHT;
							break;
							case SDL_CONTROLLER_BUTTON_A:
								x=SELECT;
							break;
							case SDL_CONTROLLER_BUTTON_B:
								x=CANCEL;
							break;
						}
					
					break;
					
					case SDL_KEYDOWN: // for keyboard input
						switch(e->key.keysym.sym)
						{
							case SDLK_RETURN:
								x = SELECT;
							break;
							case SDLK_BACKSPACE:
								x = CANCEL;
							break;
						
							case SDLK_d:
							case SDLK_RIGHT:
								x = RIGHT;
							break;
						
							case SDLK_a:
							case SDLK_LEFT:
								x = LEFT;
							break;
						
							case SDLK_w:
							case SDLK_UP:
								x = UP;
							break;
						
							case SDLK_s:
							case SDLK_DOWN:
								x = DOWN;
							break;
						}
					break;
	
					case SDL_QUIT: // clicking the x window button
						x = EXIT;
					break;
				
					default:
						x = NONE;
					break;
				}
			}
			
			state = x;
		}
	
		input_type state;
		int mouseX, mouseY;
	private:
		SDL_Event * e; // gets event inputs		
};

// main game handler, general flow of the main game
class game_handler
{
	public:
		// PUBLIC FUNCTIONS 
		
		// initalize the game/constructor 
		game_handler();
		game_handler(int x);
		
		// loads background assets 
		void loadBackground(int type);
		
		// main background in the game 
		void background();
		
		// run the current game system 
		void runSystem();
		
		// switch the current game system being run 
		void switchSystem(system_handler * game);
		
		// ending game, deconstructor  
		void close();
		
		// display background
		void background(bool down);
		
		// switching to a different background
		void switchBackground(int newOne);
		
		// PUBLIC VARIABLES 
		SDL_Renderer* renderer = NULL; // window renderer
	
		image * background_assets = NULL; // holds background images 
		
		// for displaying images 
		SDL_Surface* images;
	
		// handles input in the game 
		input_handler input = input_handler();
		
		// is the game running the current system?
		bool runningSystem = true;
		
		// the current game system the game is in 
		system_handler * currentGame = NULL;
		
		// main text handler
		text displayText;
		
		bool backgroundOn = true;
		
		Mix_Music * music; // game music
		
		// variables for animation when loading assets onto the screen 
		bool loadIn = false;
		
	private:
		// used for background assets with rendering sections 
		SDL_Rect renderRect;
		
		// variables for animation when switching out background assets
		bool switchOut = false;
		int switchTo = 0;
	
		// alpha for assets
		int megaAlpha = 0;
		
		// movement of background assets 
		bool down = true;
		
		int b = 0; // background type 
		
		// did the window load successfully?
		bool success = false;
		SDL_Window* window = NULL; // window 
		Uint8 colors[4] = {WHITE,0}; // background color of the window
	
		SDL_Texture* texture = NULL; // displayed texture
	
		timer framekeeper; // timers used for frame rate information and capping framerate
		timer capkeeper;
	
		int frames = 0; // frame counter
		double avgFPS; // the average FPS calculated throughout the window operation
};

// class layout for subsystems
class system_handler
{
	public:
		// constructor
		system_handler(){};
		
		// displaying the game system
		virtual void display(){};
	
		// handle the game system 
		virtual void handler(){};
		
		// for calling another system 
		virtual void callSystem(){};
		
		// for ending the system
		void endSystemHandler()
		{
			endSystem = true;
		};
		
		// is the current system ending?
		bool endSystem = false;
	protected:
		
		// the game system that the game is currently handling 
		game_handler * main_game;
};
