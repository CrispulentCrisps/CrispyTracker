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
	/* calc steps
		* Normalise to 0-1
		* Do something to include the offset for the scroll so the bar is scrolling down when it reaches the middle
		* Scale to screen size
		* Remove padding from scroll
	*/
	return (((double)CursorY / (double)TrackLength)) * ((TextSize + ImGui::GetStyle().CellPadding.y * 2) * TrackLength) - (GetWindowHeight() / 3.0);;
}

void Tracker::Initialise(int StartLength)
{
	//initialise all the channels
	for (int i = 0; i < 8; i++)
	{
		Channel channel = Channel();
		channel.SetUp(256);
		Channels[i] = channel;
		Channels[i].Index = i;
		Patterns pat = Patterns();
		pat.SetUp(256);
		pat.Index = i;
		orders[i].push_back(pat);
		StoragePatterns.push_back(pat);
	}

	SManager.CreateDefaultSettings();
	SManager.CheckSettingsFolder();
	SManager.ReadSettingsFile();
	UpdateSettings(1);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	Tracker* tr = static_cast<Tracker*>(glfwGetWindowUserPointer(window));
	tr->CurrentKey = key;
	tr->CurrentMod = mods;
	tr->Event = action;
}

void MyAudioCallback(void* userdata, Uint8* stream, int len)
{
	DWORD flags = 0;
	static_cast<SoundGenerator*>(userdata)->LoadData(len / 8, stream, &flags);
}

void Tracker::Run()
{	
	glfwInit();
	FilePath.reserve(2048);
	//Initialise the tracker
	Initialise(TrackLength);
	SG.SetBufferSize(SG.TRACKER_AUDIO_BUFFER);
	SG.Emu_APU.APU_Startup();
	SG.Emu_APU.APU_Init_Echo();
	SetupInstr();
	bool PlayingTrack = false;
	bool WindowIsGood = true;
	//Create window
#if CT_UNIX
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, Credits.c_str(), NULL, NULL);
	// Setup Platform/Renderer backends

	glfwMakeContextCurrent(window);
	monitor = glfwGetPrimaryMonitor();

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
	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_WindowBg] = WindowBG;
	style->FrameBorderSize = 0.1f;
	style->WindowRounding = .1f;
	style->FrameRounding = .1f;

	io = GetIO();
	io.DisplaySize.x = SCREEN_WIDTH;
	io.DisplaySize.y = SCREEN_HEIGHT;
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
		have.size = SG.TRACKER_AUDIO_BUFFER;
		have.freq = SPS;
		have.samples = have.freq / have.channels;
		have.silence = 0;
		have.padding = 512;
		have.format = AUDIO_S16;

		SDL_memset(&have, 0, sizeof(have)); /* or SDL_zero(want) */
		have.freq = AUDIO_RATE;//Coming from the SoundGenerator class
		have.format = AUDIO_S16;
		have.channels = 2;
		have.samples = SG.TRACKER_AUDIO_BUFFER;//Coming from the SoundGenerator class
		have.userdata = &SG;

		SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
		want.freq = AUDIO_RATE;//Coming from the SoundGenerator class
		want.format = AUDIO_S16;
		want.channels = 2;
		want.samples = SG.TRACKER_AUDIO_BUFFER;//Coming from the SoundGenerator class
		want.callback = NULL;
		want.userdata = &SG;
		want.silence = SG.TRACKER_AUDIO_BUFFER;

		dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
		const char* err = SDL_GetError();

		SG.PlayingNoise = true;
		//Load fonts
		font = io.Fonts->AddFontFromFileTTF(Fontpath.c_str(), TextSize, NULL, NULL);
		Largefont = io.Fonts->AddFontFromFileTTF(Fontpath.c_str(), TextSizeLarge, NULL, NULL);
		io.Fonts->Build();
		ImGui_ImplOpenGL3_CreateFontsTexture();
		WindowIsGood = true;
		for (int i = 0; i < 8; i++)
		{
			Channels[i].Index = i;
		}
		x = ImAxis_X1;
		y = ImAxis_Y1;

		PlotColours = ImPlot::AddColormap("RGBColors", colorDataRGB, 32);
	}
	ChannelEditState cstate = NOTE;
	SG.DEBUG_Open_File();
	while (running) {
		if (WindowIsGood) {
			Render();
			CheckUpdatables();
			SDL_PauseAudioDevice(dev, false);
		}
		CheckInput();
	}
	SManager.CloseSettingsStream();
	SG.DEBUG_Close_File();
	//Destroy window
	ImGui::DestroyContext();
	ImPlot::DestroyContext();
	glfwDestroyWindow(window);
	ImGui_ImplOpenGL3_Shutdown();
	glfwTerminate();
	SDL_Quit();
}

void Tracker::CheckUpdatables()//for things we cannot update in the rendering loop
{
	UpdateModule();
	if (FontUpdate)
	{
		UpdateFont();
	}
}

void Tracker::CheckInput()
{
	int TuninOff = 48;
	glfwPostEmptyEvent();
	if (glfwWindowShouldClose(window))
	{
		running = false;
	}

	if (PlayingMode)
	{
		RunTracker();
	}

	UpdateAudioBuffer();
}

void Tracker::Render()
{
	glfwWaitEvents();
	float current = (float)glfwGetTime();
	DeltaTimeHolder = current - TimeHolder;
	TimeHolder = current;
	IDOffset = 0;
	FrameCount++;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	NewFrame();
	DockSpaceOverViewport(NULL);
	if (!ShowCredits)
	{
		//ShowDemoWindow();
		MenuBar();
		Author_View();
		Speed_View();
		Patterns_View();
		Channel_View();
		Instruments();
		Instrument_View();
		Samples();
		Sample_View();
		Settings_View();
		Misc_View();
		EchoSettings();
		if (ShowExport)	Export_View();

		if (SavingFile) SaveModuleAs();
		if (LoadingFile) LoadModuleAs();

		if (LoadingSample) LoadSample();

		if (ShowError) ErrorWindow();

		if (ShowDSPDebugger) DSPDebugWindow();

		if (ShowTrackerDebugger) TrackerDebug();

		Info_View();
	}
	else
	{
		CreditsWindow();
	}
	ImGui::Render();
	if (ImGui::GetDrawData() != NULL)
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	ImGui::EndFrame();
}

