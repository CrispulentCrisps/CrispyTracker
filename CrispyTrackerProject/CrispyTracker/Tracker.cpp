#include "Tracker.h"
#include "SoundGenerator.h"

//Universal variables here
bool running = true;

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

double Tracker::ScrollValue()
{
	return (((double)CursorY / (double)TrackLength)) * ((TextSize + GetStyle().CellPadding.y * 2) * TrackLength) - (GetWindowHeight() / 3.0);;
}

void Tracker::Initialise(int StartLength)
{
	//initialise all the channels
	for (int i = 0; i < 8; i++)
	{
		Channel channel = Channel();
		channel.SetUp(256);
		Channels[i] = channel;
		Patterns pat = Patterns();
		pat.SetUp(256);
		pat.Index = i;
		patterns[i].push_back(pat);
		StoragePatterns.push_back(pat);
	}

	PlotColours = ImPlot::AddColormap("RGBColors", colorDataRGB, 32);

	SManager.CreateDefaultSettings();
	SManager.CheckSettingsFolder();
	UpdateSettings();
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	Tracker* tr = static_cast<Tracker*>(glfwGetWindowUserPointer(window));
	tr->Currentkey = key;
	tr->CurrentMod = mods;
	tr->Event = action;
}

void MyAudioCallback(void* userdata, Uint8* stream, int len)
{
	DWORD flags;
	static_cast<SoundGenerator*>(userdata)->LoadData(len / 8, stream, &flags);
}

void Tracker::Run()
{	
	SetupInstr();
	Emu_APU.APU_Startup();
	glfwInit();
	Authbuf.reserve(4096);
	Descbuf.reserve(4096);
	FilePath.reserve(4096);

	bool PlayingTrack = false;
	bool WindowIsGood = true;
	//Create window
#if CT_UNIX
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, Credits.data(), NULL, NULL);
	// Setup Platform/Renderer backends

	glfwMakeContextCurrent(window);	
	glfwSwapInterval(1); // Enable vsync

	glfwSetWindowUserPointer(window, this);

	//ImGUI setup
	IMGUI_CHECKVERSION();
	cont = ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	GetIO().AddKeyEvent(ImGuiKey_Backspace, false);
	GetIO().AddKeyEvent(ImGuiKey_PageUp, false);
	GetIO().AddKeyEvent(ImGuiKey_PageDown, false);
	GetIO().AddKeyEvent(ImGuiKey_Home, false);
	GetIO().AddKeyEvent(ImGuiKey_End, false);
	StyleColorsClassic();
	ImGuiStyle& style = GetStyle();
	style.FrameBorderSize = 0.4f;
	style.WindowRounding = 1.5f;
	style.FrameRounding = 1.5f;
	//style.Colors[ImGuiCol_WindowBg] = Default;
	io = GetIO();
	io.DisplaySize.x = SCREEN_WIDTH;
	io.DisplaySize.y = SCREEN_HEIGHT;
	io.DeltaTime = 1.f / FPS;
	GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	if (window == NULL)
	{
		printf("Window could not be created! ERROR: %s\n", SDL_GetError());
		WindowIsGood = false;
	}
	else
	{
		SDL_Init(SDL_INIT_AUDIO);
		have.channels = 2;
		have.size = TRACKER_AUDIO_BUFFER;
		have.freq = SPS;
		have.samples = have.freq / have.channels;
		have.silence = 0;
		have.padding = 512;
		have.format = AUDIO_S16;

		SDL_memset(&have, 0, sizeof(have)); /* or SDL_zero(want) */
		have.freq = AUDIO_RATE;//Coming from the SoundGenerator class
		have.format = AUDIO_S16;
		have.channels = 2;
		have.samples = TRACKER_AUDIO_BUFFER;//Coming from the SoundGenerator class
		have.userdata = &SG;

		SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
		want.freq = AUDIO_RATE;//Coming from the SoundGenerator class
		want.format = AUDIO_S16;
		want.channels = 2;
		want.samples = TRACKER_AUDIO_BUFFER;//Coming from the SoundGenerator class
		want.callback = NULL;
		want.userdata = &SG;
		want.silence = TRACKER_AUDIO_BUFFER;

		dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		const char* err = SDL_GetError();

		SG.PlayingNoise = true;
		//Load fonts
		font = io.Fonts->AddFontFromFileTTF("fonts/Manaspace.ttf", TextSize, NULL, NULL);
		Largefont = io.Fonts->AddFontFromFileTTF("fonts/Manaspace.ttf", TextSizeLarge, NULL, NULL);
		io.Fonts->Build();
		ImGui_ImplOpenGL3_CreateFontsTexture();
		WindowIsGood = true;
		for (int i = 0; i < 8; i++)
		{
			SG.ch[i] = &Channels[i];
		}

		x = ImAxis_X1;
		y = ImAxis_Y1;
	}
	SDL_QueueAudio(dev, SG.Totalbuffer, sizeof(SG.Totalbuffer));

	ChannelEditState cstate = NOTE;
	//Initialise the tracker
	Initialise(TrackLength);
	SG.DEBUG_Open_File();
	while (running) {
		if (WindowIsGood) {
			Render();
			SDL_PauseAudioDevice(dev, !PlayingMode);
		}
		CheckInput();
	}
	SManager.CloseSettingsStream();
	SG.DEBUG_Close_File();
	//Destroy window
	ImGui::DestroyContext();
	ImPlot::DestroyContext();
	glfwDestroyWindow(window);
	//Quit SDL subsystems
	ImGui_ImplOpenGL3_Shutdown();
	glfwTerminate();
	SDL_Quit();
}

void Tracker::CheckInput()
{
	int TuninOff = 48;
	glfwPostEmptyEvent();
	if (glfwWindowShouldClose(window))
	{
		running = false;
	}
	
	//cout << "Playing Mode = " << PlayingMode;
	if (PlayingMode)
	{
		RunTracker();
		
	}
	else
	{
		SDL_ClearQueuedAudio(dev);
	}
}

void Tracker::Render()
{
	IDOffset = 0;
	FrameCount++;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();
	DockSpaceOverViewport(NULL);
	if (!ShowCredits)
	{
		MenuBar();
		Author_View();
		//ShowDemoWindow();
		Patterns_View();
		Channel_View();
		Instruments();
		Instrument_View();
		Samples();
		Sample_View();
		Settings_View();
		Misc_View();
		EchoSettings();
		if (LoadingSample)
		{
			LoadSample();
		}
		Info_View();
	}
	else
	{
		CreditsWindow();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
	glfwWaitEvents();
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
		if (ImGui::MenuItem("Show Settings"))
		{
			ShowSettings = !ShowSettings;
		}
		ImGui::EndMenu();
	}

	if (BeginMenu("Help"))
	{
		ImGui::MenuItem("Effects List");
		ImGui::MenuItem("Manual");
		if (ImGui::MenuItem("Credits")) ShowCredits = true;
		ImGui::EndMenu();
	}
	
	if (BeginMenu("SNES"))
	{
		if (Selectable("Echo Settings"))
		{
			ShowEcho = !ShowEcho;
		}
		ImGui::EndMenu();
	}
	
	Text("	|	%.3f ms/frame (%.1f FPS)", 1000.0 / (ImGui::GetIO().Framerate), (ImGui::GetIO().Framerate));
	Text(VERSION.data());
	EndMainMenuBar();

}

