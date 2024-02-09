//"Plong" by Dylan Williams

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "./constants.h"

#define PI 3.14159265358979323846





//Globals, Structs, & Typedefs
//____________________________________________________________________________________________________________

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
bool isGameRunning = false;
int lastFrameTimeMs = 0;
float deltaTime;
int numPlayers;

//Colors.
int BLACK[3] = {0, 0, 0};	//Background.
int WHITE[3] = {255, 255, 255};	//Paddles, ball, center line, text.

//Assets.
TTF_Font* font;
Mix_Chunk* bounceSound;
Mix_Chunk* spinSound;
Mix_Chunk* zoomSound;
Mix_Chunk* scoreSound;
Mix_Chunk* winSound;
Mix_Chunk* newRoundSound;
Mix_Chunk* selectSound;


struct Paddle
{
	int score;
	float x;
	float y;
	float currVel;
	bool up, down;
	SDL_Rect rect;
} lpaddle, rpaddle;

struct Ball
{
	float x;
	float y;
	float lastx;	//Used to determine if ball is moving left or right. 
	float speed;
	float angle;	//In degrees. 
	float spin;
	int serveDir;	//Whether ball is served to left (-1) or right (1).
	SDL_Rect rect;
} ball, futureBall;

enum States
{
	CHOOSE_PLAYERS,
	PLAYING,
	HAS_WON
} gameState;

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________





































//Utility Functions
//____________________________________________________________________________________________________________

int randInt(int min, int max)
{
	return rand()%(max+1-min) + min;
}


//From https://www.geeksforgeeks.org/find-two-rectangles-overlap/
bool rectsColl(SDL_Rect* rect1, SDL_Rect* rect2)
{
	if (rect1->x > rect2->x+rect2->w || rect2->x > rect1->x+rect1->w)
		return false;
	if (rect1->y > rect2->y+rect2->h || rect2->y > rect1->y+rect1->h)
		return false;
	return true;
}


void drawText(char* text, int x, int y)
{
	SDL_Color white = {WHITE[0], WHITE[1], WHITE[2]};
	SDL_Surface* surface = TTF_RenderText_Solid(font, text, white); 
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect lrect = {x, y, w, h};
	SDL_RenderCopy(renderer, texture, NULL, &lrect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________



















//Setup & Initialization
//____________________________________________________________________________________________________________

bool initWindow(void)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		fprintf(stderr, "Error initalizing SDL.\n");
		return true;
	}
	
	window = SDL_CreateWindow(
		"Plong by Dylan Williams",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,	
		WINDOW_HEIGHT,
		0
	);
	if (window == NULL)
	{
		fprintf(stderr, "Error creating SDL Window.\n");
		return true;
	}
	
	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
	{
		fprintf(stderr, "Error creating SDL Renderer.\n");
		return true;
	}
	
	return false;
}


void setupPaddles()
{
	lpaddle.up = false;
	lpaddle.down = false;
	lpaddle.x = PADDLE_LONGITUDE;
	lpaddle.y = WINDOW_HEIGHT/2-PADDLE_HEIGHT/2;
	lpaddle.currVel = 0;
	lpaddle.rect = (SDL_Rect){(int)lpaddle.x, (int)lpaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT};
	lpaddle.score = 0;
	
	rpaddle.up = false;
	rpaddle.down = false;
	rpaddle.x = WINDOW_WIDTH-PADDLE_LONGITUDE-PADDLE_WIDTH;
	rpaddle.y = WINDOW_HEIGHT/2-PADDLE_HEIGHT/2;
	rpaddle.currVel = 0;
	rpaddle.rect = (SDL_Rect){(int)rpaddle.x, (int)rpaddle.y, (int)PADDLE_WIDTH, (int)PADDLE_HEIGHT};
	lpaddle.score = 0;
}


//Called at game start and whenever a player scores.
void resetBall()
{
	//Reset position to screen center.
	ball.x = WINDOW_WIDTH/2 - BALL_SIZE/2;
	ball.y = WINDOW_HEIGHT/2 - BALL_SIZE/2;
	ball.rect = (SDL_Rect){(int)ball.x, (int)ball.y, BALL_SIZE, BALL_SIZE};
	
	ball.speed = BALL_SERVE_SPEED;	//Ball is served at a slower speed. Ball returns to regular game speed upon being hit by a paddle.
	ball.spin = 0;
	
	//Shoot ball at left or right paddle.
	//ball.serveDir alternates back and forth between -1 (left) and 1 (right).
	ball.serveDir *= -1;
	if (ball.serveDir == -1)
		ball.angle = 180;	//Serve left.
	else
		ball.angle = 0;	//Serve right;
}


