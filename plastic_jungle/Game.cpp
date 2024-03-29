#include "Game.h"

using namespace std;

Game::Game()
{
	mainWindow = NULL;
	mainRenderer = NULL;

	componentMasks.resize(MAX_ENTITIES);
	componentVelocities.resize(MAX_ENTITIES);
	componentSprites.resize(MAX_ENTITIES);
	componentPositions.resize(MAX_ENTITIES);

	for (auto i : componentMasks)
	{
		i = COMPONENT_NONE;
	}

	heroNum = 0;
	scrollX = 0;
	scrollY = 0;

	heroWidth = 20;
	heroHeight = 20;

	leftRender = 2;
	rightRender = 7;

}

Game::~Game()
{
	destruct();
}

bool Game::initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "Couldn't initialize SDL: " << SDL_GetError() << "\n";
		return false;
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		cout << "Render scale quality not set" << "\n";
	}

	mainWindow = SDL_CreateWindow("plastic_jungle", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (mainWindow == NULL)
	{
		cout << "Couldn't initialize window: " << SDL_GetError() << "\n";
		return false; 
	}
	
	mainRenderer = SDL_CreateRenderer(mainWindow, -1, 
		SDL_RENDERER_ACCELERATED);

	if (mainRenderer == NULL)
	{
		cout << "Couldn't initialize renderer: " << SDL_GetError() << "\n";
		return false;
	}

	SDL_SetRenderDrawColor(mainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	//SDL_RenderFillRect(mainRenderer, &screen);
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		cout << "Couldn't initialize SDL_Image: " << SDL_GetError() << "\n";
		return false;
	}

	return true;
}

bool Game::loadMedia()
{
	if (!background.loadRandomColor())
	{
		cout << "Couldn't load background texture\n";
		return false;
	}

	if (!backgroundNoise.loadFrom("images/noise/Noise-hd.png"))
	{
		cout << "Couldn't load background noise\n";
		return false;
	}

	if (!hillTexture.loadRandomColor())
	{
		cout << "Couldn't load hill texture\n";
		return false;
	}

	return true;
}

void Game::reloadBackground()
{
	if (!background.loadRandomColor())
	{
		cout << "Couldn't load background texture\n";
	}
}

void Game::destruct()
{
	SDL_DestroyWindow(mainWindow);
	SDL_DestroyRenderer(mainRenderer);
	mainWindow = NULL;
	mainRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

void Game::displayBackground()
{
	if (scrollX > SCREEN_WIDTH) {
		scrollX = 0;
	}

	background.render(-scrollX, -scrollY, NULL);
	background.render(-scrollX + background.getWidth(), 
		-scrollY, NULL);

	backgroundNoise.setAlpha(45);

	backgroundNoise.render(-scrollX, -scrollY, NULL);
	backgroundNoise.render(-scrollX + backgroundNoise.getWidth(), 
		-scrollY, NULL);
}

void Game::drawTerrain()
{
	SDL_Point* points = terrain.getPoints();
	float x1 = 0;
	float y1 = 0;
	float x2 = 0;
	float y2 = 0;
	SDL_Point one, two, x, y;
	int hillSegments;
	float dX, dAngle, yMiddle, amplitude;
	SDL_Rect srcRect;

	for (int i = leftRender; i < rightRender; ++i)
	{
		x1 = (*(points + i - 1)).x - camera.x;
		y1 = (*(points + i - 1)).y - camera.y;
		x2 = (*(points + i)).x - camera.x;
		y2 = (*(points + i)).y - camera.y;

		//SDL_RenderDrawLine(mainRenderer, x1, y1, x2, y2);

		one = (*(points + i - 1));
		two = (*(points + i));

		hillSegments = floorf((float)(two.x - one.x)/HILL_SEGMENT_WIDTH);

		dX = (two.x - one.x)/hillSegments;
		dAngle = 3.14159/hillSegments;
		yMiddle = (one.y + two.y)/2;
		amplitude = (one.y - two.y)/2;

		x = one;
		for (int j = 0; j < hillSegments + 2; ++j)
		{
			y.x = one.x + j*dX;
			y.y = yMiddle + amplitude * cosf(dAngle*j);

			//SDL_RenderDrawLine(mainRenderer, x.x - camera.x, 
			//	x.y - camera.y, y.x - camera.x, 
			//	y.y - camera.y);

			srcRect.x = x.x - camera.x;
			srcRect.y = x.y - camera.y;
			srcRect.h = SCREEN_HEIGHT;
			srcRect.w = dX;

			hillTexture.render(x.x - camera.x, x.y - camera.y, &srcRect);
			backgroundNoise.setAlpha(45);
			backgroundNoise.render(x.x - camera.x, x.y - camera.y, &srcRect);

			x = y;

			srcRect.x += camera.x;
			srcRect.y += camera.y;

			if (x.x < componentPositions[heroNum].x + 30
				&& x.x > componentPositions[heroNum].x - 30)
			{
				hillRects.push_front(srcRect);
				//cout << srcRect.x << endl;
			}
		}
	}

	destroyHills();
	/*
	if (!hillRects.empty())
	{
		cout << hillRects.front().x << ", "; 
		cout << componentPositions[heroNum].x;
		cout << ", " << hillRects.back().x << ", ";
		cout << hillRects.size() << endl;
	}
	*/
}

void Game::destroyHills()
{
	while (hillRects.back().x < componentPositions[heroNum].x - 30)
	{
		hillRects.pop_back();
	}
}

//ECS

int Game::createEntity()
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		if (componentMasks[i] == COMPONENT_NONE)
		{
			return i;
		}
	}

	return -1;
}

