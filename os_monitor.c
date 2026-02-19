#include "common.h"
#include "os_info.h"
#include "os_monitor.h"
#include "server_monitoring_tools.h"
#include <curses.h>
#include <grp.h>
#include <termio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const DateInfo dateBuf;
extern const struct winsize wbuf;
extern int invoked_menu;
extern const char hostnameBuf[];


/**
 * @brief 리눅스 유저 계정 상태 정보 출력 함수
 * * @section FUNCTION_INFO 함수 정보
 * - 기능 설명   : 시스템에 등록된 사용자 계정의 상세 상태(로그인 정보, 비번 변경일, 그룹 등)를 출력
 * - 주요 타겟   : 보안 및 관리 목적상 UID 0(Root) 및 UID 1000 이상(일반 유저)만 필터링하여 표시
 * * @section ALGORITHM 알고리즘 및 연산
 * - 1. get_UserList()를 통해 전체 유저 정보를 수집한 후, 유효 유저(Root/일반)를 구분하기 위해 validIndices를 동적 할당
 * - 2. 유저별 최대 이름 길이, IP 길이 등을 계산하여 화면 레이아웃을 유동적으로 배치
 * - 3. get_Date_Interval()을 사용하여 마지막 비밀번호 변경일로부터 경과일(D-Day)을 계산
 * - 4. 유저별 소속 그룹이 여러 개인 경우, 반복문을 통해 그룹명과 GID를 리스트 형태로 출력
 * * @section MEMORY_INFO 메모리 관리 (중요)
 * - 할당(Malloc) : 
 * - @c userList : get_UserList() 내부에서 유저 정보 배열 할당
 * - @c validIndices : 유효 유저 인덱스 저장을 위해 현재 함수 내부에서 직접 할당
 * - 해제(Free) : 
 * - 내부 데이터 해제 : 유저별 @c userName 및 @c gid 포인터를 반복문으로 개별 해제
 * - 전체 구조체 해제 : @c userList 및 @c validIndices 를 최종적으로 명시적 해제
 * * @section INOUTPUT 입출력 자료
 * - INPUT  : 없음 (시스템 계정 파일 /etc/passwd, /etc/shadow 등 참조)
 * - OUTPUT : 유저명, UID, 마지막 로그인 IP/날짜, 비번 변경일, 소속 그룹 리스트 화면 출력
 * * @section ERROR_HANDLING 에러 처리
 * - 데이터가 없는 항목(로그인 기록 등)에 대해 "N/A" 처리를 수행하여 시각적 오류를 방지
 * * @section create 작성 정보
 * - 작성자       : 이영인
 * - 작성일       : 2026/02/19
 * -@section modify 수정 정보
 * - 수정자/수정일 : 수정 내역
 * - 이영인 / 2026.02.19 : Doxygen 가이드라인에 따른 메모리 및 알고리즘 명세 추가
 */