void Tracker::CreditsWindow()
{
	Begin("CreditsWindow"), true, UNIVERSAL_WINDOW_FLAGS;
	PushFont(Largefont);
	Text("CREDITS");
	PopFont();
	NewLine();
	NewLine();
	Text("Code:");
	
	BulletText("Crisps");
	BulletText("Alexmush");
	BulletText("Euly");

	NewLine();
	Text("Emulator Code:");
	BulletText("snes_spc by John Reagan, fork of the SPC emulation from Blargg");

	NewLine();
	Text("Driver Code:");
	BulletText("Kulor");

	if (Button("Back to editor", ImVec2(128, TextSize * 1.5))) ShowCredits = false;
	
	End();
}

void Tracker::Patterns_View()
{
	if (Begin("Patterns"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		Columns(2);

		if (BeginTable("PatternsTable", 9, TABLE_FLAGS)) {
			for (int y = 0; y < SongLength; y++)
			{
				TableNextRow();
				TableNextColumn();
				if (Selectable(to_string(y).data())) {
					SelectedPattern = y;
					UpdateAllPatterns();
				}
				TableNextColumn();

				//Row Highlighting
				ImU32 col;
				if (SelectedPattern == y)
				{
					col = H1Col;
				}
				else
				{
					col = Default;
				}
				TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);

				for (int x = 0; x < 8; x++)
				{
					PushID(IDOffset);
					Selectable(to_string(patterns[x][y].Index).data());

					if (IsItemClicked(ImGuiMouseButton_Right)) {
						patterns[x][y].Index > 0 ? patterns[x][y].Index-- : patterns[x][y].Index = 0;
						SelectedPattern = y;
						UpdatePatternIndex(x, y);
						UpdateAllPatterns();
					}
					else if (IsItemClicked(ImGuiMouseButton_Left)) {
						patterns[x][y].Index++;
						SelectedPattern = y;
						UpdatePatternIndex(x, y);
						UpdateAllPatterns();
					}

					PopID();
					IDOffset++;
					TableNextColumn();
				}
			}
			EndTable();


			NextColumn();
			PushFont(Largefont);
			if (Button("+", ImVec2(TextSizeLarge, TextSizeLarge)))
			{
				for (int i = 0; i < 8; i++)
				{
					Patterns pat;
					pat = DefaultPattern;
					pat.Index = Maxindex + i;
					//cout << "\n SavedRows: " << pat.SavedRows.size();
					patterns[i].push_back(pat);
					StoragePatterns.push_back(pat);
				}
				Maxindex += 8;
				SongLength++;
			}
			if (IsItemHovered())
			{
				PopFont();
				PushFont(font);
				SetTooltip("Add a new pattern");
				PopFont();
				PushFont(Largefont);
			}
			if (Button("-", ImVec2(TextSizeLarge, TextSizeLarge)))
			{
				if (SongLength > 1)
				{
					SelectedPattern >= SongLength ? SelectedPattern-- : SelectedPattern = SelectedPattern;
					for (int i = 0; i < 8; i++)
					{
						patterns[i].erase(patterns[i].begin() + SelectedPattern);
					}
					SongLength--;
				}
			}
			if (IsItemHovered())
			{
				PopFont();
				PushFont(font);
				SetTooltip("Delete a new pattern");
				PopFont();
				PushFont(Largefont);
			}
			if (Button("=", ImVec2(TextSizeLarge, TextSizeLarge)))
			{
				for (int i = 0; i < 8; i++)
				{
					Patterns pat;
					pat = patterns[i][SelectedPattern];
					//cout << "\n SavedRows: " << pat.SavedRows.size();
					patterns[i].push_back(pat);
				}
				Maxindex += 8;
				SongLength++;
			}
			if (IsItemHovered())
			{
				PopFont();
				PushFont(font);
				SetTooltip("Copy a selected pattern");
				PopFont();
				PushFont(Largefont);
			}
			PopFont();
			End();
		}
		else
		{
			End();
		}
	}
}

