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

Tracker::Tracker()
{
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
		Patterns pat = Patterns();
		pat.SetUp(TrackLength);
		pat.Index = i;
		patterns[i].push_back(pat);
	}
	cout << patterns->size();
}

void Tracker::Run(void)
{
	SetupInstr();

	Authbuf.reserve(128);
	Descbuf.reserve(1024);
	Output.reserve(2048);
	FilePath.reserve(4096);

	bool PlayingTrack = false;
	bool WindowIsGood = true;

	SDL_INIT_AUDIO;
	have.channels = 1;
	have.size = TRACKER_AUDIO_BUFFER;
	have.freq = SPS;
	have.samples = have.freq / have.channels;
	have.silence = 1024;
	have.padding = 512;

	have.format = AUDIO_S16;
	//ImGUI setup
	IMGUI_CHECKVERSION();
	cont = ImGui::CreateContext();
	SetCurrentContext(cont);
	StyleColorsClassic();
	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameBorderSize = 0.4f;
	style.WindowRounding = 1.5f;
	style.FrameRounding = 1.5f;
	style.Colors[ImGuiCol_WindowBg] = Default;
	io = GetIO();
	io.DisplaySize.x = SCREEN_WIDTH;
	io.DisplaySize.y = SCREEN_HEIGHT;
	io.DeltaTime = 1.f / 60.f;
	GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
			ImGui_ImplSDLRenderer2_Init(rend);
			if (window == NULL)
			{
				printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
				WindowIsGood = false;
			}
			else
			{
				//Load fonts
				ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/Inconsolata.ttf", TextSize, NULL, NULL);
				io.Fonts->Build();
				ImGui_ImplSDLRenderer2_CreateFontsTexture();
				WindowIsGood = true;
			}
	}
	ChannelEditState cstate = NOTE;
	//Initialise the tracker
	Initialise(TrackLength);
	while (running) {
		if (WindowIsGood) {
			Render();
		}
		CheckInput();
	}

	//Destroy window
	SDL_DestroyWindow(window);
	DestroyContext();
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
		Event = event;
		switch (event.type)
		{
		case SDL_KEYDOWN:
			Currentkey = event.key.keysym.sym;
			switch (event.key.keysym.sym)
			{
				break;
			case SDL_QUIT:
				running = false;
				break;
				/*
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
				*/
			}
			SG.CheckSound(want, have, dev, Channels);
			SDL_PauseAudioDevice(dev, 0);
			SDL_Delay(io.DeltaTime);

			break;
		case SDL_KEYUP:
			Currentkey = 0;
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
	FrameCount++;
	ImGui_ImplSDL2_NewFrame();
	NewFrame();
	MenuBar();
	DockSpaceOverViewport(NULL);
	if (!ShowCredits)
	{
		//ShowDemoWindow();
		Patterns_View();
		Channel_View();
		Instruments();
		Instrument_View();
		Samples();
		Sample_View();
		Settings_View();
		Misc_View();
		Author_View();
		EchoSettings();
		if (LoadingSample)
		{
			LoadSample();
		}
	}
	else
	{
		Credits();
	}
	SDL_RenderSetScale(rend, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
	ImGui::Render();
	SDL_RenderClear(rend);
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
	SDL_RenderPresent(rend);
	EndFrame();
}

void Tracker::MenuBar()
{
	BeginMainMenuBar();
	if (BeginMenu("File"))
	{
		ImGui::MenuItem("Load");
		ImGui::MenuItem("Save");
		ImGui::MenuItem("Save As");
		ImGui::MenuItem("Export Wav");
		ImGui::MenuItem("Export SPC");
		ImGui::EndMenu();
	}

	if (BeginMenu("Edit"))
	{
		ImGui::EndMenu();
	}

	if (BeginMenu("Settings"))
	{
		if (Selectable("Echo Settings"))
		{
			ShowEcho = !ShowEcho;
		}
		ImGui::EndMenu();
	}

	if (BeginMenu("Help"))
	{
		ImGui::MenuItem("Effects List");
		ImGui::MenuItem("Manual");
		if (Selectable("Credits"))
		{
			ShowCredits = !ShowCredits;
		}
		ImGui::EndMenu();
	}
	ImGui::Text("	|	%.3f ms/frame (%.1f FPS)", 1000.0 / (ImGui::GetIO().Framerate), (ImGui::GetIO().Framerate));
	Text(VERSION.data());
	EndMainMenuBar();

}

void Tracker::Patterns_View()
{
	if (Begin("Patterns"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		Columns(9);
		for (char y = 0; y < SongLength; y++)
		{
			Text(to_string(y).data());
			NextColumn();
			for (char x = 0; x < 8; x++)
			{
				if (Selectable(to_string(patterns[x][y].Index).data())) {
					patterns[x][y].Index++;
					UpdatePatternIndex(x, y);
				}

				NextColumn();
			}
		}
		End();
	}
	else
	{
		End();
	}
}

void Tracker::Instruments()
{
	if (Begin("Instruments"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth()*0.33, 24)))
		{
			Instrument newinst = DefaultInst;
			int index = inst.size();
			newinst.Name += to_string(index);
			inst.push_back(newinst);
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * 0.33, 24)) && inst.size() > 1)
		{
			if (SelectedInst >= inst.size())
			{
				SelectedInst--;
				inst.pop_back();
			}
			else
			{
				inst.pop_back();
			}
		}		
		SameLine();
		if (Button("Copy", ImVec2(GetWindowWidth() * 0.33, 24)) && inst.size() > 1)
		{
			int index = inst.size();
			Instrument newinst = inst[SelectedInst];
			newinst.Name += to_string(index);
			inst.push_back(newinst);
			cout << inst.size();
		}
		//Instrument side bar
		if (inst.size() > 0)
		{
			BeginChild("List", ImVec2(GetWindowWidth() - InstXPadding, GetWindowHeight() - InstYPadding), true, UNIVERSAL_WINDOW_FLAGS);
			BeginTable("InstList", 1, TABLE_FLAGS, ImVec2(GetWindowWidth()*0.75, 24), 24);
			for (char i = 0; i < inst.size(); i++)
			{
				//Show instruments
				Text(to_string(i).data());
				SameLine();
				if (SelectedInst <= inst.size() - 1)
				{
					if (Selectable(inst[i].Name.data()))
					{
						SelectedInst = i;
						ShowInstrument = true;
					}
				}
				else
				{
					SelectedInst = inst.size();
				}
				TableNextColumn();
			}
			EndTable();
			EndChild();
		}
		End();
	}
	else
	{
		End();
	}
}