// /* List of functions relative to 6th feature. (Display Linux User Account Status) */
void six_User_Account_Status() { // Display UserList (Name, uid, gids, Login Date, Login IP, PW Change Date)
    invoked_menu = 1;

    keypad(stdscr, TRUE);

    UserInfo* userList = NULL;
    int pos_x = 3, pos_y = 1, userCnt = 0, line = 0, nameWidth = 0, ipWidth = 0, lastDateWidth = 0, lastPWDateWidth = 0, tmp;
    int validUserCnt = 0;
    int* validIndices = NULL;

    const char* title = SUBTITLE;
    const char* footLabel = "To restore main screen, Press \"q\".";
    char groupNameBuf[UT_NAMESIZE + 1] = { '\0' }, lineBuf[BUF_MAX_LINE] = { '\0' }, ddayBuf[DDAY_LEN] = { '\0' };
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0);
    wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

    get_UserList(&userList, &userCnt);
    validIndices = (int*)malloc(sizeof(int) * userCnt);
    for (int i = 0; i < userCnt; i++) {
        // UID 0(Root) / UID 1000 이상(일반 유저)만 표시
        if (userList[i].uid == 0 || userList[i].uid >= 1000) {
            validIndices[validUserCnt++] = i;
        }
    }
    nameWidth = get_Maximum_Length_in_UserInfo(userList, userCnt, TYPE_USERNAME) + 2;
    ipWidth = get_Maximum_Length_in_UserInfo(userList, userCnt, TYPE_LOGIN_IP) + 3;
    lastDateWidth = get_Maximum_Length_in_UserInfo(userList, userCnt, TYPE_LOGIN_DATE) - 2;
    lastPWDateWidth = get_Maximum_Length_in_UserInfo(userList, userCnt, TYPE_PW_CHANGE_DATE);

    while(invoked_menu == 1) {
        wclear(headLineWin);
        wclear(userWin);
        wclear(footLineWin);

        mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
        
        pos_y = 1; 
        pos_x = 3;
        mvwprintw(userWin, pos_y, pos_x - 1, "6. Server User Account Status - Last Login / Password Changed Date, Login IP, Joined Groups");
        mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "!! Notification !!");
        mvwprintw(userWin, pos_y += 1, pos_x, "- # of User Accounts: %d", validUserCnt);
        mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, " - Only Root & user-created accounts are displayed.");
        
        wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
        wmove(userWin, pos_y + 2, 0);
        whline(userWin, ' ', wbuf.ws_col - 1);
        mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
        
        pos_x = 3;
        for (tmp = validUserCnt; tmp /= 10; pos_x++);
        mvwprintw(userWin, pos_y += 2, pos_x, "UserName(UID)");
        mvwprintw(userWin, pos_y, pos_x += nameWidth, "Last Login Date");
        mvwprintw(userWin, pos_y, pos_x += lastDateWidth, "Login IP");
        mvwprintw(userWin, pos_y, pos_x += ipWidth, "PW Changed Date");
        mvwprintw(userWin, pos_y, pos_x += lastPWDateWidth, "GroupName(GID)");
        wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

        // Control print position to prevent text overlap
        int list_start_y = pos_y + 1;
        int current_y = list_start_y;
        int win_max_y = wbuf.ws_row - 2;

        for (int k = line; k < validUserCnt; k++) { 
            int i = validIndices[k % validUserCnt];
            
            if (current_y >= win_max_y) break;

            pos_x = 3;
            for (tmp = validUserCnt; tmp /= 10; pos_x++);

            mvwprintw(userWin, current_y, 1, "%d", k + 1); 
            mvwprintw(userWin, current_y, pos_x, "%s(%d)", userList[i].userName, userList[i].uid);

            if (userList[i].lastLogin.year == 0000) {
                mvwprintw(userWin, current_y, pos_x += nameWidth, "N/A");
            } else {
                mvwprintw(userWin, current_y, pos_x += nameWidth, DATE_TIME_FORM, 
                    userList[i].lastLogin.year, userList[i].lastLogin.month, 
                    userList[i].lastLogin.day, userList[i].lastLogin.hrs, 
                    userList[i].lastLogin.min, userList[i].lastLogin.sec);
            }

            mvwprintw(userWin, current_y, pos_x += lastDateWidth, "%s", userList[i].loginIP);

            if (userList[i].lastChangePW.year == 0000) {
                mvwprintw(userWin, current_y, pos_x += ipWidth, "N/A");
            } else {
                sprintf(lineBuf, DATE_FORM, userList[i].lastChangePW.year, userList[i].lastChangePW.month, userList[i].lastChangePW.day);
                sprintf(ddayBuf, "(D+%d)", get_Date_Interval(&(userList[i].lastChangePW)));
                strcat(lineBuf, ddayBuf);
                mvwprintw(userWin, current_y, pos_x += ipWidth, "%s", lineBuf);
            }

            // Output groups (Print on next line)
            if (userList[i].grpCnt > 0) {
                get_Group_Name(userList[i].gid[0], groupNameBuf);
                mvwprintw(userWin, current_y, pos_x + lastPWDateWidth, "%s(%d)", groupNameBuf, userList[i].gid[0]);
                current_y++; 
            } else {
                current_y++;
            }

            for (int j = 1; j < userList[i].grpCnt; j++) {
                if (current_y >= win_max_y) break;
                
                get_Group_Name(userList[i].gid[j], groupNameBuf);
                mvwprintw(userWin, current_y, pos_x + lastPWDateWidth, "%s(%d)", groupNameBuf, userList[i].gid[j]);
                current_y++;
            }
        }

        mvwprintw(footLineWin, 0, 1, "%s", footLabel);
        mvwprintw(footLineWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        
        // scroll
        int ch = getch();
        switch(ch) {
            case KEY_UP:
            case 'n': 
                if (line > 0) line--;
                break;
            case KEY_DOWN:
            case 'm':  
                if (line < validUserCnt - 1) line++;
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    if (validIndices != NULL) free(validIndices);

    for (int i = 0; i < userCnt; i++) {
        free_Array((void**)&(userList[i].userName));
        free_Array((void**)&(userList[i].gid));
    }
    free_Array((void**)&userList);

    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    display_main();
}

void get_Group_Name(gid_t gid, char* gruopName) { // Convert gid to Group Name
    struct group *grp = getgrgid(gid);
    if (grp != NULL) {
        strcpy(gruopName, grp->gr_name);
    } else {
        STR_INIT(gruopName);
    }
}

int get_Maximum_Length_in_UserInfo(UserInfo* userList, int cnt, int type) { // Get the maximum length of userName among userList.
    int name_len = 0, uid_len = 0, tmp_uid_len = 0;
    for (int i = 0; i < cnt; i++) {
        if (type == TYPE_USERNAME) {
            name_len = (((int)strlen(userList[i].userName) > name_len) ? (int)strlen(userList[i].userName) : name_len);
            tmp_uid_len = 0;
            for (int j = 1; userList[i].uid / j; j *= 10) {
                tmp_uid_len++;
            }
            tmp_uid_len += 3;
            uid_len = (tmp_uid_len > uid_len) ? tmp_uid_len : uid_len;
        } else if (type == TYPE_LOGIN_IP) {
            uid_len = 0;
            name_len = (((int)strlen(userList[i].loginIP) > name_len) ? (int)strlen(userList[i].loginIP) : name_len);
        } else if (type == TYPE_LOGIN_DATE) {
            if (userList[i].lastLogin.year == 0000) {
                return DATE_TIME_LEN;
            } else {
                name_len = NA_LEN;
                uid_len = 0;
            }
        } else if (type == TYPE_PW_CHANGE_DATE) {
            if (userList[i].lastChangePW.year == 0) {
                return DATE_LEN + DDAY_LEN;
            } else {
                name_len = NA_LEN;
                uid_len = 0;
            }
        }
    }
    return name_len + uid_len;
}

/* List of functions relative to 7th feature. (Display Login History) */
void seven_Login_History() { // Display Login History (Login Success / Failed History)
    invoked_menu = 1;

    LoginInfo* loginList = NULL;
    int pos_x = 4, pos_y = 1, historyCnt = 0, line = 0, nameWidth, deviceNameWidth, tmp = 0, printListCnt = 0;
    const char* title = SUBTITLE;
    const char* footLabel = "To restore main screen, Press \"q\".";
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(2, wbuf.ws_col, wbuf.ws_row - 2, 0);
    wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

    get_Login_History(&loginList, &historyCnt);
    nameWidth = get_Maximun_Length_in_LoginInfo(loginList, historyCnt, TYPE_USERNAME);
    deviceNameWidth = get_Maximun_Length_in_LoginInfo(loginList, historyCnt, TYPE_DEVICENAME) + 6;

    mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    mvwprintw(userWin, pos_y, pos_x - 2, "7. Login History");
    mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
    mvwprintw(userWin, pos_y += 1, pos_x, "- # of Login History: %d", historyCnt);
    mvwprintw(userWin, pos_y - 3, (wbuf.ws_col / 2) - CENTER_OFFSET, "!! Notification !!");
    mvwprintw(userWin, pos_y - 2, (wbuf.ws_col / 2) - CENTER_OFFSET, " - \"NoExist\" means a non-existent user in server anymore.");
    mvwprintw(userWin, pos_y - 1, (wbuf.ws_col / 2) - CENTER_OFFSET, " - \"N/A\" means that no records exist for that user.");
    mvwprintw(userWin, pos_y, (wbuf.ws_col / 2) - CENTER_OFFSET, "(If Root's IP is N/A, Find the recent history same terminal.)");
    wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(userWin, pos_y + 2, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
    mvwprintw(userWin, pos_y += 2, pos_x, "UserName(UID)");
    mvwprintw(userWin, pos_y, pos_x += nameWidth, "Status");
    mvwprintw(userWin, pos_y, pos_x += (strlen(TYPE_LOGIN_SUCCESS_STR) + 6), "Login Date");
    mvwprintw(userWin, pos_y, pos_x += DATE_TIME_LEN, "Terminal");
    mvwprintw(userWin, pos_y, pos_x += deviceNameWidth, "Login IP");
    wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    pos_y = 6 + 1;
    printListCnt = wbuf.ws_row - pos_y - 1 - 2; // Total - Top Info - Header - Footer
    while(invoked_menu == 1) {
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }
        for (int i = 0; i < printListCnt; i++) {
            if (i >= historyCnt) {
                break;
            }
            pos_x = 3;
            for (tmp = historyCnt; tmp /= 10; pos_x++);
            if (i >= wbuf.ws_row - pos_y) {
                continue;
            }
            mvwprintw(userWin, pos_y + i, 1, "%d", i + 1 + line); // UserName & UID
            if (strstr(loginList[i + line].userName, USER_NOEXIST) == NULL) { // Existing User
                mvwprintw(userWin, pos_y + i, pos_x, "%s(%d)", loginList[i + line].userName, loginList[i + line].uid);
            } else {
                mvwprintw(userWin, pos_y + i, pos_x, "%s", loginList[i + line].userName);
            }
            if (loginList[i + line].status == TYPE_LOGIN_SUCCESS) { // Status
                mvwprintw(userWin, pos_y + i, pos_x += nameWidth, TYPE_LOGIN_SUCCESS_STR);
            } else {
                mvwprintw(userWin, pos_y + i, pos_x += nameWidth, TYPE_LOGIN_FAILED_STR);
            }
            if (loginList[i + line].logDate.year == 0000) { // Login Date
                mvwprintw(userWin, pos_y + i, (pos_x += strlen(TYPE_LOGIN_SUCCESS_STR) + 6), "N/A");
            } else {
                mvwprintw(userWin, pos_y + i, (pos_x += strlen(TYPE_LOGIN_SUCCESS_STR) + 6), DATE_TIME_FORM, loginList[i + line].logDate.year,
                loginList[i + line].logDate.month, loginList[i + line].logDate.day, loginList[i + line].logDate.hrs, loginList[i + line].logDate.min, loginList[i + line].logDate.sec);
            }
            mvwprintw(userWin, pos_y + i, pos_x += DATE_TIME_LEN, "%s", loginList[i + line].deviceName); // Terminal Type
            mvwprintw(userWin, pos_y + i, (pos_x += deviceNameWidth), "%s", loginList[i + line].loginIP); // Login Sources
        }

        mvwprintw(footLineWin, 0, 1, "To see the remaining, Press \"m\" (Next) and \"n\" (Previous).");
        mvwprintw(footLineWin, 1, 1, "%s", footLabel);
        mvwprintw(footLineWin, 1, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < historyCnt - printListCnt) ? 1 : 0);
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    free_Array((void**)&loginList); // Free LoginList Array

    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    display_main();
}

int get_Maximun_Length_in_LoginInfo(LoginInfo* historyList, int histCnt, int type) { // Get the maximun length of string
    int name = 0, uid = 0, tmp = 0;
    for (int i = 0; i < histCnt; i++) {
        if (type == TYPE_USERNAME) {
            name = ((int)strlen(historyList[i].userName) > name ? (int)strlen(historyList[i].userName) : name);
            tmp = 0;
            for (int j = 1; historyList[i].uid / j; j *= 10) {
                tmp++;
            }
            tmp += 1;
            if (strstr(historyList[i].userName, USER_NOEXIST) == NULL) {
                uid = ((tmp > uid) ? tmp : uid);
            } else {
                uid = 0;
            }
        } else if (type == TYPE_DEVICENAME) {
            uid = 0;
            name = (((int)strlen(historyList[i].deviceName) > name) ? (int)strlen(historyList[i].deviceName) : name);
        } else if (type == TYPE_LOGIN_IP) {
            uid = 0;
            name = (((int)strlen(historyList[i].loginIP) > name) ? (int)strlen(historyList[i].loginIP) : name);
        }
    }
    return name + uid;
}

/* List of functions relative to 8th feature. (Display Core File Status - Permissions, Ownership, Group) */
void eight_Core_File_Status() { // Display File Information
    invoked_menu = 1;

    FileInfo* fileList = NULL;
    int pos_x = 6, pos_y = 1, notNeedCnt = 0, needCnt = 0, failedCnt = 0, fileCnt, nameWidth, uidWidth, gidWidth, tmp, line = 0, printListCnt = 0;
    const char* title = SUBTITLE;
    const char* footLabel = "To restore main screen, Press \"q\".";
    char logPath[MAX_MOUNTPATH_LEN] = { '\0' };
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 1 - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(2, wbuf.ws_col, wbuf.ws_row - 2, 0);
    wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

    get_File_Information(&fileList, &notNeedCnt, &needCnt, &failedCnt);
    fileCnt = notNeedCnt + needCnt + failedCnt;
    nameWidth = get_Maximum_Length_in_FileInfo(fileList, fileCnt, TYPE_FILENAME) + 4;
    uidWidth = get_Maximum_Length_in_FileInfo(fileList, fileCnt, TYPE_UID) + 4;
    gidWidth = get_Maximum_Length_in_FileInfo(fileList, fileCnt, TYPE_GID) + 4;

    mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    mvwprintw(userWin, pos_y, pos_x - 4, "8. Core File Status - The Ownership, Group, and Permissions");
    mvwprintw(userWin, pos_y += 2, pos_x, "- Server Hostname: %s", hostnameBuf);
    mvwprintw(userWin, pos_y += 1, pos_x, "- # of files: %d | not Modified: %d | Modified Successfully: %d | Failed to modify: %d", fileCnt, notNeedCnt, needCnt, failedCnt); 
    get_Filename(logPath, LOG_PATH, STAT_HISTORY_LOG, &dateBuf);
    mvwprintw(userWin, pos_y += 2, pos_x, "!! Notification !! To check changing log, See \"%s\"", logPath);
    wattron(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wmove(userWin, pos_y + 2, 0);
    whline(userWin, ' ', wbuf.ws_col - 1);
    mvwaddch(userWin, pos_y + 2, wbuf.ws_col - 1, ' ');
    mvwprintw(userWin, pos_y += 2, (pos_x -= 1), "FileName");
    mvwprintw(userWin, pos_y, pos_x + nameWidth, "Ownership(UID)");
    mvwprintw(userWin, pos_y, pos_x + nameWidth + uidWidth, "Group(GID)");
    mvwprintw(userWin, pos_y, pos_x + nameWidth + uidWidth + gidWidth, "Permissions");
    wattroff(userWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    pos_y = 8 + 1;
    printListCnt = wbuf.ws_row - pos_y - 1 - 2; // Total - Top Info - Header - Footer
    while(invoked_menu == 1) {
        for (int i = 0; i < printListCnt; i++) {
            wmove(userWin, pos_y + i, 0);
            whline(userWin, ' ', wbuf.ws_col - 1);
            mvwaddch(userWin, pos_y + i, wbuf.ws_col - 1, ' ');
        }
        for (int i = 0; i < printListCnt; i++) {
            if (i >= fileCnt) {
                break;
            }
            pos_x = 5;
            for (tmp = fileCnt; tmp /= 10; pos_x++);
            if (i >= wbuf.ws_row - pos_y) {
                continue;
            }
            mvwprintw(userWin, pos_y + i, 3, "%d", i + 1 + line); // Index
            mvwprintw(userWin, pos_y + i, pos_x, "%s", fileList[i + line].path); // Filename

            if (fileList[i + line].changed[0] == NOT_NEED_CHANGING) { // Ownership
                mvwprintw(userWin, pos_y + i, (pos_x += nameWidth) , NOT_NEED_TO_CHANGE_FORM); // Not need to change
            } else if (fileList[i + line].changed[0] == NEED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += nameWidth) , CHANGE_SUCCESS_FORM, fileList[i + line].ownerUID[0], fileList[i + line].ownerUID[1]); // Changed Successfully 
            } else if (fileList[i + line].changed[0] == FAILED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += nameWidth) , CHANGE_FAILED_FORM, fileList[i + line].ownerUID[0], fileList[i + line].ownerUID[1]); // Failed to changed
            } else if (fileList[i + line].changed[0] == FAILED_CHANGING_INVALID) { // Invalid Input
                mvwprintw(userWin, pos_y + i, (pos_x += nameWidth) , CHANGE_FAILED_NOT_EXIST_FORM); // Failed to changed
            }
            
            if (fileList[i + line].changed[1] == NOT_NEED_CHANGING) { // Group
                mvwprintw(userWin, pos_y + i, (pos_x += uidWidth) , NOT_NEED_TO_CHANGE_FORM); // Not need to change
            } else if (fileList[i + line].changed[1] == NEED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += uidWidth) , CHANGE_SUCCESS_FORM, fileList[i + line].groupGID[0], fileList[i + line].groupGID[1]); // Changed Successfully
            } else if (fileList[i + line].changed[1] == FAILED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += uidWidth) , CHANGE_FAILED_FORM, fileList[i + line].groupGID[0], fileList[i + line].groupGID[1]); // Failed to changed
            } else if (fileList[i + line].changed[1] == FAILED_CHANGING_INVALID) { // Invalid Input
                mvwprintw(userWin, pos_y + i, (pos_x += uidWidth) , CHANGE_FAILED_NOT_EXIST_FORM); // Failed to changed
            }
            
            if (fileList[i + line].changed[2] == NOT_NEED_CHANGING) { // Permissions
                mvwprintw(userWin, pos_y + i, (pos_x += gidWidth) , NOT_NEED_TO_CHANGE_FORM); // Not need to change
            } else if (fileList[i + line].changed[2] == NEED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += gidWidth) , CHANGE_SUCCESS_FORM_PERM, fileList[i + line].permission[0], fileList[i + line].permission[1]); // Changed Successfully
            } else if (fileList[i + line].changed[2] == FAILED_CHANGING) {
                mvwprintw(userWin, pos_y + i, (pos_x += gidWidth) , CHANGE_FAILED_FORM_PERM, fileList[i + line].permission[0], fileList[i + line].permission[1]); // Failed to changed
            } else if (fileList[i + line].changed[2] == FAILED_CHANGING_INVALID) { // Invalid Input
                mvwprintw(userWin, pos_y + i, (pos_x += gidWidth) , CHANGE_FAILED_NOT_EXIST_FORM); // Failed to changed
            }
        }

        mvwprintw(footLineWin, 0, 1, "To see the remaining, Press \"m\" (Next) and \"n\" (Previous).");
        mvwprintw(footLineWin, 1, 1, "%s", footLabel);
        mvwprintw(footLineWin, 1, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case 'n':
            case 'N':
                line -= ((line > 0) ? 1 : 0);
                break;
            case 'm':
            case 'M':
                line += ((line < fileCnt - printListCnt) ? 1 : 0);
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    free_Array((void**)&fileList); // Free Allocated Array

    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    display_main();
}