void Game::destroyEntity(int i)
{
	componentMasks[i] = COMPONENT_NONE;
}

void Game::createHero()
{
	heroNum = createEntity();
	componentMasks[heroNum] = 
		COMPONENT_VELOCITY | COMPONENT_SPRITE | COMPONENT_POSITION;

	componentPositions[heroNum].x = SCREEN_WIDTH/2;
	componentPositions[heroNum].y = SCREEN_HEIGHT/2;

	componentVelocities[heroNum].x = 3;
	componentVelocities[heroNum].y = 1;

	componentSprites[heroNum].initialize("images/dot.bmp", 
		20, 20, 1, 1);
}

void Game::heroEventHandler(SDL_Event& event)
{
	if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_UP:
				componentVelocities[heroNum].y -= HERO_VEL;
				break;
			case SDLK_DOWN:
				componentVelocities[heroNum].y += HERO_VEL;
				break;
			case SDLK_LEFT:
				componentVelocities[heroNum].x -= HERO_VEL;
				break;
			case SDLK_RIGHT:
				componentVelocities[heroNum].x += HERO_VEL;
				break;
			}
	}
	else if (event.type == SDL_KEYUP && event.key.repeat == 0)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_UP:
				componentVelocities[heroNum].y += HERO_VEL;
				break;
			case SDLK_DOWN:
				componentVelocities[heroNum].y -= HERO_VEL;
				break;
			case SDLK_LEFT:
				componentVelocities[heroNum].x += HERO_VEL;
				break;
			case SDLK_RIGHT:
				componentVelocities[heroNum].x -= HERO_VEL;
				break;
		}
	}
}

void Game::movementSystem()
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		if ((componentMasks[i] & MOVEMENT_MASK) == MOVEMENT_MASK)
		{
			componentPositions[i].x += componentVelocities[i].x;
			componentPositions[i].y += componentVelocities[i].y;
		}

		if (componentPositions[i].y < 0 
			|| componentPositions[i].y > 
			(LEVEL_HEIGHT - componentSprites[i].getHeight()))
		{
			componentPositions[i].y -= componentVelocities[i].y;
		}


		//if you go below floor
		/*
		if (componentPositions[i].y < 0) {
			componentPositions[i].y -= componentVelocities[i].y;
		}
		*/
	}

	//cout << componentPositions[heroNum].x << ", " << componentPositions[heroNum].y << endl;
	collisionDetection();
}

void Game::animationSystem()
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		if (componentMasks[i] & COMPONENT_SPRITE == COMPONENT_SPRITE)
		{
			componentSprites[i].animate(componentPositions[i].x - camera.x, 
				componentPositions[i].y - camera.y);
		}
	}
}

void Game::gravitySystem()
{
	for (int i = 0; i < MAX_ENTITIES; ++i)
	{
		if (componentMasks[i] & COMPONENT_VELOCITY == COMPONENT_VELOCITY)
		{
			componentVelocities[i].y -= 1;
		}
	}
}

void Game::centerCamera(SDL_Rect& camera)
{
	int tempX = camera.x;
	int tempY = camera.y;
	camera.x = (componentPositions[heroNum].x 
		+ componentSprites[heroNum].getWidth()/2) - SCREEN_WIDTH/2;
	camera.y = (componentPositions[heroNum].y + 
		componentSprites[heroNum].getHeight()/2) - SCREEN_HEIGHT/2;

	if (camera.y < 0)
	{
		camera.y = 0;
	}

	if (camera.y > LEVEL_HEIGHT - camera.h)
	{
		camera.y = LEVEL_HEIGHT - camera.h;
	}

	scrollX += (camera.x - tempX);
	scrollY += (camera.y - tempY);

	SDL_Point* points = terrain.getPoints();
	if ((*(points + leftRender)).x < camera.x)
	{
		leftRender++;
		rightRender++;
	}

/*
	cout << camera.x << ", " << camera.y << ", ";
	cout << componentPositions[heroNum].x << ", " << componentPositions[heroNum].y;
	cout << ", " << scrollX << ", " << scrollY;
	cout << ", " << leftRender << ", " << rightRender << endl;
*/
}

void Game::collisionDetection()
{
	for (int i = 0; i < hillRects.size(); ++i)
	{
		if (componentPositions[heroNum].x > hillRects[i].x
			&& componentPositions[heroNum].x < hillRects[i].x + hillRects[i].w)
		{
			if (componentPositions[heroNum].y + heroHeight > hillRects[i].y)
			{
				//cout << componentPositions[heroNum].y << ", ";
				//cout << hillRects[i].y << endl;
				componentPositions[heroNum].y -= 
					(componentPositions[heroNum].y + heroHeight - hillRects[i].y);
			}
		}
	}
}