void Tracker::MenuBar()
{
	BeginMainMenuBar();
	if (BeginMenu("File"))
	{
		if(ImGui::MenuItem("Load")){
			SavingFile = false;
			LoadingFile = true;
			LoadingSample = false;
		}
		if(ImGui::MenuItem("Save...")) {
			SavingFile = true;
			LoadingFile = false;
			LoadingSample = false;
		}
		if (ImGui::MenuItem("Save as...")) {
			SavingFile = true;
			LoadingFile = false;
			LoadingSample = false;
		}

		if (Selectable("Export...")) {
			ShowExport = true;
		}
		ImGui::EndMenu();
	}

	if (BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Show Settings"))
		{
			ShowSettings = !ShowSettings;
		}
		if (ImGui::MenuItem("Debug Window"))
		{
			ShowTrackerDebugger = !ShowTrackerDebugger;
		}

		ImGui::EndMenu();
	}


	if (BeginMenu("SNES"))
	{
		if (Selectable("Echo Settings"))
		{
			ShowEcho = !ShowEcho;
		}
		if (Selectable("DSP regs"))
		{
			ShowDSPDebugger = !ShowDSPDebugger;
		}
		if (ImGui::TreeNode("Dump contents"))
		{
			if (Selectable("Dump SPC"))
			{
				SG.Emu_APU.APU_Debug_Dump_SPC();
			}
			if (Selectable("Dump BRR"))
			{
				SG.Emu_APU.APU_Debug_Dump_BRR();
			}
			if (Selectable("Dump DIR"))
			{
				SG.Emu_APU.APU_Debug_Dump_DIR();
			}
			if (Selectable("Dump FLG"))
			{
				SG.Emu_APU.APU_Debug_Dump_FLG();
			}
			if (Selectable("Dump INST"))
			{
				SG.Emu_APU.APU_Debug_Dump_INST();
			}
			TreePop();
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

	Text("	|	%.3f ms/frame (%.1f FPS)", 1000.0 / (ImGui::GetIO().Framerate), (ImGui::GetIO().Framerate));
	Text(VERSION.c_str());
	EndMainMenuBar();

}

void Tracker::CreditsWindow()
{
	if (Begin("CreditsWindow"), true, UNIVERSAL_WINDOW_FLAGS) {
		PushFont(Largefont);
		Text("CREDITS");
		PopFont();
		NewLine();
		NewLine();
		Text("Code:");

		BulletText("Crisps");
		BulletText("Alexmush");
		BulletText("Euly");
		BulletText("Franciscod");
		BulletText("Tim4242");

		NewLine();
		Text("Emulator Code:");
		BulletText("snes_spc by John Reagan, fork of the SPC emulation from Blargg");
		BulletText("BRRTools by Optiroc");

		NewLine();
		Text("Driver Code:");
		BulletText("Crisps");

		if (Button("Back to editor", ImVec2(128, TextSize * 1.5))) ShowCredits = false;
	}
	End();
}

void Tracker::Patterns_View()
{
	if (Begin("Patterns"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		Columns(2);

		if (BeginTable("PatternsTable", 9, TABLE_FLAGS)) 
		{
			for (int y = 0; y < SongLength; y++)
			{
				TableNextRow();
				ImGui::TableNextColumn();
				if (Selectable(to_string(y).c_str())) {
					SelectedPattern = y;
					UpdateAllPatterns();
				}
				ImGui::TableNextColumn();

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
					Selectable(to_string(orders[x][y].Index).c_str());

					if (IsItemClicked(ImGuiMouseButton_Right)) {
						orders[x][y].Index > 0 ? orders[x][y].Index-- : orders[x][y].Index = 0;
						SelectedPattern = y;
						UpdatePatternIndex(x, y);
						UpdateAllPatterns();
					}
					else if (IsItemClicked(ImGuiMouseButton_Left)) {
						orders[x][y].Index++;
						SelectedPattern = y;
						UpdatePatternIndex(x, y);
						UpdateAllPatterns();
					}

					PopID();
					IDOffset++;
					ImGui::TableNextColumn();
				}
			}
			ImGui::EndTable();


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
					orders[i].push_back(pat);
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
						orders[i].erase(orders[i].begin() + SelectedPattern);
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
					pat = orders[i][SelectedPattern];
					//cout << "\n SavedRows: " << pat.SavedRows.size();
					orders[i].push_back(pat);
				}
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
		}
	}
	End();
}

void Tracker::Instruments()//Showing the instruments window at the side
{
	if (Begin("Instruments"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth() * .3, 24)))
		{
			Instrument newinst = DefaultInst;
			int index = inst.size();
			newinst.Name += to_string(index);
			newinst.Index = index;
			inst.push_back(newinst);
			SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * .3, 24)) && inst.size() > 1)
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
			SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
		}		
		SameLine();
		if (Button("Copy", ImVec2(GetWindowWidth() * .3, 24)) && inst.size() > 1)
		{
			int index = inst.size();
			Instrument newinst = inst[SelectedInst];
			newinst.Name += to_string(index);
			newinst.Index = index;
			inst.push_back(newinst);
			cout << inst.size();
			SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
		}
		//Instrument side bar
		if (inst.size() > 0)
		{
			if (BeginTable("InstList", 1, TABLE_FLAGS, ImVec2(GetWindowWidth() * 0.87, 24), 24))
			{
				for (int i = 0; i < inst.size(); i++)
				{
					PushID(IDOffset);
					//Show instruments
					Text(to_string(i).c_str());
					SameLine();
					if (SelectedInst <= inst.size() - 1)
					{
						if (Selectable(inst[i].Name.c_str(), SelectedInst == i))
						{
							SelectedInst = i;
							ShowInstrument = true;
						}
					}
					else
					{
						SelectedInst = inst.size();
					}
					ImGui::TableNextColumn();
					PopID();
					IDOffset++;
				}
				ImGui::EndTable();
			}
		}
	}
	End();
}

void Tracker::Instrument_View()//Instrument editor
{
	if (ShowInstrument)
	{	
		if (Begin("Instrument Editor"), true, UNIVERSAL_WINDOW_FLAGS)
		{
			if (SelectedInst <= inst.size() - 1)
			{
				PushItemWidth(GetWindowWidth() * .75);
				InputText("InstName", (char*)inst[SelectedInst].Name.c_str(), sizeof(inst[SelectedInst].Name));
				string PrevText = "Choose a sample";

				if (inst[SelectedInst].SampleIndex < samples.size())
				{
					PrevText = samples[inst[SelectedInst].SampleIndex].SampleName;
				}
				if (BeginCombo("##Sample", PrevText.c_str())) {
					if (samples.size() > 0)
					{
						bool Selected = false;
						for (int s = 0; s < samples.size(); s++)
						{
							Selected = (inst[SelectedInst].SampleIndex == s);
							if (Selectable(samples[s].SampleName.c_str(), Selected, 0, ImVec2(GetWindowWidth() * 0.85, TextSize)))
							{
								inst[SelectedInst].SampleIndex = s;
								inst[SelectedInst].CurrentSample = samples[s];
								SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
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

				NewLine();
				if (SliderInt("Left", &inst[SelectedInst].LPan, 0, 127)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
				if (SliderInt("Right", &inst[SelectedInst].RPan, 0, 127)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
				if (SliderInt("Gain", &inst[SelectedInst].Gain, 0, 255)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);

				if (SliderInt("Note offset", &inst[SelectedInst].NoteOff, -12, 12)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);

				if (Checkbox("Envelope used", &inst[SelectedInst].EnvelopeUsed))SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
				ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.33);

				if (inst[SelectedInst].EnvelopeUsed)
				{
					NewLine();
					Text("Envelope");

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

					float attackaccum = 0;
					for (int i = 0; i < 15 - inst[SelectedInst].Attack; i++)
					{
						PlotArr[i] = attackaccum;
						attackaccum += (32.0 / (float)(15.0 - inst[SelectedInst].Attack));
					}
					PlotArr[15 - inst[SelectedInst].Attack] = 32;

					int PreviousPoint = 16 - inst[SelectedInst].Attack;

					float decayaccum = 32;
					float base = inst[SelectedInst].Sustain * 4.57142857143;
					for (int j = PreviousPoint; j < PreviousPoint + (8 - inst[SelectedInst].Decay); j++)
					{
						decayaccum -= ((32.0 - base) / (8.0 - inst[SelectedInst].Decay));
						PlotArr[j] = decayaccum;
					}

					PreviousPoint += (8 - inst[SelectedInst].Decay);

					float relaccum = base;
					for (int k = PreviousPoint; k < PreviousPoint + (32 - inst[SelectedInst].Release); k++)
					{
						if (inst[SelectedInst].Sustain > 0)
						{
							relaccum -= (base / (32 - inst[SelectedInst].Release));
							PlotArr[k] = relaccum;
						}
						else
						{
							PlotArr[k] = 0;
						}
					}
					float PlotBuff[256];
					NewLine();
					PlotLines("##Envelope output", PlotArr, 64, 0, "Envelope output", 0, 32, ImVec2(GetWindowWidth() * 0.75, GetWindowHeight() * 0.25));
				}

				Text("Special");
				NewLine();
				if (Checkbox("Invert L", &inst[SelectedInst].InvL))SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
				if (Checkbox("Invert R", &inst[SelectedInst].InvR)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);

				if (Checkbox("Pitch Mod", &inst[SelectedInst].PitchMod))SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
				if (Checkbox("Echo", &inst[SelectedInst].Echo))SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);

				if (Checkbox("Noise", &inst[SelectedInst].Noise)) SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);
			}
			else
			{
				//SelectedInst--;
			}
			if (Button("Close", ImVec2(64, TextSize * 1.5))) {
				ShowInstrument = false;
			}
		}
		else
		{
			ShowInstrument = false;
		}
		End();
	}

}

void Tracker::Channel_View()
{
	if (Begin("Channels"), 0, UNIVERSAL_WINDOW_FLAGS)
	{
		if (PlayingMode)
		{
			SetScrollY(ScrollValue());
		}
		if (BeginTable("ChannelView", 9, ImGuiTableFlags_SizingFixedFit, ImVec2(GetWindowWidth() * .9 + (TextSize * 8), 0)) != NULL)
		{
			string ind;
			ImVec2 RowVec = ImVec2((GetWindowWidth() / 9.0) / 6.0, (double)TextSize - (TextSize / 3.0));
			//Actual pattern data
			if (ImGui::TableNextColumn()) {
				// This index is for each individual channel on the snes
				// the -1 is for the left hand columns index
				for (int i = -1; i < 8; i++)//X
				{
					if (i >= 0)
					{
						PushStyleColor(ImGuiTableBgTarget_RowBg0, (ImVec4)CursorCol);
						string ChannelTitle = "Pattern: ";
						ChannelTitle += to_string(orders[i][SelectedPattern].Index);
						Text(ChannelTitle.c_str());
						PopStyleColor();
					}
					//This determines the columns on a given channel [all track lengths are constant between channels]
					for (int j = -1; j < TrackLength; j++)//Y
					{
						if (i == -1)//this is for the index on the leftmost of the screen
						{
							if (j == -1) ind = "---";
							else ind = to_string(j);

							if (j < 10)
							{
								ind += "  ";
							}
							else if (j >= 10 && j < 100)
							{
								ind += " ";
							}

							if (Selectable(ind.c_str())) {
								CursorY = j;
							}
						}
						else if (j > -1)
						{
							//Channel
							//the *12 is for the char amount + 1 for spacing
							ImVec2 sizevec = ImVec2(TextSize * 9, RowVec.y);
							if (BeginTable("RowView", 5, TABLE_FLAGS, sizevec))
							{
								ImVec2 NoteSize = ImVec2(TextSize * 1.75, RowVec.y);
								ImVec2 MiscSize = ImVec2(TextSize, RowVec.y);
								TableSetupColumn("note", ImGuiTableColumnFlags_WidthFixed, 0.0);
								TableSetupColumn("vol", ImGuiTableColumnFlags_WidthFixed, 0.0);
								TableSetupColumn("inst", ImGuiTableColumnFlags_WidthFixed, 0.0);
								TableSetupColumn("effe", ImGuiTableColumnFlags_WidthFixed, 0.0);
								TableSetupColumn("val", ImGuiTableColumnFlags_WidthFixed, 0.0);
								if (ImGui::TableNextColumn()) 
								{
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
									if (Selectable(Channels[i].NoteView(j).c_str(), IsPoint && CursorPos == NOTE, 0, NoteSize))
									{
										CursorPos = NOTE;
										CursorX = i;
										CursorY = j;
										SetScrollY(ScrollValue());
									}
									PopID();
									PushID(IDOffset + i + (j * 40) + 1);
									ImGui::TableNextColumn();

									if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
									}
									else
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
									}
									if (Selectable(Channels[i].InstrumentView(j).c_str(), IsPoint && CursorPos == INSTR, 0, MiscSize))
									{
										CursorPos = INSTR;
										CursorX = i;
										CursorY = j;
										SetScrollY(ScrollValue());
									}
									PopID();
									PushID(IDOffset + i + (j * 40) + 2);
									ImGui::TableNextColumn();

									if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
									}
									else
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
									}
									if (Selectable(Channels[i].VolumeView(j).c_str(), IsPoint && CursorPos == VOLUME, CursorPos == VOLUME, MiscSize))
									{
										CursorPos = VOLUME;
										CursorX = i;
										CursorY = j;
										SetScrollY(ScrollValue());
									}
									PopID();
									PushID(IDOffset + i + (j * 40) + 3);
									ImGui::TableNextColumn();

									if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
									}
									else
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
									}
									if (Selectable(Channels[i].EffectView(j).c_str(), IsPoint && CursorPos == EFFECT, 0, MiscSize))
									{
										CursorPos = EFFECT;
										CursorX = i;
										CursorY = j;
										SetScrollY(ScrollValue());
									}
									PopID();
									PushID(IDOffset + i + (j * 40) + 4);
									ImGui::TableNextColumn();

									if (BoxSelected && j >= SelectionBoxY1 && j <= SelectionBoxY2 && i >= SelectionBoxX1 && i <= SelectionBoxX2 && CursorPos >= SelectionBoxSubX1 && CursorPos <= SelectionBoxSubX2)
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, SelectionBoxCol);
									}
									else
									{
										TableSetBgColor(ImGuiTableBgTarget_RowBg0, col);
									}
									if (Selectable(Channels[i].Effectvalue(j).c_str(), IsPoint && CursorPos == VALUE, 0, MiscSize))
									{
										CursorPos = VALUE;
										CursorX = i;
										CursorY = j;
										SetScrollY(ScrollValue());
									}
									PopID();
								}
								ImGui::EndTable();
							}
						}
					}
					ImGui::TableNextColumn();
				}
			}
			ImGui::EndTable();
		}
	}
	End();
}

