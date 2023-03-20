using SDL2;

namespace sdl_cs;

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

        // Event loop
        bool quit = false;
        SDL.SDL_Event e;
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
                    case SDL.SDL_EventType.SDL_KEYDOWN:
                        Console.WriteLine("Key down: " + e.key.keysym.sym);
                        break;
                    case SDL.SDL_EventType.SDL_JOYBUTTONDOWN:
                        Console.WriteLine("Joystick button down: " + e.jbutton.button);
                        break;
                }
            }

            // Clear the screen
            SDL.SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL.SDL_RenderClear(renderer);

            // Update the screen
            SDL.SDL_RenderPresent(renderer);
        }

        // Clean up
        SDL.SDL_DestroyRenderer(renderer);
        SDL.SDL_DestroyWindow(window);
        SDL.SDL_Quit();
    }
}