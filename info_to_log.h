#ifndef INFO_TO_LOG_H

#define INFO_TO_LOG_H
#include "zz_struct.h"

/**
 * @brief 1초마다 온도 정보를 수집하여 로그 파일에 기록하는 스레드 함수.
 * @details TempInfo 구조체에 온도 정보를 채우고, TempLog 형태로 로그 파일에 바이너리 쓰기를 수행합니다.
 * @see get_Temperature_from_System()
 * @return NULL
 */
void* write_Temperature_to_Log();

/**
 * @brief 1초마다 CPU 및 메모리 사용량 정보를 수집하여 로그 파일에 기록하는 스레드 함수.
 * @details CpuUsage, MemUsage 구조체에 사용량 정보를 채우고, UsageLog 형태로 로그 파일에 바이너리 쓰기를 수행합니다.
 * @see get_CPU_Usage_Percent_of_All_Core(), get_Memory_Usage()
 * @return NULL
 */
void* write_Usage_to_Log();

/**
 * @brief 1초마다 자원 정보를 수집하여 임계치 초과 시 경고 로그를 기록하는 스레드 함수.
 * @details over_Critical_Point()를 통해 임계치 초과 여부를 확인하고, 초과 시 WarningLog 형태로 로그 파일에 바이너리 쓰기를 수행합니다.
 * @see over_Critical_Point()
 * @return NULL
 */
void* write_Warning_to_Log();

/**
 * @brief 시스템의 모든 온도 정보를 수집하여 TempInfo 구조체에 채웁니다.
 * @param[out] tempBuf 온도 정보를 저장할 TempInfo 구조체의 포인터.
 * @details 내부적으로 omreport와 perccli 명령어를 사용하여 온도 정보를 가져옵니다.
 * @see get_Temperature_Omreport(), get_Temperature_Perccli()
 */
void get_Temperature_from_System(TempInfo* tempBuf);

/**
 * @brief omreport 명령을 통해 Inlet, Exhaust, CPU 온도를 수집합니다.
 * @param[out] tempBuf 온도 정보를 저장할 TempInfo 구조체의 포인터.
 */
void get_Temperature_Omreport(TempInfo* tempBuf);

/**
 * @brief perccli 명령을 통해 RAID 컨트롤러, 스토리지, BBU 온도를 수집합니다.
 * @param[out] tempBuf 온도 정보를 저장할 TempInfo 구조체의 포인터.
 */
void get_Temperature_Perccli(TempInfo* tempBuf);

/**
 * @brief /proc/stat 파일을 읽어 전체 CPU 코어의 평균 사용률(%)을 계산합니다.
 * @param[out] cpuUsageBuf CPU 사용률을 저장할 CpuUsage 구조체의 포인터.
 * @details 이전 측정값과의 jiffies 차이를 통해 사용률을 계산하며, static 변수를 사용하여 이전 값을 유지합니다.
 */
void get_CPU_Usage_Percent_of_All_Core(CpuUsage* cpuUsageBuf);

/**
 * @brief /proc/meminfo 파일을 읽어 물리 메모리와 스왑 메모리의 사용량을 계산합니다.
 * @param[out] memUsageBuf 메모리 사용량을 저장할 MemUsage 구조체의 포인터.
 */
void get_Memory_Usage(MemUsage* memUsageBuf);

/**
 * @brief 수집된 자원 정보가 설정된 임계치를 넘는지 확인합니다.
 * @param[in,out] warningBuf 확인할 자원 정보 및 경고 타입을 저장할 WarningLog 구조체의 포인터.
 * @return int 임계치를 넘는 항목이 있으면 1, 없으면 0을 반환합니다.
 */
int over_Critical_Point(WarningLog* warningBuf);

#endif