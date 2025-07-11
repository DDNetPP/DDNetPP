#ifndef GAME_CLIENT_COMPONENTS_STATBOARD_H
#define GAME_CLIENT_COMPONENTS_STATBOARD_H

#include <engine/console.h>

#include <game/client/component.h>
#include <string>

class CStatboard : public CComponent
{
private:
	bool m_Active;
	bool m_ScreenshotTaken;
	int64_t m_ScreenshotTime;
	static void ConKeyStats(IConsole::IResult *pResult, void *pUserData);
	void RenderGlobalStats();
	void AutoStatScreenshot();
	void AutoStatCSV();

	std::string ReplaceCommata(char *pStr);
	void FormatStats(char *pDest, size_t DestSize);

public:
	CStatboard();
	int Sizeof() const override { return sizeof(*this); }
	void OnReset() override;
	void OnConsoleInit() override;
	void OnRender() override;
	void OnRelease() override;
	void OnMessage(int MsgType, void *pRawMsg) override;
	bool IsActive() const;
};

#endif // GAME_CLIENT_COMPONENTS_STATBOARD_H