void Tracker::Instruments()
{
	if (Begin("Instruments"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth()*0.3, 24)))
		{
			Instrument newinst = DefaultInst;
			int index = inst.size();
			newinst.Name += to_string(index);
			newinst.Index = index;
			inst.push_back(newinst);
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * 0.3, 24)) && inst.size() > 1)
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
		if (Button("Copy", ImVec2(GetWindowWidth() * 0.3, 24)) && inst.size() > 1)
		{
			int index = inst.size();
			Instrument newinst = inst[SelectedInst];
			newinst.Name += to_string(index);
			newinst.Index = index;
			inst.push_back(newinst);
			cout << inst.size();
		}
		//Instrument side bar
		if (inst.size() > 0)
		{
			//BeginChild("List", ImVec2(GetWindowWidth() - InstXPadding, GetWindowHeight() - InstYPadding), true, UNIVERSAL_WINDOW_FLAGS);
			if (BeginTable("InstList", 1, TABLE_FLAGS, ImVec2(GetWindowWidth() * 0.75, 24), 24))
			{
				for (int i = 0; i < inst.size(); i++)
				{
					PushID(IDOffset);
					//Show instruments
					Text(to_string(i).data());
					SameLine();
					if (SelectedInst <= inst.size() - 1)
					{
						if (Selectable(inst[i].Name.data(), SelectedInst == i))
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
					PopID();
					IDOffset++;
				}
				EndTable();
			}
			//EndChild();
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
				PushItemWidth(GetWindowWidth() * .75);
				InputText("InstName", (char*)inst[SelectedInst].Name.data(), 2048);
				string PrevText = "Choose a sample";

				if (inst[SelectedInst].SampleIndex < samples.size())
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
					for (int i = 0; i < 4; i++)
					{
						if (RadioButton(ADSRNames[i].data(), &inst[SelectedInst].ADSRType,i))
						{
							inst[SelectedInst].ADSRType = i;
						}
					}

					if (inst[SelectedInst].ADSRType == 0)
					{
						PushStyleColor(ImGuiCol_Text, AttackColour);
						SliderInt("Attack ", &inst[SelectedInst].Attack, 0, 15);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, DecayColour);
						SliderInt("Decay", &inst[SelectedInst].Decay, 0, 7);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, SustainColour);
						SliderInt("Sustain", &inst[SelectedInst].Sustain, 0, 7);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, ReleaseColour);
						SliderInt("Release", &inst[SelectedInst].Release, 0, 31);
						PopStyleColor();
						float PlotArr[64] = { 
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0, 
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0
						};

						for (int i = 0; i < inst[SelectedInst].Attack; i++)
						{
							PlotArr[i] = (float)i / (inst[SelectedInst].Attack + 1)*32;
						}
						PlotArr[inst[SelectedInst].Attack] = 32;
						
						int PreviousPoint = inst[SelectedInst].Attack+1;

						for (int j = PreviousPoint; j < PreviousPoint+(8-inst[SelectedInst].Decay); j++)
						{
							PlotArr[j] = ((inst[SelectedInst].Decay*4)/(((float)j + 1 - PreviousPoint))) + (inst[SelectedInst].Sustain*4);
						}
						
						PreviousPoint += (8 - inst[SelectedInst].Decay);
						
						for (int k = PreviousPoint; k < PreviousPoint+inst[SelectedInst].Release; k++)
						{
							if (inst[SelectedInst].Sustain > 0)
							{
								PlotArr[k] = (inst[SelectedInst].Sustain * 4) - ((((float)k - PreviousPoint)*32) / inst[SelectedInst].Release);
							}
							else
							{
								PlotArr[k] = 0;
							}
						}
						//float PlotArr[5] = { 0, inst[SelectedInst].Attack*2 , inst[SelectedInst].Decay*4 , inst[SelectedInst].Sustain*4 , inst[SelectedInst].Release};
						float PlotBuff[256];
						NewLine();
						PlotLines("##Envelope output", PlotArr, 64, 0, "Envelope output", 0, 32, ImVec2(GetWindowWidth() * 0.75, GetWindowHeight() * 0.25));
					}
					else
					{
						PushStyleColor(ImGuiCol_Text, AttackColour);
						SliderInt("Attack ", &inst[SelectedInst].Attack, 0, 15);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, DecayColour);
						SliderInt("Decay", &inst[SelectedInst].Decay, 0, 7);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, SustainColour);
						SliderInt("Sustain", &inst[SelectedInst].Sustain, 0, 7);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, DecayColour);
						SliderInt("Decay 2", &inst[SelectedInst].Decay2, 0, 31);
						PopStyleColor();
						PushStyleColor(ImGuiCol_Text, ReleaseColour);
						SliderInt("Release", &inst[SelectedInst].Release, 0, 31);
						PopStyleColor();
						float PlotArr[64] = {
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0,
						0, 0, 0, 0, 0, 0, 0, 0
						};

						for (int i = 0; i < inst[SelectedInst].Attack; i++)
						{
							PlotArr[i] = (float)i / (inst[SelectedInst].Attack + 1) * 32;
						}
						PlotArr[inst[SelectedInst].Attack] = 32;

						int PreviousPoint = inst[SelectedInst].Attack + 1;

						for (int j = PreviousPoint; j < PreviousPoint + (8 - inst[SelectedInst].Decay); j++)
						{
							PlotArr[j] = ((inst[SelectedInst].Decay * 4) / (((float)j - PreviousPoint + 1))) + (inst[SelectedInst].Sustain * 4);
						}

						PreviousPoint += (8 - inst[SelectedInst].Decay);

						for (int k = PreviousPoint; k < PreviousPoint + inst[SelectedInst].Decay2; k++)
						{
							if (inst[SelectedInst].Sustain > 0)
							{
								PlotArr[k] = (inst[SelectedInst].Sustain * 4) - ((((float)k - PreviousPoint) * 32) / inst[SelectedInst].Decay2);
							}
							else
							{
								PlotArr[k] = 0;
							}
						}
						//float PlotArr[5] = { 0, inst[SelectedInst].Attack*2 , inst[SelectedInst].Decay*4 , inst[SelectedInst].Sustain*4 , inst[SelectedInst].Release};
						float PlotBuff[256];
						PlotLines("##Envelope output", PlotArr, 64, 0, "Envelope output", 0, 32, ImVec2(GetWindowWidth() * 0.75, GetWindowHeight() * 0.25));
					}

					NewLine();
					SliderInt("Left   ", &inst[SelectedInst].LPan, 0, 127);
					SameLine();
					SliderInt("Right", &inst[SelectedInst].RPan, 0, 127);
					
				}

				Text("Special");
				NewLine();
				Checkbox("Invert L  ", &inst[SelectedInst].InvL);
				Checkbox("Invert R", &inst[SelectedInst].InvR);

				Checkbox("Pitch Mod ", &inst[SelectedInst].PitchMod);
				Checkbox("Echo", &inst[SelectedInst].Echo);

				Checkbox("Noise     ", &inst[SelectedInst].Noise);
				if (inst[SelectedInst].Noise)
				{
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
	GetStyle().CellPadding.x = 4;
	if (Begin("Channels"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (PlayingMode)
		{
			/* calc steps
				* Normalise to 0-1
				* Do something to include the offset for the scroll so the bar is scrolling down when it reaches the middle	
				* Scale to screen size
				* Remove padding from scroll
			*/
			SetScrollY(ScrollValue());
		}	
		if(BeginTable("ChannelView",9, TABLE_FLAGS, ImVec2(GetWindowWidth()*.9 + (TextSize*8), 0)));
		{
			string ind;
			ImVec2 RowVec = ImVec2((GetWindowWidth() / 9) / 5, TextSize - 4);
			//Actual pattern data
			TableNextColumn();

			// This index is for each individual channel on the snes
			// the -1 is for the left hand columns index
			for (int i = -1; i < 8; i++)//X
			{
				if (i >= 0)
				{
					PushStyleColor(ImGuiTableBgTarget_RowBg0, (ImVec4)CursorCol);
					string ChannelTitle = "Pattern: ";
					ChannelTitle += to_string(patterns[i][SelectedPattern].Index);
					Text(ChannelTitle.data());
					PopStyleColor();
				}
				//This determines the columns on a given channel [all track lengths are constant between channels]
				for (int j = -1; j < TrackLength; j++)//Y
				{
					// keep this for future debugging
					/*
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
					*/

					if (i == -1)//this is for the index on the leftmost of the screen
					{
						if (j == -1) ind = "|_|";
						else ind = to_string(j);

						if (j < 10)
						{
							ind += "  ";
						}
						else if (j >= 10 && j < 100)
						{
							ind += " ";
						}
						if(Selectable(ind.data())) {
							CursorY = j;
						}
					}
					else if (j > -1)
					{
						//Channel
						if (BeginTable("RowView", 5, TABLE_FLAGS))
						{
							TableNextColumn();

							//Row Highlighting
							ImU32 col;
							if (EditingMode && j == CursorY || PlayingMode && j == CursorY)
							{
								col = Editing_H2Col;
							}
							else
							{
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
							}
							TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);

							bool IsPoint = CursorX == i && CursorY == j;
							if (IsPoint)//i = x, j = y
							{
								if (IsWindowFocused())
								{
									ChannelInput(CursorPos, i, j);
								}
							}

							PushID(IDOffset + i + (j * 40));
							//Cursor highlighting
							if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
							}
							else
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
							}
							if (Selectable(Channels[i].NoteView(j).data(), IsPoint && CursorPos == NOTE, 0, RowVec))
							{
								CursorPos = NOTE;
								CursorX = i;
								CursorY = j;
							}
							PopID();
							PushID(IDOffset + i + (j * 40) + 1);
							TableNextColumn();

							if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
							}
							else
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
							}
							if (Selectable(Channels[i].InstrumentView(j).data(), IsPoint && CursorPos == INSTR, 0, RowVec))
							{
								CursorPos = INSTR;
								CursorX = i;
								CursorY = j;
							}
							PopID();
							PushID(IDOffset + i + (j * 40) + 2);
							TableNextColumn();

							if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
							}
							else
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
							}
							if (Selectable(Channels[i].VolumeView(j).data(), IsPoint && CursorPos == VOLUME, CursorPos == VOLUME, RowVec))
							{
								CursorPos = VOLUME;
								CursorX = i;
								CursorY = j;
							}
							PopID();
							PushID(IDOffset + i + (j * 40) + 3);
							TableNextColumn();

							if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
							}
							else
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
							}
							if (Selectable(Channels[i].EffectView(j).data(), IsPoint && CursorPos == EFFECT, 0, RowVec))
							{
								CursorPos = EFFECT;
								CursorX = i;
								CursorY = j;
							}
							PopID();
							PushID(IDOffset + i + (j * 40) + 4);
							TableNextColumn();

							if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
							}
							else
							{
								TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
							}
							if (Selectable(Channels[i].Effectvalue(j).data(), IsPoint && CursorPos == VALUE, 0, RowVec))
							{
								CursorPos = VALUE;
								CursorX = i;
								CursorY = j;
							}
							PopID();

							EndTable();
						}
					}
				}
				TableNextColumn();
			}
			EndTable();//keeps crashing here when it minimises, not a clue why other than EndTable seems to be executed too many times?
		}
	}
	End();
}