void Tracker::Instrument_View()
{
	if (ShowInstrument)
	{	
		if (Begin("Instrument Editor"), true, UNIVERSAL_WINDOW_FLAGS)
		{
			if (SelectedInst <= inst.size() - 1)
			{
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * .75);
				InputText("InstName", (char*)inst[SelectedInst].Name.data(), 2048);
				string PrevText = "Choose a sample";
				if (inst[SelectedInst].SampleIndex)
				{
					PrevText = samples[inst[SelectedInst].SampleIndex].SampleName;
				}
				if (BeginCombo("##Sample", PrevText.data())) {
					if (samples.size() > 0)
					{
						bool Selected = false;
						for (int s = 0; s < samples.size(); s++)
						{
							Selected = (inst[SelectedInst].SampleIndex == s);
							if (Selectable(samples[s].SampleName.data(), Selected, 0, ImVec2(GetWindowWidth() * 0.85, TextSize)))
							{
								inst[SelectedInst].SampleIndex = s;
							}
							if (Selected)
							{
								SetItemDefaultFocus();
							}
						}
					}
					EndCombo();
				}

				SliderInt("Volume", &inst[SelectedInst].Volume, 0, 127);
				SliderInt("Gain", &inst[SelectedInst].Gain, 0, 255);

				Checkbox("Envelope used", &inst[SelectedInst].EnvelopeUsed);
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.33);

				if (inst[SelectedInst].EnvelopeUsed)
				{
					NewLine();
					Text("Envelope");
					SliderInt("Attack ", &inst[SelectedInst].Attack, 0, 15);
					SameLine();
					SliderInt("Decay", &inst[SelectedInst].Decay, 0, 7);

					SliderInt("Sustain", &inst[SelectedInst].Sustain, 0, 31);
					SameLine();
					SliderInt("Release", &inst[SelectedInst].Release, 0, 31);


					SliderInt("Left   ", &inst[SelectedInst].LPan, 0, 127);
					SameLine();
					SliderInt("Right", &inst[SelectedInst].RPan, 0, 127);
				}

				NewLine();
				Text("Special");
				NewLine();
				Checkbox("Invert L  ", &inst[SelectedInst].InvL);
				SameLine();
				Checkbox("Invert R", &inst[SelectedInst].InvR);

				Checkbox("Pitch Mod ", &inst[SelectedInst].PitchMod);
				SameLine();
				Checkbox("Echo", &inst[SelectedInst].Echo);

				Checkbox("Noise     ", &inst[SelectedInst].Noise);
				if (inst[SelectedInst].Noise)
				{
					SameLine();
					SliderInt("Noise Freq ", &inst[SelectedInst].NoiseFreq, 0, 31);
				}

				if (Button("Close", ImVec2(64, TextSize*1.5))) {
					ShowInstrument = false;
				}
			}
			else
			{
				//SelectedInst--;
			}
			End();
		}
		else
		{
			ShowInstrument = false;
			End();
		}
	}

}