int get_Maximum_Length_in_FileInfo(FileInfo* fileList, int fileCnt, int type) { // Get the maximum length of path among fileList.
    int name = 0;
    char idBuf[UPDATED_FAILED_LEN] = { '\0' };

    for (int i = 0; i < fileCnt; i++) {
        if (type == TYPE_FILENAME) { // Get filename length
            name = (((int)strlen(fileList[i].path) > name) ? (int)strlen(fileList[i].path) : name);
        } else if (type == TYPE_UID) { // Get UID Content length
            if (fileList[i].changed[0] == NOT_NEED_CHANGING) { // Not need to change UID
                sprintf(idBuf, NOT_NEED_TO_CHANGE_FORM);
            } else if (fileList[i].changed[0] == NEED_CHANGING) { // Changing UID Successfully
                sprintf(idBuf, CHANGE_SUCCESS_FORM, fileList[i].ownerUID[0], fileList[i].ownerUID[1]);
            } else if (fileList[i].changed[0] == FAILED_CHANGING) { // Failed to change UID
                sprintf(idBuf, CHANGE_FAILED_FORM, fileList[i].ownerUID[0], fileList[i].ownerUID[1]);
            } else if (fileList[i].changed[0] == FAILED_CHANGING_INVALID) { // Invalid Input
                sprintf(idBuf, CHANGE_FAILED_NOT_EXIST_FORM);
            }
        } else if (type == TYPE_GID) { // Get GID Content length
            if (fileList[i].changed[1] == NOT_NEED_CHANGING) { // Not need to change UID
                sprintf(idBuf, NOT_NEED_TO_CHANGE_FORM);
            } else if (fileList[i].changed[1] == NEED_CHANGING) { // Changing UID Successfully
                sprintf(idBuf, CHANGE_SUCCESS_FORM, fileList[i].groupGID[0], fileList[i].groupGID[1]);
            } else if (fileList[i].changed[1] == FAILED_CHANGING) { // Failed to change UID
                sprintf(idBuf, CHANGE_FAILED_FORM, fileList[i].groupGID[0], fileList[i].groupGID[1]);
            } else if (fileList[i].changed[1] == FAILED_CHANGING_INVALID) { // Invalid Input
                sprintf(idBuf, CHANGE_FAILED_NOT_EXIST_FORM);
            }
        }
        name = (((int)strlen(idBuf) > name) ? (int)strlen(idBuf) : name);
    }
    return name;
}