void Tracker::Samples()
{
	if (Begin("Samples"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth() * 0.275, 24)))
		{
			LoadingSample = true;
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * 0.275, 24)) && samples.size() > 1)
		{
			cout << "SAMPLE LIST SIZE: " << samples.size();
			if (SelectedSample > samples.size())
			{
				samples.erase(samples.begin() + SelectedSample);
				SelectedSample--;
			}
			else if (SelectedSample > 0)
			{
				samples.erase(samples.begin() + SelectedSample);
			}
		}
		SameLine();
		if (Button("Copy", ImVec2(GetWindowWidth() * 0.275, 24)) && samples.size() > 1)
		{
			int index = samples.size();
			Sample newsamp = samples[SelectedSample];
			newsamp.SampleName += to_string(index);
			samples.push_back(newsamp);
			cout << samples.size();
		}

		if (samples.size() > 0)	
		{
			//BeginChild("SampleList", ImVec2(GetWindowWidth() - InstXPadding, GetWindowHeight() - InstYPadding), true, UNIVERSAL_WINDOW_FLAGS);
			BeginTable("SampleTable", 1, TABLE_FLAGS, ImVec2(GetWindowWidth() * 0.75, 24), 24);
			for (int i = 0; i < samples.size(); i++)
			{
				IDOffset++;
				PushID(IDOffset);
				Text(to_string(i).data());
				SameLine();
				if (SelectedSample <= samples.size() - 1)
				{
					if (Selectable(samples[i].SampleName.data(), SelectedSample == i))
					{
						SelectedSample = i;
						ShowSample = true;
						ImPlot::SetNextAxisToFit(x);
						//cout << "\nSelected Sample: " << SelectedSample;
					}
				}
				else
				{
					SelectedSample = samples.size() - 1;
				}
				TableNextColumn();
				PopID();
			}
			EndTable();
			//EndChild();
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
	//ImPlot::ShowDemoWindow();
	if (ShowSample)
	{
		if (Begin("Sample view"), true, UNIVERSAL_WINDOW_FLAGS)
		{
			if (SelectedSample > samples.size() - 1)
			{
				SelectedSample = samples.size() - 1;
			}
			//BeginChild("SampleTable",ImVec2(GetWindowWidth()*0.95, GetWindowHeight() * 0.85),UNIVERSAL_WINDOW_FLAGS);
			NextColumn();
			InputText("Sample Name", (char*)samples[SelectedSample].SampleName.data(), 2048);
			InputInt("Playing HZ", &samples[SelectedSample].SampleRate);
			InputInt("Fine Tune", (int*) &samples[SelectedSample].FineTune, 1,1);
			Checkbox("Loop Sample", &samples[SelectedSample].Loop);
			InputInt("Loop Start", (int*)&samples[SelectedSample].LoopStart, 16, 0);
			InputInt("Loop End", (int*)&samples[SelectedSample].LoopEnd, 16, 0);
			SliderInt("Note offset", &samples[SelectedSample].NoteOffset, -12, 12);
			if (samples.size() > 1)
			{
				vector<float> LoopView;
				vector<float> SampleView;
				for (int i = 0; i < samples[SelectedSample].SampleData.size(); i++)
				{
					SampleView.push_back(samples[SelectedSample].SampleData[i]);
					LoopView.push_back(32767 * 2);
				}
				
				ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
				ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, PlotLineWeight);

				ImPlot::SetNextAxisLimits(y, samples[SelectedSample].LPoint, samples[SelectedSample].HPoint);
				ImPlot::SetNextAxisLimits(x, 0, samples[SelectedSample].SampleData.size());

				if (ImPlot::BeginPlot("Waveform", ImVec2(GetWindowWidth() * 0.9, GetWindowHeight() * 0.7), IMPLOT_FLAGS)) 
				{
					ImPlot::SetupAxis(x, nullptr, ImPlotAxisFlags_NoLabel);
					ImPlot::SetupAxis(y, nullptr, ImPlotAxisFlags_Lock);
					ImPlot::PushColormap("RGBColors");
					ImPlot::PlotLine("Wave Data", SampleView.data(), SampleView.size());
					ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .5f * samples[SelectedSample].Loop);
					
					if (samples[SelectedSample].LoopEnd > samples[SelectedSample].SampleData.size())
					{
						samples[SelectedSample].LoopEnd = samples[SelectedSample].SampleData.size();
					}
					else if (samples[SelectedSample].LoopEnd < 16)
					{
						samples[SelectedSample].LoopEnd = 16;
					}
					else if (samples[SelectedSample].LoopStart < 0)
					{
						samples[SelectedSample].LoopStart = 0;
					}

					if (samples[SelectedSample].LoopEnd % 16 != 0)//Assumes the sample is too large to hold
					{
						BeginTooltip();
						Text("Sample End Loop is not a multiple of 16!!!");
						EndTooltip();
					}

					if (samples[SelectedSample].LoopStart % 16 != 0)//Assumes the sample is too large to hold
					{
						BeginTooltip();
						Text("Sample Start Loop is not a multiple of 16!!!");
						EndTooltip();
					}

					if (samples[SelectedSample].LoopEnd - samples[SelectedSample].LoopStart > 0)
					{
						ImPlot::PlotShaded("Loop Point Shade", LoopView.data(), samples[SelectedSample].LoopEnd - samples[SelectedSample].LoopStart, -INFINITY, 1, samples[SelectedSample].LoopStart, 0, INFINITY);
					}
					ImPlot::PopStyleVar();
					ImPlot::PopColormap();
					ImPlot::EndPlot();
				}
				ImPlot::PopStyleVar();
				ImPlot::PopStyleVar();
			}
			if (Button("Close", ImVec2(64, TextSize * 1.5))) {
				ShowSample = false;
			}
			
			//EndChild();
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
		if (Begin("Settings"), &ShowSettings, UNIVERSAL_WINDOW_FLAGS)
		{
			string ResolutionNames[4] = { "3840x2160", "2560x1440", "1920x1080", "1280x720" };
			string BufferNames[5] = { "512", "1024", "2048", "4096", "8192" };
			string NotationNames[3] = { "Sharp", "Flat", "German"};

			if (BeginTabBar("TestTab"))
			{
				if (BeginTabItem("Appearance")) 
				{
					Text("Screen Resolutions");
					
					if (BeginCombo("##Screen Resolutions", ResolutionNames[SManager.CustomData.Res].data()))
					{
						for (int i = 0; i < 4; i++)
						{
							if (Selectable(ResolutionNames[i].c_str(), i == SManager.CustomData.Res))
							{
								switch (i)
								{
								case 0:
									SManager.CustomData.Res = SManager.Res_3840x2160;
									break;
								case 1:
									SManager.CustomData.Res = SManager.Res_2560x1440;
									break;
								case 2:
									SManager.CustomData.Res = SManager.Res_1920x1080;
									break;
								case 3:
									SManager.CustomData.Res = SManager.Res_1280x720;
									break;
								}
							}
						}
						EndCombo();
					}
					
					Text("Notation Style");

					if (BeginCombo("##NotationStyle", NotationNames[SManager.CustomData.NStyle].data()))
					{
						for (int i = 0; i < 3; i++)
						{
							if (Selectable(NotationNames[i].c_str(), i == SManager.CustomData.NStyle))
							{
								switch (i)
								{
								case 0:
									SManager.CustomData.NStyle = SManager.SharpStyle;
									break;
								case 1:
									SManager.CustomData.NStyle = SManager.FlatStyle;
									break;
								case 2:
									SManager.CustomData.NStyle = SManager.GermanStyle;
									break;
								}
							}
						}
						EndCombo();
					}
					EndTabItem();

				}

				if (BeginTabItem("Technical"))
				{
					Text("FPS");
					InputInt("##FPS", &FPS, 1, 1);
					if (FPS > MAX_FPS) FPS = MAX_FPS; else if (FPS < 1) FPS = 1;

					Text("Audio Buffer Size");

					if (BeginCombo("##Buffer", BufferNames[SManager.CustomData.Buf].data()))
					{
						for (int i = 0; i < 5; i++)
						{
							if (Selectable(BufferNames[i].c_str(), i == SManager.CustomData.Buf))
							{
								switch (i)
								{
								case 0:
									SManager.CustomData.Buf = SManager.Buf_512;
									break;
								case 1:
									SManager.CustomData.Buf = SManager.Buf_1024;
									break;
								case 2:
									SManager.CustomData.Buf = SManager.Buf_2048;
									break;
								case 3:
									SManager.CustomData.Buf = SManager.Buf_4096;
									break;
								case 4:
									SManager.CustomData.Buf = SManager.Buf_8192;
									break;
								}
							}
						}
						EndCombo();
					}

					EndTabItem();
				}

				EndTabBar();

				SetCursorPosY(GetWindowHeight() - (TextSize + GetStyle().FramePadding.y) * 2);

				SetCursorPosX(GetWindowWidth() - (TextSize + GetStyle().FramePadding.x + 18) * 2);
				if (Button("Apply"))
				{
					UpdateSettings();
				}

				SameLine();
				SetCursorPosX(GetWindowWidth() - (TextSize + GetStyle().FramePadding.x + 18) * 4);
				if (Button("Cancel"))
				{
					ResetSettings();
					ShowSettings = false;
				}
			}
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
		Octave > 8 ? Octave = 8 : Octave < 1 ? Octave = 1: Octave;
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
		Text("Base tempo");
		InputInt("##Base tempo", &BaseTempo,1, 1);
		Text("Speed");
		InputInt("##Speed 1", &Speed1, 1, 31);
		Text("Tempo divider");
		InputInt("##Tempo divider", &TempoDivider,1,1);
		if (BaseTempo < 1)
		{
			BaseTempo = 1;
		}
		if (TempoDivider < 1)
		{
			TempoDivider = 1;
		}
		if (Speed1 < 1)
		{
			Speed1 = 1;
		}
		string temp = "Tempo: ";
		temp += to_string(BaseTempo / TempoDivider);
		Text(temp.data());
		NewLine();
		Text("Track Length");
		InputInt("##Track Length", &TrackLength, 1, 16);
		if (TrackLength > 256) TrackLength = 256;
		if (TrackLength < 1) TrackLength = 1;
		NewLine();
		Text("Highlight 1");
		InputInt("##Highlight 1", &Highlight1, 1, 1);
		Text("Highlight 2");
		InputInt("##Highlight 2", &Highlight2, 1, 1);
		if (Highlight1 < 1)
		{
			Highlight1 = 1;
		}
		if (Highlight2 < 1)
		{
			Highlight2 = 1;
		}
		NewLine();
		Text("Author");
		InputText("##Author", (char*)Authbuf.data(), 1024);
		Text("Desc");
		InputTextMultiline("##Desc", (char*)Descbuf.data(), 1024,ImVec2(GetWindowWidth() * 0.65, GetWindowHeight() * 0.6));
		End();
	}
	else
	{
		End();
	}
}