void Tracker::Channel_View()
{
	if (Begin("Channels"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if(BeginTable("ChannelView",9, TABLE_FLAGS, ImVec2(GetWindowWidth()*.9 + (TextSize*8), 0)));
		{
			//Actual pattern data
			TableNextColumn();
			for (char i = 0; i < 8; i++)//X
			{
				for (char j = 0; j < TrackLength; j++)//Y
				{
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					{
						string str;
						str = "i: ";
						str += to_string(i);
						str += "| j: ";
						str += to_string(j);
						str += "| CurPos: ";
						str += to_string(CursorPos);
						str += "\nCursorX: ";
						str += to_string(CursorX);
						str += "| CursorY: ";
						str += to_string(CursorY);
						SetTooltip(str.data());
					}
					if (i == 0)
					{
						Selectable(to_string(j).data());
						if (j < 10)
						{
							SameLine();
							Text("  ");
						}
						else if (j >= 10 && j < 100)
						{
							SameLine();
							Text(" ");
						}
					}
					else
					{
						//Channel
						if (BeginTable("RowView", 5, 0)) 
						{
							//Row Highlighting
							ImU32 col;
							if (j % Highlight2 == 0)
							{
								col = H2Col;

							}
							else if (j % Highlight1 == 0)
							{
								col = H1Col;

							}
							else
							{
								col = Default;
							}
							TableNextColumn();
							
							TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);

							if (CursorX == i && CursorY == j)//i = x, j = y
							{
								if (IsWindowFocused())
								{
									ChannelInput(CursorPos, i, j);
								}
								switch (CursorPos)
								{
								case 0:
									HoverNote = true;
									HoverInst = false;
									HoverVolume = false;
									HoverEffect = false;
									HoverValue = false;
									break;
								case 1:
									HoverNote = false;
									HoverInst = true;
									HoverVolume = false;
									HoverEffect = false;
									HoverValue = false;
									break;
								case 2:
									HoverNote = false;
									HoverInst = false;
									HoverVolume = true;
									HoverEffect = false;
									HoverValue = false;
									break;
								case 3:
									HoverNote = false;
									HoverInst = false;
									HoverVolume = false;
									HoverEffect = true;
									HoverValue = false;
									break;
								case 4:
									HoverNote = false;
									HoverInst = false;
									HoverVolume = false;
									HoverEffect = false;
									HoverValue = true;
									break;
								}
							}
							else
							{
								HoverNote = false;
								HoverInst = false;
								HoverVolume = false;
								HoverEffect = false;
								HoverValue = false;
							}
							//Cursor highlighting
							if (Selectable(Channels[i].NoteView(j).c_str(), HoverNote, 0, ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4)))
							{
								CursorPos = NOTE;
								CursorX = i;
								CursorY = j;
							}
							TableNextColumn();
							if (Selectable(Channels[i].InstrumentView(j).c_str(), HoverInst, 0, ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4)))
							{
								CursorPos = INSTR;
								CursorX = i;
								CursorY = j;
							}
							TableNextColumn();
							if (Selectable(Channels[i].VolumeView(j).c_str(), HoverVolume, 0, ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4)))
							{
								CursorPos = VOLUME;
								CursorX = i;
								CursorY = j;
							}
							TableNextColumn();
							if (Selectable(Channels[i].EffectView(j).c_str(), HoverEffect, 0, ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4)))
							{
								CursorPos = EFFECT;
								CursorX = i;
								CursorY = j;
							}
							TableNextColumn();
							if (Selectable(Channels[i].Effectvalue(j).c_str(), HoverValue, 0, ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4)))
							{
								CursorPos = VALUE;
								CursorX = i;
								CursorY = j;
							}
							EndTable();
						}
						else
						{
							EndTable();
						}
					}
				}
				TableNextColumn(); 
			}
			EndTable();
			End();
		}
	}
	else
	{
		End();
	}
}

