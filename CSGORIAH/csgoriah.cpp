#include "csgoriah.hpp"
#include "utility/netvars.hpp"
#include "utility/os/os.hpp"
#include <sstream>

 
 
INT CSGORIAH::SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN);
INT CSGORIAH::SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN);
INT CSGORIAH::XHAIRX = SCREEN_WIDTH / 2;
INT CSGORIAH::XHAIRY = SCREEN_HEIGHT / 2;

CSGORIAH::CSGORIAH()
{
	m_process = std::make_unique<Memoriah::HProcess>(TEXT("Counter-Strike: Global Offensive"), Memoriah::ProcessAccess::ALL_ACCESS);

	Memoriah::Manager::DebugConsoleLog("Waiting CSGO launch...", Memoriah::ConsoleTextColor::RED);
	m_process->Initialize(true);
	Memoriah::Manager::DebugConsoleLog("CSGO launch...", Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::GREEN, " ");
	Memoriah::Manager::DebugConsoleLog("[ OK ]", Memoriah::ConsoleTextColor::GREEN);

	m_enemyBrush = CreateSolidBrush(ENEMY_COLOR);

	if (m_enemyBrush == nullptr)
		std::cerr << "err" << std::endl;

	m_hdc = GetDC(FindWindow(nullptr, TEXT("Counter-Strike: Global Offensive")));


	m_thWindowResize = std::thread([this]()
		{
			HWND hWnd = FindWindow(nullptr, TEXT("Counter-Strike: Global Offensive"));
			RECT rect;
			GetWindowRect(hWnd, &rect);


			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;
			while (true)
			{
 				if (hWnd != nullptr)
				{
					GetWindowRect(hWnd, &rect);

					if (rect.right - rect.left != w || rect.bottom - rect.top != h)
					{
						std::ostringstream ss;
						ss << "CSGO Window is resize from " << w - 6 << " x " << h - 29 << " to " << rect.right - rect.left - 6 << " x " << rect.bottom - rect.top -29 << ".";
						Memoriah::Manager::DebugConsoleLog(ss.str(), Memoriah::ConsoleBackgroundColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE | Memoriah::ConsoleTextColor::GREEN);
						w = rect.right - rect.left;
						h = rect.bottom - rect.top;

						SCREEN_WIDTH = w;
						SCREEN_HEIGHT = h;

						XHAIRX = SCREEN_WIDTH / 2;
						XHAIRY = SCREEN_HEIGHT / 2;
					}
					
 
				}
			}
			 
		});

	m_thRun = std::thread([this]()
		{
			this->Run();
		});

	m_thEsp = std::thread([this]()
		{
			while (true)
				Esp();
		});

	m_thPepe = std::thread([this]() {
		while (true)
			DrawPepe();
		});

	m_thPepe.join();

  
	m_thRun.join();
	m_thEsp.join();
	m_thWindowResize.join();
}

void CSGORIAH::Run()
{
	DisplayMenu();



	while (!GetAsyncKeyState(VK_END))  
	{
		if (GetAsyncKeyState(VK_F1))
		{
			m_bunnyHobState = !m_bunnyHobState;
			Sleep(1000);
			cls(GetStdHandle(STD_OUTPUT_HANDLE));
			DisplayMenu();
		}

		if (GetAsyncKeyState(VK_F2))
		{
			m_triggerBotState = !m_triggerBotState;
			Sleep(1000);
			cls(GetStdHandle(STD_OUTPUT_HANDLE));
			DisplayMenu();

		}
		 
 		if(m_bunnyHobState)
			BunnyHop();
		if(m_triggerBotState)
			TriggerBot();
	}

	Memoriah::Manager::ExitMemoriahApp(Memoriah::ExitCode::SUCCESS);
}


DWORD CSGORIAH::GetLocalPlayer()
{
	DWORD moduleBase = m_process->GetModuleAddress(TEXT("client_panorama.dll"));
	return m_process->RPM<DWORD>(moduleBase + csgoriah::signatures::dwLocalPlayer);
}

DWORD CSGORIAH::GetPlayer(const int index)
{
	DWORD moduleBase = m_process->GetModuleAddress(TEXT("client_panorama.dll"));
	return m_process->RPM<DWORD>(moduleBase + csgoriah::signatures::dwEntityList + index * 10);
}

DWORD CSGORIAH::GetTeam(DWORD player)
{
	return m_process->RPM<DWORD>(player + csgoriah::netvars::m_iTeamNum);
}

DWORD CSGORIAH::GetCrosshairID(DWORD player)
{
	return m_process->RPM<DWORD>(player + csgoriah::netvars::m_iCrosshairId);
}

DWORD CSGORIAH::GetPlayerHealth(DWORD player)
{
	return m_process->RPM<DWORD>(player + csgoriah::netvars::m_iHealth);
}