void Tracker::Info_View()
{
	Begin("Information");

	ImDrawList* draw_list = GetWindowDrawList();
	ImVec2 p = GetCursorScreenPos();
	int xpos = p.x;
	int ypos = p.y + TextSize;
	int StepSize = (xpos - GetWindowWidth()) / inst.size();
	int MaxRange = 65536;
	int UsedSpace = (2048 * Delay);
	int InstrumentSpace = 0;
	
	int EchoSpace = UsedSpace;
	int SampleSpace = 0;

	for (int x = 0; x < samples.size(); x++)
	{
		UsedSpace += samples[x].brr.DBlocks.size() * 9;
		SampleSpace += samples[x].brr.DBlocks.size() * 9;
	}
	for (int x = 1; x < inst.size(); x++)
	{
		UsedSpace += 9;
		InstrumentSpace += 9;
	}
	int LastPos = 0;

	//Background Rect to show bounds
	if (UsedSpace > MaxRange)
	{
		Text(("Used space: " + to_string(UsedSpace) + " bytes" + " Too much data used!!!").data());
		draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + GetWindowWidth() * 0.95f, ypos + GetWindowHeight() * 0.35f), ColorConvertFloat4ToU32(ReleaseColour), .25f, 0);
	}
	else
	{
		Text(("Used space: " + to_string(UsedSpace) + " bytes").data());
		UsedSpace = 0;
		draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + GetWindowWidth() * 0.95f, ypos + GetWindowHeight() * 0.35f), ColorConvertFloat4ToU32(H2Col), .25f, 0);
		
		for (int i = 1; i < inst.size(); i++)
		{
			UsedSpace += 9;//While technically wasting 6 bits here, I can't be bothered
		}
		//Instruments
		draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + (UsedSpace * GetWindowWidth()*0.95f)/ MaxRange, ypos + GetWindowHeight() * 0.35f), ColorConvertFloat4ToU32(AttackColour));
		
		LastPos = (UsedSpace * GetWindowWidth() * 0.95f) / MaxRange;
		UsedSpace = 0;
		if (samples.size() > 1)
		{
			for (int i = 0; i < samples.size(); i++)
			{
				UsedSpace += samples[i].brr.DBlocks.size()*9;
			}
		}

		draw_list->AddRectFilled(ImVec2(xpos + LastPos, ypos), ImVec2(xpos + LastPos + (UsedSpace * GetWindowWidth()*0.95f)/ MaxRange, ypos + GetWindowHeight() * 0.35f), ColorConvertFloat4ToU32(SustainColour));
		LastPos = (UsedSpace * GetWindowWidth() * 0.95f) / MaxRange;
		UsedSpace = 0;
		UsedSpace += (2048 * Delay);

		draw_list->AddRectFilled(ImVec2(xpos + LastPos, ypos), ImVec2(xpos + LastPos + (UsedSpace * GetWindowWidth() * 0.95f) / MaxRange, ypos + GetWindowHeight() * 0.35f), ColorConvertFloat4ToU32(DecayColour));
		LastPos = (UsedSpace * GetWindowWidth() * 0.95f) / MaxRange;

	}
	for (int i = 0; i < 4; i++)
	{
		NewLine();
	}
	ypos = GetCursorScreenPos().y - (TextSize*0.125f);
	xpos = GetCursorScreenPos().x;
	
	draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos+TextSize, ypos + TextSize), ColorConvertFloat4ToU32(H2Col), .25f, 0);
	Text(("  Free: " + to_string(MaxRange - (InstrumentSpace + SampleSpace + EchoSpace)) + " bytes").data());

	ypos = GetCursorScreenPos().y - (TextSize * 0.125f);;
	draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + TextSize, ypos + TextSize), ColorConvertFloat4ToU32(AttackColour), .25f, 0);
	Text(("  Instruments: " + to_string(InstrumentSpace) + " bytes").data());

	ypos = GetCursorScreenPos().y - (TextSize * 0.125f);;
	draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + TextSize, ypos + TextSize), ColorConvertFloat4ToU32(SustainColour), .25f, 0);
	Text(("  Samples: " + to_string(SampleSpace) + " bytes").data());

	ypos = GetCursorScreenPos().y - (TextSize * 0.125f);;
	draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + TextSize, ypos + TextSize), ColorConvertFloat4ToU32(DecayColour), .25f, 0);
	Text(("  Echo: " + to_string(EchoSpace) + " bytes").data());

	End();
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

