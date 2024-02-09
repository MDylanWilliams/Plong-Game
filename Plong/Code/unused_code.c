


/*
void drawScores()
{
	//Each digit is 3x5 grid of squares. 1 for filled, 0 for not filled.
	float size = 7;
	int digits[10][15] = {{1,1,1,1,0,1,1,0,1,1,0,1,1,1,1},
			  {1,1,0,0,1,0,0,1,0,0,1,0,1,1,1},
			  {1,1,1,0,0,1,1,1,1,1,0,0,1,1,1},
			  {1,1,1,0,0,1,0,1,1,0,0,1,1,1,1},
			  {1,0,1,1,0,1,1,1,1,0,0,1,0,0,1},
			  {1,1,1,1,0,0,1,1,1,0,0,1,1,1,1},
			  {1,1,1,1,0,0,1,1,1,1,0,1,1,1,1},
			  {1,1,1,0,0,1,0,0,1,0,0,1,0,0,1},
			  {1,1,1,1,0,1,1,1,1,1,0,1,1,1,1},
			  {1,1,1,1,0,1,1,1,1,0,0,1,0,0,1}};
	
	//Draw left paddle's score.
	int x = 125;
	int y = 50;
	for (int i=1; i<=15; i++)
	{
		if (digits[lpaddle.score][i-1])
		{
			SDL_Rect rect = {x, y, size, size};
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
		x += size;
		if (i%3 == 0)
		{
			x = 125;
			y += size;
		}
	}
	
	//Draw right paddle's score.
	x = WINDOW_WIDTH-125-(3*size);
	y = 50;
	for (int i=1; i<=15; i++)
	{
		if (digits[rpaddle.score][i-1])
		{
			SDL_Rect rect = {x, y, size, size};
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(renderer, &rect);
		}
		x += size;
		if (i%3 == 0)
		{
			x = WINDOW_WIDTH-125-(3*size);
			y += size;
		}
	}
}
*/