void Tracker::Samples()
{
	if (Begin("Samples"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth() * 0.33, 24)))
		{
			LoadingSample = true;
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * 0.33, 24)) && samples.size() > 1)
		{
			cout << "SAMPLE LIST SIZE: " << samples.size();
			if (SelectedSample > samples.size())
			{
				SelectedSample--;
				samples.pop_back();
			}
			else
			{
				samples.pop_back();
			}
		}
		SameLine();
		if (Button("Copy", ImVec2(GetWindowWidth() * 0.33, 24)) && samples.size() > 1)
		{
			int index = samples.size();
			Sample newsamp = samples[SelectedSample];
			newsamp.SampleName += to_string(index);
			samples.push_back(newsamp);
			cout << samples.size();
		}

		if (samples.size() > 0)	
		{
			BeginChild("SampleList", ImVec2(GetWindowWidth() - InstXPadding, GetWindowHeight() - InstYPadding), true, UNIVERSAL_WINDOW_FLAGS);
			BeginTable("SampleTable", 1, TABLE_FLAGS, ImVec2(GetWindowWidth() * 0.75, 24), 24);
			for (char i = 0; i < samples.size(); i++)
			{
				Text(to_string(i).data());
				SameLine();
				if (SelectedSample <= samples.size() - 1)
				{
					if (Selectable(samples[i].SampleName.data()))
					{
						SelectedSample = i;
						cout << i;
						ShowSample = true;
					}
				}
				else
				{
					SelectedSample = samples.size() - 1;
				}
				TableNextColumn();
			}
			EndTable();
			EndChild();
		}
		End();
	}
	else
	{
		End();
	}
}

void Tracker::Sample_View()
{
	if (ShowSample)
	{
		if (Begin("Sample view"), true, UNIVERSAL_WINDOW_FLAGS)
		{
			if (SelectedSample > samples.size() - 1)
			{
				SelectedSample = samples.size() - 1;
			}
			BeginChild("SampleTable",ImVec2(GetWindowWidth()*0.95, GetWindowHeight() * 0.85),UNIVERSAL_WINDOW_FLAGS);
			NextColumn();
			InputText("Sample Name", (char*)samples[SelectedSample].SampleName.data(), 2048);
			InputInt("Playing HZ", &samples[SelectedSample].SampleRate);
			InputInt("Fine Tune", (int*) &samples[SelectedSample].FineTune, 1,1);
			InputInt("Loop Start", (int*)&samples[SelectedSample].LoopStart,16, 0);
			InputInt("Loop End", (int*)&samples[SelectedSample].LoopEnd, 16, 0);
			SliderInt("Note offset", &samples[SelectedSample].NoteOffset, -12, 12);
			vector<float> SampleView;
			for (size_t i = 0; i < samples[SelectedSample].SampleData.size(); i++)
			{
				SampleView.push_back(samples[SelectedSample].SampleData[i]);
			}
			PlotLines("Waveform", SampleView.data(), (size_t)samples[SelectedSample].SampleData.size(), 0, "Waveform", -32768, 32767, ImVec2(GetWindowWidth() * 0.9, GetWindowHeight() * 0.5));
			if (Button("Close", ImVec2(64, TextSize * 1.5))) {
				ShowSample = false;
			}
			EndChild();
			End();
		}
		else
		{
			End();
		}
	}
}