void Tracker::SetupInstr()
{
	DefaultInst.Index = 1;
	DefaultInst.Name += "Instrument: ";
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
	DefaultSample.SampleName = "Choose a sample!";
	DefaultSample.FineTune = 0;
	DefaultSample.SampleRate = 0;
	DefaultSample.Loop = false;
	DefaultSample.LoopStart = 0;
	DefaultSample.LoopEnd = 0;
	DefaultSample.SampleData.clear();
	DefaultSample.BRRSampleData.clear();
	samples.push_back(DefaultSample);

	DefaultPattern.Index = 0;
	DefaultPattern.SetUp(TrackLength);
}

void Tracker::RunTracker()
{
	//cout << "\n Audio Buff Queued: " << SDL_GetQueuedAudioSize(dev);
	if (SDL_GetQueuedAudioSize(dev) < TRACKER_AUDIO_BUFFER * 8)
	{
		for (int x = 0; x < TRACKER_AUDIO_BUFFER; x++)
		{
			//SG.P++;
			for (int i = 0; i < 8; i++)
			{
				Channels[i].UpdateChannel(inst, samples);
				//Channels[i].AudioDataL = 2048 * sin((SG.P) * (2 * 3.14) * 440 * (1. / AUDIO_RATE));//Left ear
				//Channels[i].AudioDataR = 2048 * sin((SG.P) * (2 * 3.14) * 440 * (1. / AUDIO_RATE));//Right ear
				//cout << "\n" << Channels[i].AudioDataR << ": Channel " << i << " Framce Counter: " << FrameCount;
			}
			SG.MixChannels(x);
			SG.Update(GetIO().DeltaTime, Channels, samples, CursorY, inst);
			//SG.DEBUG_Output_Audio_Buffer_Log(SG.Totalbuffer, FrameCount, x, SDL_GetQueuedAudioSize(dev));
		}
		SDL_QueueAudio(dev, SG.Totalbuffer, sizeof(SG.Totalbuffer));
	}

	TickTimer -= GetIO().DeltaTime;
	float BPM = (float)BaseTempo;
	if (CursorY >= TrackLength-1 && PatternIndex >= patterns->size()-1)
	{
		PlayingMode = false;
	}
	else if (CursorY >= TrackLength)
	{
		CursorY = 0;
		SelectedPattern++;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < TrackLength; j++)
			{
				Channels[i].Rows[j].note = StoragePatterns[patterns[i][SelectedPattern].Index].SavedRows[j].note;
				Channels[i].Rows[j].instrument = StoragePatterns[patterns[i][SelectedPattern].Index].SavedRows[j].instrument;
				Channels[i].Rows[j].volume = StoragePatterns[patterns[i][SelectedPattern].Index].SavedRows[j].volume;
				Channels[i].Rows[j].effect = StoragePatterns[patterns[i][SelectedPattern].Index].SavedRows[j].effect;
				Channels[i].Rows[j].effectvalue = StoragePatterns[patterns[i][SelectedPattern].Index].SavedRows[j].effectvalue;
			}
		}
	}
	else if (TickTimer < (BPM / (float)TempoDivider))
	{
		TickCounter++;
		if (TickCounter > Speed1)
		{
			CursorY++;
			UpdateRows();
			TickCounter = 0;
		}
		TickTimer = 2;
	}
}

void Tracker::UpdateRows()
{
	for (int i = 0; i < 8; i++)
	{
		Channels[i].TickCheck(CursorY % TrackLength, inst, samples);
	}

	//cout << "\nCurrent Value: " << ((double)CursorY - 1.0/(24.0 / (double)TrackLength) / (double)TrackLength) * ((TextSize + GetStyle().CellPadding.y * 2) * TrackLength);
}