bool setupSounds()
{
	//Load sounds.
	bounceSound = Mix_LoadWAV("Audio/Bounce_Sound.wav");
	if (bounceSound == NULL)
	{
		printf("Failed to load Bounce_Sound.wav\n");
		return true;
	}
	scoreSound = Mix_LoadWAV("Audio/Score_Sound.wav");
	if (scoreSound == NULL)
	{
		printf("Failed to load Score_Sound.wav\n");
		return true;
	}
	newRoundSound = Mix_LoadWAV("Audio/New_Round_Sound.wav");
	if (newRoundSound == NULL)
	{
		printf("Failed to load New_Round_Sound.wav\n");
		return true;
	}
	winSound = Mix_LoadWAV("Audio/Win_Sound.wav");
	if (winSound == NULL)
	{
		printf("Failed to load Win_Sound.wav\n");
		return true;
	}
	selectSound = Mix_LoadWAV("Audio/Select_Sound.wav");
	if (winSound == NULL)
	{
		printf("Failed to load Select_Sound.wav\n");
		return true;
	}
	spinSound = Mix_LoadWAV("Audio/Spin_Sound.wav");
	if (spinSound == NULL)
	{
		printf("Failed to load Spin_Sound.wav\n");
		return true;
	}
	zoomSound = Mix_LoadWAV("Audio/Zoom_Sound.wav");
	if (zoomSound == NULL)
	{
		printf("Failed to load Zoom_Sound.wav\n");
		return true;
	}
	
	Mix_Volume(-1, MIX_MAX_VOLUME/3);
	
	return false;
}



//Returns false if successful, true if an error ocurred.
bool setup()
{
	gameState = CHOOSE_PLAYERS;
	
	setupPaddles();
	ball.serveDir = -1;	//resetBall() flips ball.serveDir, so right player is served first.
	resetBall();
	
	//Initialize Extension Libraries.
	if (TTF_Init() < 0)
	{
		printf("Error in TTF_Init()\n");
		return true;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 1, 2048) < 0)
	{
		printf("Error in Mix_OpenAudio()\n");
		return true;
	}
	
	if (setupSounds())
		return true;
	
	//Load font.
	font = TTF_OpenFont("Share_Tech_Mono/ShareTechMono-Regular.ttf", 30);
	if (font == NULL)
	{
		printf("Error loading ShareTechMono-Regular.ttf\n");
		return true;
	}
	
	return false;
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________



























//Update: Move, Calulate, Process
//____________________________________________________________________________________________________________

void processInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				isGameRunning = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						isGameRunning = false;
						break;
					case SDLK_w:
						lpaddle.up = true;
						break;
					case SDLK_s:
						lpaddle.down = true;
						break;
					case SDLK_UP:
						rpaddle.up = true;
						break;
					case SDLK_DOWN:
						rpaddle.down = true;
						break;
				}
				break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
					case SDLK_w:
						lpaddle.up = false;
						break;
					case SDLK_s:
						lpaddle.down = false;
						break;
					case SDLK_UP:
						rpaddle.up = false;
						break;
					case SDLK_DOWN:
						rpaddle.down = false;
						break;
				}
				break;
		}
	}
}


//Overrides a paddle's up and down variables. Used to computer-control left paddle in 1-player mode.
void computeInput(struct Paddle* paddle)
{	
	paddle->up = false;
	paddle->down = false;
	if (ball.y < paddle->y)
		paddle->up = true;
	else if (ball.y > paddle->y+PADDLE_HEIGHT)
		paddle->down = true;
}


