/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_INPUT_H
#define ENGINE_INPUT_H

#include "kernel.h"

const int g_MaxKeys = 512;
extern const char g_aaKeyStrings[g_MaxKeys][20];

class IInput : public IInterface
{
	MACRO_INTERFACE("input", 0)
public:
	enum
	{
		INPUT_TEXT_SIZE = 128
	};

	class CEvent
	{
	public:
		int m_Flags;
		int m_Key;
		char m_aText[INPUT_TEXT_SIZE];
		int m_InputCount;
	};

protected:
	enum
	{
		INPUT_BUFFER_SIZE = 32
	};

	// quick access to events
	int m_NumEvents;
	IInput::CEvent m_aInputEvents[INPUT_BUFFER_SIZE];

public:
	enum
	{
		FLAG_PRESS = 1,
		FLAG_RELEASE = 2,
		FLAG_REPEAT = 4,
		FLAG_TEXT = 8,
	};
	enum ECursorType
	{
		CURSOR_NONE,
		CURSOR_MOUSE,
		CURSOR_JOYSTICK,
	};

	// events
	int NumEvents() const { return m_NumEvents; }
	virtual bool IsEventValid(CEvent *pEvent) const = 0;
	CEvent GetEvent(int Index) const
	{
		if(Index < 0 || Index >= m_NumEvents)
		{
			IInput::CEvent e = {0, 0};
			return e;
		}
		return m_aInputEvents[Index];
	}
	CEvent *GetEventsRaw() { return m_aInputEvents; }
	int *GetEventCountRaw() { return &m_NumEvents; }

	// keys
	virtual bool ModifierIsPressed() const = 0;
	virtual bool ShiftIsPressed() const = 0;
	virtual bool AltIsPressed() const = 0;
	virtual bool KeyIsPressed(int Key) const = 0;
	virtual bool KeyPress(int Key, bool CheckCounter = false) const = 0;
	const char *KeyName(int Key) const { return (Key >= 0 && Key < g_MaxKeys) ? g_aaKeyStrings[Key] : g_aaKeyStrings[0]; }
	virtual void Clear() = 0;

	// joystick
	class IJoystick
	{
	public:
		virtual int GetIndex() const = 0;
		virtual const char *GetName() const = 0;
		virtual int GetNumAxes() const = 0;
		virtual int GetNumButtons() const = 0;
		virtual int GetNumBalls() const = 0;
		virtual int GetNumHats() const = 0;
		virtual float GetAxisValue(int Axis) = 0;
		virtual void GetHatValue(int Hat, int (&HatKeys)[2]) = 0;
		virtual bool Relative(float *pX, float *pY) = 0;
		virtual bool Absolute(float *pX, float *pY) = 0;
	};
	virtual size_t NumJoysticks() const = 0;
	virtual IJoystick *GetActiveJoystick() = 0;
	virtual void SelectNextJoystick() = 0;

	// mouse
	virtual void NativeMousePos(int *pX, int *pY) const = 0;
	virtual bool NativeMousePressed(int Index) = 0;
	virtual void MouseModeRelative() = 0;
	virtual void MouseModeAbsolute() = 0;
	virtual bool MouseDoubleClick() = 0;
	virtual bool MouseRelative(float *pX, float *pY) = 0;

	// clipboard
	virtual const char *GetClipboardText() = 0;
	virtual void SetClipboardText(const char *pText) = 0;

	// text editing
	virtual bool GetIMEState() = 0;
	virtual void SetIMEState(bool Activate) = 0;
	virtual int GetIMEEditingTextLength() const = 0;
	virtual const char *GetIMEEditingText() = 0;
	virtual int GetEditingCursor() = 0;
	virtual void SetEditingPosition(float X, float Y) = 0;

	virtual bool GetDropFile(char *aBuf, int Len) = 0;

	ECursorType CursorRelative(float *pX, float *pY)
	{
		if(MouseRelative(pX, pY))
			return CURSOR_MOUSE;
		IJoystick *pJoystick = GetActiveJoystick();
		if(pJoystick && pJoystick->Relative(pX, pY))
			return CURSOR_JOYSTICK;
		return CURSOR_NONE;
	}
};

class IEngineInput : public IInput
{
	MACRO_INTERFACE("engineinput", 0)
public:
	virtual void Init() = 0;
	virtual void Shutdown() override = 0;
	virtual int Update() = 0;
	virtual int VideoRestartNeeded() = 0;
};

extern IEngineInput *CreateEngineInput();

#endif