void Tracker::ChannelInput(int CurPos, int x, int y)
{
	if (Event == GLFW_REPEAT || Event == GLFW_PRESS)
	{
		//Shit to make sure the input can do repeating and held down keys
		if (KeyTimer > MinKeyTime)
		{
			IsPressed = false;
			KeyTimer = 0;
		}
		else if (!InitPressed)
		{
			IsPressed = false;
			InitPressed = true;
		}
		else
		{
			if (InitTimer > InitKeyTime)
			{
				KeyTimer += GetIO().DeltaTime;
			}
			else
			{
				InitTimer += GetIO().DeltaTime;
			}
		}
	}
	else
	{
		InitPressed = false;
		KeyTimer = 0;
		InitTimer = 0;
	}

	if (!IsPressed)
	{
		//cout << "\nCurrent key: " << Currentkey;
		
		if (CurrentMod == GLFW_MOD_SHIFT)
		{
			if (!BoxSelected)
			{
				SelectionBoxX1 = x;
				SelectionBoxY1 = y;
				SelectionBoxX2 = SelectionBoxX1;
				SelectionBoxY2 = SelectionBoxY1;
				SelectionBoxSubX1 = CursorPos;
				SelectionBoxSubX2 = SelectionBoxSubX1;
				BoxSelected = true;
			}

			if (Currentkey == GLFW_KEY_LEFT)
			{
				CursorPos--;
				SelectionBoxSubX2--;
				if (SelectionBoxSubX2 < 0)
				{
					SelectionBoxSubX2 = 0;
				}
			}
			else if (Currentkey == GLFW_KEY_RIGHT)
			{
				CursorPos++;
				SelectionBoxSubX2++;
				if (SelectionBoxSubX2 > 40)
				{
					SelectionBoxSubX2 = 40;
				}
			}

			if (Currentkey == GLFW_KEY_DOWN)
			{
				CursorY++;
				SelectionBoxY2++;
			}
			else if (Currentkey == GLFW_KEY_UP)
			{
				CursorY--;
				SelectionBoxY2--;
			}
			else if (Currentkey == GLFW_KEY_SPACE)
			{
				EditingMode = !EditingMode;
			}
			else if (Currentkey == GLFW_KEY_ENTER)
			{
				PlayingMode = !PlayingMode;
				EditingMode = false;
			}
			cout << "\nSelectionBoxSubX1: " << SelectionBoxSubX1 << "\n";
			cout << "\nSelectionBoxSubX2: " << SelectionBoxSubX2 << "\n";
		}
		else
		{
			if (Currentkey == GLFW_KEY_HOME)
			{
				CursorY = 0;
			}
			else if (Currentkey == GLFW_KEY_END)
			{
				CursorY = TrackLength-1;
			}
			else if (Currentkey == GLFW_KEY_PAGE_UP)
			{
				CursorY -= 16;
			}
			else if (Currentkey == GLFW_KEY_PAGE_DOWN)
			{
				CursorY += 16;
			}

			if (Currentkey == GLFW_KEY_LEFT)
			{
				CursorPos--;
				BoxSelected = false;
			}
			else if (Currentkey == GLFW_KEY_RIGHT)
			{
				CursorPos++;
				BoxSelected = false;
			}

			if (Currentkey == GLFW_KEY_DOWN)
			{
				if (MoveByStep)
				{
					CursorY += Step;
				}
				else
				{
					CursorY++;
				}
				BoxSelected = false;
			}
			else if (Currentkey == GLFW_KEY_UP)
			{
				if (MoveByStep)
				{
					CursorY -= Step;
				}
				else
				{
					CursorY--;
				}
				BoxSelected = false;
			}
			else if (Currentkey == GLFW_KEY_SPACE)
			{
				EditingMode = !EditingMode;
			}
			else if (Currentkey == GLFW_KEY_ENTER)
			{
				PlayingMode = !PlayingMode;
				EditingMode = false;
				BoxSelected = false;
			}
		}
		if (EditingMode)
		{
			if (BoxSelected && Currentkey == GLFW_KEY_DELETE)
			{
				for (int z = SelectionBoxSubX1 + (SelectionBoxX1 * 5); z < (SelectionBoxSubX2 + (SelectionBoxX2 * 5))+1; z++)
				{
					for (int w = SelectionBoxY1; w < SelectionBoxY2+1; w++)
					{
						if (z % 5 == NOTE) Channels[(z / 5)].Rows[w].note = MAX_VALUE;
						if (z % 5 == INSTR) Channels[(z / 5)].Rows[w].instrument = MAX_VALUE;
						if (z % 5 == VOLUME) Channels[(z / 5)].Rows[w].volume = MAX_VALUE;
						if (z % 5 == EFFECT) Channels[(z / 5)].Rows[w].effect = MAX_VALUE;
						if (z % 5 == VALUE) Channels[(z / 5)].Rows[w].effectvalue = MAX_VALUE;
						ChangePatternData(z / 5, w);
					}

				}
				BoxSelected = false;
			}
			else
			{
				//For editing the stuff in the subcolumns
				switch (CurPos)
				{
				case NOTE:
					for (int i = 0; i < 24; i++)
					{
						if (Currentkey == NoteInput[i])
						{
							if (i < 12)
							{
								Channels[x].Rows[y].octave = (Octave);
							}
							else
							{
								Channels[x].Rows[y].octave = Octave+1;
							}
							Channels[x].Rows[y].note = i + (12 * Octave);
							if (SelectedInst != 0)
							{
								Channels[x].Rows[y].instrument = SelectedInst;
							}
							//Channels[x].Rows[y].S_Note = Channels[x].NoteNames[Channels[x].Rows[y].note % 12] + to_string(Channels[x].Rows[y].octave);
							CursorY += Step;
							if (CursorY >= TrackLength)
							{
								CursorY = TrackLength - 1;
							}
							ChangePatternData(x, y);
							break;
						}
						else if (Currentkey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].note = MAX_VALUE;
							if (MoveOnDelete)
							{
								CursorY += Step;
							}
							else
							{
								CursorY++;
							}
							ChangePatternData(x, y);
							break;
						}
					}
					break;

				case INSTR:
					for (int i = 0; i < 16; i++)
					{
						if (Currentkey == VolInput[i])
						{
							Channels[x].Rows[y].instrument = Channels[x].EvaluateHexInput(i, y, 127, INSTR);
							ChangePatternData(x, y);
							break;
						}
						else if (Currentkey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].instrument = MAX_VALUE;
							CursorY += Step;
							ChangePatternData(x, y);
							break;
						}
					}
					break;
				case VOLUME:
					for (int i = 0; i < 16; i++)
					{
						if (Currentkey == VolInput[i])
						{
							Channels[x].Rows[y].volume = Channels[x].EvaluateHexInput(i, y, 127, VOLUME);
							ChangePatternData(x, y);
							break;
						}
						else if (Currentkey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].volume = MAX_VALUE;
							CursorY += Step;
							ChangePatternData(x, y);
							break;
						}
					}
					break;
				case EFFECT:
					for (int i = 0; i < 16; i++)
					{
						if (Currentkey == VolInput[i])
						{
							Channels[x].Rows[y].effect = Channels[x].EvaluateHexInput(i, y, 255, EFFECT);
							ChangePatternData(x, y);
							break;
						}
						else if (Currentkey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].effect = MAX_VALUE;
							CursorY += Step;
							ChangePatternData(x, y);
							break;
						}
					}
					break;
				case VALUE:
					for (int i = 0; i < 16; i++)
					{
						if (Currentkey == VolInput[i])
						{
							Channels[x].Rows[y].effectvalue = Channels[x].EvaluateHexInput(i, y, 255, VALUE);
							ChangePatternData(x, y);
							break;
						}
						else if (Currentkey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].effectvalue = MAX_VALUE;
							CursorY += Step;
							ChangePatternData(x, y);
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
		}
		IsPressed = true;
	}
	if (SelectionBoxX2 < 0)
	{
		SelectionBoxX2 = 0;
	}
	else if (SelectionBoxX2 > 7)
	{
		SelectionBoxX2 = 7;
	}
	if (CurPos > VALUE)
	{
		CursorX ++;
		CursorPos = NOTE;
	}
	else if (CurPos < 0  && CursorX > 0)
	{
		CursorX--;
		CursorPos = VALUE;
	}

	if (CursorY < 0)
	{
		CursorY = 0;
	}
	else if (CursorY > TrackLength)
	{
		CursorY = TrackLength;
	}

	if (CursorPos < 0 && CursorX == 0)
	{
		CursorPos = 0;
		CurPos = 0;
		CursorX = 0;
	}
	else if (CursorPos > VALUE && CursorX == 7)
	{
		CursorPos = VALUE;
		CurPos = VALUE;
		CursorX = 7;
	}

}