void Tracker::Samples()
{
	if (Begin("Samples"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		if (Button("Add", ImVec2(GetWindowWidth() * .3, 24)))
		{
			LoadingSample = true;
		}
		SameLine();
		if (Button("Delete", ImVec2(GetWindowWidth() * .3, 24)) && samples.size() > 1)
		{
			cout << "SAMPLE LIST SIZE: " << samples.size();
			if (SelectedSample > samples.size())
			{
				samples.erase((samples.begin()) + SelectedSample);
				SelectedSample--;
			}
			else if (SelectedSample > 0)
			{
				samples.erase((samples.begin()) + SelectedSample);
			}
			SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
		}
		SameLine();
		if (Button("Copy", ImVec2(GetWindowWidth() * .3, 24)) && samples.size() > 1)
		{
			int index = samples.size();
			Sample newsamp = samples[SelectedSample];
			newsamp.SampleName += to_string(index);
			samples.push_back(newsamp);
			cout << samples.size();
			SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
		}

		if (samples.size() > 0)
		{
			if (BeginTable("SampleTable", 1, TABLE_FLAGS, ImVec2(GetWindowWidth() * 0.75, 24), 24)) {
				for (int i = 0; i < samples.size(); i++)
				{
					IDOffset++;
					PushID(IDOffset);
					Text(to_string(i).c_str());
					SameLine();
					if (SelectedSample <= samples.size() - 1)
					{
						if (Selectable(samples[i].SampleName.c_str(), SelectedSample == i))
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
					ImGui::TableNextColumn();
					PopID();
				}
				ImGui::EndTable();
			}
		}
	}
	End();
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
			InputText("Sample Name", (char*)samples[SelectedSample].SampleName.c_str(), sizeof(samples[SelectedSample].SampleName));
			InputInt("Playing HZ", &samples[SelectedSample].SampleRate);
			InputInt("Fine Tune", (int*)&samples[SelectedSample].FineTune, 1, 1);
			if (Checkbox("Loop Sample", &samples[SelectedSample].Loop)) {
				SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
				SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
			}
			if (InputInt("Loop Start", (int*)&samples[SelectedSample].LoopStart, 16, 0)) {
				SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
				SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
			}
			if(InputInt("Loop End", (int*)&samples[SelectedSample].LoopEnd, 16, 0)){
				SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
				SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
			}
			if (samples.size() > 1)
			{
				ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
				ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, PlotLineWeight);

				ImPlot::SetNextAxisLimits(y, -32768, 32767);
				ImPlot::SetNextAxisLimits(x, 0, samples[SelectedSample].SampleData.size());

				if (ImPlot::BeginPlot("Waveform", ImVec2(GetWindowWidth() * 0.9, GetWindowHeight() * 0.7), IMPLOT_FLAGS)) 
				{
					ImPlot::SetupAxis(x, nullptr, ImPlotAxisFlags_NoLabel);
					ImPlot::SetupAxis(y, nullptr, ImPlotAxisFlags_Lock);
					ImPlot::PushColormap("RGBColors");
					vector<float> LoopView;
					vector<float> SampleView;
					float PixelMult = 12;
					float PlotPixelWidth = (ImPlot::GetPlotSize().x * PixelMult) / samples[SelectedSample].SampleData.size();
					float PixelAccum = 0;
					int before = 0;
					for (int i = 0; i < samples[SelectedSample].SampleData.size(); i++)
					{
						//Downsampling check
						before = PixelAccum;
						PixelAccum += PlotPixelWidth;
						if ((int)PixelAccum > before)
						{
							SampleView.push_back(samples[SelectedSample].SampleData[i]);
							LoopView.push_back(32767 * 2);
						}
					}

					ImPlot::PlotLine("Wave Data", SampleView.data(), SampleView.size());
					ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, .5f * samples[SelectedSample].Loop);
					
					if (samples[SelectedSample].LoopEnd > samples[SelectedSample].brr.DBlocks.size()*16)
					{
						samples[SelectedSample].LoopEnd = samples[SelectedSample].SampleData.size();
						SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
						SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
					}
					else if (samples[SelectedSample].LoopEnd < 16)
					{
						samples[SelectedSample].LoopEnd = 16;
						SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
						SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
					}
					else if (samples[SelectedSample].LoopStart < 0)
					{
						samples[SelectedSample].LoopStart = 0;
						SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
						SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
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
		}
		End();
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
				//This should be for visual changes
				if (BeginTabItem("Appearance")) 
				{
					Text("Screen Resolutions");
					
					if (BeginCombo("##Screen Resolutions", ResolutionNames[SManager.CustomData.Res].c_str()))
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

					NewLine();
					Text("Notation Style");

					if (BeginCombo("##NotationStyle", NotationNames[SManager.CustomData.NStyle].c_str()))
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
					NewLine();
					Text("Font Size:");
					InputInt("##Fontsize", &SManager.CustomData.FontSize, 1, 1);

					if (SManager.CustomData.FontSize < 1) SManager.CustomData.FontSize = 1;
					if (SManager.CustomData.FontSize > 48) SManager.CustomData.FontSize = 48;
					EndTabItem();

				}

				//For general use of the tracker
				if (BeginTabItem("User based"))
				{
					Checkbox("Move cursor by step count", &MoveByStep);
					Checkbox("Move cursor on delete", &MoveOnDelete);
					EndTabItem();
				}

				if (BeginTabItem("Technical"))
				{
					Text("FPS");
					InputInt("##FPS", &FPS, 1, 1);
					if (FPS > MAX_FPS) FPS = MAX_FPS; else if (FPS < 1) FPS = 1;

NewLine();
Text("Audio Buffer Size");

if (BeginCombo("##Buffer", BufferNames[SManager.CustomData.Buf].c_str()))
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

				SetCursorPosY(GetWindowHeight() - (TextSize + ImGui::GetStyle().FramePadding.y) * 2);

				SetCursorPosX(GetWindowWidth() - (TextSize + ImGui::GetStyle().FramePadding.x + 18) * 2);
				if (Button("Apply"))
				{
					UpdateSettings(0);
				}
				if (IsItemHovered())
				{
					BeginTooltip();
					Text("Note: Changes apply ONLY when program is closed\nText size may or may not break some parts of the visual UI, so please take care when changing!");
					EndTooltip();
				}
				SameLine();
				SetCursorPosX(GetWindowWidth() - (TextSize + ImGui::GetStyle().FramePadding.x + 18) * 4);
				if (Button("Cancel"))
				{
					ResetSettings();
					ShowSettings = false;
				}
			}
		}
		End();
	}
}

