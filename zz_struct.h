#ifndef STRUCT_H

#define STRUCT_H
#include "0_usrDefine.h"
#include <grp.h>
#include <net/if.h>
#include <utmp.h>
#include <pwd.h>

/* Main-Program */
typedef struct MenuItem { // Main Menu
    const char* menuStr;
    void(*function)();
} MenuItem;

/* commom */
typedef enum UNIT { KB, MB, GB, TB, PB, EB } UNIT;

typedef struct DateInfo {
    short year;
    short month;
    short day;
    short hrs;
    short min;
    short sec;
} DateInfo;

typedef struct Unit_Mapping { // for Displaying Unit
    UNIT unit;
    char* str;
} Unit_Mapping;

typedef struct ProcessInfo { // CPU / Memory Usage
    char userName[UT_NAMESIZE + 1];
    pid_t pid;
    float cpu;
    float mem;
    size_t memUseSize;
    char tty[UT_LINESIZE + 1];
    char start[TIME_LEN];
    char time[TIME_LEN];
    char* command;
} ProcessInfo;

/* File Permissions */
typedef struct FileInfo { // Core file status
    char fullPath[MAX_MOUNTPATH_LEN];
    char path[MAX_PATH_LEN];
    int changed[3]; // [0]: uid / [1]: gid / [2]: permission | -1: ERROR, 0: Do not need to be changed, 1: Need to be changed.
    uid_t ownerUID[2]; // [0]: Before, [1]: After
    gid_t groupGID[2]; // [0]: Before, [1]: After
    mode_t permission[2]; // [0]: Before, [1]: After
} FileInfo;

/* Linux Users */
typedef struct UserInfo { // User Information + Last PW Change date
    char* userName;
    uid_t uid;
    int grpCnt;
    gid_t* gid;
    DateInfo lastLogin;
    char loginIP[UT_LINESIZE + 8];
    DateInfo lastChangePW;
} UserInfo;

typedef struct LoginInfo { // Login History
    char userName[UT_NAMESIZE + 1];
    uid_t uid;
    int status; // Success or Failed
    DateInfo logDate;
    char deviceName[UT_LINESIZE + 1];
    char loginIP[UT_LINESIZE + 1];
} LoginInfo;

/* Network Interface */
typedef struct DockerInfo { // Network Information
    short* checked;
    char containerName[MAX_DOCKER_CONTAINER_NAME_LEN];
    pid_t pid;
    short ifaCount;
    char** vethName;
    char** ipv4_addr;
    short* ifa_index;
} DockerInfo;

typedef struct IFASpeed { // Network Information
    char ipv4_addr[IPV4_LEN];
    char ifa_name[MAX_DOCKER_CONTAINER_NAME_LEN];
    float speedRX;
    float speedTX;
    size_t errorRX;
    size_t errorTX;
    size_t dropRX;
    size_t dropTX;
} IFASpeed;

/* Parts Information */
typedef struct CPUInfo { // System Information
    char name[MAX_PARTS_NAME_LEN];
    short status;
    short coreCnt;
} CPUInfo;