void Tracker::Settings_View()
{
	if (ShowSettings)
	{
		if (Begin("SettingsView"),true, UNIVERSAL_WINDOW_FLAGS)
		{
			End();
		}
		else
		{
			End();
		}
	}
}

void Tracker::Misc_View()
{
	if (Begin("MiscView"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		InputInt("Step", &Step, 1, 8);
		InputInt("Octave", &Octave, 1, 8);
		SliderInt("Volume Scale Left", &VolumeScaleL, 0, 127);
		SliderInt("Volume Scale Right", &VolumeScaleR, 0, 127);
		End();
	}
	else
	{
		End();
	}
}

void Tracker::Author_View()
{
	if (Begin("Author"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		InputInt("Base tempo", &BaseTempo,1, 1);
		InputInt("Tempo divider", &TempoDivider,1,1);
		if (BaseTempo < 1)
		{
			BaseTempo = 1;
		}
		if (TempoDivider < 1)
		{
			TempoDivider = 1;
		}
		string temp = "Tempo: ";
		temp += to_string(BaseTempo / TempoDivider);
		Text(temp.data());
		InputInt("Highlight 1", &Highlight1, 1, 1);
		InputInt("Highlight 2", &Highlight2, 1, 1);
		if (Highlight1 < 1)
		{
			Highlight1 = 1;
		}
		if (Highlight2 < 1)
		{
			Highlight2 = 1;
		}
		NewLine();
		InputText("Author", (char*)Authbuf.data(), 1024);
		InputTextMultiline("Desc", (char*)Descbuf.data(), 1024,ImVec2(GetWindowWidth() * 0.5, GetWindowHeight() * 0.9));
		End();
	}
	else
	{
		End();
	}
}

void Tracker::EchoSettings()
{
	if (ShowEcho)
	{
		if (Begin("EchoSettings"),true, UNIVERSAL_WINDOW_FLAGS)
		{
			SliderInt("Delay", &Delay, 0, 15);
			SliderInt("Feedback", &Feedback, 0, 127);

			SliderInt("Echo Volume Left", &EchoVolL, -128, 127);
			SliderInt("Echo Volume Right", &EchoVolR, -128, 127);

			Text("Echo filter");
			for (int i = 0; i < 8; i++)
			{
				SliderInt(to_string(i).data(), &EchoFilter[i], -128, 127);
			}
			if (Button("Close", ImVec2(64, TextSize * 1.5))) {
				ShowEcho = false;
			}
			End();
		}
	}
}

void Tracker::Credits()
{
	if (ShowCredits)
	{
		if (Begin("Credits"), true, UNIVERSAL_WINDOW_FLAGS)
		{
			Text("Tracker Code: Crisps, Alexmush");
			Text("Emulator Code: SPC Player");
			Text("Driver Code: Nobody yet!!!");
			End();
		}
	}
}

void Tracker::SetupInstr()
{
	DefaultInst.Index = 1;
	DefaultInst.Name = "Instrument: ";
	DefaultInst.SampleIndex = 0;
	DefaultInst.Volume = 127;
	DefaultInst.LPan = 127;
	DefaultInst.RPan = 127;
	DefaultInst.NoiseFreq = 0;
	DefaultInst.Gain = 0;
	DefaultInst.InvL = false;
	DefaultInst.InvR = false;
	DefaultInst.PitchMod = false;
	DefaultInst.Echo = false;
	DefaultInst.Noise = false;
	inst.push_back(DefaultInst);

	DefaultSample.SampleIndex = 0;
	DefaultSample.SampleName = "Sample: 0";
	DefaultSample.FineTune = 0;
	DefaultSample.SampleRate = 0;
	DefaultSample.Loop = false;
	DefaultSample.LoopStart = 0;
	DefaultSample.LoopEnd = 0;
	DefaultSample.SampleData.clear();
	DefaultSample.BRRSampleData.clear();
	samples.push_back(DefaultSample);
}

void Tracker::ChannelInput(int CurPos, int x, int y)
{
	if (Event.type == SDL_KEYDOWN)
	{
		if (!IsPressed)
		{
			if (Currentkey == SDLK_LEFT)
			{
				CursorPos--;
			}
			else if (Currentkey == SDLK_RIGHT)
			{
				CursorPos++;
				}
				if (Currentkey == SDLK_DOWN)
			{
				CursorY++;
			}
			else if (Currentkey == SDLK_UP)
			{
				CursorY--;
			}
		
				if (EditingMode)
				{
					switch (CurPos)
					{
					default:
						break;
					case NOTE:
						for (char i = 0; i < 24; i++)
						{
							if (Currentkey == NoteInput[i])
							{
								if (i < 13)
								{
									Channels[x].Rows[y].note = i + (12 * (Octave - 1));
									Channels[x].Rows[y].octave = (Octave - 1);
								}
								else
								{
									Channels[x].Rows[y].note = i + (12 * Octave);
									Channels[x].Rows[y].octave = Octave;
								}
								CursorY += Step;
								if (CursorY >= TrackLength)
								{
									CursorY = TrackLength - 1;
								}
								break;
							}
						}

						if (Currentkey == SDLK_DELETE)
						{
							Channels[x].Rows[y].note = MAX_VALUE;
							CursorY += Step;
						}
						break;
					case INSTR:
						for (char i = 0; i < 16; i++)
						{
							if (Currentkey == VolInput[i])
							{
								Channels[x].Rows[y].instrument = Channels[x].EvaluateHexInput(i, y, 127, INSTR);
								CursorY += Step;
								break;
							}
						}
						break;
					case VOLUME:
						for (char i = 0; i < 16; i++)
						{
							if (Currentkey == VolInput[i])
							{
								Channels[x].Rows[y].volume = Channels[x].EvaluateHexInput(i, y, 127, VOLUME);
								CursorY += Step;
								break;
							}
						}
						break;
					case EFFECT:
						for (char i = 0; i < 16; i++)
						{
							if (Currentkey == VolInput[i])
							{
								Channels[x].Rows[y].effect = Channels[x].EvaluateHexInput(i, y, 255, EFFECT);
								CursorY += Step;
								break;
							}
						}
						break;
					case VALUE:
						for (char i = 0; i < 16; i++)
						{
							if (Currentkey == VolInput[i])
							{
								Channels[x].Rows[y].effectvalue = Channels[x].EvaluateHexInput(i, y, 255, VALUE);
								CursorY += Step;
								break;
							}
						}
						break;
					}
					if (CursorY >= TrackLength)
					{
						CursorY = TrackLength - 1;
					}
					else if (CursorY < 0)
					{
						CursorY = 0;
					}
				}
			IsPressed = true;
		}
	}
	else if (Event.type == SDL_KEYUP)
	{
		IsPressed = false;
	}

	if (CurPos > VALUE)
	{
		CursorX += (CurPos % VALUE);
		CursorPos -= 5 * (CurPos % VALUE);
	}
	else if (CurPos < 0  && CursorX > 1)
	{
		CursorX--;
		CursorPos = VALUE;
	}
	else if (CursorX <= 1 && CurPos < 0)
	{
		CursorX = 1;
		CurPos = 0;
	}

	if (CursorY < 0)
	{
		CursorY = 0;
	}
}

void Tracker::LoadSample()
{
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".wav", ".");
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			cout << "\n" + FileName + "\n" + FilePath;
			//auto file = SDL_RWFromFile(FileName.data(), "rb");

			SF_INFO soundinfo;
			soundinfo.format = AUDIO_FORMATS;
			soundinfo.samplerate = SPS;
			soundinfo.channels = 1;
			soundinfo.frames = TRACKER_AUDIO_BUFFER;

			SNDFILE* file = sf_open(FileName.data(), SFM_READ, &soundinfo);
			if (file)//Loaded right
			{
				short FileBuffer[8192];
				Uint32 AudioLen = soundinfo.frames * soundinfo.channels;
				if (soundinfo.channels == 1)
				{
					sf_read_short(file, FileBuffer, AudioLen);
				}
				else if (soundinfo.channels > 1)
				{
					DownMix(file, soundinfo, FileBuffer);
				}
				
				if (FileBuffer != 0)
				{
					Sample cur;
					cur = DefaultSample;
					for (size_t i = 0; i < AudioLen/soundinfo.channels; i++)
					{
						cout << "\n" << i;
						/*
						cout << " " << FileBuffer[i];
						if (i % 8 == 1)
						{
							cout << "\n";
						}
						*/
						cur.SampleData.push_back(FileBuffer[i]);
					}
					cur.SampleIndex = SelectedSample;
					cur.SampleName = "Sample: ";
					cur.SampleRate = soundinfo.samplerate;
					cur.Loop = false;
					cur.LoopStart = 0;
					cur.LoopEnd = 0;
					cur.FineTune = 0;
					SelectedSample = samples.size();
					cur.SampleIndex = SelectedSample;
					samples.push_back(cur);
					sf_close(file);

				}
				else
				{
					sf_close(file);
					cout << "\n ERROR: FILE IS EITHER EMPTY OR IS OTHERWISE UNABLE TO LOAD \n ";
				}
			}
			else//fucked the file
			{
			}

		}

		cout << FileName + "\n" + FilePath;
		// close
		ImGuiFileDialog::Instance()->Close();
		LoadingSample = false;
	}
}

