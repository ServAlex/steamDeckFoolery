namespace sdl_cs;

using System;
using System.Collections.Generic;
using SDL2;
//using SDL_ttf;

class Program
{
	static void Main(string[] args)
	{
		// Initialize SDL
		SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);

		// Create a window
		IntPtr window = SDL.SDL_CreateWindow("SDL Tepst", SDL.SDL_WINDOWPOS_UNDEFINED, SDL.SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL.SDL_WindowFlags.SDL_WINDOW_SHOWN);

		// Create a renderer
		IntPtr renderer = SDL.SDL_CreateRenderer(window, -1, 0);

		// Create a font
		//IntPtr font = SDL_ttf.TTF_OpenFont("arial.ttf", 18);

		// Create a list to store log messages
		//List<string> logs = new List<string>();

		// Event loop
		bool quit = false;
		SDL.SDL_Event e;

		float xOffset = 0;
		float yOffset = 0;
		Dictionary<SDL.SDL_Keycode, bool> buttonsStates = new Dictionary<SDL.SDL_Keycode, bool>();

		while (!quit)
		{
			// Process events
			while (SDL.SDL_PollEvent(out e) != 0)
			{
				
				switch (e.type)
				{
					case SDL.SDL_EventType.SDL_QUIT:
						quit = true;
						break;
					case SDL.SDL_EventType.SDL_KEYUP:
						buttonsStates[e.key.keysym.sym] = false;
						Console.WriteLine("Key up: " + e.key.keysym.sym);
						break;
					case SDL.SDL_EventType.SDL_KEYDOWN:
						buttonsStates[e.key.keysym.sym] = true;
						Console.WriteLine("Key down: " + e.key.keysym.sym);
						break;
					case SDL.SDL_EventType.SDL_JOYBUTTONDOWN:
						Console.WriteLine("Joystick button down: " + e.jbutton.button);
						break;
					case SDL.SDL_EventType.SDL_JOYAXISMOTION:
						Console.WriteLine("Joystick axis: " + e.jaxis.which + " value " + e.jaxis.axisValue);
						break;
				}
			}

			int offset = 50;
			xOffset = 0;
			yOffset = 0;

			if (buttonsStates.ContainsKey(SDL.SDL_Keycode.SDLK_UP) && buttonsStates[SDL.SDL_Keycode.SDLK_UP])
			{
				yOffset += offset;
			}
			
			if (buttonsStates.ContainsKey(SDL.SDL_Keycode.SDLK_DOWN) && buttonsStates[SDL.SDL_Keycode.SDLK_DOWN])
			{
				yOffset -= offset;
			}
			
			if (buttonsStates.ContainsKey(SDL.SDL_Keycode.SDLK_RIGHT) && buttonsStates[SDL.SDL_Keycode.SDLK_RIGHT])
			{
				xOffset += offset;
			}
			
			if (buttonsStates.ContainsKey(SDL.SDL_Keycode.SDLK_LEFT) && buttonsStates[SDL.SDL_Keycode.SDLK_LEFT])
			{
				xOffset -= offset;
			}
			

			// Clear the screen
			SDL.SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL.SDL_RenderClear(renderer);
			
			SDL.SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
			
			var rect = new SDL.SDL_Rect 
			{ 
				x = 320 + (int)xOffset,
				y = 240 - (int)yOffset,
				w = 50,
				h = 50
			};

			SDL.SDL_RenderFillRect(renderer, ref rect);

			// Render log messages
			/*
			SDL.SDL_Color color = new SDL.SDL_Color() { r = 255, g = 255, b = 255, a = 255 };
			int y = 10;
			foreach (string log in logs)
			{
				IntPtr surface = SDL_ttf.TTF_RenderText_Solid(font, log, color);
				int surfaceWidth = 100;
				int surfaceHeight = 100;
				IntPtr texture = SDL.SDL_CreateTextureFromSurface(renderer, surface);
				SDL.SDL_Rect rect = new SDL.SDL_Rect() { x = 10, y = y, w = surfaceWidth, h = surfaceHeight };
				SDL.SDL_RenderCopy(renderer, texture, IntPtr.Zero, ref rect);
				SDL.SDL_DestroyTexture(texture);
				SDL.SDL_FreeSurface(surface);
				y += surfaceHeight + 5;
			}
			*/

			// Update the screen
			SDL.SDL_RenderPresent(renderer);
		}

		// Clean up
		//SDL_ttf.TTF_CloseFont(font);
		SDL.SDL_DestroyRenderer(renderer);
		SDL.SDL_DestroyWindow(window);
		SDL.SDL_Quit();
	}
}