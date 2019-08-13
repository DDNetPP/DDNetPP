#include <engine/shared/config.h>

#include "debug.h"
#include "gamecontext.h"

#include "captcha.h"

CCaptcha::CCaptcha(CGameContext *pGameServer, int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    m_pGameServer = pGameServer;
    m_ClientID = ClientID;
    m_aQuestion[0] = '\0';
    m_aAnswer[0] = '\0';
    m_aBigText[0] = '\0';
    m_Score = 0;
    m_Human = false;
}

bool CCaptcha::CheckGenerate()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    if (m_aQuestion[0])
        return false;

    SendChat("type '/captcha <answer>' to responde.");
    Generate();
    return true;
}

void CCaptcha::GenQuestion(int type)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    int num1 = rand() % 10;
    int num2 = rand() % 10;
    str_format(m_aQuestion, sizeof(m_aQuestion), "what is %d + %d? '/captcha <number>'", num1, num2);
    str_format(m_aAnswer, sizeof(m_aAnswer), "%d", num1 + num2);
}

void CCaptcha::GenBigText(int type)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    int i = 0;
    while(i < 6)
    {
        int c = RandAlphaNum();
        if (type == NUM) { c = RandNum(); }
        if (type == ALPHA) { c = RandAlpha(); }
        m_aAnswer[i] = c;
        i++;
    }
    m_aAnswer[i] = '\0';
    str_copy(m_aBigText, m_aAnswer, sizeof(m_aBigText));
    str_copy(m_aQuestion, "type out the big characters! '/captcha <characters>'", sizeof(m_aQuestion));
}

void CCaptcha::Generate()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    m_aBigText[0] = '\0';

    int r = rand() % 2;
    if (!r)
        GenBigText(NUM);
    else
        GenQuestion(MATH);
}

bool CCaptcha::Score(int value)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    m_Score += value;
    if (m_Score < g_Config.m_SvCaptchaScore)
        return false;

    SendChat("YOU SOLVED THE CAPTCHA! Seems like you are a human.");
    m_Human = true;
    return true;
}

bool CCaptcha::Prompt(const char * pAnswer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    bool correct = false;
    if (m_Score >= g_Config.m_SvCaptchaScore)
        return true;
    if (CheckGenerate())
        goto out;
    if (!pAnswer || !str_comp("", pAnswer))
        goto out;

    if (!str_comp_nocase(pAnswer, m_aAnswer))
    {
        Generate();
        SendChat("CORRECT ANSWER!");
        if (Score(1))
            return true;
        correct = true;
    }
    else
    {
        char aBuf[128];
        str_format(aBuf, sizeof(aBuf), "WRONG ANSWER! (correct: '%s')", m_aAnswer);
        SendChat(aBuf);
        Score(-1);
    }
    Generate();

    out:
    ShowQuestion();
    return correct;;
}

void CCaptcha::ShowQuestion()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    if (m_aBigText[0])
        GameServer()->m_pLetters->SendChat(m_ClientID, m_aBigText);
    SendChat(m_aQuestion);
}

void CCaptcha::SendChat(const char *pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
    char aBuf[128];
    str_format(aBuf, sizeof(aBuf), "[CAPTCHA] %s", pMsg);
    GameServer()->SendChatTarget(m_ClientID, aBuf);
}