void Tracker::Misc_View()
{
	if (Begin("MiscView"), true, UNIVERSAL_WINDOW_FLAGS)
	{
		InputInt("Octave", &Octave, 1, 8);
		InputInt("Step", &Step, 1, 8);
		Octave > 8 ? Octave = 8 : Octave < 0 ? Octave = 0 : Octave;
		if (SliderInt("Master Volume", &VolumeScale, -128, 127)) {
			assert(SG.Emu_APU.APU_Set_Master_Vol(VolumeScale));
		}
	}
	End();
}

void Tracker::Author_View()
{
	if (Begin("Author"), 0, UNIVERSAL_WINDOW_FLAGS)
	{
		Text("Subtunes");
		if (filehandler.mod.subtune.size() != 0)
		{
			if (BeginCombo("##Subtunes", filehandler.mod.subtune[CurrentTune].trbuf))
			{
				for (int x = 0; x < filehandler.mod.subtune.size(); x++)
				{
					PushID(IDOffset++);
					bool selected = (x == CurrentTune);
					string txt = to_string(x) + ": ";
					Text(txt.c_str());
					SameLine();
					if (Selectable(filehandler.mod.subtune[x].trbuf, selected))
					{
						CurrentTune = x;
						ApplySubtune();
					}
					if (selected)
					{
						SetItemDefaultFocus();
					}
					PopID();
				}
				EndCombo();
			}
		}
		SameLine();
		if (Button("+") && MaxTune < 255)
		{
			MaxTune++;
			CurrentTune = MaxTune;
		}
		SameLine();
		if (Button("-") && MaxTune > 1)
		{
			filehandler.mod.subtune.erase(filehandler.mod.subtune.begin() + CurrentTune);
			MaxTune--;
			CurrentTune = MaxTune;
		}
		Text("Song");
		InputText("##SongTitle", songbuf, sizeof(songbuf));
		Text("Author");
		InputText("##Author", authbuf, sizeof(authbuf));
		Text("Desc");
		InputTextMultiline("##Desc", descbuf, sizeof(descbuf), ImVec2(GetWindowWidth() * 0.65, GetWindowHeight() * 0.7));
	}
	End();
}

void Tracker::Speed_View()
{
	if (Begin("SpeedView",0, UNIVERSAL_WINDOW_FLAGS))
	{
		Text("Speed");
		InputInt("##Speed 1", &Speed1, 1, 31);
		Text("Tempo divider");
		InputInt("##Tempo divider", &TempoDivider, 1, 1);
		
		if (TempoDivider < 1) TempoDivider = 1;
		else if (TempoDivider > 255) TempoDivider = 255;

		if (Speed1 < 1)	Speed1 = 1;
		else if (Speed1 > 255) Speed1 = 255;

		string temp = "Tempo: ";
		temp += to_string(BaseTempo / TempoDivider);
		Text(temp.c_str());
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
		/*
		NewLine();
		Text("Region");
		for (int n = 0; n < 2; n++)
		{
			bool Selected = (SelectedRegion == n);
			if (RadioButton(RegionNames[n].c_str(), Selected)) {
				SelectedRegion = n;
				reg = (Region)SelectedRegion;
				SG.Emu_APU.reg = (Region)SelectedRegion;
			}
		}*/
	}
	End();
}

void Tracker::Info_View()
{
	if (Begin("Information")) 
	{
		ImDrawList* draw_list = GetWindowDrawList();
		ImVec2 p = GetCursorScreenPos();
		int xpos = p.x;
		int ypos = p.y + TextSize;
		int StepSize = (xpos - GetWindowWidth()) / inst.size();
		int MaxRange = 65536;
		int ReservedSpace = Sample_Mem_Page;
		int UsedSpace = (2048 * Delay);
		int InstrumentSpace = 0;

		int EchoSpace = UsedSpace;
		int SampleSpace = 0;
		int BoxHeight = 64;
		for (int x = 1; x < samples.size(); x++)
		{
			UsedSpace += samples[x].brr.DBlocks.size() * 9;
			SampleSpace += samples[x].brr.DBlocks.size() * 9;
		}
		for (int x = 1; x < inst.size(); x++)
		{
			UsedSpace += 9;
			InstrumentSpace += 9;//While technically wasting 6 bits here, I can't be bothered changing it
		}
		UsedSpace += ReservedSpace;
		int LastPos = 0;

		//Background Rect to show bounds
		if (UsedSpace > MaxRange)
		{
			Text(("Used space: " + to_string(UsedSpace) + " bytes" + " Too much data used!!!").c_str());
			draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + GetWindowWidth() * 0.95f, ypos + BoxHeight), ColorConvertFloat4ToU32(ReleaseColour), .25f, 0);
		}
		else
		{
			int UsedSpaceBounds = (UsedSpace * GetWindowWidth() * 0.95f) / MaxRange;

			Text(("Used space: " + to_string(UsedSpace) + " bytes").c_str());
			draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + GetWindowWidth() * 0.95f, ypos + BoxHeight), ColorConvertFloat4ToU32(H2Col), .25f, 0);
			if (GetIO().MousePos.x > xpos + UsedSpaceBounds && GetIO().MousePos.x < xpos + GetWindowWidth() * 0.95f)
			{
				if (GetIO().MousePos.y > ypos && GetIO().MousePos.y < ypos + BoxHeight)
				{
					BeginTooltip();
					Text("Empty Space");
					EndTooltip();
				}
			}
			draw_list->AddRectFilled(ImVec2(xpos, ypos), ImVec2(xpos + (ReservedSpace * GetWindowWidth() * 0.95f) / MaxRange , ypos + BoxHeight), ColorConvertFloat4ToU32(ReservedColour), .25f, 0);
			LastPos += (ReservedSpace * GetWindowWidth() * 0.95f) / MaxRange;
			if (GetIO().MousePos.x > xpos && GetIO().MousePos.x < xpos + (ReservedSpace * GetWindowWidth() * 0.95f) / MaxRange)
			{
				if (GetIO().MousePos.y > ypos && GetIO().MousePos.y < ypos + BoxHeight)
				{
					BeginTooltip();
					Text("Reserved Space");
					string byteamount = to_string(ReservedSpace);
					byteamount += " bytes";
					Text(byteamount.c_str());
					EndTooltip();
				}
			}
			draw_list->AddRectFilled(ImVec2(xpos + LastPos, ypos), ImVec2(xpos + LastPos + (SG.Emu_APU.LastSamplePoint * GetWindowWidth() * 0.95f) / MaxRange, ypos + BoxHeight), ColorConvertFloat4ToU32(SampColour), .25f, 0);
			if (GetIO().MousePos.x > xpos + LastPos && GetIO().MousePos.x < xpos + LastPos + (SG.Emu_APU.LastSamplePoint * GetWindowWidth() * 0.95f) / MaxRange)
			{
				if (GetIO().MousePos.y > ypos && GetIO().MousePos.y < ypos + BoxHeight)
				{
					BeginTooltip();
					Text("Sample Space");
					string byteamount = to_string(SG.Emu_APU.LastSamplePoint);
					byteamount += " bytes";
					Text(byteamount.c_str());
					EndTooltip();
				}
			}
			LastPos += (SG.Emu_APU.LastSamplePoint * GetWindowWidth() * 0.95f) / MaxRange;

			draw_list->AddRectFilled(ImVec2(xpos + LastPos, ypos), ImVec2(xpos + LastPos + (InstrumentSpace * GetWindowWidth() * 0.95f) / MaxRange, ypos + BoxHeight), ColorConvertFloat4ToU32(InstColour), .25f, 0);
			if (GetIO().MousePos.x > xpos + LastPos && GetIO().MousePos.x < xpos + LastPos + (InstrumentSpace * GetWindowWidth() * 0.95f) / MaxRange)
			{
				if (GetIO().MousePos.y > ypos && GetIO().MousePos.y < ypos + BoxHeight)
				{
					BeginTooltip();
					Text("Instrument Space");
					string byteamount = to_string(InstrumentSpace);
					byteamount += " bytes";
					Text(byteamount.c_str());
					EndTooltip();
				}
			}
			LastPos += (InstrumentSpace * GetWindowWidth() * 0.95f) / MaxRange;

			draw_list->AddRectFilled(ImVec2(xpos + LastPos, ypos), ImVec2(xpos + LastPos + (EchoSpace * GetWindowWidth() * 0.95f) / MaxRange, ypos + BoxHeight), ColorConvertFloat4ToU32(EchoColour), .25f, 0);
			if (GetIO().MousePos.x > xpos + LastPos && GetIO().MousePos.x < xpos + LastPos + (EchoSpace * GetWindowWidth() * 0.95f) / MaxRange)
			{
				if (GetIO().MousePos.y > ypos && GetIO().MousePos.y < ypos + BoxHeight)
				{
					BeginTooltip();
					Text("Echo Space");
					string byteamount = to_string(EchoSpace);
					byteamount += " bytes";
					Text(byteamount.c_str());
					EndTooltip();
				}
			}
			LastPos += (EchoSpace * GetWindowWidth() * 0.95f) / MaxRange;

		}
	}
	End();
}