/* Function relative to 9th feature (Display the information of this program - Version, Copyright, License, and so on.)*/
void nine_About_This_Program() { // Display File Information
    invoked_menu = 1;
    int pos_x = 6, pos_y = 1;
    const char* title = SUBTITLE;
    const char* footLabel = "To restore main screen, Press \"q\".";
    WINDOW *headLineWin = newwin(1, wbuf.ws_col, 0, 0);
    WINDOW *userWin = newwin(wbuf.ws_row - 2, wbuf.ws_col, 1, 0);
    WINDOW *footLineWin = newwin(1, wbuf.ws_col, wbuf.ws_row - 1, 0);
    wbkgd(headLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));
    wbkgd(footLineWin, COLOR_PAIR(BLACK_TEXT_WHITE_BACKGROUND));

    mvwprintw(headLineWin, 0, (wbuf.ws_col - strlen(title)) / 2, "%s", title);
    mvwprintw(userWin, pos_y, pos_x - 2, "9. About This Program");

    while(invoked_menu == 1) {
        mvwprintw(footLineWin, 0, 1, "%s", footLabel);
        mvwprintw(footLineWin, 0, wbuf.ws_col - 20 - strlen("System Time: "), "System Time: %04d-%02d-%02d %02d:%02d:%02d", dateBuf.year, dateBuf.month, dateBuf.day, dateBuf.hrs, dateBuf.min, dateBuf.sec);

        refresh();
        wrefresh(headLineWin);
        wrefresh(userWin);
        wrefresh(footLineWin);
        timeout(1000);
        switch(getch()) {
            case 'q':
            case 'Q':
            case 27: // ESC
                invoked_menu = 0;
                break;
        }
    }

    clear();
    delwin(headLineWin);
    delwin(userWin);
    delwin(footLineWin);
    display_main();
}
