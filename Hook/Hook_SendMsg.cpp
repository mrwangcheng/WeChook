/*
Hook of SendMessage
input cmd: 
	SendMessage
input args:
	string: wxid
	string: message content

output cmd:
	SendMessage_R
output args:
	constant string: "Success"
*/

#include "Hook_SendMsg.h"

Hook_SendMsg::Hook_SendMsg(Socket* s)
{
	sock = s;
}


Hook_SendMsg::~Hook_SendMsg()
{
}

void Hook_SendMsg::injectHook() {

}

void Hook_SendMsg::ejectHook() {

}

VOID Hook_SendMsg::SendTextMessage(wchar_t * wxid, wchar_t * message) {
	DWORD sendCall = getWeChatWinAddr() + F_SENDMSG_ADDR_OFFSET;

	// assemble the msg structure
	wxStr pWxid = { 0 };
	pWxid.pStr = wxid;
	pWxid.strLen = wcslen(wxid);	// get the length of unicode
	pWxid.strLen2 = wcslen(wxid) * 2;

	wxStr pMessage = { 0 };
	pMessage.pStr = message;
	pMessage.strLen = wcslen(message);
	pMessage.strLen2 = wcslen(message) * 2;

	/*
	// copied from ollydbg

		MOV EDX,DWORD PTR SS:[EBP-34]            ; wxid
		LEA EAX,DWORD PTR DS:[EBX+14]
		PUSH 1                                   ; 0x1
		PUSH EAX                                 ; can be 0x0. If you are "at" someone, here will be a pointer to the weid
		PUSH EBX                                 ; message
		LEA ECX,DWORD PTR SS:[EBP-81C]           ; buffer [0x81c]
		CALL WeChatWi.626DB4E0                   ; The message sending call
		ADD ESP,0C                               ; outer stack equalizor
	*/
	char * asmWxid = (char *)&pWxid.pStr;
	char * asmMessage = (char *)&pMessage.pStr;
	char buff[0x81c] = { 0 };
	__asm {
		mov edx, asmWxid
		mov eax, 0x0
		push 0x1
		push eax
		mov ebx, asmMessage
		push ebx
		lea ecx, buff
		call sendCall
		add esp, 0xC
	}
	char sendBuffer[0x3000];
	sprintf_s(sendBuffer,
		"{ \"cmd\": \"SendMessage_R\", \"args\": [\"Success\"] }\0"
	);
	sock->push(sendBuffer, strlen(sendBuffer));
}

VOID Hook_SendMsg::SendStringMessage(std::string wxid_str, std::string msgContent_str) {
	char wxid_ch[0x50];
	strcpy_s(wxid_ch, wxid_str.c_str());
	char msgContent_ch[0x1000];
	strcpy_s(msgContent_ch, msgContent_str.c_str());
	wchar_t* wxid_wc = UTF8ToUnicode(wxid_ch);
	wchar_t* msgContent_wc = UTF8ToUnicode(msgContent_ch);
	SendTextMessage(wxid_wc, msgContent_wc);
	delete wxid_wc, msgContent_wc;
}
