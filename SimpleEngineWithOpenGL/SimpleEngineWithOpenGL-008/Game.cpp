#include "Game.h"
#include "Actor.h"
#include "SpriteComponent.h"
#include "AnimSpriteComponent.h"
#include "Timer.h"
#include "ResourceManager.h"
#include "BackgroundSpriteComponent.h"

bool Game::initialize()
{
	bool isWindowInit = window.initialize();
	bool isRendererInit = renderer.initialize(window);
	return isWindowInit && isRendererInit; // Return bool && bool && bool ...to detect error
}

void Game::load()
{
	// Load textures
	ResourceManager::loadTexture(renderer, "Res\\Ship01.png", "Ship01");
	ResourceManager::loadTexture(renderer, "Res\\Ship02.png", "Ship02");
	ResourceManager::loadTexture(renderer, "Res\\Ship03.png", "Ship03");
	ResourceManager::loadTexture(renderer, "Res\\Ship04.png", "Ship04");
	ResourceManager::loadTexture(renderer, "Res\\Farback01.png", "Farback01");
	ResourceManager::loadTexture(renderer, "Res\\Farback02.png", "Farback02");
	ResourceManager::loadTexture(renderer, "Res\\Stars.png", "Stars");

	// Single sprite
	/*
	Actor* actor = new Actor();
	SpriteComponent* sprite = new SpriteComponent(*actor, ResourceManager::getTexture("Ship01"));
	actor->setPosition(Vector2{ 100, 100 });
	*/
	
	// Animated sprite
	vector<Texture*> animTextures {
		&ResourceManager::getTexture("Ship01"),
		&ResourceManager::getTexture("Ship02"),
		&ResourceManager::getTexture("Ship03"),
		&ResourceManager::getTexture("Ship04"),
	};
	Actor* ship = new Actor();
	AnimSpriteComponent* animatedSprite = new AnimSpriteComponent(*ship, animTextures);
	ship->setPosition(Vector2{ 100, 300 });

	
	// Background
	// Create the "far back" background
	vector<Texture*> bgTexsFar {
		&ResourceManager::getTexture("Farback01"),
		&ResourceManager::getTexture("Farback02")
	};
	Actor* bgFar = new Actor();
	BackgroundSpriteComponent* bgSpritesFar = new BackgroundSpriteComponent(*bgFar, bgTexsFar);
	bgSpritesFar->setScrollSpeed(-100.0f);
	bgFar->setPosition(Vector2(512.0f, 384.0f));

	// Create the closer background
	Actor* bgClose = new Actor();
	std::vector<Texture*> bgTexsClose {
		&ResourceManager::getTexture("Stars"),
		&ResourceManager::getTexture("Stars")
	};
	BackgroundSpriteComponent* bgSpritesClose = new BackgroundSpriteComponent(*bgClose, bgTexsClose, 50);
	bgSpritesClose->setScrollSpeed(-200.0f);
	bgClose->setPosition(Vector2(512.0f, 384.0f));
	
}

void Game::processInput()
{
	// SDL Event
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		}
	}
	// Keyboard state
	const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
	// Escape: quit game
	if (keyboardState[SDL_SCANCODE_ESCAPE])
	{
		isRunning = false;
	}
}

void Game::update(float dt)
{
	// Update actors 
	isUpdatingActors = true;
	for(auto actor: actors) 
	{
		actor->update(dt);
	}
	isUpdatingActors = false;

	// Move pending actors to actors
	for (auto pendingActor: pendingActors)
	{
		actors.emplace_back(pendingActor);
	}
	pendingActors.clear();

	// Delete dead actors
	vector<Actor*> deadActors;
	for (auto actor : actors)
	{
		if (actor->getState() == Actor::ActorState::Dead)
		{
			deadActors.emplace_back(actor);
		}
	}
	for (auto deadActor : deadActors)
	{
		delete deadActor;
	}
}

void Game::render()
{
	renderer.beginDraw();

	for (auto sprite : sprites)
	{
		sprite->draw(renderer);
	}

	renderer.endDraw();
}

void Game::loop()
{
	Timer timer;
	float dt = 0;
	while (isRunning)
	{
		float dt = timer.computeDeltaTime() / 1000.0f;
		processInput();
		update(dt);
		render();
		timer.delayTime();
	}
}

void Game::unload()
{
	// Delete actors
	// Because ~Actor calls RemoveActor, have to use a different style loop
	while (!actors.empty())
	{
		delete actors.back();
	}

	// Resources
	ResourceManager::clear();
}

void Game::close()
{
	renderer.close();
	window.close();
	SDL_Quit();
}

void Game::addActor(Actor* actor)
{
	if (isUpdatingActors)
	{
		pendingActors.emplace_back(actor);
	}
	else
	{
		actors.emplace_back(actor);
	}
}

void Game::removeActor(Actor* actor)
{
	// Erase actor from the two vectors
	auto iter = std::find(pendingActors.begin(), pendingActors.end(), actor);
	if (iter != pendingActors.end())
	{
		// Swap to end of vector and pop off (avoid erase copies)
		std::iter_swap(iter, pendingActors.end() - 1);
		pendingActors.pop_back();
	}
	iter = std::find(actors.begin(), actors.end(), actor);
	if (iter != actors.end())
	{
		std::iter_swap(iter, actors.end() - 1);
		actors.pop_back();
	}
}

void Game::addSprite(SpriteComponent* sprite)
{
	// Insert the sprite at the right place in function of drawOrder
	int spriteDrawOrder = sprite->getDrawOrder();
	auto iter = sprites.begin();
	for (; iter != sprites.end(); ++iter)
	{
		if (spriteDrawOrder < (*iter)->getDrawOrder()) break;
	}
	sprites.insert(iter, sprite);
}

void Game::removeSprite(SpriteComponent* sprite)
{
	auto iter = std::find(sprites.begin(), sprites.end(), sprite);
	sprites.erase(iter);
}

