#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "keymng_msg.h"
#include "itcast_asn1_der.h"
#include "itcastderlog.h"


typedef struct _Teacher {
	char name[64];
	int age;
	char* p;
	int pLen;
}Teacher;

void TeacherFree(Teacher** pTeacher) {
	if (pTeacher == NULL) {
		return;
	}
	if (pTeacher != NULL) {
		if ((*pTeacher)->p != NULL) {
			free((*pTeacher)->p);
			(*pTeacher)->p = NULL;
		}
		free(*pTeacher);
		*pTeacher = NULL;
	}
}

int TeacherEncode(Teacher* pTeacher, ITCAST_ANYBUF** outData) {
	int             ret = 0;
	ITCAST_ANYBUF* pTmp = NULL;
	ITCAST_ANYBUF* pHeadBuf = NULL;
	ITCAST_ANYBUF* pTmpBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;

	// 把C语言的buf 转化成 ITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pTeacher->name, strlen(pTeacher->name));
	if (ret != 0) {
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	// 编码 name
	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pHeadBuf);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);
	// 两个辅助指针变量 指向 同一个节点
	pTmp = pHeadBuf;

	// 编码 age
	ret = DER_ItAsn1_WriteInteger(pTeacher->age, &(pTmp->next));
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	// 编码 p
	ret = EncodeChar(pTeacher->p, pTeacher->pLen, &pTmp->next);
	if (ret != 0) {
		printf("func EncodeChar() err:%d\n", ret);
		return ret;
	}
	pTmp = pTmp->next;

	// 编码 pLen
	ret = DER_ItAsn1_WriteInteger(pTeacher->pLen, &(pTmp->next));
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d\n", ret);
		return ret;
	}

	DER_ITCAST_FreeQueue(pHeadBuf);

	*outData = pOutData;

	return ret;
}


int TeacherDecode(unsigned char* inData, int inLen, Teacher** pStruct) {

	int             ret = 0;
	ITCAST_ANYBUF* pTmp = NULL;
	ITCAST_ANYBUF* pHeadBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;
	ITCAST_ANYBUF* pTmpAnyBuf = NULL;

	Teacher* pStructTeacher = NULL;
	// 转码 unsigned char * 为 ITCAST_ANYBUF
	ret = DER_ITCAST_String_To_AnyBuf(&pTmpAnyBuf, inData, inLen);
	if (ret != 0) {
		if (pTmpAnyBuf != NULL) {
			DER_ITCAST_FreeQueue(pTmpAnyBuf);
		}
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}
	// 解码 Teacher 结构体
	ret = DER_ItAsn1_ReadSequence(pTmpAnyBuf, &pHeadBuf);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pTmpAnyBuf);
		printf("func DER_ItAsn1_ReadSequence() err:%d\n", ret);
		return ret;
	}

	// 给Teacher开辟内存空间
	if (pStructTeacher == NULL) {
		pStructTeacher = (Teacher*)malloc(sizeof(Teacher));
		if (pStructTeacher == NULL) {
			ret = -1;
			printf("Teacher malloc err:%d\n", ret);
			return ret;
		}
		memset(pStructTeacher, 0, sizeof(Teacher));
	}

	pTmp = pHeadBuf;

	// 解码 name
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		TeacherFree(&pStructTeacher);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	// ppPrintableString->pData  --> name
	memcpy(pStructTeacher->name, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	// 解码 age
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pStructTeacher->age));
	if (ret != 0) {
		TeacherFree(&pStructTeacher);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadInteger() err:%d\n", ret);
		return ret;
	}

	pTmp = pTmp->next;

	// 解码 p
	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		TeacherFree(&pStructTeacher);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	// ppPrintableString->pData  --> p
	pStructTeacher->p = malloc(pOutData->dataLen + 1);
	if (pStructTeacher->p == NULL) {
		TeacherFree(&pStructTeacher);
		DER_ITCAST_FreeQueue(pHeadBuf);
		ret = -1;
		printf("Teacher->p malloc err:%d\n", ret);
		return ret;
	}
	memcpy(pStructTeacher->p, pOutData->pData, pOutData->dataLen);
	pStructTeacher->p[pOutData->dataLen] = '\0';

	pTmp = pTmp->next;
	DER_ITCAST_FreeQueue(pOutData);

	// 解码 plen
	ret = DER_ItAsn1_ReadInteger(pTmp, &(pStructTeacher->pLen));
	if (ret != 0) {
		TeacherFree(&pStructTeacher);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadInteger() err:%d\n", ret);
		return ret;
	}

	*pStruct = pStructTeacher;



	return ret;
}

void ReqFree(MsgKey_Req** pReq) {
	if (pReq == NULL) {
		return;
	}
	if (pReq != NULL) {
		free(*pReq);
		*pReq = NULL;
	}
}