void movePaddle(struct Paddle* paddle)
{
	if (!paddle->up && !paddle->down)
	{
		if (paddle->currVel < 0)	//Moving upward.
		{
			paddle->currVel += PADDLE_DEC*deltaTime;	//Slow paddle down.
			if (paddle->currVel > 0)
				paddle->currVel = 0;	//Stop.
		}
		else if (paddle->currVel > 0)	//Moving downward.
		{
			paddle->currVel -= PADDLE_DEC*deltaTime;	//Slow paddle down.
			if (paddle->currVel < 0)
				paddle->currVel = 0;	//Stop.
		}
	}
	else
	{
		if (paddle->up)
		{
				if (paddle->currVel <= 0)
					paddle->currVel -= PADDLE_ACC*deltaTime;
				else
					paddle->currVel -= PADDLE_DEC*deltaTime;
		}
		else if (paddle->down)
		{
				if (paddle->currVel >= 0)
					paddle->currVel += PADDLE_ACC*deltaTime;
				else
					paddle->currVel += PADDLE_DEC*deltaTime;
		}
	}
	
	//Keep paddle on screen.
	if (paddle->y < 0)	//Paddle above screen.
	{
		paddle->y = 0;
		paddle->currVel = 0;
	}
	else if ((paddle->y+PADDLE_HEIGHT)>WINDOW_HEIGHT)	//Paddle below screen.
	{
		paddle->y = WINDOW_HEIGHT-PADDLE_HEIGHT;
		paddle->currVel = 0;
	}	
	
	//Move paddle.	
	paddle->y += paddle->currVel;
	paddle->rect = (SDL_Rect){(int)paddle->x, (int)paddle->y, (int)PADDLE_WIDTH, (int)PADDLE_HEIGHT};
}


void moveBall()
{
	//Move ball in direction of angle.
	float rads = ball.angle * (PI/180);
	ball.x += cos((double)rads) * ball.speed * deltaTime;
	ball.y += sin((double)rads) * ball.speed * deltaTime;
	
	float xdir = ball.x - ball.lastx;	//-1 for left, 1 for right.
	
	//Bounce off ceiling.
	if (ball.y < 0)	
	{
		ball.y = 0;
		ball.angle = 360 - ball.angle;
		if (ball.spin != 0)	//The ball has spin and is curving.
		{
			//Ball shoots quickly somewhat towards opposite corner. 
			ball.speed = BALL_ZOOM_SPEED;
			if (xdir > 0)
				ball.angle = 45;
			else
				ball.angle = 135;
			Mix_PlayChannel(-1, zoomSound, 0);
		}
		else
			Mix_PlayChannel(-1, bounceSound, 0);
		ball.spin = 0;
	}
	//Bounce off floor.
	else if (ball.y+BALL_SIZE > WINDOW_HEIGHT)
	{
		ball.y = WINDOW_HEIGHT-BALL_SIZE;
		ball.angle = 360 - ball.angle;
		if (ball.spin != 0)
		{
			ball.speed = BALL_ZOOM_SPEED;
			if (xdir > 0)
				ball.angle = 315;
			else
				ball.angle = 225;
			Mix_PlayChannel(-1, zoomSound, 0);
		}
		else
			Mix_PlayChannel(-1, bounceSound, 0);
		ball.spin = 0;
	}
	
	
	//Bounce off left paddle.
	if (rectsColl(&(ball.rect), &(lpaddle.rect)) && xdir<0)		//Checking xdir prevents ball from getting stuck on paddle and infinitely bouncing in place.
	{
		if (lpaddle.currVel < -CURVE_SHOT_THRESHOLD)	//Moving upward fast enough to curve-shot.
		{
			ball.angle = 340;
			ball.spin = -BALL_SPIN;
			Mix_PlayChannel(-1, spinSound, 0);
		}
		else if (lpaddle.currVel > CURVE_SHOT_THRESHOLD)	//Moving downward fast enough to curve-shot.
		{
			ball.angle = 20;
			ball.spin = BALL_SPIN;
			Mix_PlayChannel(-1, spinSound, 0);
		}
		else	//Not moving fast enough to curve-shot.
		{
			ball.angle = randInt(-BALL_BOUNCE_RANGE, BALL_BOUNCE_RANGE);	//Bounce towards the right at random angle. 
			Mix_PlayChannel(-1, bounceSound, 0);
		}
		ball.speed = BALL_REG_SPEED;
	}
	//Bounce off right paddle.
	else if (rectsColl(&(ball.rect), &(rpaddle.rect)) && xdir>0)
	{
		if (rpaddle.currVel < -CURVE_SHOT_THRESHOLD)
		{
			ball.angle = 200;
			ball.spin = BALL_SPIN;
			Mix_PlayChannel(-1, spinSound, 0);
		}
		else if (rpaddle.currVel > CURVE_SHOT_THRESHOLD)
		{
			ball.angle = 160;
			ball.spin = -BALL_SPIN;
			Mix_PlayChannel(-1, spinSound, 0);
		}
		else
		{
			ball.angle = randInt(-BALL_BOUNCE_RANGE, BALL_BOUNCE_RANGE) + 180;
			Mix_PlayChannel(-1, bounceSound, 0);
		}
		ball.speed = BALL_REG_SPEED;
	}
	
	
	ball.rect = (SDL_Rect){(int)ball.x, (int)ball.y, BALL_SIZE, BALL_SIZE};
	ball.angle += ball.spin * deltaTime;
	
	ball.lastx = ball.x;
}