Vector3 CSGORIAH::PlayerLocation(DWORD player)
{
	return m_process->RPM<Vector3>(player + csgoriah::netvars::m_vecOrigin);
}

bool CSGORIAH::DormantCheck(DWORD player)
{
	return m_process->RPM<int>(player + csgoriah::signatures::m_bDormant);
}

Vector3 CSGORIAH::GetHeadBone(DWORD player)
{
	struct boneMatrix_t
	{
		byte pad3[12];
		float x;
		byte pad1[12];
		float y;
		byte pad2[12];
		float z;
	};

	DWORD boneBase = m_process->RPM<DWORD>(player + csgoriah::netvars::m_dwBoneMatrix);
	boneMatrix_t boneMatrix = m_process->RPM<boneMatrix_t>(boneBase + (sizeof(boneMatrix) * 8));
	return Vector3(boneMatrix.x, boneMatrix.y, boneMatrix.z);
}

Vector3 CSGORIAH::WorldToScreen(const Vector3& pos, const view_matrix_t& matrix)
{
	float _x = matrix.matrix4x4[0][0] * pos.x + matrix.matrix4x4[0][1] * pos.y + matrix.matrix4x4[0][2] * pos.z + matrix.matrix4x4[0][3];
	float _y = matrix.matrix4x4[1][0] * pos.x + matrix.matrix4x4[1][1] * pos.y + matrix.matrix4x4[1][2] * pos.z + matrix.matrix4x4[1][3];
	float w = matrix.matrix4x4[3][0] * pos.x + matrix.matrix4x4[3][1] * pos.y + matrix.matrix4x4[3][2] * pos.z + matrix.matrix4x4[3][3];

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = SCREEN_WIDTH * 0.5f;
	float y = SCREEN_HEIGHT * 0.5f;

	x += 0.5f * _x * SCREEN_WIDTH + 0.5f;
	y -= 0.5f * _y * SCREEN_HEIGHT + 0.5f;
 

	return {x, y ,w};
}

int CSGORIAH::FindClosestEnemy()
{
	float finish;
	int closestEntity = 1;
	float closest = FLT_MAX;
	int localTeam = GetTeam(GetLocalPlayer());
 
	for (int i = 1; i < 32; i++)
	{

		DWORD entity = GetPlayer(i);
		int team = GetTeam(entity);

		if (team == localTeam)
			continue;

		int enemyHealth = GetPlayerHealth(entity);

		if (enemyHealth < 1 || enemyHealth > 100)
			continue;

		int dormant = DormantCheck(entity);

		if (dormant)
			continue;
 
		Vector3 headBone = WorldToScreen(GetHeadBone(entity), m_vm);
		finish = Vector3::magnitudeBeetween2Point2D(headBone.x, headBone.y, XHAIRX, XHAIRY);

 
		if (finish < closest)
		{
			closest = finish;
			closestEntity = i;
		}

		return closestEntity;
	}
}

void CSGORIAH::DrawFillRect(int x, int y, int w, int h)
{
	RECT rect{ w, y, w, h };
	FillRect(m_hdc, &rect, m_enemyBrush);
}

void CSGORIAH::DrawBorderBox(int x, int y, int w, int h, int thickness)
{
	DrawFillRect(x, y, w, thickness);
	DrawFillRect(x, y, thickness, h);
	DrawFillRect(x + w, y, thickness, h);
	DrawFillRect(x, y + h, w + thickness, thickness); 
}

void CSGORIAH::DrawLines(float startX, float startY, float endX, float endY, COLORREF color)
{
	int a = 0, b = 0;
	HPEN hOPen;
	HPEN hNPen = CreatePen(PS_SOLID, 2, color);

	hOPen = reinterpret_cast<HPEN>(SelectObject(m_hdc, hNPen));
	MoveToEx(m_hdc, startX, startY, nullptr);
	a = LineTo(m_hdc, endX, endY);
	DeleteObject(SelectObject(m_hdc, hOPen));
}

void CSGORIAH::BunnyHop()
{
	DWORD forceJumpValue = 0;
	DWORD moduleBase = m_process->GetModuleAddress(TEXT("client_panorama.dll"));
	auto localPlayer = GetLocalPlayer();
	auto flags = m_process->RPM<DWORD>(localPlayer + csgoriah::netvars::m_fFlags);

	if (flags & 1)
		forceJumpValue = 5;
	else
		forceJumpValue = 4;

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		m_process->WPM(moduleBase + csgoriah::signatures::dwForceJump, forceJumpValue);
}

void CSGORIAH::TriggerBot()
{
	int crosshairID = GetCrosshairID(GetLocalPlayer());
	int crosshairTeam = GetTeam(GetPlayer(crosshairID - 1));
	int localTeam = GetTeam(GetLocalPlayer());

	if (crosshairID > 0 && crosshairID < 32 && localTeam != crosshairTeam)
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}
}