int ReqEncode(MsgKey_Req* pMsgKeyReq, ITCAST_ANYBUF** outData) {
	int                 ret = 0;
	ITCAST_ANYBUF* pHeadBuf = NULL, * pTmp = NULL;
	ITCAST_ANYBUF* pTmpBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;

	ret = DER_ItAsn1_WriteInteger(pMsgKeyReq->cmdType, &pHeadBuf);
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}

	pTmp = pHeadBuf;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyReq->clientId, strlen(pMsgKeyReq->clientId));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyReq->authCode, strlen(pMsgKeyReq->authCode));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyReq->serverId, strlen(pMsgKeyReq->serverId));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyReq->r1, strlen(pMsgKeyReq->r1));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadBuf);

	*outData = pOutData;

	return ret;

}

int ReqDecode(unsigned char* inData, int inLen, MsgKey_Req** pStruct) {

	int                 ret = 0;
	ITCAST_ANYBUF* pTmp = NULL;
	ITCAST_ANYBUF* pHeadBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;
	ITCAST_ANYBUF* pTmpAnyBuf = NULL;

	MsgKey_Req* pStructReq = NULL;


	ret = DER_ITCAST_String_To_AnyBuf(&pTmpAnyBuf, inData, inLen);
	if (ret != 0) {
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_ReadSequence(pTmpAnyBuf, &pHeadBuf);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pTmpAnyBuf);
		printf("func DER_ItAsn1_ReadSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpAnyBuf);

	pStructReq = (MsgKey_Req*)malloc(sizeof(MsgKey_Req));
	if (pStructReq == NULL) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("malloc() err:%d\n", ret);
		return ret;
	}

	pTmp = pHeadBuf;

	ret = DER_ItAsn1_ReadInteger(pTmp, &pStructReq->cmdType);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("malloc() err:%d\n", ret);
		return ret;
	}

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ReqFree(&pStructReq);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructReq->clientId, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ReqFree(&pStructReq);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructReq->authCode, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ReqFree(&pStructReq);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructReq->serverId, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ReqFree(&pStructReq);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructReq->r1, pOutData->pData, pOutData->dataLen);

	DER_ITCAST_FreeQueue(pHeadBuf);
	DER_ITCAST_FreeQueue(pOutData);

	*pStruct = pStructReq;

	return ret;
}


void ResFree(MsgKey_Res** pRes) {
	if (pRes == NULL) {
		return;
	}
	if (pRes != NULL) {
		free(*pRes);
		*pRes = NULL;
	}
}


int ResEncode(MsgKey_Res* pMsgKeyRes, ITCAST_ANYBUF** outData) {
	int                 ret = 0;
	ITCAST_ANYBUF* pHeadBuf = NULL, * pTmp = NULL;
	ITCAST_ANYBUF* pTmpBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;

	ret = DER_ItAsn1_WriteInteger(pMsgKeyRes->rv, &pHeadBuf);
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}

	pTmp = pHeadBuf;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyRes->clientId, strlen(pMsgKeyRes->clientId));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyRes->serverId, strlen(pMsgKeyRes->serverId));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpBuf, pMsgKeyRes->r2, strlen(pMsgKeyRes->r2));
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WritePrintableString(pTmpBuf, &pTmp->next);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		DER_ITCAST_FreeQueue(pTmpBuf);
		printf("func DER_ItAsn1_WritePrintableString() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpBuf);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_WriteInteger(pMsgKeyRes->secKeyId, &pTmp->next);
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadBuf);

	*outData = pOutData;

	return ret;

}

int ResDecode(unsigned char* inData, int inLen, MsgKey_Res** pStruct) {

	int                 ret = 0;
	ITCAST_ANYBUF* pTmp = NULL;
	ITCAST_ANYBUF* pHeadBuf = NULL;
	ITCAST_ANYBUF* pOutData = NULL;
	ITCAST_ANYBUF* pTmpAnyBuf = NULL;

	MsgKey_Res* pStructRes = NULL;

	ret = DER_ITCAST_String_To_AnyBuf(&pTmpAnyBuf, inData, inLen);
	if (ret != 0) {
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_ReadSequence(pTmpAnyBuf, &pHeadBuf);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pTmpAnyBuf);
		printf("func DER_ItAsn1_ReadSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pTmpAnyBuf);

	pStructRes = (MsgKey_Res*)malloc(sizeof(MsgKey_Res));
	if (pStructRes == NULL) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("malloc() err:%d\n", ret);
		return ret;
	}

	pTmp = pHeadBuf;

	ret = DER_ItAsn1_ReadInteger(pTmp, &pStructRes->rv);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("malloc() err:%d\n", ret);
		return ret;
	}

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ResFree(&pStructRes);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructRes->clientId, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ResFree(&pStructRes);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructRes->serverId, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadPrintableString(pTmp, &pOutData);
	if (ret != 0) {
		ResFree(&pStructRes);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadPrintableString() err:%d\n", ret);
		return ret;
	}

	memcpy(pStructRes->r2, pOutData->pData, pOutData->dataLen);

	pTmp = pTmp->next;

	ret = DER_ItAsn1_ReadInteger(pTmp, &pStructRes->secKeyId);
	if (ret != 0) {
		ResFree(&pStructRes);
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("malloc() err:%d\n", ret);
		return ret;
	}

	DER_ITCAST_FreeQueue(pHeadBuf);
	DER_ITCAST_FreeQueue(pOutData);

	*pStruct = pStructRes;

	return ret;
}