void controlScoring()
{
	if (ball.x < 0)
	{
		//Right paddle scores.
		rpaddle.score++;
		if (rpaddle.score >= WIN_SCORE)
		{
			//Right paddle wins.
			gameState = HAS_WON;
			Mix_PlayChannel(-1, winSound, 0);
		}
		else
			Mix_PlayChannel(-1, scoreSound, 0);
		resetBall();
	}
	else if (ball.x > WINDOW_WIDTH)
	{
		//Left paddle scores.
		lpaddle.score++;
		Mix_PlayChannel(-1, scoreSound, 0);
		if (lpaddle.score >= WIN_SCORE)
		{
			//Left paddle wins.
			gameState = HAS_WON;
			Mix_PlayChannel(-1, winSound, 0);
		}
		else
			Mix_PlayChannel(-1, scoreSound, 0);
		resetBall();
	}	
}

void newRound()
{
	gameState = PLAYING;
	lpaddle.score = 0;
	rpaddle.score = 0;
	resetBall();
	Mix_PlayChannel(-1, newRoundSound, 0);
}


void update()
{
	deltaTime = (SDL_GetTicks() - lastFrameTimeMs) / 1000.0f;	
	lastFrameTimeMs = SDL_GetTicks();
	
	//Move paddles.
	//processInput() is called before update() in main. Both paddle's up and down values are read from keyboard.
	if (numPlayers == 1)
		computeInput(&lpaddle);	//Override left player's input by manually computing values for lpaddle.down and lpaddle.up.
	movePaddle(&lpaddle);	//Move left paddle using values of down and up -- whether they're from keyboard or computed manually.	
	movePaddle(&rpaddle);
	
	
	switch (gameState)
	{
		case CHOOSE_PLAYERS:
			if (rpaddle.y == 0)
			{
				//Right paddle is controlled by player; left paddle controlled by computer using computeInput().
				numPlayers = 1;
				gameState = PLAYING;
				Mix_PlayChannel(-1, selectSound, 0);
			}
			else if (rpaddle.y == WINDOW_HEIGHT-PADDLE_HEIGHT)
			{
				//Both right and left paddle controlled by players.
				numPlayers = 2;
				gameState = PLAYING;
				Mix_PlayChannel(-1, selectSound, 0);
			}	
			break;
			
		case PLAYING:
			moveBall();
			controlScoring();
			break;

		case HAS_WON:
			//To play another round, right paddle or both paddles must be at the bottom of the screen.
			if (numPlayers == 1)
			{
				if (rpaddle.y == WINDOW_HEIGHT-PADDLE_HEIGHT)
					newRound();
			}
			else
			{
				if (lpaddle.y == WINDOW_HEIGHT-PADDLE_HEIGHT && rpaddle.y == WINDOW_HEIGHT-PADDLE_HEIGHT)
					newRound();
			}
			break;
	}
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________



















//Render
//____________________________________________________________________________________________________________

void drawNewRoundInstruction()
{
	//Not using drawText() because width and height values are needed to center.
	SDL_Color white = {WHITE[0], WHITE[1], WHITE[2]};
	SDL_Surface* surface;
	if (numPlayers == 1)
		surface = TTF_RenderText_Solid(font, "   go down for another round  v", white); 
	else
		surface = TTF_RenderText_Solid(font, "v  go down for another round  v", white); 
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_Rect lrect = {(WINDOW_WIDTH/2)-(w/2), WINDOW_HEIGHT-h*2, w, h};
	SDL_RenderCopy(renderer, texture, NULL, &lrect);
	SDL_FreeSurface(surface);
	SDL_DestroyTexture(texture);
}


void drawBall()
{
	SDL_SetRenderDrawColor(renderer, WHITE[0], WHITE[1], WHITE[2], 255);
	SDL_Rect ballrect = {(int)ball.x, (int)ball.y, BALL_SIZE, BALL_SIZE};
	SDL_RenderFillRect(renderer, &ballrect);
}


void drawFaces()
{
	char ltext[3];
	char rtext[3];
	if (lpaddle.score >= WIN_SCORE)
	{
		strcpy(ltext, "(:");
		strcpy(rtext, ":O");
	}
	else
	{
		strcpy(rtext, ":)");
		strcpy(ltext, "O:");
	}
	drawText(ltext, WINDOW_WIDTH/2-50, 50);
	drawText(rtext, WINDOW_WIDTH/2+25, 50);
}


void drawScores()
{
	char ltext[3] = {48 + lpaddle.score, '\0'};		//'0' is ASCII decimal value 48.
	drawText(ltext, WINDOW_WIDTH/2-50, 50);

	char rtext[3] = {48 + rpaddle.score, '\0'};
	drawText(rtext, WINDOW_WIDTH/2+25, 50);
}


void drawChoosePlayerInstructions()
{
	drawText("1 Player  ^", WINDOW_WIDTH-WINDOW_WIDTH/4, 50);
	drawText("2 Players  v", WINDOW_WIDTH-WINDOW_WIDTH/4, WINDOW_HEIGHT-50);
}


//Draws a series of vertical rects from top of window to bottom.
void drawCenterLine()
{
	int width = 5;
	int height = 15;
	int vertPad = 18;
	int x = WINDOW_WIDTH/2 - width/2;
	int y = 0;
	
	SDL_SetRenderDrawColor(renderer, WHITE[0], WHITE[1], WHITE[2], 255);
	while (y < WINDOW_HEIGHT)
	{
		SDL_Rect rect = {x, y, width, height};
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rect);
		y += height + vertPad;
	}
}


