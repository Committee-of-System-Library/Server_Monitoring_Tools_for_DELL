#ifndef INFO_FROM_LOG_H

#define INFO_FROM_LOG_H
#include "zz_struct.h"

int remove_History_Log(int type);
int get_Warning_History_from_Log(WarningLog** warningList, int* warningCnt);
int get_WarningLog_Line_Cnt();

/**
 * @brief 사용량 로그 파일에서 가장 최신의 메모리 사용량 정보를 읽어옵니다.
 * @param[out] memBuf 메모리 사용량 정보를 저장할 MemUsage 구조체의 포인터.
 * @return int 성공 시 0, 실패 시 -100을 반환합니다.
 * @details usage_history-YYYYMMDD 로그 파일을 열어 마지막에 기록된 UsageLog 데이터를 읽습니다.
 */
int get_Memory_Usage_from_Log(MemUsage* memBuf);

/**
 * @brief 사용량 로그 파일에서 지정된 시간 간격 동안의 평균 CPU 또는 메모리 사용률을 계산합니다.
 * @param[out] avg_usage_buf 계산된 평균 사용률을 저장할 float 배열의 포인터.
 * @param[in] inputInterval 평균을 계산할 시간 간격 (초).
 * @param[in] type 계산할 자원 타입 (TYPE_CPU_USAGE 또는 TYPE_MEM_USAGE).
 * @param[out] usage1sec (Optional) 평균 계산이 아닌 경우, 가장 최신의 사용량 정보를 저장할 UsageLog 구조체의 포인터.
 * @return int 성공 시 0, 실패 시 -100을 반환합니다.
 * @details usage_history 로그 파일을 끝에서부터 지정된 간격만큼 읽어 평균을 계산합니다.
 */
int get_Average_Usage_Percent_from_Log(float* avg_usage_buf, int inputInterval, int type, UsageLog* usage1sec);

/**
 * @brief 온도 로그 파일에서 지정된 시간 간격 동안의 평균 온도를 계산합니다.
 * @param[out] avg_temp_buf 계산된 평균 온도를 저장할 float 배열의 포인터.
 * @param[in] inputInterval 평균을 계산할 시간 간격 (초).
 * @param[in] type 계산할 온도 타입 (예: TYPE_CPU_TEMP, TYPE_INLET_TEMP 등).
 * @param[out] temp1sec (Optional) 평균 계산이 아닌 경우, 가장 최신의 온도 정보를 저장할 TempLog 구조체의 포인터.
 * @return int 성공 시 0, 실패 시 -100을 반환합니다.
 * @details temperature_history 로그 파일을 끝에서부터 지정된 간격만큼 읽어 평균을 계산합니다.
 */
int get_Average_Temperature_from_Log(float* avg_temp_buf, int inputInterval, int type, TempLog* temp1sec);
void get_Before_day(DateInfo* buf, const DateInfo* pointDate);

#endif