int MsgEncode(
	void* pStruct,   /*in*/
	int                 type,
	unsigned char** outData, /*out*/
	int* outLen
) {
	ITCAST_ANYBUF* pHeadBuf = NULL, * pTemp = NULL;
	ITCAST_ANYBUF* pOutData = NULL;
	int             ret = 0;

	// if (pStruct == NULL && type < 0 || pOutData == NULL || outLen == NULL){
	//     ret = KeyMng_ParamErr;
	//     printf("func MsgEncoode() err:%d\n", ret);
	//     return ret;
	// }
	// 编码 type
	ret = DER_ItAsn1_WriteInteger(type, &pHeadBuf);
	if (ret != 0) {
		printf("func DER_ItAsn1_WriteInteger() err:%d\n", ret);
		return ret;
	}

	switch (type) {
	case ID_MsgKey_Teacher:
		ret = TeacherEncode((Teacher*)pStruct, &pTemp);
		break;
	case ID_MsgKey_Req:
		ret = ReqEncode((MsgKey_Req*)pStruct, &pTemp);
		break;
	case ID_MsgKey_Res:
		ret = ResEncode((MsgKey_Res*)pStruct, &pTemp);
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("type err:%d\n", ret);
		break;

	}


	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("encode err:%d\n", ret);
		return ret;
	}

	pHeadBuf->next = pTemp;

	// 对type和结构体再做一次TLV封装
	ret = DER_ItAsn1_WriteSequence(pHeadBuf, &pOutData);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_WriteSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(pHeadBuf);

	*outData = (unsigned char*)malloc(pOutData->dataLen);
	if (*outData == NULL) {
		DER_ITCAST_FreeQueue(pOutData);
		ret = KeyMng_MallocErr;
		printf("malloc() err:%d\n", ret);
		return ret;
	}
	memcpy(*outData, pOutData->pData, pOutData->dataLen);
	*outLen = pOutData->dataLen;

	DER_ITCAST_FreeQueue(pOutData);
}

int MsgDecode(
	unsigned char* inData,   /*in*/
	int                 inLen,
	void** pStruct, /*out*/
	int* type       /*out*/
) {
	ITCAST_ANYBUF* pHeadBuf = NULL, * inAnyBuf = NULL;
	int                 ret = 0;
	unsigned long       iType = 0;

	// 将 ber 格式的字节流， 转换成AnyBuf
	ret = DER_ITCAST_String_To_AnyBuf(&inAnyBuf, inData, inLen);
	if (ret != 0) {
		printf("func DER_ITCAST_String_To_AnyBuf() err:%d\n", ret);
		return ret;
	}

	ret = DER_ItAsn1_ReadSequence(inAnyBuf, &pHeadBuf);
	if (ret != 0) {
		printf("func DER_ItAsn1_ReadSequence() err:%d\n", ret);
		return ret;
	}
	DER_ITCAST_FreeQueue(inAnyBuf);

	// 解析type
	ret = DER_ItAsn1_ReadInteger(pHeadBuf, &iType);
	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("func DER_ItAsn1_ReadInteger() err:%d\n", ret);
		return ret;
	}

	switch (iType) {
	case ID_MsgKey_Teacher:
		ret = TeacherDecode(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (Teacher**)pStruct);
		break;
	case ID_MsgKey_Req:
		ret = ReqDecode(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (MsgKey_Req**)pStruct);
		break;
	case ID_MsgKey_Res:
		ret = ResDecode(pHeadBuf->next->pData, pHeadBuf->next->dataLen, (MsgKey_Res**)pStruct);
		break;
	default:
		ret = KeyMng_TypeErr;
		printf("type err:%d\n", ret);
		break;
	}

	if (ret != 0) {
		DER_ITCAST_FreeQueue(pHeadBuf);
		printf("switch decode err:%d\n", ret);
		return ret;
	}

	type = (int*)malloc(sizeof(int));
	*type = iType;
	DER_ITCAST_FreeQueue(pHeadBuf);

	return ret;

}

int MsgMemFree(
	void** point,
	int                 type
) {
	if (point == NULL) {
		return 0;
	}

	if (type == 0) {
		if (*point) {
			free(*point);
		}
		*point = NULL;
		return 0;
	}

	switch (type) {
	case ID_MsgKey_Teacher:
		TeacherFree((Teacher**)point);
		break;
	case ID_MsgKey_Req:
		ReqFree((MsgKey_Req**)point);
		break;
	case ID_MsgKey_Res:
		ResFree((MsgKey_Res**)point);
		break;
	default:
		break;
	}

	return 0;
}
