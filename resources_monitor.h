#ifndef RESOURCES_MONITOR_H

#define RESOURCES_MONITOR_H
#include "zz_struct.h"

/* List of functinos relative to 1st features (Display System Information)*/
void one_System_Information();

/**
 * @brief 하드웨어 온도 정보를 ncurses 화면에 표시합니다.
 * @details 1초마다 화면을 갱신하며, 로그 파일에서 최신 및 평균 온도 정보를 가져와 표시합니다.
 *          '+', '-' 키 입력을 통해 평균 계산 시간 간격을 동적으로 변경할 수 있습니다.
 * @see get_Average_Temperature_from_Log()
 */
void two_Hardware_Temperature();

/**
 * @brief CPU/메모리 사용량 및 프로세스 목록을 ncurses 화면에 표시합니다.
 * @details 1초마다 화면을 갱신하며, 로그 파일에서 최신 및 평균 사용량 정보를 가져옵니다.
 *          ps 명령어를 통해 실시간 프로세스 목록을 가져와 함께 표시합니다.
 *          '+', '-' 키로 평균 계산 시간 간격을, 위/아래 화살표 키로 프로세스 목록을 스크롤할 수 있습니다.
 * @see get_Average_Usage_Percent_from_Log(), get_Process_Status()
 */
void three_CPU_Memory_Usage();
int get_Maximum_Length_of_ProcessInfo(ProcessInfo* psList, int psCnt, int type);

/* List of functions relative to 4th features (Display Disk Information)*/
void four_Disk_Information();
int get_Maximum_Length_of_DiskInfo(DiskInfo* diskList, int diskCnt, int type);
void display_Partition_Information();
int get_Maximum_Length_of_PartitionInfo(PartitionInfo* partList, int partCnt, int type);

/**
 * @brief 주어진 백분율에 따라 화면에 표시할 상태 바 문자열을 생성합니다.
 * @param[out] barBuf 생성된 상태 바 문자열을 저장할 버퍼.
 * @param[in] percent 백분율 (0-100).
 * @param[in] length 상태 바의 전체 길이.
 * @code
 * [########              ] 50.0%
 * @endcode
 */
void get_Percent_Bar(char* barBuf, float percent, int length);

/* List of functions relative to 5th features (Display Network (IFA) Information) */
void five_Network_Information();
int get_Maximum_Length_of_Physical_IFAInfo(PHYSICAL_IFA_Info* ifaList, int ifaCount, int type);
void display_IFA_Speed(DockerInfo** containerList, IFASpeed* ifaList, int ifaCount);
int get_Maximum_Length_of_Physical_IFASpeed(IFASpeed* ifaList, int ifaCount);

#endif