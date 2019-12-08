#include <stdio.h>
#include "keymng_msg.h"

int myWriteFileReq(unsigned char *buf, int len){
    FILE *fp = NULL;
    fp = fopen("/home/yxy/req.ber", "wb+");
    if (fp == NULL){
        printf("fopen file error!\n");
        return -1;
    }

    fwrite(buf, 1, len, fp);
    fclose(fp);
    return 0;
}

int myWriteFileRes(unsigned char *buf, int len){
    FILE *fp = NULL;
    fp = fopen("/home/yxy/res.ber", "wb+");
    if (fp == NULL){
        printf("fopen file error!\n");
        return -1;
    }

    fwrite(buf, 1, len, fp);
    fclose(fp);
    return 0;
}

int main(){
    int             ret = 0;
    MsgKey_Req      *t1;
    MsgKey_Req      *pT1 = NULL;
    MsgKey_Res      *t2;
    MsgKey_Res      *pT2 = NULL;
    unsigned char   *myOut = NULL;
    int             myOutLen;
    int             *type = NULL;

    t1 = (MsgKey_Req *)malloc(sizeof(MsgKey_Req));
    t1->cmdType = 9527;
    strcpy(t1->clientId, "clientId");
    strcpy(t1->authCode, "authCode");
    strcpy(t1->serverId, "serverId");
    strcpy(t1->r1, "random");



    MsgEncode(t1, ID_MsgKey_Req, &myOut, &myOutLen);

    printf("req encode finished -- myOut = %s, myOutLen = %d\n", myOut, myOutLen);

    myWriteFileReq(myOut, myOutLen);


    MsgDecode(myOut, myOutLen, &pT1, type);


    if (memcmp(pT1->clientId, t1->clientId, strlen(t1->clientId)) == 0
    && memcmp(pT1->serverId, t1->serverId, strlen(t1->serverId)) == 0){
        printf("req编解码成功\n");
    } else {
        printf("req编解码失败\n");
    }
    free(t1);

    t2 = (MsgKey_Res *)malloc(sizeof(MsgKey_Res));
    t2->rv = 404;
    strcpy(t2->clientId, "clientId");
    strcpy(t2->serverId, "serverId");
    strcpy(t2->r2, "random");
    t2->secKeyId = 9527;


    MsgEncode(t2, ID_MsgKey_Res, &myOut, &myOutLen);

    printf("res encode finished -- myOut = %s, myOutLen = %d\n", myOut, myOutLen);

    myWriteFileRes(myOut, myOutLen);


    MsgDecode(myOut, myOutLen, &pT2, type);


    if (memcmp(pT2->clientId, t2->clientId, strlen(t2->clientId)) == 0
    && memcmp(pT2->serverId, t2->serverId, strlen(t2->serverId)) == 0){
        printf("req编解码成功\n");
    } else {
        printf("req编解码失败\n");
    }

    free(t2);
    MsgMemFree(&pT1, ID_MsgKey_Req);
    MsgMemFree(&pT2, ID_MsgKey_Res);

    return 0;
    
}
