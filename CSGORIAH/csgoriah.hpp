#pragma once

#include "../Memoriah/Memoriah.hpp"
#include "utility/math/Vector3.hpp"
#include "utility/math/view_matrix_t.hpp"
#include <memory>
#include <thread>

class CSGORIAH
{
public:
	CSGORIAH();
	CSGORIAH(const CSGORIAH&) = delete;
	CSGORIAH(const CSGORIAH&&) = delete;
	CSGORIAH operator=(const CSGORIAH&) = delete;

	void Run();


private:
	static INT SCREEN_WIDTH;
	static INT SCREEN_HEIGHT;
	static INT XHAIRX;
	static INT XHAIRY;

	static constexpr COLORREF ENEMY_COLOR{ 0x00000000FF };
	static constexpr COLORREF TEAMMATE_COLOR{ 0xA400FFFF };


	std::unique_ptr<Memoriah::HProcess> m_process{nullptr};
	bool m_triggerBotState{ false };
	bool m_bunnyHobState{ false };
	view_matrix_t m_vm;
	std::thread m_thWindowResize;
	std::thread m_thRun;
	std::thread m_thEsp;
	std::thread m_thPepe;

	HBRUSH m_enemyBrush;
	HDC m_hdc;

	DWORD GetLocalPlayer();
	DWORD GetPlayer(const int index);
	DWORD GetTeam(DWORD player);
	DWORD GetCrosshairID(DWORD player);
	DWORD GetPlayerHealth(DWORD player);
	Vector3 PlayerLocation(DWORD player);
	bool DormantCheck(DWORD player);
	Vector3 GetHeadBone(DWORD player);
	Vector3 WorldToScreen(const Vector3& pos, const view_matrix_t& matrix);
	int FindClosestEnemy();

	void DrawPepe();

	void DrawFillRect(int x, int y, int w, int h);
	void DrawBorderBox(int x, int y, int w, int h, int thickness);
	void DrawLines(float startX, float startY, float endX, float endY, COLORREF color);

	void DisplayMenu();
 
	void BunnyHop();
	void TriggerBot();
	void Aimbot();
	void Esp();
};