void drawPaddles()
{
	SDL_Rect lrect = {(int)lpaddle.x, (int)lpaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT};
	SDL_Rect rrect = {(int)rpaddle.x, (int)rpaddle.y, PADDLE_WIDTH, PADDLE_HEIGHT};
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(renderer, &lrect);	
	SDL_RenderFillRect(renderer, &rrect);
}


void render()
{
	SDL_SetRenderDrawColor(renderer, BLACK[0], BLACK[1], BLACK[2], 255);
	SDL_RenderClear(renderer);
	
	drawCenterLine();
	drawPaddles();
	
	switch (gameState)
	{
		case CHOOSE_PLAYERS:
			drawChoosePlayerInstructions();
			break;
		
		case PLAYING:
			drawBall();
			drawScores();
			break;
			
		case HAS_WON:
			drawNewRoundInstruction();
			drawFaces();
			break;
	}
	
	SDL_RenderPresent(renderer);	//Double buffer swap. 
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________
























//Cleanup
//____________________________________________________________________________________________________________

void freeSounds()
{
	Mix_FreeChunk(bounceSound);
	Mix_FreeChunk(scoreSound);
	Mix_FreeChunk(newRoundSound);
	Mix_FreeChunk(winSound);
	Mix_FreeChunk(selectSound);
	Mix_FreeChunk(spinSound);
	Mix_FreeChunk(zoomSound);
}

	
void cleanup()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	freeSounds();
	TTF_Quit();
	SDL_Quit();
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________




























//____________________________________________________________________________________________________________
//____________________________________________________________________________________________________________
//------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if (initWindow())
		exit(1);
	if (setup())
		exit(1);
	
	isGameRunning = true;
	while (isGameRunning)
	{
		processInput();
		update();
		render();
	}
	cleanup();
	
	return 0;
}

//------------------------------------------------------------------------------------------------------------
//____________________________________________________________________________________________________________