void Tracker::EchoSettings()
{
	if (ShowEcho)
	{
		if (Begin("EchoSettings"),true, UNIVERSAL_WINDOW_FLAGS)
		{
			SliderInt("Echo Volume", &EchoVol, -128, 127);

			if(SliderInt("Delay", &Delay, 0, 15)) {
				SG.Emu_APU.APU_Set_Echo(Delay, EchoFilter, Feedback, EchoVol);
			}
			if(SliderInt("Feedback", &Feedback, 0, 127)) {
				SG.Emu_APU.APU_Set_Echo(Delay, EchoFilter, Feedback, EchoVol);
			}

			int FilterAccum = 0;
			Text("Echo filter");
			for (int i = 0; i < 8; i++)
			{
				FilterAccum += EchoFilter[i];
				if(SliderInt(to_string(i).c_str(), &EchoFilter[i], -128, 127)) {
					SG.Emu_APU.APU_Set_Echo(Delay, EchoFilter, Feedback, EchoVol);
				}
			}
			string FilterText = "Filter total: " + to_string(FilterAccum);
			Text(FilterText.c_str());
			if (Button("Close", ImVec2(64, TextSize * 1.5))) {
				ShowEcho = false;
			}
		}
		End();
	}
}

void Tracker::SetupInstr()
{
	inst.clear();
	DefaultInst.Index = 0;
	DefaultInst.Name = "Instrument: ";
	DefaultInst.SampleIndex = 0;
	DefaultInst.Volume = 127;
	DefaultInst.LPan = 127;
	DefaultInst.RPan = 127;
	DefaultInst.Gain = 127;
	DefaultInst.InvL = false;
	DefaultInst.InvR = false;
	DefaultInst.PitchMod = false;
	DefaultInst.Echo = false;
	DefaultInst.Noise = false;
	DefaultInst.Attack = 0;
	DefaultInst.Decay = 0;
	DefaultInst.Sustain = 0;
	DefaultInst.Release = 0;
	inst.push_back(DefaultInst);

	samples.clear();
	DefaultSample.SampleIndex = 0;
	DefaultSample.SampleName = "Choose a sample!";
	DefaultSample.FineTune = 0;
	DefaultSample.SampleRate = 0;
	DefaultSample.Loop = false;
	DefaultSample.LoopStart = 0;
	DefaultSample.LoopEnd = 0;
	DefaultSample.SampleData.clear();
	samples.push_back(DefaultSample);

	DefaultPattern.Index = 0;
	DefaultPattern.SetUp(TrackLength);
}

void Tracker::UpdateFont()
{
	if ((TextSize - 12.0f) != 0)
	{
	}
	else
	{
	}
	FontUpdate = false;
}

void Tracker::Export_View()
{
	string TypeNames[3] = { "WAV","MP3","OGG"};
	string TechnicalTypeNames[3] = {"SPC","ASM","SHVC" };
	string Qualitynames[8] = { "8KHz","11KHz","16KHz","22KHz","24KHz","32KHz","44KHz","48KHz" };
	string DepthName[2] = { "8 bit", "16 bit"};
	string SignName[2] = { "Unsigned", "Signed"};
	ImVec2 center = GetMainViewport()->GetCenter();
	SetNextWindowPos(center, 0, ImVec2(0.5f, 0.5f));
	if(Begin("Export", 0, UNIVERSAL_WINDOW_FLAGS)){
		if (BeginTabBar("Audio"), TAB_FLAGS) {
			if (BeginTabItem("Audio Export", 0, TAB_ITEM_FLAGS)) {
				Text("Export Type");
				if (BeginCombo("##Export Type", TypeNames[SelectedExportType].c_str())) {
					for (int x = 0; x < 3; x++)
					{
						bool Selected = (SelectedExportType == x);
						if (Selectable(TypeNames[x].c_str(), Selected))
						{
							SelectedExportType = x;
						}
						if (Selected)
						{
							SetItemDefaultFocus();
						}
					}
					EndCombo();
				}
				NewLine();
				Text("Export Quality");
				if (BeginCombo("##Export Quality", Qualitynames[SelectedQualityType].c_str())) {
					for (int x = 0; x < 8; x++)
					{
						bool Selected = (SelectedQualityType == x);
						if (Selectable(Qualitynames[x].c_str(), Selected))
						{
							SelectedQualityType = x;
						}
						if (Selected)
						{
							SetItemDefaultFocus();
						}
					}
					EndCombo();
				}
				NewLine();
				Text("Bit Depth");
				for (int n = 0; n < 2; n++)
				{
					bool Selected = (SelectedDepthType == n);
					if (RadioButton(DepthName[n].c_str(), Selected)) {
						SelectedDepthType = n;
					}
					SameLine();
				}
				NewLine();
				Text("Bit Sign");
				for (int n = 0; n < 2; n++)
				{
					bool Selected = (SelectedSignType == n);
					if (RadioButton(SignName[n].c_str(), Selected)) {
						SelectedSignType = n;
					}
					SameLine();
				}

				EndTabItem();
			}
			if (BeginTabItem("Technical Export",0, TAB_FLAGS))
			{
				Text("Export Type");
				if (BeginCombo("##Export Type", TechnicalTypeNames[SelectedTechnicalType].c_str())) {
					for (int x = 0; x < 2; x++)
					{
						bool Selected = (SelectedTechnicalType == x);
						if (Selectable(TechnicalTypeNames[x].c_str(), Selected))
						{
							SelectedTechnicalType = x;
						}
						if (Selected)
						{
							SetItemDefaultFocus();
						}
					}
					EndCombo();
				}

				NewLine();
				/*
				Text("Address");
				if (SG.Emu_APU.SONG_ADDR > 65535) {
					SG.Emu_APU.SONG_ADDR = 65535;
				}
				if (SG.Emu_APU.SONG_ADDR < 0) {
					SG.Emu_APU.SONG_ADDR = 0;
				}
				InputInt("##addr", (int*)&SG.Emu_APU.SONG_ADDR, 0x10, 0x1000);
				*/
				EndTabItem();
			}
			NewLine();
			NewLine();
			ImVec2 size = ImVec2(GetWindowWidth() * 0.45, TextSize * 1.5);
			if (Button("CANCEL", size)) {
				ShowExport = false;
			}
			SameLine();
			if (Button("EXPORT", size)) {
			}

			EndTabBar();
		}
	}
	End();
}

void Tracker::EffectsText(int effect)
{
}

//Core updates to the tracker state
void Tracker::RunTracker()
{
	//Upate the row index based off of tick timing

	TickTimer -= SG.Emu_APU.APU_Return_Cycle_Since_Last_Frame();

	if (CursorY >= TrackLength - 1 && PatternIndex >= orders->size() - 1)
	{
		PlayingMode = false;
	}
	else if (CursorY >= TrackLength)
	{
		CursorY = 0;
		SelectedPattern++;
		PatternIndex++;
		int currentindex = 0;
		for (int i = 0; i < 8; i++)
		{
			currentindex = orders[i][SelectedPattern].Index;
			for (int j = 0; j < TrackLength; j++)
			{
				Channels[i].Rows[j].note =			StoragePatterns[currentindex].SavedRows[j].note;
				Channels[i].Rows[j].instrument =	StoragePatterns[currentindex].SavedRows[j].instrument;
				Channels[i].Rows[j].volume =		StoragePatterns[currentindex].SavedRows[j].volume;
				Channels[i].Rows[j].effect =		StoragePatterns[currentindex].SavedRows[j].effect;
				Channels[i].Rows[j].effectvalue =	StoragePatterns[currentindex].SavedRows[j].effectvalue;
			}
		}
	}
	else if (TickTimer < 0)
	{
		TickTimer = 32000.0 / (float)BaseTempo * TempoDivider;
		TickCounter++;

		if (TickCounter > Speed1 * TempoDivider)
		{
			UpdateRows();
			TickCounter = 0;
		}
		UpdateTicks();
	}
}