void Tracker::DownMix(SNDFILE* sndfile, SF_INFO sfinfo, Sint16 outputBuffer[])
{
	//Thank you AlexMush for the downmixing code :]
	Sint16 constexpr sampleBufferSize = 8192;
	int sampleLength = sfinfo.frames / sfinfo.channels;
	vector<Sint16> sampleBuffer;
	sampleBuffer.reserve(sampleBufferSize * sfinfo.channels);
	Sint64 sum;
	for (int a = 0; a < sampleLength; a += sampleBufferSize) {
		sf_read_short(sndfile, sampleBuffer.data(), sampleBufferSize);
		for (int i = 0; i < sampleBufferSize; i++) {
			sum = 0;
			for (int j = 0; j < sfinfo.channels; j++)
				sum += sampleBuffer.data()[i * sfinfo.channels + j];
			outputBuffer[i] = (Sint16)(sum / (double)sfinfo.channels);
		}
	}

}

void Tracker::UpdatePatternIndex(int x, int y)
{
	if (patterns[x][y].Index > Maxindex)
	{
		Maxindex = patterns[x][y].Index;
		Patterns pat;
		pat = DefaultPattern;
		patterns->push_back(pat);

	}
	for (char i = 0; i < TrackLength; i++)
	{
		Channels[x].Rows[i].note = patterns[x][y].SavedRows[i].note;
		Channels[x].Rows[i].volume = patterns[x][y].SavedRows[i].volume;
		Channels[x].Rows[i].effect = patterns[x][y].SavedRows[i].effect;
		Channels[x].Rows[i].effectvalue = patterns[x][y].SavedRows[i].effectvalue;
	}
}
/*
if (Begin("Main"), true, UNIVERSAL_WINDOW_FLAGS)
{
	Text("This is some text");
	BeginChild("Sub", ImVec2(240, 120), true, UNIVERSAL_WINDOW_FLAGS);
		Text("More text");
		EndChild();
	End();
}
else
{
	End();
}

//ImGui::ShowDemoWindow();
{
	Begin("Main Window", &ShowMain);

	Text("This is some useful text.");               // Display some text (you can use a format strings too)
	Checkbox("Demo Window", &ShowMain);      // Edit bools storing our window open/close state
	Checkbox("Another Window", &ShowMain);

	End();
}
*/