typedef struct MEM_UNIT_INFO { // System Information
    int status;
    char connectorName[4];
    char type[MAX_PARTS_NAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
} MEM_UNIT_INFO;

typedef struct MEMInfo { // System Information
    short slotsTotal;
    short slotsUsed;
    char installedCapacity[MAX_CAPACITY_LEN];
    char errorCorrection[MAX_PARTS_NAME_LEN];
    MEM_UNIT_INFO* unit;
} MEMInfo;

typedef struct CMOS_BAT_INFO { // System Information
    char name[MAX_PARTS_NAME_LEN];
    int status;
} CMOS_BAT_INFO;

typedef struct FANInfo { // System Information
    int status;
    char rpm[MAX_CAPACITY_LEN];
    char name[MAX_PARTS_NAME_LEN];
} FANInfo;

typedef struct PHYSICAL_IFA_Info { // System Information
    char name[MAX_PARTS_NAME_LEN];
    char ifName[MAX_PARTS_NAME_LEN];
    int connected;
    char speed[MAX_CAPACITY_LEN];
} PHYSICAL_IFA_Info;

typedef struct SystemInfo { // System Information
    char hostname[MAX_PARTS_NAME_LEN];
    char serverModel[MAX_PARTS_NAME_LEN];
    char serviceTag[MAX_PARTS_NAME_LEN];
    char serviceCode[MAX_PARTS_NAME_LEN];
    CPUInfo cpu[MAX_CPU_COUNT];
    MEMInfo mem;
    CMOS_BAT_INFO cmosBattery;
    FANInfo fan[MAX_FAN_COUNT];
    int psuStatus[MAX_PSU_COUNT];
} SystemInfo;

/* Disk Information */
typedef struct PartitionInfo { // DIsk Information
    char fileSystem[MAX_FILESYSTEM_LEN];
    char mountPath[MAX_MOUNTPATH_LEN];
    size_t spaceTotal;
    size_t spaceUse;
} PartitionInfo;

typedef struct VDInfo { // DIsk Information
    short driveGroup;
    short virtualDrive;
    char type[MAX_VDISK_TYPE_LEN];
    short status;
    short access;
    char capacity[MAX_CAPACITY_LEN];
    char vdName[MAX_VDISK_NAME_LEN];
    char fileSystem[MAX_FILESYSTEM_LEN];
    int mountPathCnt;
    char** mountPath;
} VDInfo;

typedef struct DiskInfo { // DIsk Information
    short enclosureNum;
    short slotNum;
    short driveGroup;
    short status;
    int deviceID;
    char modelName[MAX_DISK_MODELNAME_LEN];
    char capacity[MAX_CAPACITY_LEN];
    char mediaType[MAX_DISK_MEDIATYPE_LEN];
    char interface[MAX_DISK_INTERFACE_LEN];
    char mappedPartition[MAX_MOUNTPATH_LEN];
} DiskInfo;

typedef struct BBUInfo { // DIsk Information
    int status;
    char voltage[MAX_BBU_VOLTAGE_LEN];
    char designCapacity[MAX_BBU_CAPACITY_LEN];
    char fullCapacity[MAX_BBU_CAPACITY_LEN];
    char remainCapacity[MAX_BBU_CAPACITY_LEN];
} BBUInfo;

typedef struct HBAInfo { // DIsk Information
    char HBA_Name[MAX_HBA_NAME_LEN];
    char HBA_Bios_Ver[MAX_HBA_BIOS_VER_LEN];
    char HBA_Serial_Num[MAX_HBA_SERIAL_LEN];
    char HBA_Firmware_Ver[MAX_HBA_FIRMWARE_VER_LEN];
    char HBA_Driver_Name[MAX_HBA_DRIVER_NAME_LEN];
    char HBA_Driver_Ver[MAX_HBA_DRIVER_VER_LEN];
    short status;
    short HBA_Cur_Personality; // Mode
    int HBA_Drive_Groups_Cnt;
    BBUInfo bbuStatus;
} HBAInfo;

/**
 * @brief 하드웨어 온도 정보를 저장하는 구조체 (단위: Celcius)
 */
typedef struct TempInfo {
    short inlet;            ///< 서버 유입 공기 온도
    short exhaust;          ///< 서버 배출 공기 온도
    short cpu[MAX_CPU_COUNT]; ///< CPU 패키지 온도
    short raidCore;         ///< RAID 컨트롤러 코어 온도
    short raidController;   ///< RAID 컨트롤러 온도
    short bbu;              ///< BBU (Battery Backup Unit) 온도
    short storage_cnt;      ///< 감지된 스토리지 개수
    short storage[MAX_STORAGE_COUNT]; ///< 각 스토리지 온도
} TempInfo;

/**
 * @brief CPU 사용률 정보를 저장하는 구조체 (단위: %)
 */
typedef struct CpuUsage {
    float usage;    ///< 전체 CPU 코어의 평균 사용률
} CpuUsage;

/**
 * @brief 메모리 사용량 정보를 저장하는 구조체 (단위: KB)
 */
typedef struct MemUsage {
    size_t memTotal;    ///< 전체 물리 메모리 크기
    size_t memUse;      ///< 사용 중인 물리 메모리 크기
    size_t swapTotal;   ///< 전체 스왑 메모리 크기
    size_t swapUse;     ///< 사용 중인 스왑 메모리 크기
} MemUsage;

/**
 * @brief 온도 정보 로그 파일에 저장하기 위한 구조체
 */
typedef struct TempLog {
    DateInfo date;  ///< 측정 시각
    TempInfo temp;  ///< 온도 정보
} TempLog;

/**
 * @brief 자원 사용량 로그 파일에 저장하기 위한 구조체
 */
typedef struct UsageLog {
    DateInfo date;  ///< 측정 시각
    CpuUsage cpu;   ///< CPU 사용량 정보
    MemUsage mem;   ///< 메모리 사용량 정보
} UsageLog;

/**
 * @brief 임계치 초과 시 경고 로그를 저장하기 위한 구조체
 */
typedef struct WarningLog {
    DateInfo date;      ///< 측정 시각
    TempInfo temp;      ///< 온도 정보
    CpuUsage cpuUsage;  ///< CPU 사용량 정보
    MemUsage memUsage;  ///< 메모리 사용량 정보
    short type;         ///< 임계치를 초과한 자원 종류 (0_usrDefine.h 참고)
} WarningLog;

#endif