void Tracker::UpdateRows()
{
	for (int i = 0; i < 8; i++)
	{
		Channels[i].TickCheck(CursorY % TrackLength, inst, samples);
	}
	//CursorY++;
	//cout << "\nCurrent Value: " << ((double)CursorY - 1.0/(24.0 / (double)TrackLength) / (double)TrackLength) * ((TextSize + GetStyle().CellPadding.y * 2) * TrackLength);
}

void Tracker::UpdateTicks()
{
	for (int x = 0; x < 8; x++)
	{
		SG.Emu_APU.APU_Process_Effects(&Channels[x], &inst[Channels[x].CurrentInstrument], CursorY, Speed1, PatternIndex, TickCounter);
	}
}

//Update audio buffer, also runs SPC and DSP emulation
void Tracker::UpdateAudioBuffer()
{
	if (SDL_GetQueuedAudioSize(dev) < SG.TRACKER_AUDIO_BUFFER * 8)
	{
		SG.Update(DeltaTimeHolder, Channels, samples, CursorY, inst);
		SG.Emu_APU.APU_Update(SG.Totalbuffer[0].data(), 2 * SG.TRACKER_AUDIO_BUFFER);
		SDL_QueueAudio(dev, SG.Totalbuffer[0].data(), sizeof(Sint16) * 2 * SG.TRACKER_AUDIO_BUFFER);
	}
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
		else if (InitTimer > InitKeyTime)
			KeyTimer += DeltaTimeHolder;
		else
			InitTimer += DeltaTimeHolder;
	}
	else
	{
		InitPressed = false;
		KeyTimer = 0;
		InitTimer = 0;
	}

	if (!IsPressed)
	{
		//cout << "\nCurrent key: " << CurrentKey;
		if (CurrentMod & GLFW_MOD_CONTROL)
		{
			switch (CurrentKey)
			{
			case GLFW_KEY_S:
				SavingFile = true;
				break;
			}
		}
		else if (CurrentMod & GLFW_MOD_SHIFT)
		{
			if (!BoxSelected)
			{
				SelectionBoxX1 = SelectionBoxX2 = x;
				SelectionBoxY1 = SelectionBoxY2 = y;
				SelectionBoxSubX1 = SelectionBoxSubX2 = CursorPos;
				BoxSelected = true;
			}

			switch (CurrentKey) {
			case GLFW_KEY_LEFT:
				CursorPos--;
				SelectionBoxSubX2 = max(SelectionBoxSubX2 - 1, 0);
				break;
			case GLFW_KEY_RIGHT:
				CursorPos++;
				SelectionBoxSubX2 = min(SelectionBoxSubX2 + 1, 40);
				break;
			case GLFW_KEY_DOWN:
				CursorY++;
				SelectionBoxY2++;
				break;
			case GLFW_KEY_UP:
				CursorY--;
				SelectionBoxY2--;
				break;
			}
			cout << "\nSelectionBoxSubX1: " << SelectionBoxSubX1 << "\n";
			cout << "\nSelectionBoxSubX2: " << SelectionBoxSubX2 << "\n";
		}
		else
		{
			switch (CurrentKey) {
			case GLFW_KEY_HOME:
				CursorY = 0;
				SetScrollY(ScrollValue());
				break;
			case GLFW_KEY_END:
				CursorY = TrackLength - 1;
				SetScrollY(ScrollValue());
				break;
			case GLFW_KEY_PAGE_UP:
				CursorY -= 16;
				SetScrollY(ScrollValue());
				break;
			case GLFW_KEY_PAGE_DOWN:
				CursorY += 16;
				SetScrollY(ScrollValue());
				break;
			case GLFW_KEY_LEFT:
				CursorPos--;
				BoxSelected = false;
				break;
			case GLFW_KEY_RIGHT:
				CursorPos++;
				BoxSelected = false;
				break;
			case GLFW_KEY_DOWN:
				CursorY += (MoveByStep) ? Step : 1;
				BoxSelected = false;
				SetScrollY(ScrollValue());
				break;
			case GLFW_KEY_UP:
				CursorY -= (MoveByStep) ? Step : 1;
				BoxSelected = false;
				SetScrollY(ScrollValue());
				break;
			}
		}

		switch (CurrentKey) {
		case GLFW_KEY_SPACE:
			EditingMode = !EditingMode;
			break;
		case GLFW_KEY_ENTER://Controls if the tracker is in "Play" mode
			PlayingMode = !PlayingMode;
			if (!PlayingMode) {
				SG.Emu_APU.APU_Audio_Stop();
				SG.Emu_APU.APU_Audio_Start();
			}
			EditingMode = false;
			break;
		}

		for (int i = 0; i < 24; i++)
		{
			if (CurrentKey == NoteInput[i] && inst.size() > 1)
			{
				SG.Emu_APU.APU_Play_Note_Editor(&Channels[CursorX], &inst[SelectedInst], i + (12 * Octave), CurPos == NOTE);
			}
		}
		
		if (EditingMode)
		{
			if (BoxSelected && CurrentKey == GLFW_KEY_DELETE)
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
				SetScrollY(ScrollValue());
				//For editing the stuff in the subcolumns
				switch (CurPos)
				{
				case NOTE:
					for (int i = 0; i < 24; i++)
					{
						if (CurrentKey == NoteInput[i])
						{
							Channels[x].Rows[y].note = i + (12 * Octave);
							(i < 12) ? Channels[x].Rows[y].octave = Octave : Channels[x].Rows[y].octave = Octave+1;
							if (SelectedInst) Channels[x].Rows[y].instrument = SelectedInst;
							//Channels[x].Rows[y].S_Note = Channels[x].NoteNames[Channels[x].Rows[y].note % 12] + to_string(Channels[x].Rows[y].octave);
							CursorY += Step;
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_DELETE)
						{
							Channels[x].Rows[y].note = NULL_COMMAND;
							Channels[x].Rows[y].instrument = NULL_COMMAND;
							CursorY += (MoveOnDelete) ? Step : 1;
							ChangePatternData(x, y);
							break;
						}
						else if(CurrentKey == GLFW_KEY_GRAVE_ACCENT)//Release
						{
							Channels[x].Rows[y].note = RELEASE_COMMAND;
							Channels[x].Rows[y].instrument = NULL_COMMAND;
							CursorY += (MoveOnDelete) ? Step : 1;
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_1)//Stop
						{
							Channels[x].Rows[y].note = STOP_COMMAND;
							Channels[x].Rows[y].instrument = NULL_COMMAND;
							CursorY += (MoveOnDelete) ? Step : 1;
							ChangePatternData(x, y);
							break;
						}
					}
					break;

				case INSTR:
					for (int i = 0; i < 16; i++)
					{
						if (CurrentKey == VolInput[i])
						{
							Channels[x].Rows[y].instrument = Channels[x].EvaluateHexInput(i, y, MAX_INSTRUMENT, INSTR);
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_DELETE)
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
						if (CurrentKey == VolInput[i])
						{
							Channels[x].Rows[y].volume = Channels[x].EvaluateHexInput(i, y, MAX_VOLUME, VOLUME);
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_DELETE)
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
						if (CurrentKey == VolInput[i])
						{
							Channels[x].Rows[y].effect = Channels[x].EvaluateHexInput(i, y, MAX_EFFECT, EFFECT);
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_DELETE)
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
						if (CurrentKey == VolInput[i])
						{
							Channels[x].Rows[y].effectvalue = Channels[x].EvaluateHexInput(i, y, MAX_EFFECT_VALUE, VALUE);
							ChangePatternData(x, y);
							break;
						}
						else if (CurrentKey == GLFW_KEY_DELETE)
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
				UpdateAllPatterns();
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
			cout << "\n" + FileName + "\n" + FilePath + "\n";
			SF_INFO soundinfo;
			soundinfo.format = AUDIO_FORMATS;
			soundinfo.samplerate = SPS;
			soundinfo.channels = 1;
			soundinfo.frames = SG.TRACKER_AUDIO_BUFFER;

			SNDFILE* file = sf_open(FileName.c_str(), SFM_READ, &soundinfo);
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
						cur.SampleData.push_back(FileBuffer[i]);
					}

					if (cur.SampleData.size() < 16)//Assuming we have an invalid sample
					{
						ErrorMessage = FILE_ERORR_03;
						ShowError = true;
					}
					else //Assume we have a valid sample
					{
						cur.SampleIndex = SelectedSample+1;
						FileName.erase(0, FilePath.length() + 1);
						cur.SampleName = FileName;
						cur.SampleRate = soundinfo.samplerate;
						cur.Loop = false;
						cur.LoopStart = 0;
						cur.LoopEnd = 0;
						cur.FineTune = 0;
						SelectedSample = samples.size();
						cur.SampleIndex = SelectedSample;
						//Assume the file isn't fucked and we can move to the BRR conversion
						cur.BRRConvert();
						samples.push_back(cur);
						//Memory shit
						SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[SelectedSample], samples[SelectedSample].LoopEnd);
						SG.Emu_APU.APU_Evaluate_BRR_Loop_Start(&samples[SelectedSample]);
						SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);
					}
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
		else
		{
			LoadingSample = false;
			ImGuiFileDialog::Instance()->Close();
		}
		LoadingSample = false;
		ImGuiFileDialog::Instance()->Close();
		cout << FileName + "\n" + FilePath;
		// close
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
	if (orders[x][y].Index > Maxindex - 1)
	{
		Maxindex = orders[x][y].Index;
		Patterns pat;
		pat = DefaultPattern;
		StoragePatterns.push_back(pat);
		//patterns->push_back(pat);
	}

	cout << "\n" << x << "\n" << y;
	for (int i = 0; i < TrackLength; i++)
	{
		Channels[x].Rows[i].note = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].note;
		Channels[x].Rows[i].octave = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].octave;
		Channels[x].Rows[i].instrument = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].instrument;
		Channels[x].Rows[i].volume = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].volume;
		Channels[x].Rows[i].effect = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].effect;
		Channels[x].Rows[i].effectvalue = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].effectvalue;
	}
}