void CSGORIAH::Aimbot()
{
	DWORD moduleBase = m_process->GetModuleAddress(TEXT("client_panorama.dll"));

	m_vm = m_process->RPM<view_matrix_t>(moduleBase + csgoriah::signatures::dwViewMatrix);
	auto closest = FindClosestEnemy();
 	Vector3 closestw2shead = WorldToScreen(GetHeadBone(GetPlayer(closest)), m_vm);

	if (GetAsyncKeyState(VK_RIGHT))
		SetCursorPos(closestw2shead.x, closestw2shead.y);
}

void CSGORIAH::Esp()
{
	DWORD moduleBase = m_process->GetModuleAddress(TEXT("client_panorama.dll"));

	view_matrix_t vm = m_process->RPM<view_matrix_t>(moduleBase+csgoriah::signatures::dwViewMatrix);
	DWORD localTeam = m_process->RPM<DWORD>(m_process->RPM<DWORD>(moduleBase + csgoriah::signatures::dwEntityList) + csgoriah::netvars::m_iTeamNum);

	for (auto i = 1; i < 64; i++)
	{
		DWORD entity = m_process->RPM<DWORD>(moduleBase + csgoriah::signatures::dwEntityList + (i * 0x10));
		DWORD health = m_process->RPM<DWORD>(entity + csgoriah::netvars::m_iHealth);
		DWORD team = m_process->RPM<DWORD>(entity + csgoriah::netvars::m_iTeamNum);
		Vector3 pos = m_process->RPM<Vector3>(entity + csgoriah::netvars::m_vecOrigin);
		Vector3 head;
		head.x = pos.x;
		head.y = pos.y;
		head.z = pos.z + 75.f;

		Vector3 screenpos = WorldToScreen(pos, vm);
		Vector3 screenhead = WorldToScreen(head, vm);

		float height = screenhead.y - screenpos.y;
		float width = height / 2.4f;

		if (screenpos.z >= 0.01f && health > 0 && health < 101)
		{
			if(team != localTeam)
				DrawLines(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, screenpos.x, screenpos.y, ENEMY_COLOR);
			else 
				DrawLines(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, screenpos.x, screenpos.y, TEAMMATE_COLOR);

			DrawBorderBox(screenpos.x - (width / 2), screenpos.y, width, height, 1);
 		}
	}
}


void CSGORIAH::DisplayMenu()
{
	Memoriah::Manager::DebugConsoleLog("===========================", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE | Memoriah::ConsoleTextColor::GREEN);
	Memoriah::Manager::DebugConsoleLog("=         CSGORIAH        =", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE | Memoriah::ConsoleTextColor::GREEN);
	Memoriah::Manager::DebugConsoleLog("===========================", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE | Memoriah::ConsoleTextColor::GREEN);

	if (m_bunnyHobState)
	{
		Memoriah::Manager::DebugConsoleLog("BunnyHop [ ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE, "");
		Memoriah::Manager::DebugConsoleLog("ON ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::GREEN, "");
		Memoriah::Manager::DebugConsoleLog("]", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE);
	}
	else
	{
		Memoriah::Manager::DebugConsoleLog("BunnyHop [ ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE, "");
		Memoriah::Manager::DebugConsoleLog("OFF ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED, "");
		Memoriah::Manager::DebugConsoleLog("]", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE);
	}

	if (m_triggerBotState)
	{
		Memoriah::Manager::DebugConsoleLog("TriggerBot [ ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE, "");
		Memoriah::Manager::DebugConsoleLog("ON ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::GREEN, "");
		Memoriah::Manager::DebugConsoleLog("]", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE);
	}
	else
	{
		Memoriah::Manager::DebugConsoleLog("TriggerBot [ ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE, "");
		Memoriah::Manager::DebugConsoleLog("OFF ", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED, "");
		Memoriah::Manager::DebugConsoleLog("]", Memoriah::ConsoleTextColor::INTENSITY | Memoriah::ConsoleTextColor::RED | Memoriah::ConsoleTextColor::BLUE);
	}
 
}
 
void CSGORIAH::DrawPepe()
{
	auto x = 400; // +400
	auto y = 0; // + 400

	HDC hdc = CreateCompatibleDC(nullptr);
	HBITMAP image = (HBITMAP)LoadImageA(
		NULL,                           // not loading from a module, so this is NULL
		"ressources/pepe.bmp",                       // the path we're loading from
		IMAGE_BITMAP,                   // we are loading a bitmap
		0, 0,                            // don't need to specify width/height
		LR_DEFAULTSIZE | LR_LOADFROMFILE// use the default bitmap size (whatever the file is), and load it from a file
	);

	auto old = SelectObject(hdc, image);

	BitBlt(m_hdc, x, y, 400, 400, hdc, 0, 0, SRCCOPY);

	SelectObject(hdc, old);      // put the old bmp back in our DC
	DeleteObject(image);                 // delete the bmp we loaded
	DeleteDC(hdc);
}