void Tracker::LoadSample()
{
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".wav, .ogg, .mp3", ".");
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			cout << "\n" + FileName + "\n" + FilePath;
			SF_INFO soundinfo;
			soundinfo.format = AUDIO_FORMATS;
			soundinfo.samplerate = SPS;
			soundinfo.channels = 1;
			soundinfo.frames = TRACKER_AUDIO_BUFFER;

			SNDFILE* file = sf_open(FileName.data(), SFM_READ, &soundinfo);
			if (file)//Loaded right
			{
				short FileBuffer[65536];
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
					for (int i = 0; i < AudioLen/soundinfo.channels; i++)
					{
						//cout << "\n" << i;
						/*
						cout << " " << FileBuffer[i];
						if (i % 8 == 1)
						{
							cout << "\n";
						}
						*/
						cur.SampleData.push_back(FileBuffer[i]);
					}
					cur.LargestPoint();
					cur.SampleIndex = SelectedSample;
					cur.SampleName = "Sample: ";
					cur.SampleRate = soundinfo.samplerate;
					cur.Loop = false;
					cur.LoopStart = 0;
					cur.LoopEnd = 0;
					cur.FineTune = 0;
					SelectedSample = samples.size();
					cur.SampleIndex = SelectedSample;
					cur.SampleIndex = SelectedSample;
					//Assume the file isn't fucked and we can move to the BRR conversion
					cur.BRRConvert();
					samples.push_back(cur);
					sf_close(file);
				}
				else
				{
					sf_close(file);
					cout << "\n ERROR: FILE IS EITHER EMPTY OR IS OTHERWISE UNABLE TO LOAD \n ";
				}
				ImPlot::SetNextAxisToFit(x);
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
	
	for (int a = 0; a < sampleLength; a += sampleBufferSize) 
	{
		sf_read_short(sndfile, sampleBuffer.data(), sampleBufferSize);
		for (int i = 0; i < sampleBufferSize; i++) 
		{
			sum = 0;
			for (int j = 0; j < sfinfo.channels; j++)
				sum += sampleBuffer.data()[i * sfinfo.channels + j];
		
			outputBuffer[i] = (Sint16)(sum / (double)sfinfo.channels);
		}
	}

}

void Tracker::UpdatePatternIndex(int x, int y)//For when you are switching patterns in the top menu item
{
	if (patterns[x][y].Index > Maxindex)
	{
		Maxindex = patterns[x][y].Index;
		Patterns pat;
		pat = DefaultPattern;
		StoragePatterns.push_back(pat);
		patterns->push_back(pat);
	}

	cout << "\n" << x << "\n" << y;
	for (int i = 0; i < TrackLength; i++)
	{
		Channels[x].Rows[i].note = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].note;
		Channels[x].Rows[i].octave = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].octave;
		Channels[x].Rows[i].instrument = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].instrument;
		Channels[x].Rows[i].volume = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].volume;
		Channels[x].Rows[i].effect = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].effect;
		Channels[x].Rows[i].effectvalue = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].effectvalue;
	}

}

void Tracker::UpdateAllPatterns()
{
	for (int x = 0; x < 8; x++)
	{
		for (int i = 0; i < TrackLength; i++)
		{
			Channels[x].Rows[i].note = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].note;
			Channels[x].Rows[i].octave = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].octave;
			Channels[x].Rows[i].instrument = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].instrument;
			Channels[x].Rows[i].volume = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].volume;
			Channels[x].Rows[i].effect = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].effect;
			Channels[x].Rows[i].effectvalue = StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[i].effectvalue;
		}
	}
}

void Tracker::ChangePatternData(int x, int y)
{
	cout << "\nCHANGED PATTERN DATA:" << "\nX: " << x << "\nY: " << y << "\nSelected Pattern " << SelectedPattern;
	cout << "\n" << patterns->size() << " : " << patterns[x].size();

	//Put data into channel
	patterns[x][SelectedPattern].SavedRows[y].note = Channels[x].Rows[y].note;
	patterns[x][SelectedPattern].SavedRows[y].octave = Channels[x].Rows[y].octave;
	patterns[x][SelectedPattern].SavedRows[y].instrument = Channels[x].Rows[y].instrument;
	patterns[x][SelectedPattern].SavedRows[y].volume = Channels[x].Rows[y].volume;
	patterns[x][SelectedPattern].SavedRows[y].effect = Channels[x].Rows[y].effect;
	patterns[x][SelectedPattern].SavedRows[y].effectvalue = Channels[x].Rows[y].effectvalue;

	//Put data into channel
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].note = Channels[x].Rows[y].note;
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].octave = Channels[x].Rows[y].octave;
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].instrument = Channels[x].Rows[y].instrument;
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].volume = Channels[x].Rows[y].volume;
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].effect = Channels[x].Rows[y].effect;
	StoragePatterns[patterns[x][SelectedPattern].Index].SavedRows[y].effectvalue = Channels[x].Rows[y].effectvalue;
}

void Tracker::UpdateSettings()
{
	for (int i = 0; i < 8; i++)
	{
		Channels[i].NoteType = SManager.CustomData.NStyle;
		SManager.SetNotation(&Channels[i].NoteType);
	}
	SManager.SetBuffer(&TRACKER_AUDIO_BUFFER);
	SManager.SetResolution(&SCREEN_WIDTH, &SCREEN_HEIGHT);
	FPS = SManager.CustomData.FPS;
	TextSize = SManager.CustomData.FontSize;
	TextSizeLarge = TextSize*2;
	TrackLength = SManager.CustomData.DefaultTrackSize;
	MoveOnDelete = SManager.CustomData.DeleteMovesAtStepCount;
	MoveByStep = SManager.CustomData.CursorMovesAtStepCount;
}

void Tracker::ResetSettings()
{
	SManager.CustomData.Buf = SManager.DefaultData.Buf;
	SManager.CustomData.NStyle = SManager.DefaultData.NStyle;
	SManager.CustomData.CursorMovesAtStepCount = SManager.DefaultData.CursorMovesAtStepCount;
	SManager.CustomData.DefaultTrackSize = SManager.DefaultData.DefaultTrackSize;
	SManager.CustomData.DeleteMovesAtStepCount = SManager.DefaultData.DeleteMovesAtStepCount;
	SManager.CustomData.FontSize = SManager.DefaultData.FontSize;
	SManager.CustomData.FPS = SManager.DefaultData.FPS;
	SManager.CustomData.Res = SManager.DefaultData.Res;
	UpdateSettings();
}