void Tracker::UpdateAllPatterns()
{
	for (int x = 0; x < 8; x++)
	{
		for (int i = 0; i < TrackLength; i++)
		{
			Channels[x].Rows[i].note = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].note;
			Channels[x].Rows[i].octave = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].octave;
			Channels[x].Rows[i].instrument = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].instrument;
			Channels[x].Rows[i].volume = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].volume;
			Channels[x].Rows[i].effect = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].effect;
			Channels[x].Rows[i].effectvalue = StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[i].effectvalue;
		}
	}
}

void Tracker::ChangePatternData(int x, int y)
{
	cout << "\nCHANGED PATTERN DATA:" << "\nX: " << x << "\nY: " << y << "\nSelected Pattern " << SelectedPattern;
	cout << "\n" << orders->size() << " : " << orders[x].size();

	//Put data into channel
	orders[x][SelectedPattern].SavedRows[y].note = Channels[x].Rows[y].note;
	orders[x][SelectedPattern].SavedRows[y].octave = Channels[x].Rows[y].octave;
	orders[x][SelectedPattern].SavedRows[y].instrument = Channels[x].Rows[y].instrument;
	orders[x][SelectedPattern].SavedRows[y].volume = Channels[x].Rows[y].volume;
	orders[x][SelectedPattern].SavedRows[y].effect = Channels[x].Rows[y].effect;
	orders[x][SelectedPattern].SavedRows[y].effectvalue = Channels[x].Rows[y].effectvalue;

	//Put data into channel
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].note = Channels[x].Rows[y].note;
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].octave = Channels[x].Rows[y].octave;
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].instrument = Channels[x].Rows[y].instrument;
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].volume = Channels[x].Rows[y].volume;
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].effect = Channels[x].Rows[y].effect;
	StoragePatterns[orders[x][SelectedPattern].Index].SavedRows[y].effectvalue = Channels[x].Rows[y].effectvalue;
}

void Tracker::UpdateSettings(int w)
{
	for (int i = 0; i < 8; i++)
	{
		Channels[i].NoteType = SManager.CustomData.NStyle;
		SManager.GetNotation(Channels[i].NoteType, true);
	}
	SManager.GetBuffer(SG.TRACKER_AUDIO_BUFFER, true);
	SManager.GetResolution(SCREEN_WIDTH, SCREEN_HEIGHT, true);
	FPS = SManager.CustomData.FPS;
	TextSize = SManager.CustomData.FontSize;
	TextSizeLarge = TextSize*2;
	TrackLength = SManager.CustomData.DefaultTrackSize;
	if (w == 1)
	{
		MoveOnDelete = SManager.CustomData.DeleteMovesAtStepCount;
		MoveByStep = SManager.CustomData.CursorMovesAtStepCount;
	}
	else
	{
		SManager.CustomData.DeleteMovesAtStepCount = MoveOnDelete;
		SManager.CustomData.CursorMovesAtStepCount = MoveByStep;
	}
	SManager.DefaultData = SManager.CustomData;
	SG.SetBufferSize(SG.TRACKER_AUDIO_BUFFER);
	SManager.CreateSettings();
	FontUpdate = true;
}

void Tracker::ResetSettings()
{
	SManager.CustomData = SManager.DefaultData;
	UpdateSettings(0);
}

void Tracker::UpdateModule()
{
	//Tracker
	filehandler.mod.subtune.resize(MaxTune + 1);
	memcpy(filehandler.mod.subtune[CurrentTune].aubuf, authbuf, sizeof(authbuf));
	memcpy(filehandler.mod.subtune[CurrentTune].trbuf, songbuf, sizeof(songbuf));
	memcpy(filehandler.mod.subtune[CurrentTune].dcbuf, descbuf, sizeof(descbuf));
	filehandler.mod.subtune[CurrentTune].AuthorName = filehandler.mod.subtune[CurrentTune].aubuf;
	filehandler.mod.subtune[CurrentTune].TrackName =  filehandler.mod.subtune[CurrentTune].trbuf;
	filehandler.mod.subtune[CurrentTune].TrackDesc =  filehandler.mod.subtune[CurrentTune].dcbuf;
	filehandler.mod.samples = samples;
	filehandler.mod.inst = inst;
	filehandler.mod.patterns = StoragePatterns;
	filehandler.mod.subtune[CurrentTune].TrackLength = TrackLength;
	filehandler.mod.subtune[CurrentTune].SongLength = SongLength;
	filehandler.mod.subtune[CurrentTune].Speed1 = Speed1;
	filehandler.mod.subtune[CurrentTune].TempoDivider = TempoDivider;
	filehandler.mod.subtune[CurrentTune].Highlight1 = Highlight1;
	filehandler.mod.subtune[CurrentTune].Highlight2 = Highlight2;
	
	for (int y = 0; y < filehandler.mod.subtune[CurrentTune].SongLength; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			filehandler.mod.subtune[CurrentTune].Orders[x].resize(orders[x].size());
			filehandler.mod.subtune[CurrentTune].Orders[x][y] = orders[x][y].Index;
		}
	}
	
	//SNES
	filehandler.mod.SelectedRegion = 0;//Stand in for the moment
	filehandler.mod.subtune[CurrentTune].EchoVol = EchoVol;
	filehandler.mod.subtune[CurrentTune].Feedback = Feedback;
	filehandler.mod.subtune[CurrentTune].Delay = Delay;
	for (int x = 0; x < 8; x++)
	{
		filehandler.mod.subtune[CurrentTune].EchoFilter[x] = EchoFilter[x];
	}
}

void Tracker::SaveModuleAs()
{
	ImGuiFileDialog::Instance()->OpenDialog("Save Module", "Choose Path", FILE_EXT, ".");
	if (ImGuiFileDialog::Instance()->Display("Save Module"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			if (filehandler.SaveModule(FileName))
			{
				cout << "File save Successful :D";
				ImGuiFileDialog::Instance()->Close();
				SavingFile = false;
			}
			else
			{
				cout << "File save faled";
				ImGuiFileDialog::Instance()->Close();
				SavingFile = false;
			}
		}
		else
		{
			SavingFile = false;
			ImGuiFileDialog::Instance()->Close();
		}
	}
}

void Tracker::LoadModuleAs()
{
	ImGuiFileDialog::Instance()->OpenDialog("Load Module", "Choose File", FILE_EXT, ".");
	if (ImGuiFileDialog::Instance()->Display("Load Module"))
	{
		// action if OK
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			FileName = ImGuiFileDialog::Instance()->GetFilePathName();
			FilePath = ImGuiFileDialog::Instance()->GetCurrentPath();
			if (filehandler.LoadModule(FileName))
			{
				LoadingFile = false;
				cout << "File load Successful :D";
				ApplyLoad();
				ImGuiFileDialog::Instance()->Close();
			}
			else
			{
				LoadingFile = false;
				cout << "File load faled";
				ImGuiFileDialog::Instance()->Close();
			}
		}
		else
		{
			LoadingFile = false;
			ImGuiFileDialog::Instance()->Close();
		}
	}
}

