#include "../jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
          strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}


static int GetJsonElement(char *a_pucJsonData, int a_udJDataLen, char *a_pucKeyWord, char *a_pucElemData)
{
    int i;
    int r;
    jsmn_parser p;
    jsmntok_t t[300]; /* We expect no more than 128 tokens */
    char *Json_Data = a_pucJsonData;

    jsmn_init(&p);
    r = jsmn_parse(&p, Json_Data, a_udJDataLen, t,
                   sizeof(t) / sizeof(t[0]));
    if (r < 0) {
        printf("Failed to parse JSON: %d\n", r);
        return -1;
    }

    /* Assume the top-level element is an object */
    if (r < 1 || t[0].type != JSMN_OBJECT) {
        printf("Object expected\n");
        return -1;
    }

    // printf(">>>>  r = %d \n", r);

    for (i = 1; i < r; i++) {
        if (jsoneq(Json_Data, &t[i], a_pucKeyWord) == 0)
        {
            strncpy(a_pucElemData, (Json_Data + t[i + 1].start), (t[i + 1].end - t[i + 1].start));
            break;
        }
    }

    return EXIT_SUCCESS;
}


int main() {
    FILE *pFile;
    char Json_Data[4096];
    int sz;
    char ElemData[1024]={0};
    char UserData_Tmp[1024]={0};
    char UserHeader[1024]={0};
    char BinFifo_Tmp[1024]={0};
    char str[256]={0};
    int i, udBinCnt;


    pFile = fopen( "Spinor_PTM_Info.json","r" );

    if( NULL == pFile )
    {
        printf( "open failure" );
        return 1;
    }

    fseek(pFile, 0L, SEEK_END);
    sz = ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);

    // printf(">>> sz = %d\n", sz);
    fread(&Json_Data, 1, sz, pFile);




    // GetJsonElement(Json_Data, sz, "Version", ElemData);
    // printf(">>>> Version = [%s]\n", ElemData);

    // GetJsonElement(Json_Data, sz, "MiniBoot", ElemData);
    // printf(">>>> MiniBoot = [%s]\n", ElemData);

    // GetJsonElement(Json_Data, sz, "PartitionTable", ElemData);
    // printf(">>>> PartitionTable = [%s]\n", ElemData);

    // GetJsonElement(Json_Data, sz, "MainFW", ElemData);
    // printf(">>>> MainFW = [%s]\n", ElemData);



    // -------- User Data --------
    GetJsonElement(Json_Data, sz, "UserData", UserData_Tmp);
    // printf(">>>> UserData = [%s]\n", UserData_Tmp);

    GetJsonElement(UserData_Tmp, sz, "Address", ElemData);
    printf(">>>> UserData: Addr = [%s]\n", ElemData);
    GetJsonElement(UserData_Tmp, sz, "Length", ElemData);
    printf(">>>> UserData: Len = [%s]\n", ElemData);
    GetJsonElement(UserData_Tmp, sz, "Data", BinFifo_Tmp);
    // printf(">>>> UserData: Data = [%s]\n", BinFifo_Tmp);

    GetJsonElement(BinFifo_Tmp, sz, "Header", UserHeader);
    printf(">>>> UserData: Header = [%s]\n", UserHeader);


    memset(ElemData, '\0', sizeof(ElemData));

    GetJsonElement(UserHeader, sz, "Bin_Count", ElemData);
    printf(">>>> User: Bin_Count = [%s]\n", ElemData);


    udBinCnt = atoi(ElemData);
    printf("udBinCnt = %d\n", udBinCnt);

    for(i=1 ; i<=udBinCnt ; i++)
    {
        // memset(str, '\0', sizeof(str));
    //     memset(ElemData, '\0', sizeof(ElemData));

        // sprintf(str, "bin%02d", i);
        // printf("%s\n", str);
    //     GetJsonElement(BinFifo_Tmp, sz, str, ElemData);
    //     printf(">>>> %s = [%s]\n", str, ElemData);
    }


    // GetJsonElement(BinFifo_Tmp, sz, "bin01", ElemData);
    // printf(">>>> bin01 = [%s]\n", ElemData);
    // GetJsonElement(BinFifo_Tmp, sz, "bin02", ElemData);
    // printf(">>>> bin02 = [%s]\n", ElemData);
    // GetJsonElement(BinFifo_Tmp, sz, "bin03", ElemData);
    // printf(">>>> bin03 = [%s]\n", ElemData);
    // GetJsonElement(BinFifo_Tmp, sz, "bin04", ElemData);
    // printf(">>>> bin04 = [%s]\n", ElemData);
    // GetJsonElement(BinFifo_Tmp, sz, "bin05", ElemData);
    // printf(">>>> bin05 = [%s]\n", ElemData);






    fclose(pFile);

    return EXIT_SUCCESS;
}
