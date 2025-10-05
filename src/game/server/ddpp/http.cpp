#include <engine/shared/config.h>
#include <engine/shared/http.h>
#include <engine/shared/json.h>
#include <engine/shared/jsonwriter.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>

void CGameContext::HttpGetStable(const char *pUrl, const char *pContent)
{
}

void CGameContext::HttpPostStable(const char *pUrl, const char *pContent) const
{
	const int ContentSize = str_length(pContent);
	// TODO: use HttpPostJson()
	std::shared_ptr<CHttpRequest> pDiscord = HttpPost(pUrl, (const unsigned char *)pContent, ContentSize);
	pDiscord->LogProgress(HTTPLOG::FAILURE);
	pDiscord->IpResolve(IPRESOLVE::V4);
	pDiscord->Timeout(CTimeout{4000, 15000, 500, 5});
	pDiscord->HeaderString("Content-Type", "application/json");
	m_pDdppHttp->Run(pDiscord);
}

void CGameContext::SendDiscordWebhook(const char *pWebhookUrl, const char *pContent) const
{
	char aPayload[4048];
	char aContentStr[4000];
	str_format(
		aPayload,
		sizeof(aPayload),
		"{\"allowed_mentions\": {\"parse\": []}, \"content\": \"%s\"}",
		EscapeJson(aContentStr, sizeof(aContentStr), pContent));
	HttpPostStable(g_Config.m_SvChatDiscordWebhook, aPayload);
}