//Applies the loade file contents to the project
void Tracker::ApplyLoad()
{
	samples.clear();
	inst.clear();
	StoragePatterns.clear();
	memcpy(authbuf, filehandler.mod.subtune[CurrentTune].AuthorName.c_str(), min(filehandler.mod.subtune[CurrentTune].AuthorName.size(), sizeof(authbuf)));
	memcpy(songbuf, filehandler.mod.subtune[CurrentTune].TrackName.c_str(), min(filehandler.mod.subtune[CurrentTune].TrackName.size(), sizeof(songbuf)));
	memcpy(descbuf, filehandler.mod.subtune[CurrentTune].TrackDesc.c_str(), min(filehandler.mod.subtune[CurrentTune].TrackDesc.size(), sizeof(descbuf)));
	MaxTune = filehandler.mod.subtune.size()-1;
	TrackLength = filehandler.mod.subtune[CurrentTune].TrackLength;
	SongLength = filehandler.mod.subtune[CurrentTune].SongLength;
	Speed1 = filehandler.mod.subtune[CurrentTune].Speed1;
	TempoDivider = filehandler.mod.subtune[CurrentTune].TempoDivider;
	Highlight1 = filehandler.mod.subtune[CurrentTune].Highlight1;
	Highlight2 = filehandler.mod.subtune[CurrentTune].Highlight2;

	SetupInstr();
	for (int x = 0; x < filehandler.mod.samples.size(); x++)
	{
		Sample samp = filehandler.mod.samples[x];
		Sample cur;
		cur.SampleIndex = samp.SampleIndex;
		cur.SampleName = samp.SampleName;
		cur.SampleData = samp.SampleData;
		cur.SampleRate = samp.SampleRate;
		cur.Loop = samp.Loop;
		cur.LoopStart = samp.LoopStart;
		cur.LoopEnd = samp.LoopEnd;
		cur.FineTune = samp.FineTune;
		cur.BRRConvert();
		samples.push_back(cur);
		//Memory shit
		SG.Emu_APU.APU_Evaluate_BRR_Loop(&samples[samp.SampleIndex], samples[samp.SampleIndex].LoopEnd);
		SG.Emu_APU.APU_Evaluate_BRR_Loop_Start(&samples[samp.SampleIndex]);
	}
	SG.Emu_APU.APU_Rebuild_Sample_Memory(samples);

	for (int x = 0; x < filehandler.mod.inst.size(); x++)
	{
		Instrument currentinst = filehandler.mod.inst[x];
		Instrument newinst;
		newinst.Index = currentinst.Index;
		newinst.Name = currentinst.Name;
		newinst.SampleIndex = currentinst.SampleIndex;
		newinst.Volume = currentinst.Volume;
		newinst.LPan = currentinst.LPan;
		newinst.RPan = currentinst.RPan;
		newinst.Gain = currentinst.Gain;
		newinst.InvL = currentinst.InvL;
		newinst.InvR = currentinst.InvR;
		newinst.PitchMod = currentinst.PitchMod;
		newinst.Echo = currentinst.Echo;
		newinst.EnvelopeUsed = currentinst.EnvelopeUsed;
		newinst.Noise = currentinst.Noise;
		newinst.Attack = currentinst.Attack;
		newinst.Decay =	currentinst.Decay;
		newinst.Sustain = currentinst.Sustain;
		newinst.Release = currentinst.Release;
		newinst.NoteOff = currentinst.NoteOff;
		newinst.CurrentSample = samples[newinst.SampleIndex];
		inst.push_back(newinst);
	}

	StoragePatterns.clear();
	for (int x = 0; x < filehandler.mod.patterns.size(); x++)
	{
		Patterns pat = Patterns();
		Patterns curpat = filehandler.mod.patterns[x];
		
		pat.Index = curpat.Index;
		for (int y = 0; y < 256; y++)
		{
			pat.SavedRows[y].note = curpat.SavedRows[y].note;
			pat.SavedRows[y].octave = curpat.SavedRows[y].octave;
			pat.SavedRows[y].instrument = curpat.SavedRows[y].instrument;
			pat.SavedRows[y].volume = curpat.SavedRows[y].volume;
			pat.SavedRows[y].effect = curpat.SavedRows[y].effect;
			pat.SavedRows[y].effectvalue = curpat.SavedRows[y].effectvalue;
		}
		StoragePatterns.push_back(pat);
	}

	for (int x = 0; x < 8; x++)
	{
		orders[x].clear();
		orders[x].resize(filehandler.mod.subtune[CurrentTune].Orders[x].size());

	}

	for (int y = 0; y < filehandler.mod.subtune[CurrentTune].SongLength; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			orders[x][y].Index = filehandler.mod.subtune[CurrentTune].Orders[x][y];
			UpdatePatternIndex(x, y);
		}
	}

	SelectedRegion = filehandler.mod.SelectedRegion;
	EchoVol = filehandler.mod.subtune[CurrentTune].EchoVol;
	Delay = filehandler.mod.subtune[CurrentTune].Delay;
	Feedback = filehandler.mod.subtune[CurrentTune].Feedback;
	for (int x = 0; x < 8; x++)
	{
		EchoFilter[x] = filehandler.mod.subtune[CurrentTune].EchoFilter[x];
	}
	SG.Emu_APU.APU_Set_Echo(Delay, EchoFilter, Feedback, EchoVol);
	SG.Emu_APU.APU_Update_Instrument_Memory(StoragePatterns, inst, TrackLength);

	for (int x = 0; x < 8; x++)
	{
		Channels[x].CurrentInstrument = 0;
	}

	UpdateAllPatterns();
}

void Tracker::ApplySubtune()
{	
	memcpy(authbuf, filehandler.mod.subtune[CurrentTune].aubuf, sizeof(authbuf));
	memcpy(songbuf, filehandler.mod.subtune[CurrentTune].trbuf, sizeof(songbuf));
	memcpy(descbuf, filehandler.mod.subtune[CurrentTune].dcbuf, sizeof(descbuf));
	TrackLength = filehandler.mod.subtune[CurrentTune].TrackLength;
	SongLength = filehandler.mod.subtune[CurrentTune].SongLength;
	Speed1 = filehandler.mod.subtune[CurrentTune].Speed1;
	TempoDivider = filehandler.mod.subtune[CurrentTune].TempoDivider;
	Highlight1 = filehandler.mod.subtune[CurrentTune].Highlight1;
	Highlight2 = filehandler.mod.subtune[CurrentTune].Highlight2;
	for (int y = 0; y < SongLength; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			orders[x].resize(filehandler.mod.subtune[CurrentTune].Orders[x].size());
			orders[x][y].Index = filehandler.mod.subtune[CurrentTune].Orders[x][y];
			UpdatePatternIndex(x, y);
		}
	}
	//UpdateModule();
}

static void TextCentered(std::string text) {
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::Text(text.c_str());
}

void Tracker::ErrorWindow()
{
	SetNextWindowPos(GetMainViewport()->GetCenter(), 0, ImVec2(0.5f, 0.5f));
	if(Begin("ERROR", 0, UNIVERSAL_WINDOW_FLAGS)) {
		TextCentered("AN ERROR HAS OCCURED");
		NewLine();
		TextCentered(ErrorMessage);
		NewLine();
		if (Button("Close", ImVec2(64, TextSize * 1.5))) {
			ShowError = false;
		}

	}
	End();
}

void Tracker::DSPDebugWindow()
{
	if(Begin("DSP Debug Window", 0)) 
	{
		Text("DSP regs");
		Text("KON: ");
		SameLine();
		Text(to_string(SG.Emu_APU.APU_Debug_KON_State()).data());
		Text("KOF: ");
		SameLine();
		Text(to_string(SG.Emu_APU.APU_Debug_KOF_State()).data());
		NewLine();
		if (BeginTable("Pitch Table", 2, TABLE_FLAGS)) {
			for (int x = 0; x < 8; x++)
			{
				ImGui::TableNextColumn();
				string channeltext = "Channel ";
				channeltext += to_string(x) + " PIT_L: " + to_string(SG.Emu_APU.APU_Debug_PIT_State(x, 0));
				Text(channeltext.data());
				ImGui::TableNextColumn();
				string channeltext2 = "Channel ";
				channeltext2 += to_string(x) + " PIT_H: " + to_string(SG.Emu_APU.APU_Debug_PIT_State(x, 1));
				Text(channeltext2.data());
				ImGui::TableNextColumn();
				TableNextRow();
			}
			ImGui::EndTable();
		}
		if (BeginTable("Volume Table", 2, TABLE_FLAGS)) {
			for (int x = 0; x < 8; x++)
			{
				ImGui::TableNextColumn();
				string channeltext = "Channel ";
				channeltext += to_string(x) + " VOL_L: " + to_string(SG.Emu_APU.APU_Debug_VOL_State(x, 0));
				Text(channeltext.data());
				ImGui::TableNextColumn();
				string channeltext2 = "Channel ";
				channeltext2 += to_string(x) + " VOL_R: " + to_string(SG.Emu_APU.APU_Debug_VOL_State(x, 1));
				Text(channeltext2.data());
				ImGui::TableNextColumn();
				TableNextRow();
			}
			ImGui::EndTable();
		}
	}
	End();
}

void Tracker::TrackerDebug()
{
	if (Begin("Tracker Debug", 0))
	{
		Checkbox("Compress files", &filehandler.Compress);
	}
	End();
}
