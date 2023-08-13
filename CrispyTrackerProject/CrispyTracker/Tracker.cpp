#include "Tracker.h"
#include "SoundGenerator.h"

//Universal variables here
SoundGenerator SG(1, 56, 1);
bool running = true;

//Screen dimension constants
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

//The surface contained by the window
SDL_Surface* screenSurface = NULL;

SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;
cont = ImGui::CreateContext();
ImGuiIO& io = ImGui::GetIO();

Tracker::Tracker()
{
	Run();
}

Tracker::~Tracker()
{
}

void Tracker::Initialise(int StartLength)
{
	//initialise all the channels
	for (size_t i = 0; i < 8; i++)
	{
		Channel channel = Channel();
		channel.SetUp(StartLength);
		Channels[i] = channel;
	}
}

void Tracker::Run()
{
	bool PlayingTrack = false;
	bool WindowIsGood = true;

	//ImGUI setup
	IMGUI_CHECKVERSION();
	cont = ImGui::CreateContext();
	ImGui::SetCurrentContext(cont);
	ImGui::StyleColorsDark();

	io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	io.DisplaySize.x = SCREEN_WIDTH;
	io.DisplaySize.y = SCREEN_HEIGHT;
	io.DeltaTime = 1.f / 60.f;
	
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		WindowIsGood = false;
	}
	else
	{
		//Create window
		window = SDL_CreateWindow("CrispyTracker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
		rend = SDL_CreateRenderer(window, 0, SDL_RENDERER_PRESENTVSYNC);
		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForSDLRenderer(window, rend);
		
		if (window == NULL)
		{
			printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			WindowIsGood = false;
		}
		else
		{
			//Load fonts
			io.Fonts->AddFontDefault();
			io.Fonts->Build();
			WindowIsGood = true;
		}
	}

	//Initialise the tracker
	Initialise(8);
	while (running) {
		if (WindowIsGood) {
			Render();
		}
		CheckInput();
	}

	//Destroy window
	SDL_DestroyWindow(window);
	ImGui::DestroyContext();
	//Quit SDL subsystems
	SDL_Quit();
}

void Tracker::CheckInput()
{
	int TuninOff = 48;
	SDL_Event event;
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	if (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			default:
				return;
				break;
			case SDL_QUIT:
				running = false;
				break;
				//upper octave
			case SDLK_q:
				SG.NoteIndex = TuninOff;
				printf("PRESSED Q");
				SG.PlayingNoise = true;
				break;
			case SDLK_2:
				SG.NoteIndex = TuninOff + 1;
				printf("PRESSED 1");
				SG.PlayingNoise = true;
				break;
			case SDLK_w:
				SG.NoteIndex = TuninOff + 2;
				printf("PRESSED W");
				SG.PlayingNoise = true;
				break;
			case SDLK_3:
				SG.NoteIndex = TuninOff + 3;
				printf("PRESSED 3");
				SG.PlayingNoise = true;
				break;
			case SDLK_e:
				SG.NoteIndex = TuninOff + 4;
				printf("PRESSED E");
				SG.PlayingNoise = true;
				break;
			case SDLK_r:
				SG.NoteIndex = TuninOff + 5;
				printf("PRESSED R");
				SG.PlayingNoise = true;
				break;
			case SDLK_5:
				SG.NoteIndex = TuninOff + 6;
				printf("PRESSED 5");
				SG.PlayingNoise = true;
				break;
			case SDLK_t:
				SG.NoteIndex = TuninOff + 7;
				printf("PRESSED T");
				SG.PlayingNoise = true;
				break;
			case SDLK_6:
				SG.NoteIndex = TuninOff + 8;
				printf("PRESSED 6");
				SG.PlayingNoise = true;
				break;
			case SDLK_y:
				SG.NoteIndex = TuninOff + 9;
				printf("PRESSED Y");
				SG.PlayingNoise = true;
				break;
			case SDLK_7:
				SG.NoteIndex = TuninOff + 10;
				printf("PRESSED 7");
				SG.PlayingNoise = true;
				break;
			case SDLK_u:
				SG.NoteIndex = TuninOff + 11;
				printf("PRESSED U");
				SG.PlayingNoise = true;
				break;
			case SDLK_i:
				SG.NoteIndex = TuninOff + 12;
				printf("PRESSED I");
				SG.PlayingNoise = true;
				break;
			}
			SG.CheckSound(want, have, dev, Channels);
			SDL_PauseAudioDevice(dev, 0);
			SDL_Delay(1);

			break;
		case SDL_KEYUP:
			SG.PlayingNoise = false;
			SDL_PauseAudioDevice(dev, 1);
			break;

		case SDL_QUIT:
			running = false;
			break;
		}
	}
}

void Tracker::Render()
{
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	{
		ImGui::Begin("Main Window", &ShowMain);

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &ShowMain);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &ShowMain);

		ImGui::End();
	}
	ImGui::Render();
	SDL_RenderSetScale(rend, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	SDL_SetRenderDrawColor(rend,22, 22, 22, 255);
	SDL_RenderClear(rend);
	ImGui::EndFrame();
}
